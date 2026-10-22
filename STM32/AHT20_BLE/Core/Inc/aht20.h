#ifndef __AHT20_H
#define __AHT20_H

#include "main.h"

#define AHT20_ADDR_WRITE  0x70  // (0x38 << 1)
#define AHT20_ADDR_READ   0x71  // ((0x38 << 1) | 1)

// 初始化与探测
uint8_t AHT20_Init(void);
uint8_t AHT20_Probe(void);

// 阻塞式完整读取 (耗时约 80ms)
uint8_t AHT20_Read(float *temp, float *humidity);

// 非阻塞异步读取接口 (用于高主频无感刷新)
void    AHT20_StartMeasurement(void);
uint8_t AHT20_GetResult(float *temp, float *humidity);

#endif /* __AHT20_H */
