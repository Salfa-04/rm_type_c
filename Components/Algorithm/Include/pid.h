#ifndef __PID_H
#define __PID_H

#include "type_def.h"

#ifdef __cplusplus
extern "C" {
#endif

enum PID_MODE { PID_POSITION = 0, PID_DELTA };

typedef struct {
  uint8_t mode;

  fp32 Kp, Ki, Kd;  // PID 三参数
  fp32 max_out;     // 最大输出
  fp32 max_iout;    // 最大积分输出
  fp32 set, fdb, out, Pout, Iout, Dout;
  fp32 Dbuf[3];   // 微分项 0最新 1上一次 2上上次
  fp32 error[3];  // 误差项 0最新 1上一次 2上上次
} pid_type_def;

//   pid: PID结构数据指针
//   mode: PID_POSITION:普通PID; PID_DELTA: 差分PID
//   PID: 0: kp, 1: ki, 2:kd
//   max_out: pid最大输出
//   max_iout: pid最大积分输出
void PID_init(pid_type_def *pid, uint8_t mode, const fp32 PID[3], fp32 max_out,
              fp32 max_iout);

fp32 PID_calc(pid_type_def *pid, fp32 ref, fp32 set);
void PID_clear(pid_type_def *pid);

#ifdef __cplusplus
}
#endif

#endif /* __PID_H */
