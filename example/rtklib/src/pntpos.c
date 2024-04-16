/*------------------------------------------------------------------------------
* pntpos.c : standard positioning
*
*          Copyright (C) 2007-2015 by T.TAKASU, All rights reserved.
*
* version : $Revision:$ $Date:$
* history : 2010/07/28 1.0  moved from rtkcmn.c
*                           changed api:
*                               pntpos()
*                           deleted api:
*                               pntvel()
*           2011/01/12 1.1  add option to include unhealthy satellite
*                           reject duplicated observation data
*                           changed api: ionocorr()
*           2011/11/08 1.2  enable snr mask for single-mode (rtklib_2.4.1_p3)
*           2012/12/25 1.3  add variable snr mask
*           2014/05/26 1.4  support galileo and beidou
*           2015/03/19 1.5  fix bug on ionosphere correction for GLO and BDS
*-----------------------------------------------------------------------------*/
#include "../include/rtklib.h"
#include "../include/rtklib_constansts.h"


//static const char rcsid[]="$Id:$";

/* constants -----------------------------------------------------------------*/

#define SQR(x)      ((x)*(x))

#define NX          (4+3)       /* # of estimated parameters */

#define MAXITR      10          /* max number of iteration for point pos */
#define ERR_ION     5.0         /* ionospheric delay std (m) */
#define ERR_TROP    3.0         /* tropspheric delay std (m) */
#define ERR_SAAS    0.3         /* saastamoinen model error std (m) */
#define ERR_BRDCI   0.5         /* broadcast iono model error factor */
#define ERR_CBIAS   0.3         /* code bias error std (m) */
#define REL_HUMI    0.7         /* relative humidity for saastamoinen model */

/* pseudorange measurement error variance ------------------------------------*/
static double varerr(const prcopt_t *opt, double el, int sys)
{
    double fact,varr;
    fact=sys==SYS_GLO?EFACT_GLO:(sys==SYS_SBS?EFACT_SBS:EFACT_GPS);
    varr=SQR(opt->err[0])*(SQR(opt->err[1])+SQR(opt->err[2])/sin(el));
    if (opt->ionoopt==IONOOPT_IFLC) varr*=SQR(3.0); /* iono-free */
    return SQR(fact)*varr;
}

/* pseudorange measurement error variance ------------------------------------*/
extern double varerr_ekf(const prcopt_t *opt, double el, int sys)
{
    double fact,varr;
    fact=sys==SYS_GLO?EFACT_GLO:(sys==SYS_SBS?EFACT_SBS:EFACT_GPS);
    varr=SQR(opt->err[0])*(SQR(opt->err[1])+SQR(opt->err[2])/sin(el));
    if (opt->ionoopt==IONOOPT_IFLC) varr*=SQR(3.0); /* iono-free */
    return SQR(fact)*varr;
}

/* get tgd parameter (m) -----------------------------------------------------*/
static double gettgd(int sat, const nav_t *nav)
{
    int i;
    for (i=0;i<nav->n;i++) {
        if (nav->eph[i].sat!=sat) continue;
        return CLIGHT*nav->eph[i].tgd[0];
    }
    return 0.0;
}

