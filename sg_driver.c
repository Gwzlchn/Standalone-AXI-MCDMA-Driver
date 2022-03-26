#include <fcntl.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define DATA_MEM_BASE_ADDR 0x86000000
#define MEM_TOTAL_SIZE (1 << 21)  // 2MB

#define DMA_CTRL_ADDR 0x87000000
#define REG_TOTAL_SIZE (1 << 20)

#define XAXIDMA_BD_CTRL_TXSOF_MASK 0x80000000 /**< First tx packet */
#define XAXIDMA_BD_CTRL_TXEOF_MASK 0x40000000 /**< Last tx packet */

#define XAXIDMA_TX_OFFSET                                                                          \
  0x00000000 /**< TX channel registers base                                                        \
              *  offset */
#define XAXIDMA_RX_OFFSET                                                                          \
  0x00000030                                /**< RX channel registers base                         \
                                             * offset */
#define XAXIDMA_CR_OFFSET 0x00000000        /**< Channel control */
#define XAXIDMA_SR_OFFSET 0x00000004        /**< Status */
#define XAXIDMA_CR_RUNSTOP_MASK 0x00000001  /**< Start/stop DMA channel */
#define XAXIDMA_CR_RESET_MASK 0x00000004    /**< Reset DMA engine */
#define XAXIDMA_IRQ_ALL_MASK 0x00007000     /**< All interrupts */
#define XAXIDMA_CDESC_OFFSET 0x00000008     /**< Current descriptor pointer */
#define XAXIDMA_CDESC_MSB_OFFSET 0x0000000C /**< Current descriptor pointer */
#define XAXIDMA_TDESC_OFFSET 0x00000010     /**< Tail descriptor pointer */
#define XAXIDMA_TDESC_MSB_OFFSET 0x00000014 /**< Tail descriptor pointer */
#define XAXIDMA_HALTED_MASK 0x00000001      /**< DMA channel halted */
#define XAXIDMA_IDLE_MASK 0x00000002        /**< DMA channel idle */

volatile void *dma_ctrl_map_base_mmio;
void *         mem_map_base;

typedef struct {
  uint32_t next_desc;        // 00h
  uint32_t next_desc_msb;    // 04h
  uint32_t buffer_addr;      // 08h
  uint32_t buffer_addr_msb;  // 0ch
  uint32_t not_used[2];      // 10h & 14h
  uint32_t ctrl;             // 18h
  uint32_t status;           // 1ch
  uint32_t app[5];           // 20h 24h 28h 2ch 30h
  uint32_t aligned[3];       // 34h 38h 3ch
} SG_Desc;                   // aligned to 64

volatile SG_Desc *DMA_Src_Desc[3];
volatile SG_Desc *DMA_Dst_Desc[3];

volatile uint32_t *data_src0;
volatile uint32_t *data_src1;
volatile uint32_t *data_src2;

volatile uint32_t *data_dst0;
volatile uint32_t *data_dst1;
volatile uint32_t *data_dst2;

#define data_size0 0x2000  // 40MB
#define data_size1 0x2000  // 40MB
#define data_size2 0x2000  // 48MB

#define data_src_offset0 0x200
#define data_src_offset1 (data_src_offset0 + data_size0)
#define data_src_offset2 (data_src_offset1 + data_size1)

#define data_dst_offset0 (data_src_offset2 + data_size2)
#define data_dst_offset1 (data_dst_offset0 + data_size0)
#define data_dst_offset2 (data_dst_offset1 + data_size1)

void data_init(void) {
  data_src0 = (uint32_t *)(mem_map_base + data_src_offset0);
  data_src1 = (uint32_t *)(mem_map_base + data_src_offset1);
  data_src2 = (uint32_t *)(mem_map_base + data_src_offset2);

  uint32_t i;
  for (i = 0; i < data_size0 / sizeof(uint32_t); i++) {
    data_src0[i] = i;
  }
  for (; i < (data_size0 + data_size1) / sizeof(uint32_t); i++) {
    data_src1[i - (data_size0 / sizeof(uint32_t))] = i;
  }

  for (; i < (data_size0 + data_size1 + data_size2) / sizeof(uint32_t); i++) {
    data_src2[i - ((data_size0 + data_size1) / sizeof(uint32_t))] = i;
  }

  data_dst0 = (uint32_t *)(mem_map_base + data_dst_offset0);
  data_dst1 = (uint32_t *)(mem_map_base + data_dst_offset1);
  data_dst2 = (uint32_t *)(mem_map_base + data_dst_offset2);
  memset(data_dst0, 0, data_size0);
  memset(data_dst1, 0, data_size1);
  memset(data_dst2, 0, data_size2);
}

