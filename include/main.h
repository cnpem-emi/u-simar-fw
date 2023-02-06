#ifndef _MAIN_H_

#define _MAIN_H_

#include "display.h"
#include "bme280.h"
#include "local_redis.h"
#include "deep_sleep.h"


void submit_for_redis(void);

#endif //_MAIN_H_