/* get group delay parameter (m) ---------------------------------------------*/
static double gettgd_2(int sat, const nav_t *nav, int type)
{
    int i,sys=satsys(sat,NULL);

    if (sys==SYS_GLO) {
        for (i=0;i<nav->ng;i++) {
            if (nav->geph[i].sat==sat) break;
        }
        return (i>=nav->ng)?0.0:-nav->geph[i].dtaun*CLIGHT;
    }
    else {
        for (i=0;i<nav->n;i++) {
            if (nav->eph[i].sat==sat) break;
        }
        return (i>=nav->n)?0.0:nav->eph[i].tgd[type]*CLIGHT;
    }
}
/* psendorange with code bias correction -------------------------------------*/
static double prange(const obsd_t *obs, const nav_t *nav, const double *azel,
                     int iter, const prcopt_t *opt, double *var, cezeri_log* cezeriLog)
{
    double lam[NFREQ] = {0};
    lam[0]=obs->lam[0];
    lam[1]=obs->lam[1];

    if (lam[0] == lam[1])
    {
        lam[1] = 0.0;
    }
    double PC,P1,P2,P1_P2,P1_C1,P2_C2,gamma;
    int i,j,sys;

    if(opt->sigtype[obs->sat-1] == 0)
    {
        i=0;j=1;
    }
    else if(opt->sigtype[obs->sat-1] == 1)
    {
        i=1;j=0;
    }

    *var=0.0;

    if (!(sys=satsys(obs->sat,NULL))) return 0.0;

    /* L1-L2 for GPS/GLO/QZS, L1-L5 for GAL/SBS */
//    if (NFREQ>=3&&(sys&(SYS_GAL|SYS_SBS))) j=2;
    if (NFREQ>=3&&(sys == SYS_SBS)) j=2;

    if (NFREQ<2||lam[i]==0.0||lam[j]==0.0) return 0.0;

    /* test snr mask */
    if (iter>0) {
        if (testsnr(0,i,azel[1],obs->SNR[i]*0.25,&opt->snrmask)) {
            trace(4,"snr mask: %s sat=%2d el=%.1f snr=%.1f\n",
                  time_str(obs->time,0),obs->sat,azel[1]*R2D,obs->SNR[i]*0.25);
            return 0.0;
        }
        if (opt->ionoopt==IONOOPT_IFLC) {
            if (testsnr(0,j,azel[1],obs->SNR[j]*0.25,&opt->snrmask)) return 0.0;
        }
    }
    cezeriLog->cn0[cezeriLog->ns] = obs->SNR[i] * 0.25;
    gamma=SQR(lam[j])/SQR(lam[i]); /* f1^2/f2^2 */
    P1=obs->P[i];
    P2=obs->P[j];
    P1_P2=nav->cbias[obs->sat-1][0];
    P1_C1=nav->cbias[obs->sat-1][1];
    P2_C2=nav->cbias[obs->sat-1][2];
    
    /* if no P1-P2 DCB, use TGD instead */
    if (P1_P2==0.0&&(sys&(SYS_GPS|SYS_GAL|SYS_QZS))) {
        P1_P2=(1.0-gamma)*gettgd(obs->sat,nav);
    }
    if (opt->ionoopt==IONOOPT_IFLC) { /* dual-frequency */
        
        if (P1==0.0||P2==0.0) return 0.0;
        if (obs->code[i]==CODE_L1C) P1+=P1_C1; /* C1->P1 */
        if (obs->code[j]==CODE_L2C) P2+=P2_C2; /* C2->P2 */
        
        /* iono-free combination */
        PC=(gamma*P1-P2)/(gamma-1.0);
    }
    else { /* single-frequency */

        if (P1==0.0) return 0.0;
        if (obs->code[i]==CODE_L1C) P1+=P1_C1; /* C1->P1 */
        PC=P1-P1_P2/(1.0-gamma);
    }
    if (opt->sateph==EPHOPT_SBAS) PC-=P1_C1; /* sbas clock based C1 */
    
    *var=SQR(ERR_CBIAS);
    
    return PC;
}

/* psendorange with code bias correction -------------------------------------*/
extern double prange_ekf(const obsd_t *obs, const nav_t *nav, const prcopt_t *opt, double *var, int kestirim_band)
{
    double lam[NFREQ] = {0};
    lam[0]=obs->lam[0];
    lam[1]=obs->lam[1];
    if (lam[0] == lam[1])
    {
        lam[1] = 0.0;
    }
    double PC,P1,P2,P1_P2,P1_C1,P2_C2,gamma;
    int i,j,sys;

    if(kestirim_band == 0)
    {
        i=0;j=1;
    }
    else if(kestirim_band == 1)
    {
        i=1;j=0;
    }

    *var=0.0;

    if (!(sys=satsys(obs->sat,NULL))) return 0.0;

    /* L1-L2 for GPS/GLO/QZS, L1-L5 for GAL/SBS */
    if (NFREQ>=3&&(sys&(SYS_GAL|SYS_SBS))) j=2;

    if (NFREQ<2||lam[i]==0.0||lam[j]==0.0) return 0.0;

    gamma=SQR(lam[j])/SQR(lam[i]); /* f1^2/f2^2 */
    P1=obs->P[i];
    P2=obs->P[j];
    P1_P2=nav->cbias[obs->sat-1][0];
    P1_C1=nav->cbias[obs->sat-1][1];
    P2_C2=nav->cbias[obs->sat-1][2];

    /* if no P1-P2 DCB, use TGD instead */
    if (P1_P2==0.0&&(sys&(SYS_GPS|SYS_GAL|SYS_QZS))) {
        P1_P2=(1.0-gamma)*gettgd(obs->sat,nav);
    }
    if (opt->ionoopt==IONOOPT_IFLC) { /* dual-frequency */

        if (P1==0.0||P2==0.0) return 0.0;
        if (obs->code[i]==CODE_L1C) P1+=P1_C1; /* C1->P1 */
        if (obs->code[j]==CODE_L2C) P2+=P2_C2; /* C2->P2 */

        /* iono-free combination */
        PC=(gamma*P1-P2)/(gamma-1.0);
    }
    else { /* single-frequency */

        if (P1==0.0) return 0.0;
        if (obs->code[i]==CODE_L1C) P1+=P1_C1; /* C1->P1 */
        PC=P1-P1_P2/(1.0-gamma);
    }
    if (opt->sateph==EPHOPT_SBAS) PC-=P1_C1; /* sbas clock based C1 */

    *var=SQR(ERR_CBIAS);

    return PC;
}