int DataCheck(void) {
  FILE *fp = fopen("log.txt", "w+");
  if (NULL == fp) {
    printf("open failed\n");
    return -1;
  }

  int i;
  int err = 0;

  for (i = 0; i < data_size0 / sizeof(uint32_t); i++) {
    if (data_src0[i] != data_dst0[i]) {
      err++;
      fprintf(fp, "data_src0[%d]:%ld, data_dst0[%d]:%ld\n", i, data_src0[i], i, data_dst0[i]);
    }
  }

  for (i = 0; i < data_size1 / sizeof(uint32_t); i++) {
    if (data_src1[i] != data_dst1[i]) {
      err++;
      fprintf(fp, "data_src1[%d]:%ld, data_dst1[%d]:%ld\n", i, data_src1[i], i, data_dst1[i]);
    }
  }

  for (i = 0; i < data_size2 / sizeof(uint32_t); i++) {
    if (data_src2[i] != data_dst2[i]) {
      err++;
      fprintf(fp, "data_src2[%d]:%ld, data_dst2[%d]:%ld\n", i, data_src2[i], i, data_dst2[i]);
    }
  }
  fclose(fp);
  return err;
}

void DMA_Src_Desc_Init(void) {
  DMA_Src_Desc[0] = (SG_Desc *)mem_map_base;
  DMA_Src_Desc[1] = (SG_Desc *)(mem_map_base + 0x40);
  DMA_Src_Desc[2] = (SG_Desc *)(mem_map_base + 0x80);

  // send start
  DMA_Src_Desc[0]->next_desc       = (DATA_MEM_BASE_ADDR + 0x40);
  DMA_Src_Desc[0]->next_desc_msb   = ((DATA_MEM_BASE_ADDR + 0x40) >> 32);
  DMA_Src_Desc[0]->buffer_addr     = (DATA_MEM_BASE_ADDR + data_src_offset0);
  DMA_Src_Desc[0]->buffer_addr_msb = ((DATA_MEM_BASE_ADDR + data_src_offset0) >> 32);
  DMA_Src_Desc[0]->ctrl            = (data_size0) | XAXIDMA_BD_CTRL_TXSOF_MASK;

  DMA_Src_Desc[1]->next_desc       = (DATA_MEM_BASE_ADDR + 0x80);
  DMA_Src_Desc[1]->next_desc_msb   = ((DATA_MEM_BASE_ADDR + 0x80) >> 32);
  DMA_Src_Desc[1]->buffer_addr     = (DATA_MEM_BASE_ADDR + data_src_offset1);
  DMA_Src_Desc[1]->buffer_addr_msb = ((DATA_MEM_BASE_ADDR + data_src_offset1) >> 32);
  DMA_Src_Desc[1]->ctrl            = data_size1;

  // send end
  DMA_Src_Desc[2]->next_desc       = DATA_MEM_BASE_ADDR;
  DMA_Src_Desc[2]->next_desc_msb   = (DATA_MEM_BASE_ADDR >> 32);
  DMA_Src_Desc[2]->buffer_addr     = (DATA_MEM_BASE_ADDR + data_src_offset2);
  DMA_Src_Desc[2]->buffer_addr_msb = ((DATA_MEM_BASE_ADDR + data_src_offset2) >> 32);
  DMA_Src_Desc[2]->ctrl            = (data_size2) | XAXIDMA_BD_CTRL_TXEOF_MASK;
}

void DMA_Dst_Desc_Init(void) {
  DMA_Dst_Desc[0] = (SG_Desc *)(mem_map_base + 0x100);
  DMA_Dst_Desc[1] = (SG_Desc *)(mem_map_base + 0x140);
  DMA_Dst_Desc[2] = (SG_Desc *)(mem_map_base + 0x180);

  // receive start
  DMA_Dst_Desc[0]->next_desc       = (DATA_MEM_BASE_ADDR + 0x140);
  DMA_Dst_Desc[0]->next_desc_msb   = ((DATA_MEM_BASE_ADDR + 0x140) >> 32);
  DMA_Dst_Desc[0]->buffer_addr     = (DATA_MEM_BASE_ADDR + data_dst_offset0);
  DMA_Dst_Desc[0]->buffer_addr_msb = ((DATA_MEM_BASE_ADDR + data_dst_offset0) >> 32);
  DMA_Dst_Desc[0]->ctrl            = (data_size0);

  DMA_Dst_Desc[1]->next_desc       = (DATA_MEM_BASE_ADDR + 0x180);
  DMA_Dst_Desc[1]->next_desc_msb   = ((DATA_MEM_BASE_ADDR + 0x180) >> 32);
  DMA_Dst_Desc[1]->buffer_addr     = (DATA_MEM_BASE_ADDR + data_dst_offset1);
  DMA_Dst_Desc[1]->buffer_addr_msb = ((DATA_MEM_BASE_ADDR + data_dst_offset1) >> 32);
  DMA_Dst_Desc[1]->ctrl            = data_size1;

  // receive end
  DMA_Dst_Desc[2]->next_desc       = (DATA_MEM_BASE_ADDR + 0x100);
  DMA_Dst_Desc[2]->next_desc_msb   = ((DATA_MEM_BASE_ADDR + 0x100) >> 32);
  DMA_Dst_Desc[2]->buffer_addr     = (DATA_MEM_BASE_ADDR + data_dst_offset2);
  DMA_Dst_Desc[2]->buffer_addr_msb = ((DATA_MEM_BASE_ADDR + data_dst_offset2) >> 32);
  DMA_Dst_Desc[2]->ctrl            = (data_size2);
}

