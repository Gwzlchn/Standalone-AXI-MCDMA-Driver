#ifndef _MCDMA_H_
#define _MCDMA_H_
#include "mcdma_regs.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

typedef struct {
  uint32_t next_desc;        // 00h
  uint32_t next_desc_msb;    // 04h
  uint32_t buffer_addr;      // 08h
  uint32_t buffer_addr_msb;  // 0ch
  uint32_t not_used;         // 10h
  uint32_t ctrl;             // 14h
  uint32_t ctrl_sideband;    // 18h
  uint32_t status;           // 1ch
  uint32_t app[5];           // 20h 24h 28h 2ch 30h
  uint32_t unused[3];
} MCDMA_SG_MM2S_Desc __attribute__((__aligned__(64)));  // aligned to 64B: 0x40

typedef struct {
  uint32_t next_desc;        // 00h
  uint32_t next_desc_msb;    // 04h
  uint32_t buffer_addr;      // 08h
  uint32_t buffer_addr_msb;  // 0ch
  uint32_t not_used;         // 10h
  uint32_t ctrl;             // 14h
  uint32_t status;           // 18h
  uint32_t sideband_status;  // 1ch
  uint32_t app[5];           // 20h 24h 28h 2ch 30h
  uint32_t unused[3];
} MCDMA_SG_S2MM_Desc __attribute__((__aligned__(64)));  // aligned to 64B: 0x40

// MM2S Control register mask of each descriptor
#define MCDMA_BD_CTRL_TXSOF_MASK 0x80000000
#define MCDMA_BD_CTRL_TXEOF_MASK 0x40000000

uint32_t write_mcdma_reg(volatile uint32_t *ctrl_reg_virtual_addr,
                         uint32_t           offset,
                         uint32_t           value) {
#ifdef DEBUG
  printf("[MCDMA]-WRITE MCDMA Reg[0x%X]=0x%X\n", offset, value);
#endif
  ctrl_reg_virtual_addr[offset >> 2] = value;
  return 0;
}

uint32_t read_mcdma_reg(volatile uint32_t *ctrl_reg_virtual_addr, uint32_t offset) {
  uint32_t read_val = ctrl_reg_virtual_addr[offset >> 2];
#ifdef DEBUG
  printf("[MCDMA]-READ MCDMA Reg[0x%X]=0x%X\n", offset, read_val);
#endif
  return read_val;
}

void check_mcdma_mm2s_error_register(volatile uint32_t *ctrl_reg_virtual_addr) {
#ifdef DEBUG
  uint32_t err = read_mcdma_reg(ctrl_reg_virtual_addr, MCDMA_MM2S_ERR);
  if (err & 0x40) {
    printf("[MCDMA]-MM2S SGDecErr\n");
  }
  if (err & 0x20) {
    printf("[MCDMA]-MM2S SGSlvErr\n");
  }
  if (err & 0x10) {
    printf("[MCDMA]-MM2S SGIntErr\n");
  }
  if (err & 0x08) {
    printf("[MCDMA]-MM2S RSVD ERR\n");
  }
  if (err & 0x04) {
    printf("[MCDMA]-MM2S DMA Dec Err\n");
  }
  if (err & 0x02) {
    printf("[MCDMA]-MM2S DMA SLv Err\n");
  }
  if (err & 0x01) {
    printf("[MCDMA]-MM2S DMA Intr Err\n");
  }
#endif
}

void check_mcdma_s2mm_error_register(volatile uint32_t *ctrl_reg_virtual_addr) {
#ifdef DEBUG
  uint32_t err = read_mcdma_reg(ctrl_reg_virtual_addr, MCDMA_S2MM_ERR);
  if (err & 0x40) {
    printf("[MCDMA]-S2MM SGDecErr\n");
  }
  if (err & 0x20) {
    printf("[MCDMA]-S2MM SGSlvErr\n");
  }
  if (err & 0x10) {
    printf("[MCDMA]-S2MM SGIntErr\n");
  }
  if (err & 0x08) {
    printf("[MCDMA]-S2MM RSVD ERR\n");
  }
  if (err & 0x04) {
    printf("[MCDMA]-S2MM DMA Dec Err\n");
  }
  if (err & 0x02) {
    printf("[MCDMA]-S2MM DMA SLv Err\n");
  }
  if (err & 0x01) {
    printf("[MCDMA]-S2MM DMA Intr Err\n");
  }
#endif
}

