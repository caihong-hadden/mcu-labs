#include "aht20.h"
#include "telemetry.h"

static uint16_t aht_scl = GPIO_PIN_6;
static uint16_t aht_sda = GPIO_PIN_7;

/* 宽容时序：~50kHz 工业级软件 I2C */
static inline void I2C_Delay(void)
{
  for (volatile int i = 0; i < 80; i++);
}

static inline void SCL_High(void) { GPIOB->BSRR = aht_scl; }
static inline void SCL_Low(void)  { GPIOB->BRR  = aht_scl; }
static inline void SDA_High(void) { GPIOB->BSRR = aht_sda; }
static inline void SDA_Low(void)  { GPIOB->BRR  = aht_sda; }
static inline uint8_t SDA_Read(void) { return (GPIOB->IDR & aht_sda) ? 1 : 0; }

static void I2C_BusClear(void)
{
  SDA_High();
  I2C_Delay();
  for (int i = 0; i < 9; i++)
  {
    SCL_High();
    I2C_Delay();
    SCL_Low();
    I2C_Delay();
  }
  SCL_Low();
  SDA_Low();
  I2C_Delay();
  SCL_High();
  I2C_Delay();
  SDA_High();
  I2C_Delay();
}

static void I2C_Start(void)
{
  SDA_High();
  SCL_High();
  I2C_Delay();
  SDA_Low();
  I2C_Delay();
  SCL_Low();
  I2C_Delay();
}

static void I2C_Stop(void)
{
  SCL_Low();
  SDA_Low();
  I2C_Delay();
  SCL_High();
  I2C_Delay();
  SDA_High();
  I2C_Delay();
}

static uint8_t I2C_WriteByte(uint8_t byte)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (byte & 0x80)
      SDA_High();
    else
      SDA_Low();
    I2C_Delay();
    SCL_High();
    I2C_Delay();
    SCL_Low();
    I2C_Delay();
    byte <<= 1;
  }

  // 释放 SDA 接收从机 ACK (0: ACK, 1: NACK)
  SDA_High();
  I2C_Delay();
  SCL_High();
  I2C_Delay();
  uint8_t ack = SDA_Read();
  SCL_Low();
  I2C_Delay();
  return ack;
}

static uint8_t I2C_ReadByte(uint8_t ack)
{
  uint8_t byte = 0;
  SDA_High();
  I2C_Delay();
  for (uint8_t i = 0; i < 8; i++)
  {
    byte <<= 1;
    SCL_High();
    I2C_Delay();
    if (SDA_Read()) byte |= 0x01;
    SCL_Low();
    I2C_Delay();
  }

  // 主机应答：ack=1 发 ACK (拉低 SDA)，ack=0 发 NACK (释放 SDA 置高)
  if (ack)
    SDA_Low();
  else
    SDA_High();
  I2C_Delay();
  SCL_High();
  I2C_Delay();
  SCL_Low();
  I2C_Delay();
  SDA_High();
  return byte;
}

static uint8_t Scan_And_Probe(uint16_t scl_pin, uint16_t sda_pin, const char *name)
{
  aht_scl = scl_pin;
  aht_sda = sda_pin;

  // SCL 与 SDA 均配置为开漏输出 (符合标准 I2C 规范，支持从机时钟延展 Clock Stretching)
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = aht_scl;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = aht_sda;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  SDA_High();
  SCL_High();
  I2C_BusClear();
  HAL_Delay(10);

  uint8_t found_aht = 0;
  uint8_t found_any = 0;

  for (uint8_t a = 1; a < 127; a++)
  {
    I2C_Start();
    uint8_t ack = I2C_WriteByte(a << 1);
    I2C_Stop();
    if (ack == 0)
    {
      found_any++;
      Telemetry_Printf("[I2C 发现设备] %s -> 发现从机地址 0x%02X (写=0x%02X)\r\n", name, a, a << 1);
      if (a == 0x38)
      {
        found_aht = 1;
      }
    }
  }

  if (!found_any)
  {
    Telemetry_Printf("[I2C 扫描] %s -> 无从机响应 (总线空闲/3.3V)\r\n", name);
  }

  return found_aht;
}

