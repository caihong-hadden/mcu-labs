#ifndef __SCOPE_ART_H
#define __SCOPE_ART_H

#include "main.h"

/* 示波器艺术画模式定义 */
typedef enum {
  ART_MODE_ECG = 1,       // 模式 1: 💓 真实医院心电监护仪 (心跳波形 + 真实滴滴心率声)
  ART_MODE_SKYLINE = 2,   // 模式 2: 🏰 宏伟城市天际线与古典城堡 (高楼、城堞、尖塔、跨海大桥)
  ART_MODE_MARIO = 3,     // 模式 3: 🍄 超级马里奥横版关卡 (水管、浮空问号方块、台阶与终点旗杆)
  ART_MODE_HEART = 4,     // 模式 4: 💖 动态呼吸脉动爱心包络 (PWM 密度调制的心脏搏动)
  ART_MODE_SERVO = 5      // 模式 5: ⚙️ 原舵机控制模式 (PB0 保持 50Hz 舵机信号输出)
} ScopeArtMode_t;

void           ScopeArt_Init(void);
void           ScopeArt_SetMode(ScopeArtMode_t mode);
ScopeArtMode_t ScopeArt_GetMode(void);
const char*    ScopeArt_GetModeName(void);
void           ScopeArt_NextMode(void);

/* 在主循环中高频无阻塞渲染波形 */
void           ScopeArt_RenderFrame(void);

#endif /* __SCOPE_ART_H */
