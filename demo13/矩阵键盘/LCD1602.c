/**
 * @file LCD1602.c
 * @brief 字符型液晶显示屏 LCD1602 底层驱动模块源文件
 * @details 硬件平台: STC15F2K60S2 / 传统 8051 兼容总线
 *          引脚配置:
 *          - RS (数据/命令选择): P2.0 (0-命令, 1-数据)
 *          - RW (读写选择):       P2.1 (0-写, 1-读)
 *          - EN (使能控制):       P1.2 (下降沿执行命令/锁存数据)
 *          - DataPort (数据总线): P0 端口 (8位并行数据输入)
 */

#include <STC15F2K60S2.H>

/* 引脚配置 */
sbit LCD_RS = P2^0;
sbit LCD_RW = P2^1;
sbit LCD_EN = P1^2;
#define LCD_DataPort P0

/**
 * @brief LCD1602 软件阻塞延时约 1ms
 */
void LCD_Delay(void)
{
	unsigned char i, j;
	i = 2;
	j = 239;
	do
	{
		while (--j);
	} while (--i);
}

/**
 * @brief 向 LCD1602 写入一条指令
 * @param Command 要写入的控制指令字
 */
void LCD_WriteCommand(unsigned char Command)
{
	LCD_RS = 0;             // 选择命令寄存器
	LCD_RW = 0;             // 选择写入模式
	LCD_DataPort = Command; // 送指令数据至 P0 总线
	LCD_EN = 1;             // 拉高使能端
	LCD_Delay();
	LCD_EN = 0;             // 下降沿执行指令
	LCD_Delay();
}

/**
 * @brief 向 LCD1602 写入一个字节数据
 * @param Data 要写入的显示字符 ASCII 码
 */
void LCD_WriteData(unsigned char Data)
{
	LCD_RS = 1;          // 选择数据寄存器
	LCD_RW = 0;          // 选择写入模式
	LCD_DataPort = Data; // 送数据至 P0 总线
	LCD_EN = 1;          // 拉高使能端
	LCD_Delay();
	LCD_EN = 0;          // 下降沿写入数据
	LCD_Delay();
}

/**
 * @brief 设置光标位置 (内部函数)
 * @param Line 行号 (1 ~ 2)
 * @param Column 列号 (1 ~ 16)
 */
void LCD_SetCursor(unsigned char Line, unsigned char Column)
{
	if (Line == 1)
	{
		LCD_WriteCommand(0x80 | (Column - 1));
	}
	else if (Line == 2)
	{
		LCD_WriteCommand(0x80 | (Column - 1 + 0x40));
	}
}

/**
 * @brief LCD1602 屏幕初始化
 */
void LCD_Init(void)
{
	LCD_WriteCommand(0x38); // 8位双行显示, 5x7 点阵字符
	LCD_WriteCommand(0x0C); // 开显示，关光标，关闪烁
	LCD_WriteCommand(0x06); // 读写后光标自动右移，整屏不移动
	LCD_WriteCommand(0x01); // 清屏
}

/**
 * @brief 在指定行列显示单个字符
 * @param Line 行号 (1 ~ 2)
 * @param Column 列号 (1 ~ 16)
 * @param Char 显示字符
 */
void LCD_ShowChar(unsigned char Line, unsigned char Column, char Char)
{
	LCD_SetCursor(Line, Column);
	LCD_WriteData(Char);
}

/**
 * @brief 在指定行列显示字符串
 * @param Line 起始行号 (1 ~ 2)
 * @param Column 起始列号 (1 ~ 16)
 * @param String 字符串指针
 */
void LCD_ShowString(unsigned char Line, unsigned char Column, char *String)
{
	unsigned char i;
	LCD_SetCursor(Line, Column);
	for (i = 0; String[i] != '\0'; i++)
	{
		LCD_WriteData(String[i]);
	}
}

/**
 * @brief 计算 X 的 Y 次方 (内部辅助函数)
 */
int LCD_Pow(int X, int Y)
{
	unsigned char i;
	int Result = 1;
	for (i = 0; i < Y; i++)
	{
		Result *= X;
	}
	return Result;
}

/**
 * @brief 在指定行列显示十进制无符号整数
 * @param Line 行号 (1 ~ 2)
 * @param Column 列号 (1 ~ 16)
 * @param Number 显示数值 (0 ~ 65535)
 * @param Length 显示位数
 */
void LCD_ShowNum(unsigned char Line, unsigned char Column, unsigned int Number, unsigned char Length)
{
	unsigned char i;
	LCD_SetCursor(Line, Column);
	for (i = Length; i > 0; i--)
	{
		LCD_WriteData(Number / LCD_Pow(10, i - 1) % 10 + '0');
	}
}

/**
 * @brief 在指定行列显示十进制有符号整数
 * @param Line 行号 (1 ~ 2)
 * @param Column 列号 (1 ~ 16)
 * @param Number 显示数值 (-32768 ~ 32767)
 * @param Length 显示位数 (包含符号位)
 */
void LCD_ShowSignedNum(unsigned char Line, unsigned char Column, int Number, unsigned char Length)
{
	unsigned char i;
	unsigned int Number1;
	LCD_SetCursor(Line, Column);
	if (Number >= 0)
	{
		LCD_WriteData('+');
		Number1 = Number;
	}
	else
	{
		LCD_WriteData('-');
		Number1 = -Number;
	}
	for (i = Length; i > 0; i--)
	{
		LCD_WriteData(Number1 / LCD_Pow(10, i - 1) % 10 + '0');
	}
}

/**
 * @brief 在指定行列显示十六进制整数
 * @param Line 行号 (1 ~ 2)
 * @param Column 列号 (1 ~ 16)
 * @param Number 显示数值 (0 ~ 65535)
 * @param Length 显示位数
 */
void LCD_ShowHexNum(unsigned char Line, unsigned char Column, unsigned int Number, unsigned char Length)
{
	unsigned char i, SingleNumber;
	LCD_SetCursor(Line, Column);
	for (i = Length; i > 0; i--)
	{
		SingleNumber = Number / LCD_Pow(16, i - 1) % 16;
		if (SingleNumber < 10)
		{
			LCD_WriteData(SingleNumber + '0');
		}
		else
		{
			LCD_WriteData(SingleNumber - 10 + 'A');
		}
	}
}

/**
 * @brief 在指定行列显示二进制整数
 * @param Line 行号 (1 ~ 2)
 * @param Column 列号 (1 ~ 16)
 * @param Number 显示数值 (0 ~ 65535)
 * @param Length 显示位数
 */
void LCD_ShowBinNum(unsigned char Line, unsigned char Column, unsigned int Number, unsigned char Length)
{
	unsigned char i;
	LCD_SetCursor(Line, Column);
	for (i = Length; i > 0; i--)
	{
		LCD_WriteData(Number / LCD_Pow(2, i - 1) % 2 + '0');
	}
}
