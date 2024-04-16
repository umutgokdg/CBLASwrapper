#include "../include/rtklib.h"
#include "../include/rtklib_constansts.h"

/* constants -----------------------------------------------------------------*/

/* sbas igp definition -------------------------------------------------------*/
const short x1[]={-75,-65,-55,-50,-45,-40,-35,-30,-25,-20,-15,-10,- 5,  0,  5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 65, 75, 85};
const short x2[]={-55,-50,-45,-40,-35,-30,-25,-20,-15,-10, -5,  0,  5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55};
const short x3[]={-75,-65,-55,-50,-45,-40,-35,-30,-25,-20,-15,-10,- 5,  0,  5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 65, 75};
const short x4[]={-85,-75,-65,-55,-50,-45,-40,-35,-30,-25,-20,-15,-10,- 5,  0,  5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 65, 75};
const short x5[]={-180,-175,-170,-165,-160,-155,-150,-145,-140,-135,-130,-125,-120,-115,
      -110,-105,-100,- 95,- 90,- 85,- 80,- 75,- 70,- 65,- 60,- 55,- 50,- 45,
      - 40,- 35,- 30,- 25,- 20,- 15,- 10,-  5,   0,   5,  10,  15,  20,  25,
        30,  35,  40,  45,  50,  55,  60,  65,  70,  75,  80,  85,  90,  95,
       100, 105, 110, 115, 120, 125, 130, 135, 140, 145, 150, 155, 160, 165,
       170, 175};
const short x6[]={-180,-170,-160,-150,-140,-130,-120,-110,-100,- 90,- 80,- 70,- 60,- 50,
      - 40,- 30,- 20,- 10,   0,  10,  20,  30,  40,  50,  60,  70,  80,  90,
       100, 110, 120, 130, 140, 150, 160, 170};
const short x7[]={-180,-150,-120,- 90,- 60,- 30,   0,  30,  60,  90, 120, 150};
const short x8[]={-170,-140,-110,- 80,- 50,- 20,  10,  40,  70, 100, 130, 160};

const sbsigpband_t igpband1[9][8]={ /* band 0-8 */
    {{-180,x1,  1, 28},{-175,x2, 29, 51},{-170,x3, 52, 78},{-165,x2, 79,101},
     {-160,x3,102,128},{-155,x2,129,151},{-150,x3,152,178},{-145,x2,179,201}},
    {{-140,x4,  1, 28},{-135,x2, 29, 51},{-130,x3, 52, 78},{-125,x2, 79,101},
     {-120,x3,102,128},{-115,x2,129,151},{-110,x3,152,178},{-105,x2,179,201}},
    {{-100,x3,  1, 27},{- 95,x2, 28, 50},{- 90,x1, 51, 78},{- 85,x2, 79,101},
     {- 80,x3,102,128},{- 75,x2,129,151},{- 70,x3,152,178},{- 65,x2,179,201}},
    {{- 60,x3,  1, 27},{- 55,x2, 28, 50},{- 50,x4, 51, 78},{- 45,x2, 79,101},
     {- 40,x3,102,128},{- 35,x2,129,151},{- 30,x3,152,178},{- 25,x2,179,201}},
    {{- 20,x3,  1, 27},{- 15,x2, 28, 50},{- 10,x3, 51, 77},{-  5,x2, 78,100},
     {   0,x1,101,128},{   5,x2,129,151},{  10,x3,152,178},{  15,x2,179,201}},
    {{  20,x3,  1, 27},{  25,x2, 28, 50},{  30,x3, 51, 77},{  35,x2, 78,100},
     {  40,x4,101,128},{  45,x2,129,151},{  50,x3,152,178},{  55,x2,179,201}},
    {{  60,x3,  1, 27},{  65,x2, 28, 50},{  70,x3, 51, 77},{  75,x2, 78,100},
     {  80,x3,101,127},{  85,x2,128,150},{  90,x1,151,178},{  95,x2,179,201}},
    {{ 100,x3,  1, 27},{ 105,x2, 28, 50},{ 110,x3, 51, 77},{ 115,x2, 78,100},
     { 120,x3,101,127},{ 125,x2,128,150},{ 130,x4,151,178},{ 135,x2,179,201}},
    {{ 140,x3,  1, 27},{ 145,x2, 28, 50},{ 150,x3, 51, 77},{ 155,x2, 78,100},
     { 160,x3,101,127},{ 165,x2,128,150},{ 170,x3,151,177},{ 175,x2,178,200}}
};
const sbsigpband_t igpband2[2][5]={ /* band 9-10 */
    {{  60,x5,  1, 72},{  65,x6, 73,108},{  70,x6,109,144},{  75,x6,145,180},
     {  85,x7,181,192}},
    {{- 60,x5,  1, 72},{- 65,x6, 73,108},{- 70,x6,109,144},{- 75,x6,145,180},
     {- 85,x8,181,192}}
};

