#ifndef __TIMER_H__
#define __TIMER_H__

#include <esp_err.h>

esp_err_t start_timer( double intervallInSec, void (*fn)(void *) );

#endif /* __TIMER_H__ */