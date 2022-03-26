
#include "mcdma.h"
#include "mcdma_regs.h"
#include "ram.h"
#include "ram_mem_layout.h"
#include "traffic_gen.h"

#include <fcntl.h>
#include <memory.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

volatile void *uram_base_addr_mapped;
volatile void *mcdma_base_addr_mapped;
volatile void *axis_traffic_gen_addr_mapped;

void uram_init_data(uint32_t *uram_arr_base_addr, int write_bytes) {
  int write_cnt = write_bytes / sizeof(uint32_t);
  printf("init %d bytes numbers in URAM, Array Count: %d\n", write_bytes, write_cnt);
  for (int i = 0; i < write_cnt; i++) {
    uram_arr_base_addr[i] = i;
  }
}

void memory_map_init() {
  int fd;

  fd = open("/dev/mem", O_RDWR | O_SYNC);
  if (fd == -1) {
    perror("[DRIVER]-ERROR-init_map open failed:");
    exit(1);
  }

  uram_base_addr_mapped =
      mmap(NULL, RAM_TOTAL_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, RAM_BASE_ADDR);

  if (uram_base_addr_mapped == NULL) {
    perror("[DRIVER]-ERROR-init_map mmap failed:");
    close(fd);
    exit(1);
  }
  // memset((void *)uram_base_addr_mapped, 0, RAM_TOTAL_SIZE);

  mcdma_base_addr_mapped =
      mmap(NULL, MCDMA_ADDR_RANGE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, MCDMA_BASE_ADDR);

  if (mcdma_base_addr_mapped == NULL) {
    perror("init_map reg mmap failed:");
    close(fd);
    exit(1);
  }

  axis_traffic_gen_addr_mapped = mmap(
      NULL, TRAFFIC_GEN_TOTAL_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, TRAFFIC_GEN_BASE_ADDR);

  if (mcdma_base_addr_mapped == NULL) {
    perror("init_map reg mmap failed:");
    close(fd);
    exit(1);
  }
#ifdef DEBUG
  printf("[DRIVER]-Remap uram and dma register done\n");
  printf("[DRIVER]-URAM BASE ADDR: \t\t%p\n", uram_base_addr_mapped);
  printf("[DRIVER]-MCDMA CTRL REG BASE ADDR:\t%p\n", mcdma_base_addr_mapped);
#endif
}

/**
 * @brief Setup all mm2s bd for single channel
 *
 * @param[inout] mm2s_bd_desc_base_addr_mapped: Base address for buffer descriptor of current
 * channel
 * @param[in] chan: current channel
 * @param[in] desc_cnt_per_chan: the number of the Buffer Descriptor for each channel
 * **/
void mcdma_setup_mm2s_single_chan_bd_desc(
    volatile MCDMA_SG_MM2S_Desc *mm2s_bd_desc_base_addr_mapped,
    uint32_t                     chan,
    uint32_t                     desc_cnt_per_chan) {
  // uint32_t each_bd_buffer_size = (MM2S_DATA_BUFFER_RANGE_SINGLE_CHAN / desc_cnt_per_chan);
  uint32_t each_bd_buffer_size = 0x10;
  for (uint32_t desc = 0; desc < desc_cnt_per_chan; desc++) {
    volatile MCDMA_SG_MM2S_Desc *cur_desc = (mm2s_bd_desc_base_addr_mapped + (desc));
    uint32_t                     cur_buffer_addr =
        RAM_BASE_ADDR + RAM_MM2S_DATA_CHX_BASE(chan) + (desc * each_bd_buffer_size);

    cur_desc->next_desc       = RAM_BASE_ADDR + RAM_SG_MM2S_CHX_BD(chan, (desc + 1));
    cur_desc->next_desc_msb   = 0x0;
    cur_desc->buffer_addr     = (uint32_t)cur_buffer_addr;
    cur_desc->buffer_addr_msb = 0x0;
    cur_desc->ctrl            = (each_bd_buffer_size & 0xFFFFFF);
    cur_desc->status          = 0x0;
    if (desc == 0) {
      cur_desc->ctrl |= MCDMA_BD_CTRL_TXSOF_MASK;
    }
    if (desc == (desc_cnt_per_chan - 1)) {
      cur_desc->ctrl |= MCDMA_BD_CTRL_TXEOF_MASK;
    }
#ifdef DEBUG
    print_single_mm2s_bd(cur_desc);
#endif
  }
}

