#ifndef _MCDMA_ADDR_H_
#define _MCDMA_ADDR_H_

#define MCDMA_BASE_ADDR 0x87000000
#define MCDMA_ADDR_RANGE (1 << 16)  // 64KB

// Comomn MM2S Register Offset
#define MCDMA_MM2S_CCR 0x000
#define MCDMA_MM2S_CSR 0x004
#define MCDMA_MM2S_CHEN 0x008
#define MCDMA_MM2S_CHSER 0x00C
#define MCDMA_MM2S_ERR 0x010
#define MCDMA_MM2S_CH_SCHD_TYPE 0x014
#define MCDMA_MM2S_WRR_REG1 0x018
#define MCDMA_MM2S_WRR_REG2 0x01C
#define MCDMA_MM2S_CHANNELS_SERVICED 0x020
#define MCDMA_MM2S_ARCACHE_ARUSER 0x024
#define MCDMA_MM2S_INTR_STATUS 0x028
// MM2S Common Control Register
// R/W • 0 = Reset not in progress. Normal operation. • 1 = Reset in progress.
#define MCDMA_MM2S_CCR_RST 0x00000004
// R/W  • 0 = Stop – MCDMA stops when current (if any) MCDMA operations are complete.
//      • 1 = Run  – Start MCDMA operations.
#define MCDMA_MM2S_CCR_RS 0x00000001
// MM2S Common Status Register
// R/O • 0 = MCDMA is not idle. • 1 = MCDMA is idle.
#define MCDMA_MM2S_CSR_IDLE 0x00000002
// R/O  • 0 = MCDMA channel running. • 1= MCDMA.RS bit is set to 0.
#define MCDMA_MM2S_CSR_HALTED 0x00000001

// Any Channel registers offset, CHX means CH1, CH2, etc
#define MCDMA_MM2S_CHX_CR 0x00
#define MCDMA_MM2S_CHX_SR 0x04
#define MCDMA_MM2S_CHX_CURDESC_LSB 0x08
#define MCDMA_MM2S_CHX_CURDESC_MSB 0x0C
#define MCDMA_MM2S_CHX_TAILDESC_LSB 0x10
#define MCDMA_MM2S_CHX_TAILDESC_MSB 0x14
#define MCDMA_MM2S_CHX_PKTCOUNT_STAT 0x18

// MM2S channel register base addr
#define MCDMA_MM2S_CHX_REG_ADDR_RANGE 0x40
#define MCDMA_MM2S_CH1_BASE_ADDR 0x040
#define MCDMA_MM2S_CH2_BASE_ADDR 0x080
#define MCDMA_MM2S_CH3_BASE_ADDR 0x0C0
#define MCDMA_MM2S_CH4_BASE_ADDR 0x100
#define MCDMA_MM2S_CH5_BASE_ADDR 0x140
#define MCDMA_MM2S_CH6_BASE_ADDR 0x180
#define MCDMA_MM2S_CH7_BASE_ADDR 0x1C0
#define MCDMA_MM2S_CH8_BASE_ADDR 0x200
#define MCDMA_MM2S_CH9_BASE_ADDR 0x240
#define MCDMA_MM2S_CH10_BASE_ADDR 0x280

// Comomn S2MM Register Offset
#define MCDMA_S2MM_CCR 0x500
#define MCDMA_S2MM_CSR 0x504
#define MCDMA_S2MM_CHEN 0x508
#define MCDMA_S2MM_CHSER 0x50C
#define MCDMA_S2MM_ERR 0x510
#define MCDMA_S2MM_PKTDROP 0x514
#define MCDMA_S2MM_CHANNELS_SERVICED 0x518
#define MCDMA_S2MM_ARCACHE_ARUSER 0x51C
#define MCDMA_S2MM_INTR_STATUS 0x520
// S2MM Common Control Register
// R/W • 0 = Reset not in progress. Normal operation. • 1 = Reset in progress.
#define MCDMA_S2MM_CCR_RST 0x00000004
// R/W  • 0 = Stop – MCDMA stops when current (if any) MCDMA operations are complete.
//      • 1 = Run  – Start MCDMA operations.
#define MCDMA_S2MM_CCR_RS 0x00000001
// S2MM Common Status Register
// R/O • 0 = MCDMA is not idle. • 1 = MCDMA is idle.
#define MCDMA_S2MM_CSR_IDLE 0x00000002
// R/O  • 0 = MCDMA channel running. • 1= MCDMA.RS bit is set to 0.
#define MCDMA_S2MM_CSR_HALTED 0x00000001

// Any Channel registers offset, CHX means CH1, CH2, etc
#define MCDMA_S2MM_CHX_CR 0x00
#define MCDMA_S2MM_CHX_SR 0x04
#define MCDMA_S2MM_CHX_CURDESC_LSB 0x08
#define MCDMA_S2MM_CHX_CURDESC_MSB 0x0C
#define MCDMA_S2MM_CHX_TAILDESC_LSB 0x10
#define MCDMA_S2MM_CHX_TAILDESC_MSB 0x14
#define MCDMA_S2MM_CHX_PKTDROP_STAT 0x18
#define MCDMA_S2MM_CHX_PKTCOUNT_STAT 0x1C

// S2MM channel register base addr
#define MCDMA_S2MM_CHX_REG_ADDR_RANGE 0x40
#define MCDMA_S2MM_CH1_BASE_ADDR 0x540
#define MCDMA_S2MM_CH2_BASE_ADDR 0x580
#define MCDMA_S2MM_CH3_BASE_ADDR 0x5C0
#define MCDMA_S2MM_CH4_BASE_ADDR 0x600
#define MCDMA_S2MM_CH5_BASE_ADDR 0x640
#define MCDMA_S2MM_CH6_BASE_ADDR 0x680
#define MCDMA_S2MM_CH7_BASE_ADDR 0x6C0
#define MCDMA_S2MM_CH8_BASE_ADDR 0x700
#define MCDMA_S2MM_CH9_BASE_ADDR 0x740
#define MCDMA_S2MM_CH10_BASE_ADDR 0x780

#endif