void print_single_mm2s_bd(volatile MCDMA_SG_MM2S_Desc *desc) {
#ifdef DEBUG
  printf("[RAM]-Current MM2S BD Virtual Addr \t%p\n", desc);
  printf("[RAM]-Next Desc \t%X, MSB \t%X\n", desc->next_desc, desc->next_desc_msb);
  printf("[RAM]-Buffer Addr \t%X, MSB \t%X, Length %X, Tx SOF %X, TX EOF %X\n",
         desc->buffer_addr,
         desc->buffer_addr_msb,
         ((desc->ctrl) & 0xFFFFFFF),
         (desc->ctrl) >> 31,
         (((desc->ctrl) >> 30) & 0x1));
#endif
}

void print_single_mm2s_bd_stat(volatile MCDMA_SG_MM2S_Desc *desc) {
#ifdef DEBUG
  uint32_t bd_stat = desc->status;
  if (bd_stat & 0x80000000) {
    printf("[MCDMA]-MM2S Completed\n");
  }
  if (bd_stat & 0x40000000) {
    printf("[MCDMA]-MM2S ERROR-DMA DecErr\n");
  }
  if (bd_stat & 0x20000000) {
    printf("[MCDMA]-MM2S ERROR-DMA SlvErr\n");
  }
  if (bd_stat & 0x10000000) {
    printf("[MCDMA]-MM2S ERROR-DMA IntErr\n");
  }
  if (bd_stat & 0xFFFFFF) {
    printf("[MCDMA]-MM2S Transferred Bytes :%X\n", (bd_stat & 0xFFFFFF));
  }
#endif
}

void print_single_s2mm_bd(volatile MCDMA_SG_S2MM_Desc *desc) {
#ifdef DEBUG
  printf("[RAM]-Current S2MM BD Virtual Addr %p\n", desc);
  printf("[RAM]-Next Desc \t%X, MSB \t%X\n", desc->next_desc, desc->next_desc_msb);
  printf("[RAM]-Buffer Addr \t%X, MSB \t%X, Length %X\n",
         desc->buffer_addr,
         desc->buffer_addr_msb,
         (desc->ctrl) & 0xFFFFFFF);
#endif
}

void print_single_s2mm_bd_stat(volatile MCDMA_SG_S2MM_Desc *desc) {
#ifdef DEBUG
  uint32_t bd_stat = desc->status;
  if (bd_stat & 0x80000000) {
    printf("[MCDMA]-S2MM Completed\n");
  }
  if (bd_stat & 0x40000000) {
    printf("[MCDMA]-S2MM ERROR-DMA DecErr\n");
  }
  if (bd_stat & 0x20000000) {
    printf("[MCDMA]-S2MM ERROR-DMA SlvErr\n");
  }
  if (bd_stat & 0x10000000) {
    printf("[MCDMA]-S2MM ERROR-DMA IntErr\n");
  }
  if (bd_stat & 0x08000000) {
    printf("[MCDMA]-S2MM RXSOF\n");
  }
  if (bd_stat & 0x04000000) {
    printf("[MCDMA]-S2MM RXEOF\n");
  }
  if (bd_stat & 0xFFFFFF) {
    printf("[MCDMA]-S2MM Transferred Bytes :0x%X\n", (bd_stat & 0xFFFFFF));
  }
#endif
}

