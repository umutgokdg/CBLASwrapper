/**
 * @file    : Ublox-Driver project file
 * @author  : MEHMET AKSU
 * @note    : mmtaksu.25@gmail.com
 * @date    : 03 / Aralık / 2021
 * @code    : rtklib_veri_yapilari.h file
 * @details : 
 */


#ifndef RTKLIB_VERI_YAPILARI_H
#define RTKLIB_VERI_YAPILARI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

typedef struct {        /* time struct */
    time_t time;        /* time (s) expressed by standard time_t */
    double sec;         /* fraction of second under 1 s */
} gtime_t;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* RTKLIB_VERI_YAPILARI_H */