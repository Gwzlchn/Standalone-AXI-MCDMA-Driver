/*
 * Total MM2S BD buffer size = 32KB
 * Total S2MM BD buffer size = 32KB
 * Total MM2S Data Buffer size = 32KB
 * Total S2MM Data Buffer size = 32KB
 *
 *  +---------------------------+  128KB
 *  |    S2MM Channel8 Buffer   |
 *  +---------------------------+  124KB
 *  |    S2MM Channel7 Buffer   |
 *  +---------------------------+  120KB
 *  |    S2MM Channel6 Buffer   |
 *  +---------------------------+  116KB
 *  |    S2MM Channel5 Buffer   |
 *  +---------------------------+  112KB
 *  |    S2MM Channel4 Buffer   |
 *  +---------------------------+  108KB
 *  |    S2MM Channel3 Buffer   |
 *  +---------------------------+  104KB
 *  |    S2MM Channel2 Buffer   |
 *  +---------------------------+  100KB
 *  |    S2MM Channel1 Buffer   |
 *  +---------------------------+  96KB
 *  |    MM2S Channel8 Buffer   |
 *  +---------------------------+  92KB
 *  |    MM2S Channel7 Buffer   |
 *  +---------------------------+  88KB
 *  |    MM2S Channel6 Buffer   |
 *  +---------------------------+  84KB
 *  |    MM2S Channel5 Buffer   |
 *  +---------------------------+  80KB
 *  |    MM2S Channel4 Buffer   |
 *  +---------------------------+  76KB
 *  |    MM2S Channel3 Buffer   |
 *  +---------------------------+  72KB
 *  |    MM2S Channel2 Buffer   |
 *  +---------------------------+  68KB
 *  |    MM2S Channel1 Buffer   |
 *  +---------------------------+  64KB
 *  |                           |
 *  ~                           ~  the size of the BD buffer for each S2MM channel is 2KB
 *  |         S2MM BD           |
 *  +---------------------------+  32KB
 *  |                           |
 *  ~                           ~  the size of the BD buffer for each MM2S channel is 2KB
 *  |         MM2S BD           |
 *  +---------------------------+  0
 */

#ifndef _RAM_ADDR_H_
#define _RAM_ADDR_H_

#define RAM_BASE_ADDR 0x86000000
#define RAM_TOTAL_SIZE (1 << 21)  // 2MB

// SG-Buffer Descriptor BASE and RANGE
#define RAM_SG_MM2S_BD_BASE (0x0)
#define RAM_SG_MM2S_BD_TOTAL_RANGE (1 << 15)        // 32KB=0x8000, 32KB/64B = 512
#define RAM_SG_MM2S_BD_SINGLE_CHAN_RANGE (1 << 11)  // 2KB=0x800, 2KB/64B = 32

#define RAM_SG_S2MM_BD_BASE (RAM_SG_MM2S_BD_BASE + RAM_SG_MM2S_BD_TOTAL_RANGE)
#define RAM_SG_S2MM_BD_TOTAL_RANGE (1 << 15)        // 32KBs
#define RAM_SG_S2MM_BD_SINGLE_CHAN_RANGE (1 << 11)  // 2KB, 2KB/64B = 32

// MM2S Channels, shell read data from RAM to network-stack
#define MM2S_DATA_BUFFER_BASE_ADDR 0x10000         // 64KB
#define MM2S_DATA_BUFFER_RANGE_SINGLE_CHAN 0x1000  // 4KB

#define MM2S_CH1_BUFFER_ADDR 0x10000    // 64KB
#define MM2S_CH1_SINGLE_DATA_BYTES 0x4  // 4Bytes
#define MM2S_CH2_BUFFER_ADDR 0x11000    // 68KB
#define MM2S_CH2_SINGLE_DATA_BYTES 0x2  // 2Bytes
#define MM2S_CH3_BUFFER_ADDR 0x12000    // 72KB
#define MM2S_CH3_SINGLE_DATA_BYTES 0x2  // 2Bytes
#define MM2S_CH4_BUFFER_ADDR 0x13000    // 76KB
#define MM2S_CH4_SINGLE_DATA_BYTES 0x6  // 6Bytes
#define MM2S_CH5_BUFFER_ADDR 0x14000    // 80KB
#define MM2S_CH5_SINGLE_DATA_BYTES 0x4  // 4Bytes

// S2MMM Channels, network-stack write data to RAM
#define S2MM_DATA_BUFFER_BASE_ADDR 0x20000         // 128KB
#define S2MM_DATA_BUFFER_RANGE_SINGLE_CHAN 0x1000  // 4KB

#define S2MM_CH1_BUFFER_ADDR 0x20000  // 128KB
#define S2MM_CH1_SINGLE_DATA_BYTES 0x2
#define S2MM_CH2_BUFFER_ADDR 0x21000  // 132KB
#define S2MM_CH2_SINGLE_DATA_BYTES 0x3
#define S2MM_CH3_BUFFER_ADDR 0x22000  // 136KB
#define S2MM_CH3_SINGLE_DATA_BYTES 0x5
#define S2MM_CH4_BUFFER_ADDR 0x23000  // 140KB
#define S2MM_CH4_SINGLE_DATA_BYTES 0x3
#define S2MM_CH5_BUFFER_ADDR 0x24000  // 144KB
#define S2MM_CH5_SINGLE_DATA_BYTES 0x11
#define S2MM_CH6_BUFFER_ADDR 0x25000  // 148KB
#define S2MM_CH6_SINGLE_DATA_BYTES 0x5

#endif