
#ifndef __SYSTIMER_H_
#define __SYSTIMER_H_

#include "esp_timer.h"

/*初始化定时器 tps单位 微秒*/
void systimer_init(uint64_t tps);

#endif