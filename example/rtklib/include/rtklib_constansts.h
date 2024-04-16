/**
 * @file    : Ublox-Driver project file
 * @author  : MEHMET AKSU
 * @note    : mmtaksu.25@gmail.com
 * @date    : 06 / July / 2021
 * @code    : rtklib_constansts.h file
 * @details : 
 */



#ifndef RTKLIB_CONSTANSTS_H
#define RTKLIB_CONSTANSTS_H

#ifdef __cplusplus
extern "C" {
#endif

#define PI          3.1415926535897932  /* pi */
#define D2R         (PI/180.0)          /* deg to rad */
#define R2D         (180.0/PI)          /* rad to deg */
#define CLIGHT      299792458.0         /* speed of light (m/s) */
#define SC2RAD      3.1415926535898     /* semi-circle to radian (IS-GPS) */
#define AU          149597870691.0      /* 1 AU (m) */
#define AS2R        (D2R/3600.0)        /* arc sec to radian */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* RTKLIB_CONSTANSTS_H */