void DMA_Src_Init(void) {
  // reset
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_CR_OFFSET) = XAXIDMA_CR_RESET_MASK;

  // disable all interrupt
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_CR_OFFSET) =
      (*(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_CR_OFFSET) & (~XAXIDMA_IRQ_ALL_MASK));
  // It can be ignored due to system reset;

  // set DMA_Desc address
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_CDESC_OFFSET)     = DATA_MEM_BASE_ADDR;
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_CDESC_MSB_OFFSET) = (DATA_MEM_BASE_ADDR >> 32);

  /*
  //start
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_CR_OFFSET) = (*(uint32_t *)(dma_ctrl_map_base_mmio
  + XAXIDMA_CR_OFFSET) | XAXIDMA_CR_RUNSTOP_MASK); while(1)
  {
    if((*(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_SR_OFFSET) & XAXIDMA_HALTED_MASK) == 0)
      break;
  }
   //trigger
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_TDESC_OFFSET) = (DATA_MEM_BASE_ADDR + 0x80);
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_TDESC_MSB_OFFSET) = ((DATA_MEM_BASE_ADDR + 0x80) >>
  32);
  */
}

void DMA_Src_Trig(void) {
  // start
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_CR_OFFSET) =
      (*(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_CR_OFFSET) | XAXIDMA_CR_RUNSTOP_MASK);
  while (1) {
    if ((*(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_SR_OFFSET) & XAXIDMA_HALTED_MASK) == 0)
      break;
  }
  // trigger
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_TDESC_OFFSET) = (DATA_MEM_BASE_ADDR + 0x80);
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_TDESC_MSB_OFFSET) =
      ((DATA_MEM_BASE_ADDR + 0x80) >> 32);
}

uint32_t DMA_Src_Status(void) {
  return (*(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_SR_OFFSET) & XAXIDMA_IDLE_MASK);
}

void DMA_Dst_Init(void) {
  // reset
  //*(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_RX_OFFSET + XAXIDMA_CR_OFFSET) =
  // XAXIDMA_CR_RESET_MASK;

  // disable all interrupt
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_RX_OFFSET + XAXIDMA_CR_OFFSET) =
      (*(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_RX_OFFSET + XAXIDMA_CR_OFFSET) &
       (~XAXIDMA_IRQ_ALL_MASK));
  // It can be ignored due to system reset;

  // set DMA_Desc address
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_RX_OFFSET + XAXIDMA_CDESC_OFFSET) =
      (DATA_MEM_BASE_ADDR + 0x100);
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_RX_OFFSET + XAXIDMA_CDESC_MSB_OFFSET) =
      ((DATA_MEM_BASE_ADDR + 0x100) >> 32);

  /*
  //start
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_RX_OFFSET + XAXIDMA_CR_OFFSET) = (*(uint32_t
  *)(dma_ctrl_map_base_mmio + XAXIDMA_RX_OFFSET + XAXIDMA_CR_OFFSET) | XAXIDMA_CR_RUNSTOP_MASK);
  while(1)
  {
    if((*(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_RX_OFFSET +
  XAXIDMA_SR_OFFSET)&XAXIDMA_HALTED_MASK)==0) break;
  }

  //trigger
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_RX_OFFSET + XAXIDMA_TDESC_OFFSET) =
  (DATA_MEM_BASE_ADDR + 0x180);
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_RX_OFFSET + XAXIDMA_TDESC_MSB_OFFSET) =
  ((DATA_MEM_BASE_ADDR + 0x180) >> 32);
  */
}

void DMA_Dst_Trig(void) {
  // start
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_RX_OFFSET + XAXIDMA_CR_OFFSET) =
      (*(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_RX_OFFSET + XAXIDMA_CR_OFFSET) |
       XAXIDMA_CR_RUNSTOP_MASK);
  while (1) {
    if ((*(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_RX_OFFSET + XAXIDMA_SR_OFFSET) &
         XAXIDMA_HALTED_MASK) == 0)
      break;
  }

  // trigger
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_RX_OFFSET + XAXIDMA_TDESC_OFFSET) =
      (DATA_MEM_BASE_ADDR + 0x180);
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_RX_OFFSET + XAXIDMA_TDESC_MSB_OFFSET) =
      ((DATA_MEM_BASE_ADDR + 0x180) >> 32);
}

