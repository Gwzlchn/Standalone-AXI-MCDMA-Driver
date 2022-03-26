#ifndef _RAM_H_
#define _RAM_H_

#include "mcdma.h"
#include "ram_mem_layout.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

int check_two_array_in_ram(volatile uint32_t *arr1, volatile uint32_t *arr2, uint32_t arr_len) {
  int      err_cnt = 0;
  uint32_t i;
  for (i = 0; i < arr_len; i++) {
    if (!(arr1 + i) || !(arr2 + i)) {
      printf("index %d get null pointer\n", i);
      break;
    }
    if (arr1[i] != arr2[i]) {
      err_cnt++;
      printf("[RAM]-data_src0[%d]:%08X, data_dst0[%d]:%08X\n", i, arr1[i], i, arr2[i]);
    }
  }
  return err_cnt;
}

void init_ram_multi_channel_buffer_data(volatile void *uram_init_data_base_addr,
                                        uint32_t       uram_single_channel_data_init_bytes,
                                        uint32_t       uram_init_channel_cnt) {
  uint32_t i = 0;
  for (; i < uram_init_channel_cnt; i++) {
    void *cur_channel_base_addr =
        (char *)uram_init_data_base_addr + i * uram_single_channel_data_init_bytes;
    memset(cur_channel_base_addr, (i + 1), uram_single_channel_data_init_bytes);
  }
}

/**
 * @param mm2s_or_s2mm = true: mm2s bd base addr
 *                     = false: s2mm bd base addr
 * @param chan maybe 1,2,3...16
 * **/
uint32_t get_single_chan_bd_base_addr(uint32_t chan, bool mm2s_or_s2mm) {
  if (mm2s_or_s2mm) {
    return RAM_SG_MM2S_BD_BASE + ((chan - 1) * RAM_SG_MM2S_BD_SINGLE_CHAN_RANGE);
  } else {
    return RAM_SG_S2MM_BD_BASE + ((chan - 1) * RAM_SG_S2MM_BD_SINGLE_CHAN_RANGE);
  }
}
#endif