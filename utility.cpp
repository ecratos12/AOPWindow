#include "utility.h"
#include <cmath>

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

double utility::inter_parabola(double x0, double y0, double x1, double y1, double x2, double y2, double x)
{
    return y0 * (x - x1) * (x - x2) / (x0 - x1) / (x0 - x2)
           + y1 * (x - x0) * (x - x2) / (x1 - x0) / (x1 - x2)
           + y2 * (x - x0) * (x - x1) / (x2 - x0) / (x2 - x1);
}

double utility::delayZenith(double lat, double hEllips, double pres, double wpres, double l_mkm)
{
    double xc = 375,
           k0 = 238.0185, k1 = 19990.975, k2 = 57.362, k3 = 579.55174,
           w0 = 295.235, w1 = 2.6422, w2 = -0.03238, w3 = 0.004028,
           sigma = 1./l_mkm;

    double f = 1 - 0.00266*cos(2*PI/180*lat) - 2.8e-7*hEllips;
    double corr = 1 + 0.534e-6*(xc-450);
    double fh = 0.01*corr*((k1*(k0+sigma*sigma))/((k0-sigma*sigma)*(k0-sigma*sigma))
                           + k3*(k2+sigma*sigma)/((k2-sigma*sigma)*(k2-sigma*sigma)));
    double fnh = 0.003101*(w0 + 3.0*w1*sigma*sigma + 5.0*w2*pot(sigma,4) + 7.0*w3*pot(sigma,6));

    double fcul_zhd = 2.416579e-3*fh*pres/f,
           fcul_zwd = 1.e-4*(5.316*fnh-3.759*fh)*wpres/f;

    return fcul_zhd + fcul_zwd;
}

double utility::delayMappingElev(double lat, double hSea, double T, double elev)
{
    double sine = sin(elev*PI/180.),
           cosphi = cos(lat*PI/180.),
           Tc = T - 273.15;
    double  A10 =  0.121008e-02,
            A11 =  0.17295e-05,
            A12 =  0.3191e-04,
            A13 = -0.18478e-07,
            A20 =  0.304965e-02,
            A21 =  0.2346e-05,
            A22 = -0.1035e-03,
            A23 = -0.1856e-07,
            A30 =  0.68777e-01,
            A31 =  0.1972e-04,
            A32 = -0.3458e-02,
            A33 =  0.1060e-06;
    double  A1 = A10+A11*Tc+A12*cosphi+A13*hSea,
            A2 = A20+A21*Tc+A22*cosphi+A23*hSea,
            A3 = A30+A31*Tc+A32*cosphi+A33*hSea;

    return (1 + A1/(1 + A2/(1+A3))) / (sine+A1/(sine+A2/(sine+A3)));
}


void utility::FanFilter::minmax()
{
    int i;
    dmax=-10000.;
    dmin=10000.;
    for(i=0;i<npoint;i++) {
        if(r[i] < dmin) dmin=r[i];
        if(r[i] > dmax) dmax=r[i];
    }
    my=480/(dmax-dmin);
    tn=t[0];
    tk=t[npoint];
    mx=640/(tk-tn);
}

void utility::FanFilter::sigmaFilter()
{
    int i,j;
    float ss,sr,sig;
    ss=0;
    for(i=0;i<npoint;i++) {
        ss += r[i];
        r1[i]=r[i];
    }
    sr=ss/(float)npoint;
    ss=0;
    for(i=0;i<npoint;i++) {
        r1[i]=r1[i]-sr;
        ss += r1[i]*r1[i];
    }
    sig=sqrt(ss/(float)npoint);
    j=0;
    for(i=0;i<npoint;i++) {
        ss=fabs(r1[i]);
        if(ss < sig) {
            r[j]=r[i];
            t[j]=t[i];
            j++;
        }
    }
    npoint=j-1;
}

