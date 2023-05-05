#ifndef EVISOR_ARCH_SCHED_H_
#define EVISOR_ARCH_SCHED_H_

#include "kernel/task/task.h"

#ifdef __cplusplus
extern "C" {
#endif

void SchedContextSwitch(CpuContext* prev, CpuContext* next);

#ifdef __cplusplus
}
#endif

#endif  // EVISOR_ARCH_SCHED_H_
