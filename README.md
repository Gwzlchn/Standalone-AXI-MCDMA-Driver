# Standalone-AXI-MCDMA-Driver

一个可以独立使用的AXI Multichannel DMA驱动。

## 功能
1. 轮询方式使用MCDMA（TODO）
2. 中断方式使用MCDMA（TODO）

## 如何使用
编辑 `ram_addr.h` 文件，为MCDMA指定一段独立的地址空间，其中包括buffer descriptor的空间和数据的空间
编辑 `mcdma_addr.h` 文件，修改 MCDMA_BASE_ADDR 定义，这是AXI MCDMA 用到的寄存器基地址

## 性能测试
1. RAM地址空间使用URAM时的性能（TODO）
2. RAM地址空间使用DRAM时的性能（TODO）