void utility::FanFilter::gaus()
{
    float s1,s2;
    int k1,i,j,l,k2,m;
    k1=k+1;
    s = 0.;

    for(i=1;i<k1+1;i++)
        for(j=1;j<k+1;j++)
            c[j][i]=0.;
    s /= 3;

    for(l=1;l<k+1;l++)
        for(i=1;i<n+1;i++) {
            s=cd[i][l];
            for(j=1;j<k1+1;j++) c[l][j] += s*cd[i][j];
        }
    for(i=1;i<k+1;i++)
        for(j=1;j<k+1;j++)
            cobr[j][i]=c[j][i];

    k2=k-1;
    for(j=1;j<k+1;j++) {
        s=1./c[1][j];
        for(i=1;i<k2+1;i++) c[i][j]=-s*c[i+1][j];
        c[k][j]=s;
        for(l=1;l<k+1;l++) {
            if(l!=j) {
                s1=c[1][l];
                for(i=1;i<k2+1;i++) c[i][l]=c[i+1][l]+s1*c[i][j];
                c[k][l]=s*s1;
            }
        }
    }
    for(m=1;m<11;m++) {
        for(l=1;l<k+1;l++) {
            for(i=1;i<k+1;i++) {
                s=0.;
                if(i==l) s=2.;
                s2=0.;
                for(j=1;j<k+1;j++) s2 += cobr[i][j]*c[j][l];
                cob9[i][l]=s-s2;
            }
        }
        for(l=1;l<k+1;l++) {
            for(i=1;i<k+1;i++) {
                s2=0.;
                for(j=1;j<k+1;j++) s2 += c[i][j]*cob9[j][l];
                c[i][l]=s2;
            }
        }
    }
    for(i=1;i<k+1;i++) {
        s=0.;
        for(j=1;j<k+1;j++) s += c[i][j]*c[j][k1];
        d[i]=-s;
    }

    d[k1]=1;
    k2=k+1;
    s1=0.;
    for(i=1;i<n+1;i++) {
        s=0.;
        for(j=1;j<k1+1;j++) s += d[j]*cd[i][j];
        s1 += s*s;
        cd[i][k2]=s;
    }
    s=sqrt(s1/(n-k));
    for(i=1;i<k+1;i++) {
        c[i][k1]=s*sqrt(c[i][i]);
        cl[i]=c[i][k1];
    }
}

void utility::FanFilter::filtr()
{
    int i,j,n1,nmax,nm,imax,jmax,kn=0,ll,kt,pmax;
    float t0,shag,al,y1,y2,x,y,rsn,smain;
    rsn = 0.5;
    kt = 15;
    pmax = 40;
    smain = 3.0;

    pmax=(int)(t[npoint]-t[1])/9;
    if(pmax > 120) pmax=120;
    pmin=(int)((float)kt*rsn);

    t0=t[0];
    mpoint=npoint;
    for (i=0;i<npoint;i++) {
        r1[i]=r[i];
        t1[i]=t[i];
    }

    while(1) {
        j=0;
        t0=t1[kn];
        for(i=kn;i<(kn+kt);i++) {
            r[j]=r1[i];
            t[j]=t1[i]-t0;
            j++;
        }

        npoint=kt;
        minmax();
        shag=(dmax-dmin)/50;
        shag=fabs(shag);
        n1=2*(int)fabs((dmax-dmin)/shag);
        al=shag/t[kt-1]/2;
        nmax=0;
        y2=0;
        for(i=0;i<n1;i++) {
            for(j=0;j<n1;j++) {
                nm=0;
                for(k=0;k<kt;k++) {
                    y1=dmax-shag/2*(float)(i-1)+(float)(i-j)*al*t[k];
                    y2=dmax-shag/2*(float)i+(float)(i-j)*al*t[k];
                    if((y1 >= r[k])&(r[k] > y2)) nm++;
                }
                if(nm > nmax) {
                    nmax=nm;
                    imax=i;
                    jmax=j;
                }
            }
        }
        if(nmax>pmin) break;
        y=(int)((r1[kn]-dmin1)*my1);
        x=(int)((t1[kn]-tn1)*mx1);
//        putpixel(x,y,1);
        kn++;
    }

    i=imax;
    j=jmax;
    ll=0;
    for(k=0;k<kt;k++) {
        y1=dmax-shag/2*(float)(i-1)+(float)(i-j)*al*t[k];
        y2=dmax-shag/2*(float)i+(float)(i-j)*al*t[k];
        if((y1 > r[k])&(r[k] >= y2)) {
            y=(int)((r1[k+kn]-dmin1)*my1);
            x=(int)((t1[k+kn]-tn1)*mx1);
//            putpixel(x,y,11);
            r[ll]=r[k+kn];
            t[ll]=t[k+kn];
            r1[ll]=r1[k+kn];
            t1[ll]=t1[k+kn];
            ll++;
        }
        if(ll == pmin) break;
    }

    for(i=k;i<mpoint;i++) {
        r1[ll]=r1[i+kn];
        t1[ll]=t1[i+kn];
        ll++;
    }
    mpoint=ll--;
    ll=0;

    n=pmin;
    k=2;
    for(i=1;i<n+1;i++) {
        cd[i][1]=1;
        cd[i][2]=(t1[i]-t0)/60;
        cd[i][3]=-r1[i];
        ck[i][2]=cd[i][2];
        ck[i][3]=cd[i][3];
    }
    gaus();

    i=n;
    while(!(i==mpoint)) {
        for(j=1;j<n;j++) {
            cd[j][1]=1;
            cd[j][2]=ck[j+1][2];
            cd[j][3]=ck[j+1][3];
        }
        for(j=1;j<n;j++) {
            ck[j][2]=cd[j][2];
            ck[j][3]=cd[j][3];
        }
        m1:
        for(j=1;j<n;j++) {
            cd[j][1]=1;
            cd[j][2]=ck[j][2];
            cd[j][3]=ck[j][3];
        }

        i++;
        if(i==mpoint)break;
        cd[n][1]=1;
        cd[n][2]=(t1[i]-t0)/60;
        cd[n][3]=-r1[i];
        ck[n][3]=cd[n][3];
        ck[n][2]=cd[n][2];

        gaus();

        if((s < smain)&(fabs(cd[n][4]) < s*3)) {
            y=(int)((r1[i]-dmin1)*my1);
            x=(int)((t1[i]-tn1)*mx1);
//            putpixel(x,y,12);

            r[ll]=r[i];
            t[ll]=t[i];
            n++;
            ll++;
            if(n < pmax) goto m1;
            n--;
        } else {
            y=(int)((r1[i]-dmin1)*my1);
            x=(int)((t1[i]-tn1)*mx1);
//            putpixel(x,y,2);
            goto m1;
        }
    }
}

