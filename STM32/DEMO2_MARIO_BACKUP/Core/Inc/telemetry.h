#ifndef __TELEMETRY_H
#define __TELEMETRY_H

#include "main.h"
#include <stdint.h>
#include <stdio.h>

void Telemetry_Init(void);
void Telemetry_SendByte(uint8_t ch);
void Telemetry_SendStr(const char *str);
void Telemetry_Printf(const char *fmt, ...);
void Telemetry_LogKey(const char *key_name, uint8_t pressed);
void Telemetry_LogEvent(const char *event_name);
void Telemetry_LogFrame(uint32_t fps, uint32_t frame_ms, uint32_t oled_ms, uint32_t logic_ms,
                        int16_t mario_x, int16_t mario_y, int16_t cam_x,
                        uint8_t kj, uint8_t kl, uint8_t kr, uint8_t ka,
                        const char *state_str, const char *ps2_str);

#endif /* __TELEMETRY_H */
