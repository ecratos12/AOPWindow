#ifndef UTILITY_H
#define UTILITY_H

#include <QCoreApplication>
#include <QVector>
#include <string>
#include <fstream>
#include <iostream>
#include <string>

namespace utility {

/* constants */

static const double C = 299792458;
static const double PI = 3.1415926535897932384626433;

/* date & time functions */

void mjd_dat(int &d, int &m, int &y, int64_t mjd);
int64_t dat_mjd(int d, int m, int y);
int day_year(int d, int m, int y);
void sgms(int &hh, int &mm, double &ss, double &secd);


/* math functions */

double pod(double x, double y);
double pot(double x, int l);
double inter_parabola(double x0, double y0,
                      double x1, double y1,
                      double x2, double y2, double x);

/* atmosperic refraction */

// used model Mendes and Pavlis, 2004
// see ftp://tai.bipm.org/iers/conv2010/chapter9 for details

double delayZenith(double lat, double hEllips, double pres, double wpres, double l);
double delayMappingElev(double lat, double hSea, double T, double elev);


void filter_omc(QVector<double> &x, QVector<double> &y,
                const QVector<double> &et, const QVector<double> &er,
                int bandLength, double bandWidth, double snr, double sigmaRate);

/* utility classes and structures */

struct plot_data {
    double plo0,plo1;
    int    ind0,ind1,ind2;
};

class FanFilter {
    void minmax();
    void sigmaFilter();
    void gaus();
    void filtr();

    float ck[325][5],cd[325][6],s;
    double cl[8],cobr[8][8],d[8],cob9[8][8],c[8][8];
    float mx,my,tn,tk,dmax,dmin,mx1,my1,tn1,dmin1;
    int npoint,k;

public:
    Filter();
    float t[2700],r[2700],r1[2700],t1[2700];
    void run(const QVector<double> &at, const QVector<double> &ar);
    int mpoint,n,pmin;
};

}
#endif // UTILITY_H