/* ionospheric correction ------------------------------------------------------
* compute ionospheric correction
* args   : gtime_t time     I   time
*          nav_t  *nav      I   navigation data
*          int    sat       I   satellite number
*          double *pos      I   receiver position {lat,lon,h} (rad|m)
*          double *azel     I   azimuth/elevation angle {az,el} (rad)
*          int    ionoopt   I   ionospheric correction option (IONOOPT_???)
*          double *ion      O   ionospheric delay (L1) (m)
*          double *var      O   ionospheric delay (L1) variance (m^2)
* return : status(1:ok,0:error)
*-----------------------------------------------------------------------------*/
extern int ionocorr(gtime_t time, const nav_t *nav, int sat, const double *pos,
                    const double *azel, int ionoopt, double *ion, double *var)
{
    trace(4,"ionocorr: time=%s opt=%d sat=%2d pos=%.3f %.3f azel=%.3f %.3f\n",
          time_str(time,3),ionoopt,sat,pos[0]*R2D,pos[1]*R2D,azel[0]*R2D,
          azel[1]*R2D);
    
    /* broadcast model */
    if (ionoopt==IONOOPT_BRDC) {
        *ion=ionmodel(time,nav->ion_gps,pos,azel);
        *var=SQR(*ion*ERR_BRDCI);
        return 1;
    }
    /* sbas ionosphere model */
    if (ionoopt==IONOOPT_SBAS) {
        return sbsioncorr(time,nav,pos,azel,ion,var);
    }
    *ion=0.0;
    *var=ionoopt==IONOOPT_OFF?SQR(ERR_ION):0.0;
    return 1;
}
/* tropospheric correction -----------------------------------------------------
* compute tropospheric correction
* args   : gtime_t time     I   time
*          nav_t  *nav      I   navigation data
*          double *pos      I   receiver position {lat,lon,h} (rad|m)
*          double *azel     I   azimuth/elevation angle {az,el} (rad)
*          int    tropopt   I   tropospheric correction option (TROPOPT_???)
*          double *trp      O   tropospheric delay (m)
*          double *var      O   tropospheric delay variance (m^2)
* return : status(1:ok,0:error)
*-----------------------------------------------------------------------------*/
extern int tropcorr(gtime_t time, const nav_t *nav, const double *pos,
                    const double *azel, int tropopt, double *trp, double *var)
{

    /* saastamoinen model */
    if (tropopt==TROPOPT_SAAS||tropopt==TROPOPT_EST||tropopt==TROPOPT_ESTG) {
        *trp=tropmodel(time,pos,azel,REL_HUMI);
        *var=SQR(ERR_SAAS/(sin(azel[1])+0.1));
        return 1;
    }
    /* sbas troposphere model */
    if (tropopt==TROPOPT_SBAS) {
        *trp=sbstropcorr(time,pos,azel,var);
        return 1;
    }
    /* no correction */
    *trp=0.0;
    *var=tropopt==TROPOPT_OFF?SQR(ERR_TROP):0.0;
    return 1;
}
/* pseudorange residuals -----------------------------------------------------*/
static int rescode(int iter, const obsd_t *obs, int n, const double *rs,
                   const double *dts, const double *vare, const int *svh,
                   const nav_t *nav, const double *x, const prcopt_t *opt,
                   double *v, double *H, double *var, double *azel, int *vsat,
                   double *resp, int *ns, cezeri_log* cezeriLog)
{
    double r,dion,dtrp,vmeas,vion,vtrp,rr[3],pos[3],dtr,e[3],P, lam_L1, dion_l1, dion_l2, vion_l1, vion_l2;
    int i,j,nv=0,sys,mask[4]={0};
    
    trace(3,"resprng : n=%d\n",n);
    
    for (i=0;i<3;i++)
    {
    	rr[i]=x[i];
    	dtr=x[3];
    }
    
    ecef2pos(rr,pos);
    
    for (i=*ns=0;i<n&&i<MAXOBS;i++) {
        vsat[i]=0; azel[i*2]=azel[1+i*2]=resp[i]=0.0;
        
        if (!(sys=satsys(obs[i].sat,NULL))) continue;

        /* reject duplicated observation data */
        if (i<n-1&&i<MAXOBS-1&&obs[i].sat==obs[i+1].sat) {
            trace(2,"duplicated observation data %s sat=%2d\n",
                  time_str(obs[i].time,3),obs[i].sat);
            i++;
            continue;
        }
        /* geometric distance/azimuth/elevation angle */
        if ((r=geodist(rs+i*6,rr,e)) <= 0.0 ||
            ((satazel(pos,e,azel+i*2)<opt->elmin) && (rr[2] > 0)))
            continue;
        
        /* psudorange with code bias correction */
        if ((P=prange(obs+i,nav,azel+i*2,iter,opt,&vmeas,cezeriLog))==0.0)
            continue;
        
        /* excluded satellite? */
        if (satexclude(obs[i].sat,svh[i],opt)) continue;
        
        /* ionospheric corrections */
        if (!ionocorr(obs[i].time,nav,obs[i].sat,pos,azel+i*2,
                      iter>0?opt->ionoopt:IONOOPT_BRDC,&dion,&vion)) continue;

        /** GPS-L1 -> L1/B1 */
        if ((lam_L1 = obs[i].lam[0]) > 0.0)
        {
            dion_l1 = dion * pow(FREQ1 / (CLIGHT / lam_L1), 2);
            vion_l1 = vion * pow(FREQ1 / (CLIGHT / lam_L1), 2);
            dion_l2 = dion * pow(FREQ1 / (CLIGHT / obs[i].lam[1]), 2);
            vion_l2 = vion * pow(FREQ1 / (CLIGHT / obs[i].lam[1]), 2);
        }

        /* GPS-L1 -> L1/B1 */
//        if ((lam_L1=obs[i].lam[0])>0.0) {
//            dion*=SQR(lam_L1/lam_carr[0]);
//        }

        if(opt->sigtype[obs->sat-1] == 0)
        {
            dion = dion_l1;
            vion = vion_l1;
        }
        else if(opt->sigtype[obs->sat-1] == 1)
        {
            dion = dion_l2;
            vion = vion_l2;
        }

        /* tropospheric corrections */
        if (!tropcorr(obs[i].time,nav,pos,azel+i*2,
                      iter>0?opt->tropopt:TROPOPT_SAAS,&dtrp,&vtrp)) {
            continue;
        }
        /* pseudorange residual */
        v[nv]=P-(r+dtr-CLIGHT*dts[i*2]+dion+dtrp);
        
        /* design matrix */
        for (j=0;j<NX;j++) H[j+nv*NX]=j<3?-e[j]:(j==3?1.0:0.0);
        
        /* time system and receiver bias offset correction */
        switch (sys)
        {
            case SYS_GPS:
            {
                mask[0]=1;
//                cezeriLog->sat_sys_sol_state_t.gps = 1;
                break;
            }
            case SYS_SBS:
            {
                mask[0]=1;
//                cezeriLog->sat_sys_sol_state_t.sbas = 1;
                break;
            }
            case SYS_GLO:
            {
                v[nv]-=x[4];
                H[4+nv*NX]=1.0;
                mask[1]=1;
//                cezeriLog->sat_sys_sol_state_t.glo = 1;
                break;
            }
            case SYS_GAL:
            {
                v[nv]-=x[5];
                H[5+nv*NX]=1.0;
                mask[2]=1;
//                cezeriLog->sat_sys_sol_state_t.gal = 1;
                break;
            }
            case SYS_QZS:
            {
                mask[0]=1;
//                cezeriLog->sat_sys_sol_state_t.qzs = 1;
                break;
            }
            case SYS_CMP:
            {
                v[nv]-=x[6];
                H[6+nv*NX]=1.0;
                mask[3]=1;
//                cezeriLog->sat_sys_sol_state_t.bei = 1;
                break;
            }
            case SYS_IRN:
            {
                mask[0]=1;
//                cezeriLog->sat_sys_sol_state_t.irn = 1;
                break;
            }
            case SYS_LEO:
            {
                mask[0]=1;
//                cezeriLog->sat_sys_sol_state_t.leo = 1;
                break;
            }
            default:
            {
                mask[0]=1;
                break;
            }
        }

        
        vsat[i]=1; resp[i]=v[nv]; (*ns)++;

        cezeriLog->azel[cezeriLog->ns][0] = azel[i*2];
        cezeriLog->azel[cezeriLog->ns][1] = azel[i*2+1];

        cezeriLog->satellites_prres[cezeriLog->ns] = v[nv];
        cezeriLog->satellites_type[cezeriLog->ns]  = sys;
        cezeriLog->satellites[cezeriLog->ns]       = obs[i].sat;
        if(opt->sigtype[obs[i].sat-1] == 0)
        {
            cezeriLog->cn0[cezeriLog->ns]              = obs[i].SNR[0] * 0.25;
            cezeriLog->doppler_freq[cezeriLog->ns]     = obs[i].D[0];
        }
        else
        {
            cezeriLog->cn0[cezeriLog->ns]              = obs[i].SNR[1] * 0.25;
            cezeriLog->doppler_freq[cezeriLog->ns]     = obs[i].D[1];
        }
        cezeriLog->ns++;

        /* error variance */
        var[nv++]=varerr(opt,azel[1+i*2],sys)+vare[i]+vmeas+vion+vtrp;

    }
    /* constraint to avoid rank-deficient */
    for (i=0;i<4;i++) {
        if (mask[i]) continue;
        v[nv]=0.0;
        for (j=0;j<NX;j++) H[j+nv*NX]=j==i+3?1.0:0.0;
        var[nv++]=0.01;
    }
    return nv;
}
/* validate solution ---------------------------------------------------------*/
static int valsol(const double *azel, const int *vsat, int n,
                  const prcopt_t *opt, const double *v, int nv, int nx,
                  char *msg, cezeri_log* cezeriLog)
{
    double azels[MAXOBS*2],dop[4],vv;
    int i,ns;
    
    trace(3,"valsol  : n=%d nv=%d\n",n,nv);

    /* chi-square validation of residuals */
    vv=dot(v,v,nv);
    if (nv>nx && vv > chisqr[nv-nx-1] ) {
        sprintf(msg,"chi-square error nv=%d vv=%.1f cs=%.1f",nv,vv,chisqr[nv-nx-1]);
        cezeriLog->hata_kodu = HATA_CHI_SQUARE;
        return 0;
    }
    /* large gdop check */
    for (i=ns=0;i<n;i++) {
        if (!vsat[i]) continue;
        azels[  ns*2]=azel[  i*2];
        azels[1+ns*2]=azel[1+i*2];
        ns++;
    }
    dops(ns,azels,opt->elmin,dop);
    if (dop[0]<=0.0||dop[0]>opt->maxgdop) {
        sprintf(msg,"gdop error nv=%d gdop=%.1f",nv,dop[0]);
        cezeriLog->hata_kodu = HATA_GDOP;
        return 0;
    }
    return 1;
}
/* estimate receiver position ------------------------------------------------*/
static int estpos(const obsd_t *obs, int n, const double *rs, const double *dts,
                  const double *vare, const int *svh, const nav_t *nav,
                  const prcopt_t *opt, sol_t *sol, double *azel, int *vsat,
                  double *resp, char *msg,cezeri_log* cezeriLog)
{
    double x[NX]={0},dx[NX],Q[NX*NX],*v,*H,*var,sig;
    int i,j,k,info,stat,nv,ns;
    
    trace(3,"estpos  : n=%d\n",n);
    
    v=mat(n+4,1); H=mat(NX,n+4); var=mat(n+4,1);
    
    for (i=0;i<3;i++) x[i]=sol->rr[i];
    
    for (i=0;i<MAXITR;i++) {
        cezeriloginit(cezeriLog);
        /* pseudorange residuals */
        nv = rescode(i,obs,n,rs,dts,vare,svh,nav,x,opt,v,H,var,azel,vsat,resp,
                   &ns,cezeriLog);

        if (nv<NX) {
            sprintf(msg,"lack of valid sats ns=%d",nv);
            cezeriLog->hata_kodu = HATA_YETERLI_UYDU_YOK;
            break;
        }
        /* weight by variance */
        for (j=0;j<nv;j++) {
            sig=sqrt(var[j]);
            v[j]/=sig;
            for (k=0;k<NX;k++) H[k+j*NX]/=sig;
        }
        /* least square estimation */
        if ((info=lsq(H,v,NX,nv,dx,Q))) {
            sprintf(msg,"lsq error info=%d",info);
            cezeriLog->hata_kodu = HATA_LSQ;
            break;
        }
        for (j=0;j<NX;j++) x[j]+=dx[j];
        
        if (norm(dx,NX)<1E-4) {
            sol->type=0;
            sol->time=timeadd(obs[0].time,-x[3]/CLIGHT);
            sol->dtr[0]=x[3]/CLIGHT; /* receiver clock bias (s) */
            sol->dtr[1]=x[4]/CLIGHT; /* glo-gps time offset (s) */
            sol->dtr[2]=x[5]/CLIGHT; /* gal-gps time offset (s) */
            sol->dtr[3]=x[6]/CLIGHT; /* bds-gps time offset (s) */
            for (j=0;j<6;j++) sol->rr[j]=j<3?x[j]:0.0;
            for (j=0;j<3;j++) sol->qr[j]=(float)Q[j+j*NX];
            sol->qr[3]=(float)Q[1];    /* cov xy */
            sol->qr[4]=(float)Q[2+NX]; /* cov yz */
            sol->qr[5]=(float)Q[2];    /* cov zx */
            sol->ns=(unsigned char)ns;
            sol->age=sol->ratio=0.f;
            for (j=0;j<6;j++) cezeriLog->ecef[j]=sol->rr[j];
            ecef2pos(cezeriLog->ecef,cezeriLog->rr);
            cezeriLog->time   = sol->time;

            cezeriLog->clk_bias[0] = x[3];        //!< GPS
            cezeriLog->clk_bias[1] = x[3] + x[4]; //!< GLO
            cezeriLog->clk_bias[2] = x[3] + x[6]; //!< BEI
            cezeriLog->clk_bias[3] = x[3] + x[5]; //!< GAL


            if (cezeriLog->sat_sys_sol_state_t.gps && cezeriLog->sat_sys_sol_state_t.glo && cezeriLog->sat_sys_sol_state_t.bei && cezeriLog->sat_sys_sol_state_t.gal)
                cezeriLog->sat_sys_sol_state_t.all = 1;
            else
                cezeriLog->sat_sys_sol_state_t.all = 0;

            cezeriLog->sat_sys_sol_state_t.pos_ok = 1;

            for (j=0;j<2;j++) cezeriLog->rr[j]*=R2D;
            /* validate solution */
            if ((stat=valsol(azel,vsat,n,opt,v,nv,NX,msg, cezeriLog))) {
                sol->stat=opt->sateph==EPHOPT_SBAS?SOLQ_SBAS:SOLQ_SINGLE;
            }
            free(v); free(H); free(var);
            
            return stat;
        }
    }
    if (i>=MAXITR)
    {
        sprintf(msg,"iteration divergent i=%d",i);
        cezeriLog->hata_kodu = HATA_YAKINSAMA_YOK;
    }
    
    free(v); free(H); free(var);
    
    return 0;
}
/* raim fde (failure detection and exclusion) -------------------------------*/
static int raim_fde(const obsd_t *obs, int n, const double *rs,
                    const double *dts, const double *vare, const int *svh,
                    const nav_t *nav, const prcopt_t *opt, sol_t *sol,
                    double *azel, int *vsat, double *resp, char *msg)
{
    cezeri_log  cezeriLog;
    obsd_t *obs_e;
    sol_t sol_e={{0}};
    char tstr[32],name[16],msg_e[128];

    double *rs_e,*dts_e,*vare_e,*azel_e,*resp_e,rms_e,rms=100.0;
    int i,j,k,nvsat,stat=0,*svh_e,*vsat_e,sat=0;
    
    trace(3,"raim_fde: %s n=%2d\n",time_str(obs[0].time,0),n);
    
    if (!(obs_e=(obsd_t *)malloc(sizeof(obsd_t)*n))) return 0;
    rs_e = mat(6,n); dts_e = mat(2,n); vare_e=mat(1,n); azel_e=zeros(2,n);
    svh_e=imat(1,n); vsat_e=imat(1,n); resp_e=mat(1,n); 
    
    for (i=0;i<n;i++) {
        
        /* satellite exclusion */
        for (j=k=0;j<n;j++) {
            if (j==i) continue;
            obs_e[k]=obs[j];
            matcpy(rs_e +6*k,rs +6*j,6,1);
            matcpy(dts_e+2*k,dts+2*j,2,1);
            vare_e[k]=vare[j];
            svh_e[k++]=svh[j];
        }
        /* estimate receiver position without a satellite */
        if (!estpos(obs_e,n-1,rs_e,dts_e,vare_e,svh_e,nav,opt,&sol_e,azel_e,
                    vsat_e,resp_e,msg_e,&cezeriLog)) {
            trace(3,"raim_fde: exsat=%2d (%s)\n",obs[i].sat,msg);
            continue;
        }
        for (j=nvsat=0,rms_e=0.0;j<n-1;j++) {
            if (!vsat_e[j]) continue;
            rms_e+=SQR(resp_e[j]);
            nvsat++;
        }
        if (nvsat<5) {
            trace(3,"raim_fde: exsat=%2d lack of satellites nvsat=%2d\n",
                  obs[i].sat,nvsat);
            continue;
        }
        rms_e=sqrt(rms_e/nvsat);
        
        trace(3,"raim_fde: exsat=%2d rms=%8.3f\n",obs[i].sat,rms_e);
        
        if (rms_e>rms) continue;
        
        /* save result */
        for (j=k=0;j<n;j++) {
            if (j==i) continue;
            matcpy(azel+2*j,azel_e+2*k,2,1);
            vsat[j]=vsat_e[k];
            resp[j]=resp_e[k++];
        }
        stat=1;
        *sol=sol_e;
        sat=obs[i].sat;
        rms=rms_e;
        vsat[i]=0;
        strcpy(msg,msg_e);
    }
    if (stat) {
        time2str(obs[0].time,tstr,2); satno2id(sat,name);
        trace(2,"%s: %s excluded by raim\n",tstr+11,name);
    }
    free(obs_e);
    free(rs_e ); free(dts_e ); free(vare_e); free(azel_e);
    free(svh_e); free(vsat_e); free(resp_e);
    return stat;
}
/* doppler residuals ---------------------------------------------------------*/
static int resdop(const obsd_t *obs, int n, const double *rs, const double *dts,
                  const nav_t *nav, const double *rr, const double *x,
                  const double *azel, const int *vsat, double *v, double *H, const prcopt_t* opt, cezeri_log* cezeriLog)
{
    double lam,rate,pos[3],E[9],a[3],e[3],vs[3],cosel;
    int i,j,nv=0, mask[4] = {0}, sys;
    trace(3,"resdop  : n=%d\n",n);
    
    ecef2pos(rr,pos); xyz2enu(pos,E);
    
    for (i=0;i<n&&i<MAXOBS;i++) {

        if(opt->sigtype[obs[i].sat-1] == 0)
        {
            lam=obs[i].lam[0];
            if (obs[i].D[0]==0.0||lam==0.0||!vsat[i]||norm(rs+3+i*6,3)<=0.0) {
                continue;
            }
        }
        else if (opt->sigtype[obs[i].sat-1] == 1)
        {
            lam=obs[i].lam[1];
            if(obs[i].D[1]==0.0||lam==0.0||!vsat[i]||norm(rs+3+i*6,3)<=0.0){
                continue;
            }
        }

        if (!(sys=satsys(obs[i].sat,NULL))) continue;

        /* line-of-sight vector in ecef */
        cosel=cos(azel[1+i*2]);
        a[0]=sin(azel[i*2])*cosel;
        a[1]=cos(azel[i*2])*cosel;
        a[2]=sin(azel[1+i*2]);
        matmul("TN",3,1,3,1.0,E,a,0.0,e);
        
        /* satellite velocity relative to receiver in ecef */
        for (j=0;j<3;j++) vs[j]=rs[j+3+i*6]-x[j];
        
        /* range rate with earth rotation correction */
        rate=dot(vs,e,3)+OMGE/CLIGHT*(rs[4+i*6]*rr[0]+rs[1+i*6]*x[0]-
                                      rs[3+i*6]*rr[1]-rs[  i*6]*x[1]);
        
        /* doppler residual */
        if(opt->sigtype[obs[i].sat-1] == 0)
        {
            v[nv]=-lam*obs[i].D[0]-(rate+x[3]-CLIGHT*dts[1+i*2]);
        }
        else if(opt->sigtype[obs[i].sat-1] == 1)
        {
            v[nv]=-lam*obs[i].D[1]-(rate+x[3]-CLIGHT*dts[1+i*2]);
        }

        /* design matrix */
        for (j=0;j<NX;j++) H[j+nv*NX]=j<3?-e[j]:(j==3?1.0:0.0);

        /* time system and receiver drift offset correction */
        /* time system and receiver bias offset correction */
        switch (sys)
        {
            case SYS_GPS:
            {
                mask[0]=1;
                cezeriLog->sat_sys_sol_state_t.gps = 1;
                break;
            }
            case SYS_SBS:
            {
                mask[0]=1;
                cezeriLog->sat_sys_sol_state_t.sbas = 1;
                break;
            }
            case SYS_GLO:
            {
                v[nv]-=x[4];
                H[4+nv*NX]=1.0;
                mask[1]=1;
                cezeriLog->sat_sys_sol_state_t.glo = 1;
                break;
            }
            case SYS_GAL:
            {
                v[nv]-=x[5];
                H[5+nv*NX]=1.0;
                mask[2]=1;
                cezeriLog->sat_sys_sol_state_t.gal = 1;
                break;
            }
            case SYS_QZS:
            {
                mask[0]=1;
                cezeriLog->sat_sys_sol_state_t.qzs = 1;
                break;
            }
            case SYS_CMP:
            {
                v[nv]-=x[6];
                H[6+nv*NX]=1.0;
                mask[3]=1;
                cezeriLog->sat_sys_sol_state_t.bei = 1;
                break;
            }
            case SYS_IRN:
            {
                mask[0]=1;
                cezeriLog->sat_sys_sol_state_t.irn = 1;
                break;
            }
            case SYS_LEO:
            {
                mask[0]=1;
                cezeriLog->sat_sys_sol_state_t.leo = 1;
                break;
            }
            default:
            {
                mask[0]=1;
                break;
            }
        }

        nv++;
    }

    /* constraint to avoid rank-deficient */
    for (i=0;i<4;i++) {
        if (mask[i]) continue;
        v[nv]=0.0;
        for (j=0;j<NX;j++) H[j+nv*NX]=j==i+3?1.0:0.0;
        nv++;
    }
    return nv;
}
/* estimate receiver velocity ------------------------------------------------*/
static void estvel(const obsd_t *obs, int n, const double *rs, const double *dts,
                   const nav_t *nav, const prcopt_t *opt, sol_t *sol,
                   const double *azel, const int *vsat, cezeri_log* cezeri_log_ptr)
{
    double x[NX]={0},dx[NX],Q[NX*NX],*v,*H;
    int i,j,nv;
    
    trace(3,"estvel  : n=%d\n",n);
    
    v=mat(n+4,1); H=mat(NX,n+4);
    
    for (i=0;i<MAXITR;i++) {
        
        /* doppler residuals */
        if ((nv=resdop(obs,n,rs,dts,nav,sol->rr,x,azel,vsat,v,H,opt, cezeri_log_ptr))  < NX) {
            break;
        }
        /* least square estimation */
        if (lsq(H,v,NX,nv,dx,Q)) break;
        
        for (j=0;j<NX;j++) x[j] += dx[j];
        
        if (norm(dx,NX)<1E-6)
        {
            for (i=0;i<3;i++)
            {
                sol->rr[i+3]=x[i];
                cezeri_log_ptr->rr[i+3] = x[i];
            }
            cezeri_log_ptr->sat_sys_sol_state_t.vel_ok = 1;

            cezeri_log_ptr->clk_drift[0] = x[3]; //!< GPS
            cezeri_log_ptr->clk_drift[1] = x[3] + x[4]; //!< GLO
            cezeri_log_ptr->clk_drift[2] = x[3] + x[6]; //!< BEI
            cezeri_log_ptr->clk_drift[3] = x[3] + x[5]; //!< GAL
            break;
        }
    }
    free(v); free(H);
}
/* single-point positioning ----------------------------------------------------
* compute receiver position, velocity, clock bias by single-point positioning
* with pseudorange and doppler observables
* args   : obsd_t *obs      I   observation data
*          int    n         I   number of observation data
*          nav_t  *nav      I   navigation data
*          prcopt_t *opt    I   processing options
*          sol_t  *sol      IO  solution
*          double *azel     IO  azimuth/elevation angle (rad) (NULL: no output)
*          ssat_t *ssat     IO  satellite status              (NULL: no output)
*          char   *msg      O   error message for error exit
* return : status(1:ok,0:error)
* notes  : assuming sbas-gps, galileo-gps, qzss-gps, compass-gps time offset and
*          receiver bias are negligible (only involving glonass-gps time offset
*          and receiver bias)
*-----------------------------------------------------------------------------*/
extern int pntpos(const obsd_t *obs, int n, const nav_t *nav,
                  const prcopt_t *opt, sol_t *sol, double *azel, ssat_t *ssat,
                  char *msg, cezeri_log* cezeriLog)
{
    double *rs, * dts, * var, * azel_, * resp;
    int i, stat, vsat[MAXOBS] = { 0 }, svh[MAXOBS];
    sol->stat = SOLQ_NONE;

    if (n <= 0)
    {
        strcpy(msg, "no observation data");
        cezeriLog->hata_kodu = HATA_OBS_YOK;
        return 0;
    }

    trace(3, "pntpos  : tobs=%s n=%d\n", time_str(obs[0].time, 3), n);
    sol->time = obs[0].time; msg[0] = '\0';
    rs = mat(6, n); dts = mat(2, n); var = mat(1, n); azel_ = zeros(2, n); resp = mat(1, n);

// TODO: parametrize edilebilir (Faruk Emre Kendirli)
//    if (opt_.mode != PMODE_SINGLE) { /* for precise positioning */
//#if 0
//        opt_.sateph = EPHOPT_BRDC;
//#endif
//        opt_.ionoopt = IONOOPT_BRDC;
//        opt_.tropopt = TROPOPT_SAAS;
//    }

//    if (opt->mode != PMODE_SINGLE) { /* for precise positioning */
//#if 0
//        opt->sateph = EPHOPT_BRDC;
//#endif
//        opt->ionoopt = IONOOPT_BRDC;
//        opt->tropopt = TROPOPT_SAAS;
//    }

    /* satellite positons, velocities and clocks */
    satposs(sol->time, obs, n, nav, opt->sateph, rs, dts, var, svh, opt->navtype_t);

      /* estimate receiver position with pseudorange */
    stat = estpos(obs, n, rs, dts, var, svh, nav, opt, sol, azel_, vsat, resp, msg, cezeriLog);
    //stat = estpos(obs, n, rs, dts, var, svh, nav, &opt_, sol, azel_, vsat, resp, msg, cezeriLog);

     /* raim fde */
    if (!stat && n >= 6 && opt->posopt[4]) {
        stat = raim_fde(obs, n, rs, dts, var, svh, nav, opt, sol, azel_, vsat, resp, msg);
        //stat = raim_fde(obs, n, rs, dts, var, svh, nav, &opt_, sol, azel_, vsat, resp, msg);
        if (!stat)
            cezeriLog->hata_kodu = HATA_RAIM;
    }
    /* estimate receiver velocity with doppler */
    if (stat)
       estvel(obs, n, rs, dts, nav, opt, sol, azel_, vsat, cezeriLog);
       //estvel(obs, n, rs, dts, nav, &opt_, sol, azel_, vsat, cezeriLog);

   if (azel) {
       for (i = 0; i < n * 2; i++) azel[i] = azel_[i];
   }
    if (ssat) {
        for (i = 0; i < MAXSAT; i++) {
            ssat[i].vs = 0;
            ssat[i].azel[0] = ssat[i].azel[1] = 0.0;
            ssat[i].resp[0] = ssat[i].resc[0] = 0.0;
            ssat[i].snr[0] = 0;
        }
        for (i = 0; i < n; i++) {
            ssat[obs[i].sat - 1].azel[0] = azel_[i * 2];
            ssat[obs[i].sat - 1].azel[1] = azel_[1 + i * 2];
            ssat[obs[i].sat - 1].snr[0] = obs[i].SNR[0];
            if (!vsat[i]) continue;
            ssat[obs[i].sat - 1].vs = 1;
            ssat[obs[i].sat - 1].resp[0] = resp[i];
        }
    }
    free(rs); free(dts); free(var); free(azel_); free(resp);
    return stat;
}