/**
 * @brief Setup all mm2s bd for all channel
 *
 * @param[inout] uram_mm2s_all_chan_bd_base_addr_mapped: A pointer array, each element is the base
 * address for buffer descriptor of corresponding channel
 * @param[in]  chan_cnt: the number of the channel
 *
 * **/
void mcdma_set_mm2s_bd_desc_base_addr(MCDMA_SG_MM2S_Desc **uram_mm2s_all_chan_bd_base_addr_mapped,
                                      uint32_t             desc_cnt_per_chan,
                                      uint32_t             chan_cnt) {
  uint32_t chan;
  for (chan = 0; chan < chan_cnt; chan++) {
    uram_mm2s_all_chan_bd_base_addr_mapped[chan] =
        (MCDMA_SG_MM2S_Desc *)(uram_base_addr_mapped + RAM_SG_MM2S_CHX_BD(chan, 0));
    mcdma_setup_mm2s_single_chan_bd_desc(
        uram_mm2s_all_chan_bd_base_addr_mapped[chan], chan, desc_cnt_per_chan);
#ifdef DEBUG
    printf("[DRIVER]-Current channel bd base addr[%d]: %p\n",
           chan,
           uram_mm2s_all_chan_bd_base_addr_mapped[chan]);

#endif
  }
}

/**
 * @brief Setup all s2mm bd for single channel
 *
 * @param[inout] s2mm_bd_desc_base_addr_mapped: Base address for buffer descriptor of current
 * channel
 * @param[in] chan: current channel
 * @param[in] desc_cnt_per_chan: the number of the Buffer Descriptor for each channel
 * **/
void mcdma_setup_s2mm_single_chan_bd_desc(
    volatile MCDMA_SG_S2MM_Desc *s2mm_bd_desc_base_addr_mapped,
    uint32_t                     chan,
    uint32_t                     desc_cnt_per_chan) {
  uint32_t each_bd_buffer_size = (S2MM_DATA_BUFFER_RANGE_SINGLE_CHAN / desc_cnt_per_chan);
  // uint32_t each_bd_buffer_size = 0x10;
  for (uint32_t desc = 0; desc < desc_cnt_per_chan; desc++) {
    volatile MCDMA_SG_S2MM_Desc *cur_desc = (s2mm_bd_desc_base_addr_mapped + (desc));
    uint32_t                     cur_buffer_addr =
        RAM_BASE_ADDR + RAM_S2MM_DATA_CHX_BASE(chan) + (desc * each_bd_buffer_size);

    cur_desc->next_desc       = RAM_BASE_ADDR + RAM_SG_S2MM_CHX_BD(chan, (desc + 1));
    cur_desc->next_desc_msb   = 0x0;
    cur_desc->buffer_addr     = (uint32_t)cur_buffer_addr;
    cur_desc->buffer_addr_msb = 0x0;
    cur_desc->ctrl            = (each_bd_buffer_size & 0xFFFFFF);
    cur_desc->status          = 0x0;
    if (desc == (desc_cnt_per_chan - 1)) {
      cur_desc->next_desc = RAM_BASE_ADDR + RAM_SG_S2MM_CHX_BD(chan, (desc));
    }
#ifdef DEBUG
    print_single_s2mm_bd(cur_desc);
#endif
  }
}

/**
 * @brief Setup all s2mm bd for all channel
 *
 * @param[inout] s2mm_bd_desc_base_addr_mapped: A pointer array, each element is the base address
 * for buffer descriptor of corresponding channel
 * @param[in]  chan_cnt: the number of the channel
 *
 * **/
void mcdma_set_s2mm_bd_desc_base_addr(MCDMA_SG_S2MM_Desc **uram_s2mm_all_chan_bd_base_addr_mapped,
                                      uint32_t             desc_cnt_per_chan,
                                      uint32_t             chan_cnt) {
  uint32_t chan;
  for (chan = 0; chan < chan_cnt; chan++) {
    uram_s2mm_all_chan_bd_base_addr_mapped[chan] =
        (MCDMA_SG_S2MM_Desc *)(uram_base_addr_mapped + RAM_SG_S2MM_CHX_BD(chan, 0));
    mcdma_setup_s2mm_single_chan_bd_desc(
        uram_s2mm_all_chan_bd_base_addr_mapped[chan], chan, desc_cnt_per_chan);
#ifdef DEBUG
    printf("[DRIVER]-Current channel bd base addr[%d]: %p\n",
           chan,
           uram_s2mm_all_chan_bd_base_addr_mapped[chan]);

#endif
  }
}

