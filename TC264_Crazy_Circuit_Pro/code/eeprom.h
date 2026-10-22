/*
 * eeprom.h
 *
 *  EEPROM参数存储模块头文件 — 声明Flash参数读写相关宏与函数
 *
 *  Created on: 2026年4月2日
 *      Author: Administrator
 */

#ifndef CODE_EEPROM_H_
#define CODE_EEPROM_H_

#include "zf_common_headfile.h"

#define FLASH_SECTION_INDEX       (0)   // 存储参数使用的Flash扇区编号
#define FLASH_PAGE_INDEX          (8)   // 存储参数使用的Flash页编号（每个扇区最后一个页）

void data_init(void);                   // 从EEPROM读取参数，若无有效数据则使用默认值初始化
void flash_write_mydata(void);          // 将当前全局参数写入EEPROM持久化存储

#endif /* CODE_EEPROM_H_ */
