#include "utility.h"
#include <math.h>

void utility::mjd_dat(int &d, int &m, int &y, int64_t mjd)
{
    int64_t a,b,e,f,c,st1,st;
    st = 678881LL;
    st1 = 146097LL;
    a = 4LL*(mjd+st)+3LL;
    b = a/st1;
    c = a-b*st1;
    f = 4*(c/4)+3;
    c = f/1461;
    f = (f-c*1461+4)/4;
    y = static_cast<int>(b*100+c);
    e = f*5-3;
    d = static_cast<int>((e-(e/153)*153+5)/5);
    m = static_cast<int>(e/153+3);
    if(m > 12){
      m -= 12;
      ++y;
    }
}

int64_t utility::dat_mjd(int d, int m, int y)
{
    static int k[]={0,31,59,90,120,151,181,212,243,273,304,334};
    if(y%4==0 && m>2) ++d;
    return(static_cast<int64_t>(k[--m]+d+(y-1972)*365+(y-1969)/4)+static_cast<int64_t>(41316.));
}

int utility::day_year(int d, int m, int y)
{
    int k[]={0,31,59,90,120,151,181,212,243,273,304,334};
    if(y%4==0 && m>2) ++d;
    return(k[--m]+d);
}

void utility::sgms(int &hh, int &mm, double &ss, double &secd)
{
    hh=static_cast<int>(secd/3600.);
    mm=static_cast<int>((secd-static_cast<double>(hh)*3600.)/60.);
    ss=secd-static_cast<double>(hh)*3600.-static_cast<double>(mm)*60.;
}

double utility::pod(double x, double y)
{
    double z;
    if(y==0.)return(1.);
    if(x==0.)return(0.);
    if(y==1.)return(x);
    z=exp(fabs(y)*log(fabs(x)));
    if(y<0.)z=1./z;
    if(x<0. && fmod(y,2.)!=0.)z=-z;
    return(z);
}

double utility::pot(double x, int l)
{
    double z;
    if(l==0)return(1.);
    if(x==0.)return(0.);
    if(l==1)return(x);
    z=exp(fabs(static_cast<double>(l))*log(fabs(x)));
     if(l<0)z=1./z;
    if(x<0. && (l%2)!=0)z=-z;
    return(z);
}