uint32_t DMA_Dst_Status(void) {
  return (*(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_RX_OFFSET + XAXIDMA_SR_OFFSET) &
          XAXIDMA_IDLE_MASK);
}

void DMA_Init() {
  // reset
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_CR_OFFSET) = XAXIDMA_CR_RESET_MASK;
  //*(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_RX_OFFSET + XAXIDMA_CR_OFFSET) =
  // XAXIDMA_CR_RESET_MASK;

  sleep(5);

  // disable all interrupt
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_CR_OFFSET) =
      (*(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_CR_OFFSET) & (~XAXIDMA_IRQ_ALL_MASK));
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_RX_OFFSET + XAXIDMA_CR_OFFSET) =
      (*(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_RX_OFFSET + XAXIDMA_CR_OFFSET) &
       (~XAXIDMA_IRQ_ALL_MASK));

  // set DMA_Desc address
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_CDESC_OFFSET)     = DATA_MEM_BASE_ADDR;
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_CDESC_MSB_OFFSET) = (DATA_MEM_BASE_ADDR >> 32);

  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_RX_OFFSET + XAXIDMA_CDESC_OFFSET) =
      (DATA_MEM_BASE_ADDR + 0x100);
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_RX_OFFSET + XAXIDMA_CDESC_MSB_OFFSET) =
      ((DATA_MEM_BASE_ADDR + 0x100) >> 32);

  // start
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_CR_OFFSET) =
      (*(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_CR_OFFSET) | XAXIDMA_CR_RUNSTOP_MASK);
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_RX_OFFSET + XAXIDMA_CR_OFFSET) =
      (*(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_RX_OFFSET + XAXIDMA_CR_OFFSET) |
       XAXIDMA_CR_RUNSTOP_MASK);

  // trigger
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_TDESC_OFFSET) = (DATA_MEM_BASE_ADDR + 0x80);
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_TDESC_MSB_OFFSET) =
      ((DATA_MEM_BASE_ADDR + 0x80) >> 32);

  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_RX_OFFSET + XAXIDMA_TDESC_OFFSET) =
      (DATA_MEM_BASE_ADDR + 0x180);
  *(uint32_t *)(dma_ctrl_map_base_mmio + XAXIDMA_RX_OFFSET + XAXIDMA_TDESC_MSB_OFFSET) =
      ((DATA_MEM_BASE_ADDR + 0x180) >> 32);
}

void init_map() {
  int fd;

  fd = open("/dev/mem", O_RDWR | O_SYNC);
  if (fd == -1) {
    perror("init_map open failed:");
    exit(1);
  }

  // physical mapping to virtual memory
  mem_map_base =
      mmap(NULL, MEM_TOTAL_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, DATA_MEM_BASE_ADDR);

  if (mem_map_base == NULL) {
    perror("init_map mmap failed:");
    close(fd);
    exit(1);
  }
  memset(mem_map_base, 0, MEM_TOTAL_SIZE);

  // physical mapping to virtual memory
  dma_ctrl_map_base_mmio =
      mmap(NULL, REG_TOTAL_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, DMA_CTRL_ADDR);

  if (dma_ctrl_map_base_mmio == NULL) {
    perror("init_map reg mmap failed:");
    close(fd);
    exit(1);
  }
}

int main(void) {
  printf("Hello application!\r\n");

  init_map();
  printf("init_map() end\n");

  data_init();
  printf("data_init() end\n");

  DMA_Src_Desc_Init();
  DMA_Dst_Desc_Init();
  printf("DMA_Desc_init() end\n");

  DMA_Src_Init();
  DMA_Dst_Init();
  printf("DMA_init() end\n");
  // DMA_Init();

  DMA_Src_Trig();
  DMA_Dst_Trig();
  printf("DMA_Trig() end\n");

  int i = 0;
  while (1) {
    if (i >= 5)
      break;
    if (DMA_Src_Status())
      break;
    sleep(6);
    i++;
  }
  printf("try #%d, send data suceessfully!\r\n", i);

  i = 0;
  while (1) {
    if (i >= 5)
      break;
    if (DMA_Dst_Status())
      break;
    sleep(6);
    i++;
  }
  printf("try #%d, receive data suceessfully!\r\n", i);

  int err_cnt = 0;
  err_cnt     = DataCheck();
  printf("Err cnt:%d\n", err_cnt);

  printf("End application!\r\n");
  return 0;
}