/***
 * @brief: Set the CURDESC register for all MM2S channel, then set the fetch bit
 */
void mcdma_setup_all_mm2s_chan_cur_desc(volatile void *mcdma_base_addr, uint32_t chan_cnt) {
  uint32_t chan;
  for (chan = 0; chan < chan_cnt; chan++) {
    uint32_t cur_chan_first_desc = (RAM_BASE_ADDR + RAM_SG_MM2S_CHX_BD(chan, 0));
    write_mcdma_reg(mcdma_base_addr, MCDMA_MM2S_CHX_CURDESC_LSB(chan), cur_chan_first_desc);
  }
  // set the fetch bit
  for (chan = 0; chan < chan_cnt; chan++) {
    write_mcdma_reg(mcdma_base_addr,
                    MCDMA_MM2S_CHX_CR(chan),
                    MCDMA_MM2S_CHAN_CR_FETCH_MASK | MCDMA_MM2S_CHAN_CR_IOC_IRQEN_MASK);
  }
}

/***
 * @brief: Set the CURDESC register for all S2MM channel, then set the fetch bit
 */
void mcdma_setup_all_s2mm_chan_cur_desc(volatile void *mcdma_base_addr, uint32_t chan_cnt) {
  uint32_t chan;
  for (chan = 0; chan < chan_cnt; chan++) {
    uint32_t cur_chan_first_desc = (RAM_BASE_ADDR + RAM_SG_S2MM_CHX_BD(chan, 0));
    write_mcdma_reg(mcdma_base_addr, MCDMA_S2MM_CHX_CURDESC_LSB(chan), cur_chan_first_desc);
  }
  // set the fetch bit
  for (chan = 0; chan < chan_cnt; chan++) {
    write_mcdma_reg(mcdma_base_addr,
                    MCDMA_S2MM_CHX_CR(chan),
                    MCDMA_S2MM_CHAN_CR_FETCH_MASK | MCDMA_S2MM_CHAN_CR_IOC_IRQEN_MASK);
  }
}

/***
 * @brief: Set the TAILDESC register for all MM2S channel
 */
void mcdma_setup_all_mm2s_chan_tail_desc(volatile void *mcdma_base_addr,
                                         uint32_t       chan_cnt,
                                         uint32_t       desc_cnt_per_chan) {
  uint32_t chan;
  for (chan = 0; chan < chan_cnt; chan++) {
    uint32_t cur_chan_tail_desc =
        (RAM_BASE_ADDR + RAM_SG_MM2S_CHX_BD(chan, (desc_cnt_per_chan - 1)));
    write_mcdma_reg(mcdma_base_addr, MCDMA_MM2S_CHX_TAILDESC_LSB(chan), cur_chan_tail_desc);
  }
}

/***
 * @brief: Set the TAILDESC register for all S2MM channel
 */
void mcdma_setup_all_s2mm_chan_tail_desc(volatile void *mcdma_base_addr,
                                         uint32_t       chan_cnt,
                                         uint32_t       desc_cnt_per_chan) {
  uint32_t chan;
  for (chan = 0; chan < chan_cnt; chan++) {
    uint32_t cur_chan_tail_desc =
        (RAM_BASE_ADDR + RAM_SG_S2MM_CHX_BD(chan, (desc_cnt_per_chan - 1)));
    write_mcdma_reg(mcdma_base_addr, MCDMA_S2MM_CHX_TAILDESC_LSB(chan), cur_chan_tail_desc);
  }
}

/***
 * @brief: check all MM2S bd stat value
 */