uint8_t AHT20_Init(void)
{
  __HAL_RCC_GPIOB_CLK_ENABLE();

  // 1. 电气短路与互连排查
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
  HAL_Delay(2);
  uint8_t pb6_when_pb7_low = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6);

  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
  HAL_Delay(2);
  uint8_t pb7_when_pb6_low = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7);

  GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_Delay(20);
  uint8_t lvl6 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6);
  uint8_t lvl7 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7);

  // 测试 D: 浮空输入模式 (GPIO_NOPULL)，测试硬件 PCB 上是否有真正的物理外部上拉电阻
  GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  HAL_Delay(20);
  uint8_t nopull6 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6);
  uint8_t nopull7 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7);
  Telemetry_Printf("[外部上拉电阻检测] PB6浮空=%d, PB7浮空=%d (%s)\r\n",
                   nopull6, nopull7, (nopull6 && nopull7) ? "板载有物理外部上拉电阻" : "警告：无物理外部上拉电阻！总线可能悬空浮动！");

  // 2. 依次测试方案 A 与方案 B
  if (Scan_And_Probe(GPIO_PIN_6, GPIO_PIN_7, "方案A: SCL=PB6, SDA=PB7"))
  {
    aht_scl = GPIO_PIN_6; aht_sda = GPIO_PIN_7;
    Telemetry_Printf("[AHT20] >>> 成功锁定方案 A: SCL=PB6, SDA=PB7 <<<\r\n");
  }
  else if (Scan_And_Probe(GPIO_PIN_7, GPIO_PIN_6, "方案B: SCL=PB7, SDA=PB6"))
  {
    aht_scl = GPIO_PIN_7; aht_sda = GPIO_PIN_6;
    Telemetry_Printf("[AHT20] >>> 成功锁定方案 B: SCL=PB7, SDA=PB6 <<<\r\n");
  }
  else
  {
    Telemetry_Printf("[AHT20] >>> PB6/PB7 均未探测到 AHT20(0x38)。 <<<\r\n");
    return 0;
  }

  HAL_Delay(100);

  // 3. 发送软复位 (0xBA) 并打印 ACK
  I2C_Start();
  uint8_t ack_w = I2C_WriteByte(AHT20_ADDR_WRITE);
  uint8_t ack_rst = I2C_WriteByte(0xBA);
  I2C_Stop();
  Telemetry_Printf("[AHT20 软复位] ADDR_W ACK=%d, 0xBA ACK=%d\r\n", ack_w, ack_rst);
  HAL_Delay(50);

  // 4. 发送校准指令 0xBE 0x08 0x00 (官方规范：开机强制校准一次)
  I2C_Start();
  uint8_t ack_be0 = I2C_WriteByte(AHT20_ADDR_WRITE);
  uint8_t ack_be1 = I2C_WriteByte(0xBE);
  uint8_t ack_be2 = I2C_WriteByte(0x08);
  uint8_t ack_be3 = I2C_WriteByte(0x00);
  I2C_Stop();
  Telemetry_Printf("[AHT20 校准初始化] 0xBE ACK=%d, 0x08 ACK=%d, 0x00 ACK=%d\r\n", ack_be1, ack_be2, ack_be3);
  HAL_Delay(50);

  // 5. 读取芯片初始状态字
  I2C_Start();
  uint8_t ack_r = I2C_WriteByte(AHT20_ADDR_READ);
  uint8_t status = I2C_ReadByte(0); // NACK 结束
  I2C_Stop();
  Telemetry_Printf("[AHT20 状态探测] 0x71 ACK=%d, 状态字=0x%02X (Bit3校准=%d, Bit7忙=%d)\r\n",
                   ack_r, status, (status & 0x08) ? 1 : 0, (status & 0x80) ? 1 : 0);

  return 1;
}

uint8_t AHT20_Probe(void)
{
  if (SDA_Read() == 0) return 0;
  I2C_Start();
  uint8_t ack = I2C_WriteByte(AHT20_ADDR_WRITE);
  I2C_Stop();
  return (ack == 0);
}

void AHT20_StartMeasurement(void)
{
  I2C_Start();
  uint8_t a0 = I2C_WriteByte(AHT20_ADDR_WRITE);
  uint8_t a1 = I2C_WriteByte(0xAC);
  uint8_t a2 = I2C_WriteByte(0x33);
  uint8_t a3 = I2C_WriteByte(0x00);
  I2C_Stop();
  if (a0 != 0 || a1 != 0 || a2 != 0 || a3 != 0)
  {
    Telemetry_Printf("[触发测量 ACK 诊断] 0x70:%d, 0xAC:%d, 0x33:%d, 0x00:%d\r\n", a0, a1, a2, a3);
  }
}

uint8_t AHT20_GetResult(float *temp, float *humidity)
{
  I2C_Start();
  uint8_t ack = I2C_WriteByte(AHT20_ADDR_READ);
  if (ack != 0)
  {
    I2C_Stop();
    Telemetry_Printf("[AHT20 错误] 读取地址 0x71 无应答 (ack=%d)\r\n", ack);
    return 0;
  }

  uint8_t data[6];
  data[0] = I2C_ReadByte(1); // 状态
  data[1] = I2C_ReadByte(1); // 湿度高
  data[2] = I2C_ReadByte(1); // 湿度低
  data[3] = I2C_ReadByte(1); // 混编
  data[4] = I2C_ReadByte(1); // 温度高
  data[5] = I2C_ReadByte(0); // 温度低
  I2C_Stop();

  if (data[0] & 0x80)
  {
    Telemetry_Printf("[AHT20 状态] 芯片转换忙碌中 (状态字: 0x%02X)\r\n", data[0]);
    return 0;
  }

  Telemetry_Printf("[AHT20 原始报文] 状态:0x%02X | D1=0x%02X D2=0x%02X D3=0x%02X D4=0x%02X D5=0x%02X\r\n",
                   data[0], data[1], data[2], data[3], data[4], data[5]);

  uint32_t raw_h = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | (data[3] >> 4);
  uint32_t raw_t = (((uint32_t)data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | data[5];

  if (raw_h == 0 && raw_t == 0)
  {
    Telemetry_Printf("[AHT20 错误] 报文全0 (未准备好)\r\n");
    return 0;
  }

  if (humidity) *humidity = ((float)raw_h * 100.0f) / 1048576.0f;
  if (temp)     *temp     = (((float)raw_t * 200.0f) / 1048576.0f) - 50.0f;

  return 1;
}

uint8_t AHT20_Read(float *temp, float *humidity)
{
  AHT20_StartMeasurement();
  HAL_Delay(100);
  return AHT20_GetResult(temp, humidity);
}
