#ifndef _TRAFFIC_GEN_
#define _TRAFFIC_GEN_

#define TRAFFIC_GEN_BASE_ADDR 0x87200000
#define TRAFFIC_GEN_TOTAL_SIZE (1 << 16)  // 64KB

#define TRFC_GEN_STREAM_CTRL 0x30
#define TRFC_GEN_STREAM_CONF 0x34
#define TRFC_GEN_TRANS_LENGTH 0x38
#define TRFC_GEN_TRANS_CNT 0x3C

uint32_t write_trfc_gen_reg(volatile uint32_t *ctrl_reg_virtual_addr,
                            uint32_t           offset,
                            uint32_t           value) {
#ifdef DEBUG
  printf("[TRFC_GEN]-WRITE TRFC_GEN Reg[0x%X]=0x%X\n", offset, value);
#endif
  ctrl_reg_virtual_addr[offset >> 2] = value;
  return 0;
}

uint32_t read_trfc_gen_reg(volatile uint32_t *ctrl_reg_virtual_addr, uint32_t offset) {
  uint32_t read_val = ctrl_reg_virtual_addr[offset >> 2];
#ifdef DEBUG
  printf("[TRFC_GEN]-READ TRFC_GEN Reg[0x%X]=0x%X\n", offset, read_val);
#endif
  return read_val;
}

#endif