void mcdma_check_all_mm2s_chan_stat(volatile void *uram_base_addr,
                                    uint32_t       chan_cnt,
                                    uint32_t       desc_cnt_per_chan) {
  uint32_t chan;
  uint32_t desc;
  for (chan = 0; chan < chan_cnt; chan++) {
    for (desc = 0; desc < desc_cnt_per_chan; desc++) {
      volatile MCDMA_SG_MM2S_Desc *cur_desc = (uram_base_addr + RAM_SG_MM2S_CHX_BD(chan, desc));
      print_single_mm2s_bd_stat(cur_desc);
    }
  }
}

/***
 * @brief: check all S2MM bd stat value
 */
void mcdma_check_all_s2mm_chan_stat(volatile void *uram_base_addr,
                                    uint32_t       chan_cnt,
                                    uint32_t       desc_cnt_per_chan) {
  uint32_t chan;
  uint32_t desc;
  for (chan = 0; chan < chan_cnt; chan++) {
    for (desc = 0; desc < desc_cnt_per_chan; desc++) {
      volatile MCDMA_SG_S2MM_Desc *cur_desc = (uram_base_addr + RAM_SG_S2MM_CHX_BD(chan, desc));
      print_single_s2mm_bd_stat(cur_desc);
    }
  }
}

int mcdma_test_polled(uint32_t chan_cnt) {
  /**
   * 1. Initialize the source data in ram
   * each MM2S channel buffer write some data
   * Each byte in MM2S Channel1 Buffer is set to 0x01,
   *              MM2S Channel2 Buffer is set to 0x02,etc
   **/
  init_ram_multi_channel_buffer_data(uram_base_addr_mapped + MM2S_DATA_BUFFER_BASE_ADDR,
                                     MM2S_DATA_BUFFER_RANGE_SINGLE_CHAN,
                                     chan_cnt);

  /**
   * 2. Init MM2S SG BD & S2MM SG BD in ram
   **/
  uint32_t             desc_cnt_per_chan = 1;
  MCDMA_SG_MM2S_Desc **uram_mm2s_all_chan_bd_base_addr_mapped;
  uram_mm2s_all_chan_bd_base_addr_mapped = malloc(sizeof(MCDMA_SG_MM2S_Desc *) * chan_cnt);
  mcdma_set_mm2s_bd_desc_base_addr(
      uram_mm2s_all_chan_bd_base_addr_mapped, desc_cnt_per_chan, chan_cnt);

  MCDMA_SG_S2MM_Desc **uram_s2mm_all_chan_bd_base_addr_mapped;
  uram_s2mm_all_chan_bd_base_addr_mapped = malloc(sizeof(MCDMA_SG_S2MM_Desc *) * chan_cnt);
  mcdma_set_s2mm_bd_desc_base_addr(
      uram_s2mm_all_chan_bd_base_addr_mapped, desc_cnt_per_chan, chan_cnt);

  /***
   * 3.1. Enable the required MM2S channels.
   */
  // mcdma_enable_mm2s_channel(mcdma_base_addr_mapped, chan_cnt);
  /***
   * 4.1  Enable the required S2MM channels.
   */
  mcdma_enable_s2mm_channel(mcdma_base_addr_mapped, chan_cnt);

  /***
   * 3.2 MM2S: Program the Current Descriptor (CD) registers of all the enabled channels.
   * 3.3 MM2S: Program the CHANNEL.Fetch bit of channel control registers.
   */
  // mcdma_setup_all_mm2s_chan_cur_desc(mcdma_base_addr_mapped, chan_cnt);
  /***
   * 3.4 MM2S: Start the MCDMA by programming MCDMA.RS bit
   */
  // write_mcdma_reg(mcdma_base_addr_mapped, MCDMA_MM2S_CCR, MCDMA_MM2S_CCR_RS);
  /***
   * 3.5 MM2S: Program the Interrupt threshold values, Enable Interrupts.
   * 3.6 MM2S: Program the Queue Scheduler register, if applicable.
   * ignored
   */
  /***
   * 3.7 MM2S: Program the TD register of channels, this channel will be triggered.
   */
  // mcdma_setup_all_mm2s_chan_tail_desc(mcdma_base_addr_mapped, chan_cnt, desc_cnt_per_chan);

  /***
   * 4.2 S2MM: Program the Current Descriptor (CD) registers of all the enabled channels.
   * 4.3 S2MM: Program the CHANNEL.Fetch bit of channel control registers.
   */
  mcdma_setup_all_s2mm_chan_cur_desc(mcdma_base_addr_mapped, chan_cnt);
  /***
   * 4.4 S2MM: Start the MCDMA by programming MCDMA.RS bit (0x500).
   */
  write_mcdma_reg(mcdma_base_addr_mapped, MCDMA_S2MM_CCR, MCDMA_S2MM_CCR_RS);
  /***
   * 4.5 S2MM: Program the interrupt thresholds, Enable Interrupts.
   * ignored
   */
  /***
   *
   * 4.6 S2MM: Program the TD register of channels, this channel will be triggered.
   */
  mcdma_setup_all_s2mm_chan_tail_desc(mcdma_base_addr_mapped, chan_cnt, desc_cnt_per_chan);

  printf("[DRIVER]-MM2S + S2MM all channels done!\n");

  return 0;
}

