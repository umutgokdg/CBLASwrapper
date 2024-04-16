#include "../include/rtklib.h"
#include "../include/rtklib_constansts.h"

/* constants/macros ----------------------------------------------------------*/

#define SQR(x)      ((x)*(x))

static const int navsys[]={             /* satellite systems */
        SYS_GPS,SYS_GLO,SYS_GAL,SYS_QZS,SYS_SBS,SYS_CMP,SYS_IRN,0 };

static const char syscodes[]="GREJSCI"; /* satellite system codes */

static const char obscodes[]="CLDS";    /* observation type codes */

static const double ura_eph[]={         /* RAa values (ref [3] 20.3.3.3.1.1) */
        2.4,3.4,4.85,6.85,9.65,13.65,24.0,48.0,96.0,192.0,384.0,768.0,1536.0,
        3072.0,6144.0,0.0
};

static const double ura_nominal[]={     /* URA nominal values */
        2.0,2.8,4.0,5.7,8.0,11.3,16.0,32.0,64.0,128.0,256.0,512.0,1024.0,
        2048.0,4096.0,8192.0
};

/* adjust time considering week handover -------------------------------------*/
gtime_t g_adjweek(gtime_t t, gtime_t t0)
{
    double tt=timediff(t,t0);
    if (tt<-302400.0) return timeadd(t, 604800.0);
    if (tt> 302400.0) return timeadd(t,-604800.0);
    return t;
}

/* adjust time considering week handover -------------------------------------*/
static gtime_t adjday(gtime_t t, gtime_t t0)
{
    double tt=timediff(t,t0);
    if (tt<-43200.0) return timeadd(t, 86400.0);
    if (tt> 43200.0) return timeadd(t,-86400.0);
    return t;
}

/* URA index to URA nominal value (m) ----------------------------------------*/
double uravalue(int sva)
{
    return 0<=sva&&sva<15?ura_nominal[sva]:8192.0;
}

/* URA value (m) to URA index ------------------------------------------------*/
int uraindex(double value)
{
    int i=0;
    for (int i=0;i<15;i++)
        if (ura_eph[i]>=value)
            break;
    return i;
}

/* Galileo SISA index to SISA nominal value (m) ------------------------------*/
double sisa_value(int sisa)
{
    if (sisa<= 49) return sisa*0.01;
    if (sisa<= 74) return 0.5+(sisa- 50)*0.02;
    if (sisa<= 99) return 1.0+(sisa- 75)*0.04;
    if (sisa<=125) return 2.0+(sisa-100)*0.16;
    return -1.0; /* unknown or NAPA */
}

/* Galileo SISA value (m) to SISA index --------------------------------------*/
static int sisa_index(double value)
{
    if (value<0.0 || value>6.0) return 255; /* unknown or NAPA */
    else if (value<=0.5) return (int)(value/0.01);
    else if (value<=1.0) return (int)((value-0.5)/0.02)+50;
    else if (value<=2.0) return (int)((value-1.0)/0.04)+75;
    return ((int)(value-2.0)/0.16)+100;
}
