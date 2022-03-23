#ifndef _MCDMA_H_
#define _MCDMA_H_
#include "mcdma_addr.h"

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
  ctrl_reg_virtual_addr[offset >> 2] = value;
  return 0;
}

uint32_t read_mcdma_reg(volatile uint32_t *ctrl_reg_virtual_addr, uint32_t offset) {
  return ctrl_reg_virtual_addr[offset >> 2];
}

void check_mcdma_mm2s_error_register(volatile uint32_t *ctrl_reg_virtual_addr) {
#ifdef DEBUG
  uint32_t err = read_mcdma_reg(ctrl_reg_virtual_addr, MCDMA_MM2S_ERR);
  if (err & 0x40) {
    printf("MM2S SGDecErr\n");
  }
  if (err & 0x20) {
    printf("MM2S SGSlvErr\n");
  }
  if (err & 0x10) {
    printf("MM2S SGIntErr\n");
  }
  if (err & 0x08) {
    printf("MM2S RSVD ERR\n");
  }
  if (err & 0x04) {
    printf("MM2S DMA Dec Err\n");
  }
  if (err & 0x02) {
    printf("MM2S DMA SLv Err\n");
  }
  if (err & 0x01) {
    printf("MM2S DMA Intr Err\n");
  }
#endif
}

void check_mcdma_s2mm_error_register(volatile uint32_t *ctrl_reg_virtual_addr) {
#ifdef DEBUG
  uint32_t err = read_mcdma_reg(ctrl_reg_virtual_addr, MCDMA_S2MM_ERR);
  if (err & 0x40) {
    printf("S2MM SGDecErr\n");
  }
  if (err & 0x20) {
    printf("S2MM SGSlvErr\n");
  }
  if (err & 0x10) {
    printf("S2MM SGIntErr\n");
  }
  if (err & 0x08) {
    printf("S2MM RSVD ERR\n");
  }
  if (err & 0x04) {
    printf("S2MM DMA Dec Err\n");
  }
  if (err & 0x02) {
    printf("S2MM DMA SLv Err\n");
  }
  if (err & 0x01) {
    printf("S2MM DMA Intr Err\n");
  }
#endif
}

void print_single_mm2s_bd(volatile MCDMA_SG_MM2S_Desc *desc) {
#ifdef DEBUG
  printf("Current MM2S BD Virtual Addr \t%p\n", desc);
  printf("Next Desc \t%X, MSB %X\n", desc->next_desc, desc->next_desc_msb);
  printf("Buffer Addr \t%X, MSB %X\n", desc->buffer_addr, desc->buffer_addr_msb);
  printf("Ctrl Byte: TX SOF %X, TX EOF %X, Buffer Length 0x%X\n",
         (desc->ctrl) >> 31,
         (((desc->ctrl) >> 30) & 0x1),
         ((desc->ctrl) & 0xFFFFFFF));
#endif
}

void print_single_mm2s_bd_stat(volatile MCDMA_SG_MM2S_Desc *desc) {
#ifdef DEBUG
  uint32_t bd_stat = desc->status;
  if (bd_stat & 0x80000000) {
    printf("Completed\n");
  }
  if (bd_stat & 0x40000000) {
    printf("DMA DecErr\n");
  }
  if (bd_stat & 0x20000000) {
    printf("DMA SlvErr\n");
  }
  if (bd_stat & 0x10000000) {
    printf("DMA IntErr\n");
  }
  if (bd_stat & 0xFFFFFF) {
    printf("Transferred Bytes :%X\n", (bd_stat & 0xFFFFFF));
  }
#endif
}

void print_single_s2mm_bd(volatile MCDMA_SG_S2MM_Desc *desc) {
#ifdef DEBUG
  printf("Current S2MM BD Virtual Addr %p\n", desc);
  printf("Next Desc \t%X, MSB %X\n", desc->next_desc, desc->next_desc_msb);
  printf("Buffer Addr \t%X, MSB %X\n", desc->buffer_addr, desc->buffer_addr_msb);
  printf("Buffer Length %X\n", ((desc->ctrl) & 0xFFFFFFF));
#endif
}

void print_single_s2mm_bd_stat(volatile MCDMA_SG_S2MM_Desc *desc) {
#ifdef DEBUG
  uint32_t bd_stat = desc->status;
  if (bd_stat & 0x80000000) {
    printf("Completed\n");
  }
  if (bd_stat & 0x40000000) {
    printf("DMA DecErr\n");
  }
  if (bd_stat & 0x20000000) {
    printf("DMA SlvErr\n");
  }
  if (bd_stat & 0x10000000) {
    printf("DMA IntErr\n");
  }
  if (bd_stat & 0x08000000) {
    printf("RXSOF\n");
  }
  if (bd_stat & 0x04000000) {
    printf("RXEOF\n");
  }
  if (bd_stat & 0xFFFFFF) {
    printf("Transferred Bytes :%X\n", (bd_stat & 0xFFFFFF));
  }
#endif
}

void enable_mcdma_mm2s_channel(volatile uint32_t *ctrl_reg_virtual_addr, uint32_t chan_cnt) {
  uint32_t chan_cnt_mask = 0;
  while (chan_cnt) {
    chan_cnt--;
    chan_cnt_mask |= (1 << chan_cnt);
  }
  write_mcdma_reg(ctrl_reg_virtual_addr, MCDMA_MM2S_CHEN, chan_cnt_mask);
}

/**
 * @brief: This resets all the channels and clears all the registers.
 * Reconfigure the MCDMA after a soft reset.
 * Reset the MM2S or resets the S2MM
 *
 * @param 2s_or_s2mm = true: reset all mm2s channels
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
  while (!(read_mcdma_reg(ctrl_reg_virtual_addr, MCDMA_MM2S_CSR) & MCDMA_MM2S_CSR_IDLE))
    ;
}

void mcdma_wait_s2mm_all_channel_done(volatile uint32_t *ctrl_reg_virtual_addr) {
  while (!(read_mcdma_reg(ctrl_reg_virtual_addr, MCDMA_S2MM_CSR) & MCDMA_S2MM_CSR_IDLE))
    ;
}
#endif