//
/***
 * @brief enable traffic generator
  Master Only/Master Loopback
  1. Configure Transfer Length (0x40009) to Transfer Length register.
  2. Enable the core to start generating streaming traffic.
  3. Poll for done bit in Streaming Control register.
  4. Read the Status register to compare the number of transactions generated versus programed
 value.
*/

void trfc_gen_enable_length_and_cnt(volatile uint32_t *ctrl_reg_virtual_addr,
                                    uint16_t           trans_length,
                                    uint16_t           trans_cnt) {
  write_trfc_gen_reg(
      axis_traffic_gen_addr_mapped, TRFC_GEN_TRANS_LENGTH, (trans_cnt << 16) | trans_length);
  write_trfc_gen_reg(axis_traffic_gen_addr_mapped, TRFC_GEN_STREAM_CTRL, 0x0);
  write_trfc_gen_reg(axis_traffic_gen_addr_mapped, TRFC_GEN_STREAM_CTRL, 0x1);
  write_trfc_gen_reg(axis_traffic_gen_addr_mapped, TRFC_GEN_STREAM_CONF, 0x0);
}

int main() {
#ifdef DEBUG
  printf("[DRIVER]-Struct SG MM2S BD Size is %ldB\n", sizeof(MCDMA_SG_MM2S_Desc));
  printf("[DRIVER]-Struct SG S2MM BD Size is %ldB\n", sizeof(MCDMA_SG_S2MM_Desc));
#endif
  memory_map_init();
  mcdma_reset_all_channel(mcdma_base_addr_mapped, true);
  mcdma_reset_all_channel(mcdma_base_addr_mapped, false);
  // trans bytes = trans_length * traffic_gen_data_width/8
  uint16_t trans_length = 256;
  uint16_t trans_cnt    = 0x1;
  trfc_gen_enable_length_and_cnt(axis_traffic_gen_addr_mapped, trans_length, trans_cnt);
  // Address in RAM after memory mapped
  uint32_t channel_cnt = 1;
  mcdma_test_polled(channel_cnt);
  // 5. wait for mm2s done and s2mm done
  // mcdma_wait_mm2s_all_channel_done(mcdma_base_addr_mapped, chan_cnt);
  mcdma_wait_s2mm_all_channel_done(mcdma_base_addr_mapped);

  mcdma_check_all_mm2s_chan_stat(uram_base_addr_mapped, channel_cnt, 1);
  mcdma_check_all_s2mm_chan_stat(uram_base_addr_mapped, channel_cnt, 1);
  // check_mcdma_mm2s_error_register(mcdma_base_addr_mapped);
  check_mcdma_s2mm_error_register(mcdma_base_addr_mapped);
  printf("S2MM Packet Drop 0x%X\n", read_mcdma_reg(mcdma_base_addr_mapped, 0x514));
  printf("AXI Traffic generator done 0x%X, transfer count 0x%X\n",
         read_trfc_gen_reg(axis_traffic_gen_addr_mapped, TRFC_GEN_STREAM_CTRL),
         read_trfc_gen_reg(axis_traffic_gen_addr_mapped, TRFC_GEN_TRANS_CNT));

  // Check identity between MM2S pages and S2MM pages
  check_two_array_in_ram(
      (volatile uint32_t *)((char *)uram_base_addr_mapped + RAM_MM2S_DATA_CHX_BASE(0)),
      (volatile uint32_t *)((char *)uram_base_addr_mapped + RAM_S2MM_DATA_CHX_BASE(0)),
      1024);
  return 0;
}