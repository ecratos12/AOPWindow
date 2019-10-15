#ifndef UTILITY_H
#define UTILITY_H

#include <QCoreApplication>

namespace utility {

/* File lines formats */

static const char* FormatKAT_OBS_R = "%c%d%d%ld%ld%ld%d%2s%8s%d%lf%lf%d%lf%lf%lf%d%lf";
static const char* FormatKAT_OBS_W = "e%3d%3d%6ld%6ld%8ld%5d%3s%9s%6d%6.1lf%7.1lf%3d%8.1lf%10.2lf%6.3lf%3d%7.1lf";

static const char* FormatKAT_KBO_R = "%c%ld%ld%d%3s%ld%ld%lf%lf%d%d%lf%d%d%d%8s%lf";
static const char* FormatKAT_KBO_W = "e%7ld%8ld%5d%3s%6ld%6ld%6.1lf%7.1lf%3d%6d%6.3lf%3d%4d%4d% 9s%7.1lf";

static const char* FormatOGA_R = "lf%lf%lf%lf%d";
static const char* FormatCGA_W = " %20.12lf%10.4lf%8.4lf%14.3lf%15.7lf%5d";
static const char* FormatCGA_R = " %lf%lf%lf%lf%lf%d";
static const char* FormatEGA_R = "%c%ld%lf%lf%lf";
static const char* FormatPGA_W = " %20.12lf%12.4lf%12.4lf%12.4lf";
static const char* FormatPGA_R = "%lf%lf%lf%lf";

static const char* FormatKAT_KAL_R = "%c%d%2s%2s%ld%ld%d%lf%lf%lf%lf%lf%d%d%d%d%ld%d%lf%lf%lf%12s";
static const char* FormatKAT_KAL_W = "e %5d %2s %2s%6ld%6ld%4d%9.3lf%6.3lf%9.3lf%6.1lf%7.1lf%3d%5d%4d%3d%7ld%3d%7.3lf%7.3lf%12.4lf % 12s";

static const char* FormatSYS_CONF_R = "%lf%s%s%s%s%s";
static const char* FormatKAT_TAR_R = "%*d%*2s%2s%lf%*lf%*lf%*20s";
static const char* FormatEPA_AUTO_W = "%4d%6ld%4d%4d%8ld %10s";
static const char* FormatEPA_AUTO_R = "%d%ld%d%d%ld";

static const char* FormatCOPY_W = "%20.12lf%9.4lf%8.4lf%14.3lf%15.7lf%5d";

static const char* FormatSATELLIT_R = "%10s%ld%d%ld%d%d%d%d";


/* date & time functions */

void mjd_dat(int &d, int &m, int &y, int64_t mjd);
int64_t dat_mjd(int d, int m, int y);
int day_year(int d, int m, int y);
void sgms(int &hh, int &mm, double &ss, double &secd);


/* math functions */

double pod(double x, double y);
double pot(double x, int l);

}
#endif // UTILITY_H
