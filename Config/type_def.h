#ifndef __TYPE_DEF_H
#define __TYPE_DEF_H

#include <stdint.h>

/// 重新对应的数据类型
typedef _Bool bool_t;
typedef float fp32;
typedef double fp64;

#ifdef __cplusplus
extern "C" {
#endif

/// 重新导出部分函数
void Error_Handler(void);
bool_t uprint(uint8_t *data, uint8_t len);
bool_t uprintf(const char *format, ...);
uint32_t usb_bufget(uint8_t **buf);

#ifdef __cplusplus
}
#endif

#endif /* __TYPE_DEF_H */