void utility::FanFilter::run(const QVector<double> &at, const QVector<double> &ar)
{
    int i;

    for (i=0; i<at.size(); ++i) {
        t[i] = at[i];
        r[i] = ar[i];
    }
    npoint = i-1;
    minmax();

    sigmaFilter();
    minmax();
    sigmaFilter();
    minmax();
    mx1=mx;
    my1=my;
    tn1=tn;
    dmin1=dmin;
    filtr();
}

void utility::filter_omc(QVector<double> &x, QVector<double> &y,
                         const QVector<double> &et, const QVector<double> &er,
                         int bandLength, double bandWidth, double snr, double sigmaRate)
{
    QVector<double> acc(et.size()-2);
    for (int i=0; i<acc.size(); ++i) // calculate velocity in efemeris
        acc[i] = utility::inter_parabola(et[i], er[i], et[i+1], er[i+1], et[i+2], er[i+2], et[i]+1e-1) - er[i];
    for (int i=0; i<acc.size()-2; ++i) // calculate acceleration in efemeris
        acc[i] = utility::inter_parabola(et[i], acc[i], et[i+1], acc[i+1], et[i+2], acc[i+2], et[i]+1e-1) - acc[i];
    acc.resize(acc.size()-2);

    double maxAcc = 0; // calculate max acceleration
    for (int i=0; i<acc.size(); ++i) {
        double facc = fabs(acc[i]);
        if (facc > maxAcc) maxAcc = facc;
    }

    QVector<double> x1,y1;

    // filter sigma criteria
    double gates = 0., aver = 0.;
    for (int i=0; i<y.size(); ++i) aver += y[i];
    aver /= y.size();
    for (int i=0; i<y.size(); ++i) gates += (y[i]-aver)*(y[i]-aver);
    gates = sigmaRate*sqrt(gates/y.size());
    for (int i=0; i<y.size(); ++i)
        if (fabs(y[i]-aver) < gates) {
            x1.push_back(x[i]);
            y1.push_back(y[i]);
        }
    x = x1;
    y = y1;

    double minR = *std::min_element(y.begin(), y.end());
    double maxR = *std::max_element(y.begin(), y.end());

    // variables for algo
    int i,segmentFinishPos,inPoints;
    double k,b,yBot,yTop;
    bool moveBandFlag;

//    struct LineSection {
//        double k,b,xstart,xfinish;
//    };
//    QVector<LineSection> lines;
    x1.clear();
    y1.clear();

    for (int pos=bandLength; pos<=x.size(); ++pos) { // move band end position
        for (k = -maxAcc; k < maxAcc+1e-10; k += maxAcc/50) {
            for (b = minR; b < maxR+1e-10; b += bandWidth) { // create band based on r=kt+b
                inPoints = 0;
                moveBandFlag = false;
                for (i=bandLength; i>0; --i) {
                    yBot = k*(x[pos-i]-x[pos-bandLength]) + b - bandWidth/2;
                    yTop = yBot + bandWidth;
                    if (y[pos-i] > yBot && y[pos-i] < yTop)
                        ++inPoints;
                }

                if (inPoints > snr*bandLength) {
//                    lines.push_back(LineSection{k, b-x[pos-bandLength], x[pos-bandLength], x[pos]});
                    segmentFinishPos = 0;
                    for (i=bandLength; i>0; --i) {
                        yBot = k*(x[pos-i]-x[pos-bandLength]) + b - bandWidth/2;
                        yTop = yBot + bandWidth;
                        if (y[pos-i] > yBot && y[pos-i] < yTop) {
                            x1.push_back(x[pos-i]);
                            y1.push_back(y[pos-i]);
                            segmentFinishPos = pos-i;
                        }
                    }
                    pos = segmentFinishPos+bandLength+1;
                    moveBandFlag = true;
                    break;
                }
            }
            if (moveBandFlag) break;
        }
        std::cout << pos << std::endl;
    }

    x = x1;
    y = y1;
}