/* variance of fast correction (udre=UDRE+1) ---------------------------------*/
static double varfcorr(int udre)
{
    const double var[14]={
        0.052,0.0924,0.1444,0.283,0.4678,0.8315,1.2992,1.8709,2.5465,3.326,
        5.1968,20.7870,230.9661,2078.695
    };
    return 0<udre&&udre<=14?var[udre-1]:0.0;
}
/* variance of ionosphere correction (give=GIVEI+1) --------------------------*/
static double varicorr(int give)
{
    const double var[15]={
        0.0084,0.0333,0.0749,0.1331,0.2079,0.2994,0.4075,0.5322,0.6735,0.8315,
        1.1974,1.8709,3.326,20.787,187.0826
    };
    return 0<give&&give<=15?var[give-1]:0.0;
}
/* fast correction degradation -----------------------------------------------*/
static double degfcorr(int ai)
{
    const double degf[16]={
        0.00000,0.00005,0.00009,0.00012,0.00015,0.00020,0.00030,0.00045,
        0.00060,0.00090,0.00150,0.00210,0.00270,0.00330,0.00460,0.00580
    };
    return 0<ai&&ai<=15?degf[ai]:0.0058;
}

/* search igps ---------------------------------------------------------------*/
static void searchigp(gtime_t time, const double *pos, const sbsion_t *ion,
                      const sbsigp_t **igp, double *x, double *y)
{
    int i,latp[2],lonp[4];
    double lat=pos[0]*R2D,lon=pos[1]*R2D;
    const sbsigp_t *p;
    
    trace(4,"searchigp: pos=%.3f %.3f\n",pos[0]*R2D,pos[1]*R2D);
    
    if (lon>=180.0) lon-=360.0;
    if (-55.0<=lat&&lat<55.0) {
        latp[0]=(int)floor(lat/5.0)*5;
        latp[1]=latp[0]+5;
        lonp[0]=lonp[1]=(int)floor(lon/5.0)*5;
        lonp[2]=lonp[3]=lonp[0]+5;
        *x=(lon-lonp[0])/5.0;
        *y=(lat-latp[0])/5.0;
    }
    else {
        latp[0]=(int)floor((lat-5.0)/10.0)*10+5;
        latp[1]=latp[0]+10;
        lonp[0]=lonp[1]=(int)floor(lon/10.0)*10;
        lonp[2]=lonp[3]=lonp[0]+10;
        *x=(lon-lonp[0])/10.0;
        *y=(lat-latp[0])/10.0;
        if (75.0<=lat&&lat<85.0) {
            lonp[1]=(int)floor(lon/90.0)*90;
            lonp[3]=lonp[1]+90;
        }
        else if (-85.0<=lat&&lat<-75.0) {
            lonp[0]=(int)floor((lon-50.0)/90.0)*90+40;
            lonp[2]=lonp[0]+90;
        }
        else if (lat>=85.0) {
            for (i=0;i<4;i++) lonp[i]=(int)floor(lon/90.0)*90;
        }
        else if (lat<-85.0) {
            for (i=0;i<4;i++) lonp[i]=(int)floor((lon-50.0)/90.0)*90+40;
        }
    }
    for (i=0;i<4;i++) if (lonp[i]==180) lonp[i]=-180;
    for (i=0;i<=MAXBAND;i++) {
        for (p=ion[i].igp;p<ion[i].igp+ion[i].nigp;p++) {
            if (p->t0.time==0) continue;
            if      (p->lat==latp[0]&&p->lon==lonp[0]&&p->give>0) igp[0]=p;
            else if (p->lat==latp[1]&&p->lon==lonp[1]&&p->give>0) igp[1]=p;
            else if (p->lat==latp[0]&&p->lon==lonp[2]&&p->give>0) igp[2]=p;
            else if (p->lat==latp[1]&&p->lon==lonp[3]&&p->give>0) igp[3]=p;
            if (igp[0]&&igp[1]&&igp[2]&&igp[3]) return;
        }
    }
}
/* sbas ionospheric delay correction -------------------------------------------
* compute sbas ionosphric delay correction
* args   : gtime_t  time    I   time
*          nav_t    *nav    I   navigation data
*          double   *pos    I   receiver position {lat,lon,height} (rad/m)
*          double   *azel   I   satellite azimuth/elavation angle (rad)
*          double   *delay  O   slant ionospheric delay (L1) (m)
*          double   *var    O   variance of ionospheric delay (m^2)
* return : status (1:ok, 0:no correction)
* notes  : before calling the function, sbas ionosphere correction parameters
*          in navigation data (nav->sbsion) must be set by callig 
*          sbsupdatecorr()
*-----------------------------------------------------------------------------*/
extern int sbsioncorr(gtime_t time, const nav_t *nav, const double *pos,
                      const double *azel, double *delay, double *var)
{
    const double re=6378.1363,hion=350.0;
    int i,err=0;
    double fp,posp[2],x=0.0,y=0.0,t,w[4]={0};
    const sbsigp_t *igp[4]={0}; /* {ws,wn,es,en} */
    
    trace(4,"sbsioncorr: pos=%.3f %.3f azel=%.3f %.3f\n",pos[0]*R2D,pos[1]*R2D,
          azel[0]*R2D,azel[1]*R2D);
    
    *delay=*var=0.0;
    if (pos[2]<-100.0||azel[1]<=0) return 1;
    
    /* ipp (ionospheric pierce point) position */
    fp=ionppp(pos,azel,re,hion,posp);
    
    /* search igps around ipp */
    searchigp(time,posp,nav->sbsion,igp,&x,&y);
    
    /* weight of igps */
    if (igp[0]&&igp[1]&&igp[2]&&igp[3]) {
        w[0]=(1.0-x)*(1.0-y); w[1]=(1.0-x)*y; w[2]=x*(1.0-y); w[3]=x*y;
    }
    else if (igp[0]&&igp[1]&&igp[2]) {
        w[1]=y; w[2]=x;
        if ((w[0]=1.0-w[1]-w[2])<0.0) err=1;
    }
    else if (igp[0]&&igp[2]&&igp[3]) {
        w[0]=1.0-x; w[3]=y;
        if ((w[2]=1.0-w[0]-w[3])<0.0) err=1;
    }
    else if (igp[0]&&igp[1]&&igp[3]) {
        w[0]=1.0-y; w[3]=x;
        if ((w[1]=1.0-w[0]-w[3])<0.0) err=1;
    }
    else if (igp[1]&&igp[2]&&igp[3]) {
        w[1]=1.0-x; w[2]=1.0-y;
        if ((w[3]=1.0-w[1]-w[2])<0.0) err=1;
    }
    else err=1;
    
    if (err) {
        trace(2,"no sbas iono correction: lat=%3.0f lon=%4.0f\n",posp[0]*R2D,
              posp[1]*R2D);
        return 0;
    }
    for (i=0;i<4;i++) {
        if (!igp[i]) continue;
        t=timediff(time,igp[i]->t0);
        *delay+=w[i]*igp[i]->delay;
        *var+=w[i]*varicorr(igp[i]->give)*9E-8*fabs(t);
    }
    *delay*=fp; *var*=fp*fp;
    
    trace(5,"sbsioncorr: dion=%7.2f sig=%7.2f\n",*delay,sqrt(*var));
    return 1;
}
/* get meterological parameters ----------------------------------------------*/
static void getmet(double lat, double *met)
{
    static const double metprm[][10]={ /* lat=15,30,45,60,75 */
        {1013.25,299.65,26.31,6.30E-3,2.77,  0.00, 0.00,0.00,0.00E-3,0.00},
        {1017.25,294.15,21.79,6.05E-3,3.15, -3.75, 7.00,8.85,0.25E-3,0.33},
        {1015.75,283.15,11.66,5.58E-3,2.57, -2.25,11.00,7.24,0.32E-3,0.46},
        {1011.75,272.15, 6.78,5.39E-3,1.81, -1.75,15.00,5.36,0.81E-3,0.74},
        {1013.00,263.65, 4.11,4.53E-3,1.55, -0.50,14.50,3.39,0.62E-3,0.30}
    };
    int i,j;
    double a;
    lat=fabs(lat);
    if      (lat<=15.0) for (i=0;i<10;i++) met[i]=metprm[0][i];
    else if (lat>=75.0) for (i=0;i<10;i++) met[i]=metprm[4][i];
    else {
        j=(int)(lat/15.0); a=(lat-j*15.0)/15.0;
        for (i=0;i<10;i++) met[i]=(1.0-a)*metprm[j-1][i]+a*metprm[j][i];
    }
}
/* tropospheric delay correction -----------------------------------------------
* compute sbas tropospheric delay correction (mops model)
* args   : gtime_t time     I   time
*          double   *pos    I   receiver position {lat,lon,height} (rad/m)
*          double   *azel   I   satellite azimuth/elavation (rad)
*          double   *var    O   variance of troposphric error (m^2)
* return : slant tropospheric delay (m)
*-----------------------------------------------------------------------------*/
extern double sbstropcorr(gtime_t time, const double *pos, const double *azel,
                          double *var)
{
    const double k1=77.604,k2=382000.0,rd=287.054,gm=9.784,g=9.80665;
    static double pos_[3]={0},zh=0.0,zw=0.0;
    int i;
    double c,met[10],sinel=sin(azel[1]),h=pos[2],m;
    
    trace(4,"sbstropcorr: pos=%.3f %.3f azel=%.3f %.3f\n",pos[0]*R2D,pos[1]*R2D,
          azel[0]*R2D,azel[1]*R2D);
    
    if (pos[2]<-100.0||10000.0<pos[2]||azel[1]<=0) {
        *var=0.0;
        return 0.0;
    }
    if (zh==0.0||fabs(pos[0]-pos_[0])>1E-7||fabs(pos[1]-pos_[1])>1E-7||
        fabs(pos[2]-pos_[2])>1.0) {
        getmet(pos[0]*R2D,met);
        c=cos(2.0*PI*(time2doy(time)-(pos[0]>=0.0?28.0:211.0))/365.25);
        for (i=0;i<5;i++) met[i]-=met[i+5]*c;
        zh=1E-6*k1*rd*met[0]/gm;
        zw=1E-6*k2*rd/(gm*(met[4]+1.0)-met[3]*rd)*met[2]/met[1];
        zh*=pow(1.0-met[3]*h/met[1],g/(rd*met[3]));
        zw*=pow(1.0-met[3]*h/met[1],(met[4]+1.0)*g/(rd*met[3])-1.0);
        for (i=0;i<3;i++) pos_[i]=pos[i];
    }
    m=1.001/sqrt(0.002001+sinel*sinel);
    *var=0.12*0.12*m*m;
    return (zh+zw)*m;
}
/* long term correction ------------------------------------------------------*/
static int sbslongcorr(gtime_t time, int sat, const sbssat_t *sbssat,
                       double *drs, double *ddts)
{
    const sbssatp_t *p;
    double t;
    int i;
    
    trace(3,"sbslongcorr: sat=%2d\n",sat);
    
    for (p=sbssat->sat;p<sbssat->sat+sbssat->nsat;p++) {
        if (p->sat!=sat||p->lcorr.t0.time==0) continue;
        t=timediff(time,p->lcorr.t0);
        if (fabs(t)>MAXSBSAGEL) {
            trace(2,"sbas long-term correction expired: %s sat=%2d t=%5.0f\n",
                  time_str(time,0),sat,t);
            return 0;
        }
        for (i=0;i<3;i++) drs[i]=p->lcorr.dpos[i]+p->lcorr.dvel[i]*t;
        *ddts=p->lcorr.daf0+p->lcorr.daf1*t;
        
        trace(5,"sbslongcorr: sat=%2d drs=%7.2f%7.2f%7.2f ddts=%7.2f\n",
              sat,drs[0],drs[1],drs[2],*ddts*CLIGHT);
        
        return 1;
    }
    /* if sbas satellite without correction, no correction applied */
    if (satsys(sat,NULL)==SYS_SBS) return 1;
    
    trace(2,"no sbas long-term correction: %s sat=%2d\n",time_str(time,0),sat);
    return 0;
}
/* fast correction -----------------------------------------------------------*/
static int sbsfastcorr(gtime_t time, int sat, const sbssat_t *sbssat,
                       double *prc, double *var)
{
    const sbssatp_t *p;
    double t;
    
    trace(3,"sbsfastcorr: sat=%2d\n",sat);
    
    for (p=sbssat->sat;p<sbssat->sat+sbssat->nsat;p++) {
        if (p->sat!=sat) continue;
        if (p->fcorr.t0.time==0) break;
        t=timediff(time,p->fcorr.t0)+sbssat->tlat;
        
        /* expire age of correction or UDRE==14 (not monitored) */
        if (fabs(t)>MAXSBSAGEF||p->fcorr.udre>=15) continue;
        *prc=p->fcorr.prc;
#ifdef RRCENA
        if (p->fcorr.ai>0&&fabs(t)<=8.0*p->fcorr.dt) {
            *prc+=p->fcorr.rrc*t;
        }
#endif
        *var=varfcorr(p->fcorr.udre)+degfcorr(p->fcorr.ai)*t*t/2.0;
        
        trace(5,"sbsfastcorr: sat=%3d prc=%7.2f sig=%7.2f t=%5.0f\n",sat,
              *prc,sqrt(*var),t);
        return 1;
    }
    trace(2,"no sbas fast correction: %s sat=%2d\n",time_str(time,0),sat);
    return 0;
}
/* sbas satellite ephemeris and clock correction -------------------------------
* correct satellite position and clock bias with sbas satellite corrections
* args   : gtime_t time     I   reception time
*          int    sat       I   satellite
*          nav_t  *nav      I   navigation data
*          double *rs       IO  sat position and corrected {x,y,z} (ecef) (m)
*          double *dts      IO  sat clock bias and corrected (s)
*          double *var      O   sat position and clock variance (m^2)
* return : status (1:ok,0:no correction)
* notes  : before calling the function, sbas satellite correction parameters 
*          in navigation data (nav->sbssat) must be set by callig
*          sbsupdatecorr().
*          satellite clock correction include long-term correction and fast
*          correction.
*          sbas clock correction is usually based on L1C/A code. TGD or DCB has
*          to be considered for other codes
*-----------------------------------------------------------------------------*/
extern int sbssatcorr(gtime_t time, int sat, const nav_t *nav, double *rs,
                      double *dts, double *var)
{
    double drs[3]={0},dclk=0.0,prc=0.0;
    int i;
    
    trace(3,"sbssatcorr : sat=%2d\n",sat);
    
    /* sbas long term corrections */
    if (!sbslongcorr(time,sat,&nav->sbssat,drs,&dclk)) {
        return 0;
    }
    /* sbas fast corrections */
    if (!sbsfastcorr(time,sat,&nav->sbssat,&prc,var)) {
        return 0;
    }
    for (i=0;i<3;i++) rs[i]+=drs[i];
    
    dts[0]+=dclk+prc/CLIGHT;
    
    trace(5,"sbssatcorr: sat=%2d drs=%6.3f %6.3f %6.3f dclk=%.3f %.3f var=%.3f\n",
          sat,drs[0],drs[1],drs[2],dclk,prc/CLIGHT,*var);
    
    return 1;
}
