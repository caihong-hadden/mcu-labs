#ifndef __TELEMETRY_H
#define __TELEMETRY_H

#include "main.h"

void Telemetry_Init(void);
void Telemetry_Printf(const char *fmt, ...);
uint8_t Telemetry_ReadBLERx(char *out_buf, uint16_t max_len);

#endif /* __TELEMETRY_H */