void print_single_mm2s_chan_stat(volatile uint32_t *ctrl_reg_virtual_addr, uint32_t chan_no) {
#ifdef DEBUG
  uint32_t single_chan_stat_reg = read_mcdma_reg(ctrl_reg_virtual_addr, MCDMA_MM2S_CHX_SR(chan_no));
  printf("[MCDMA]-MM2S Chan[%d] Stat \t", chan_no);
  if (single_chan_stat_reg & (0x1 << 7)) {
    printf("Err Irq ");
  }
  if (single_chan_stat_reg & (0x1 << 6)) {
    printf("Dly Irq ");
  }
  if (single_chan_stat_reg & (0x1 << 5)) {
    printf("IOC Irq ");
  }
  if (single_chan_stat_reg & (0x1 << 3)) {
    printf("Err_on_other_Irq ");
  }
  if (single_chan_stat_reg & 0x1) {
    printf("IDLE");
  }
  printf("\n");
#endif
}

void print_single_s2mm_chan_stat(volatile uint32_t *ctrl_reg_virtual_addr, uint32_t chan_no) {
#ifdef DEBUG
  uint32_t single_chan_stat_reg = read_mcdma_reg(ctrl_reg_virtual_addr, MCDMA_S2MM_CHX_SR(chan_no));
  printf("[MCDMA]-S2MM Chan[%d] Stat \t", chan_no);
  printf("[31:24] IRQ Delay Stat: %X ", ((single_chan_stat_reg)&0xFF000000) >> 24);
  printf("[23:16] IRQ Treadhold Stat: %X ", ((single_chan_stat_reg)&0x00FF0000) >> 16);
  printf("[15:8] IRQ Pktdrop Stat: %X ", ((single_chan_stat_reg)&0x0000FF00) >> 8);
  if (single_chan_stat_reg & (0x1 << 7)) {
    printf("Err Irq ");
  }
  if (single_chan_stat_reg & (0x1 << 6)) {
    printf("Dly Irq ");
  }
  if (single_chan_stat_reg & (0x1 << 5)) {
    printf("IOC Irq ");
  }
  if (single_chan_stat_reg & (0x1 << 4)) {
    printf("Pktdrop_irq ");
  }
  if (single_chan_stat_reg & (0x1 << 3)) {
    printf("Err_on_other_ch_irq ");
  }
  if (single_chan_stat_reg & (0x1 << 1)) {
    printf("BD ShortFall ");
  }
  if (single_chan_stat_reg & 0x1) {
    printf("IDLE ");
  }
  printf("\n");
  uint32_t single_chan_paket_drop_stat =
      read_mcdma_reg(ctrl_reg_virtual_addr, MCDMA_S2MM_CHX_PKTDROP_STAT(chan_no));
  uint32_t single_chan_paket_process_stat =
      read_mcdma_reg(ctrl_reg_virtual_addr, MCDMA_S2MM_CHX_PKTCOUNT_STAT(chan_no));
  printf("[MCDMA]-S2MM-Drop Count: 0x%X, Processed Count: 0x%X",
         single_chan_paket_drop_stat,
         single_chan_paket_process_stat);
  printf("\n");

#endif
}

void mcdma_enable_mm2s_channel(volatile uint32_t *ctrl_reg_virtual_addr, uint32_t chan_cnt) {
  uint32_t chan_cnt_mask = 0;
  while (chan_cnt) {
    chan_cnt--;
    chan_cnt_mask |= (1 << chan_cnt);
  }
#ifdef DEBUG
  printf("[MCDMA]-Enabling MM2S Channels (each bit is a channel) %X\n", chan_cnt_mask);
#endif
  write_mcdma_reg(ctrl_reg_virtual_addr, MCDMA_MM2S_CHEN, chan_cnt_mask);
}

void mcdma_enable_s2mm_channel(volatile uint32_t *ctrl_reg_virtual_addr, uint32_t chan_cnt) {
  uint32_t chan_cnt_mask = 0;
  while (chan_cnt) {
    chan_cnt--;
    chan_cnt_mask |= (1 << chan_cnt);
  }
#ifdef DEBUG
  printf("[MCDMA]-Enabling S2MM Channels (each bit is a channel) %X\n", chan_cnt_mask);
#endif
  write_mcdma_reg(ctrl_reg_virtual_addr, MCDMA_S2MM_CHEN, chan_cnt_mask);
}

/**
 * @brief: This resets all the channels and clears all the registers.
 * Reconfigure the MCDMA after a soft reset.
 * Reset the MM2S or resets the S2MM
 *
 * @param mm2s_or_s2mm = true: reset all mm2s channels
 *              = false: reset all s2mm channels
 **/
void mcdma_reset_all_channel(volatile uint32_t *ctrl_reg_virtual_addr, bool mm2s_or_s2mm) {
  if (mm2s_or_s2mm) {
    write_mcdma_reg(ctrl_reg_virtual_addr, MCDMA_MM2S_CCR, MCDMA_MM2S_CCR_RST);
    while (read_mcdma_reg(ctrl_reg_virtual_addr, MCDMA_MM2S_CCR) & MCDMA_MM2S_CCR_RST) {
      ;
    }

  } else {
    write_mcdma_reg(ctrl_reg_virtual_addr, MCDMA_S2MM_CCR, MCDMA_S2MM_CCR_RST);
    while (read_mcdma_reg(ctrl_reg_virtual_addr, MCDMA_S2MM_CCR) & MCDMA_S2MM_CCR_RST) {
      ;
    }
  }
}

void mcdma_wait_mm2s_all_channel_done(volatile uint32_t *ctrl_reg_virtual_addr) {
  uint32_t chan_cnt_mask      = read_mcdma_reg(ctrl_reg_virtual_addr, MCDMA_MM2S_CHEN);
  uint32_t idle_chan_cnt_mask = 0;
  uint32_t cur_chan = 0, cur_chan_bit = 0x1;
  while (idle_chan_cnt_mask != chan_cnt_mask) {
    if (cur_chan_bit & chan_cnt_mask) {
      uint32_t cur_chan_stat_reg =
          read_mcdma_reg(ctrl_reg_virtual_addr, MCDMA_MM2S_CHX_SR(cur_chan));
      if (cur_chan_stat_reg & (MCDMA_MM2S_CHAN_SR_IDLE_MASK | MCDMA_MM2S_CHAN_SR_IOC_IRQ_MASK)) {
        idle_chan_cnt_mask |= cur_chan_bit;
      }
      print_single_mm2s_chan_stat(ctrl_reg_virtual_addr, cur_chan);
    }
    if (cur_chan == 15) {
      cur_chan     = 0;
      cur_chan_bit = 0x1;
    } else {
      cur_chan++;
      cur_chan_bit = cur_chan << 1;
    }
  }
}

void mcdma_wait_s2mm_all_channel_done(volatile uint32_t *ctrl_reg_virtual_addr) {
  uint32_t chan_cnt_mask      = read_mcdma_reg(ctrl_reg_virtual_addr, MCDMA_S2MM_CHEN);
  uint32_t idle_chan_cnt_mask = 0;
  uint32_t cur_chan = 0, cur_chan_bit = 0x1;

  while (idle_chan_cnt_mask != chan_cnt_mask) {
    if (cur_chan_bit & chan_cnt_mask) {
      uint32_t cur_chan_stat_reg =
          read_mcdma_reg(ctrl_reg_virtual_addr, MCDMA_S2MM_CHX_SR(cur_chan));
      if (cur_chan_stat_reg & (MCDMA_S2MM_CHAN_SR_IDLE_MASK | MCDMA_S2MM_CHAN_SR_IOC_IRQ_MASK)) {
        idle_chan_cnt_mask |= cur_chan_bit;
      }
      print_single_s2mm_chan_stat(ctrl_reg_virtual_addr, cur_chan);
    }
    if (cur_chan == 15) {
      cur_chan     = 0;
      cur_chan_bit = 0x1;
    } else {
      cur_chan++;
      cur_chan_bit = cur_chan << 1;
    }
  }
}
#endif