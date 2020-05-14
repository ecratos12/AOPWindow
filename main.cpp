#include <QCoreApplication>
#include "windows.h"
#include <conio.h>
#include <utilapiset.h>
#include <cmath>
#include <cstdio>
#include <ctime>

#include <utility.h>
#include <qcustomplot/qcustomplot.h>

#include <aopconsolewindow.h>
#include <plot.h>
#include <datamodels.h>

int copy_obs(int nr)
{
    int	ampl, n, jj, i, ws, ws1, nsta, POLY, npoint, humid, j9;
    int64_t nsat, secday, mjd, secobs;
    double	sec1, sec, azym, elev, range, TB, RB, RMS, clock_cor, temp, pres;
    char	namefe[9], nam[33], name[33], kodst[3], c;
    double  c_velosity = 299792458.0;

    std::ifstream obs_catalogue("D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    if (!obs_catalogue.is_open()) {
        std::cout << "Unable to open observations catalogue!" << std::endl;
        return 5;
    }
    std::ofstream obs_catalogue_copy("D:\\LASER-2\\DATA\\KAT_OBS.ROB");
    if (!obs_catalogue_copy.is_open()) {
        std::cout << "Unable to write observations catalogue copy!" << std::endl;
        return 5;
    }

    n = 0;
    i = 0;
    j9 = 0;
    std::string temp_line;
    while (std::getline(obs_catalogue, temp_line)) {
        obs_catalogue_copy << temp_line << std::endl;
        n++;
        if (n == nr) {
            if (sscanf(temp_line.c_str(), datamodels::FormatKAT_OBS_R,
                        &c, &ws, &ws1, &mjd, &secday, &nsat, &nsta, kodst, namefe,
                        &npoint, &temp, &pres, &humid, &TB, &RB, &RMS, &POLY, &clock_cor)!=18) {
                std::cout << std::endl << "ERROR in reading OBS catalogue in copy_obs, line # " << n+1 << std::endl;
                remove("D:\\LASER-2\\DATA\\KAT_OBS.ROB");
                return 5;
            }

            sprintf(name, "D:\\LASER-2\\DATA\\%8s.c%2s", namefe, kodst);
            sprintf(nam, "D:\\LASER-2\\DATA\\%8s.o%2s", namefe, kodst);
            std::ifstream obs(nam);
            if (!obs.is_open()) {
                std::cout << "Unable to open observation FILE : " << nam << std::endl;
                obs_catalogue_copy.close();
                remove("D:\\LASER-2\\DATA\\KAT_OBS.ROB");
                return 5;
            }
            std::fstream obs_copy(name, std::ios_base::in);
            if (obs_copy.is_open()) {
                std::cout << "Copy of this observation is present : " << nam << std::endl;
                obs_catalogue_copy.close();
                remove("D:\\LASER-2\\DATA\\KAT_OBS.ROB");
                return 5;
            }
            obs_copy.open(name, std::ios_base::out);
            if (!obs_copy.is_open()) {
                obs_catalogue_copy.close();
                std::cout << "Unable to write copy of this observation!" << std::endl;
                remove("D:\\LASER-2\\DATA\\KAT_OBS.ROB");
                return 5;
            }
            std::getline(obs, temp_line);
            if ((sscanf(temp_line.c_str(), datamodels::FormatOGA_R, &sec, &azym, &elev, &range, &ampl))!=5)	{
                obs_catalogue_copy.close();
                obs_copy.close();
                std::cout << "Error reading observation file " << nam << std::endl;
                remove("D:\\LASER-2\\DATA\\KAT_OBS.ROB");
                remove(name);
                return 5;
            }
            char buf[128];
            sprintf(buf, datamodels::FormatCGA_W, sec, azym, elev, range, range*0.5e-12*c_velosity, ampl);
            obs_copy << buf << std::endl;

            secobs = static_cast<int64_t>(sec);
            j9 = 1;
            for (jj = 2; jj <= npoint; jj++) {
                if (!std::getline(obs, temp_line)) break;
                if ((sscanf(temp_line.c_str(), datamodels::FormatOGA_R, &sec1, &azym, &elev, &range, &ampl))!=5) {
                    obs_catalogue_copy.close();
                    obs_copy.close();
                    std::cout << "Error reading observation file " << nam << std::endl;
                    remove("D:\\LASER-2\\DATA\\KAT_OBS.ROB");
                    remove(name);
                    return 5;
                }
                if(sec1 > sec) {
                    j9++;
                    sprintf(buf, datamodels::FormatCGA_W, sec1, azym, elev, range, range*0.5e-12*c_velosity, ampl);
                    obs_copy << buf << std::endl;
                }
                sec = sec1;
            }
            obs.close();
            obs_copy.close();

            if (j9 < npoint)
                printf("Removed %4d  points !\n", npoint-j9);
            i = -1;
            ws1 = 1;
            npoint = j9;
            if (secobs < secday) mjd += 1;
            sprintf(buf, datamodels::FormatKAT_OBS_W, ws, ws1, mjd, secobs, nsat, nsta, kodst, namefe,
                    npoint, temp, pres, humid, TB, RB, RMS, POLY, clock_cor);
            obs_catalogue_copy << buf << std::endl; // ????????
        }
    }
    obs_catalogue.close();
    obs_catalogue_copy.close();

    if (i == 0) {
        std::cout << "No such observation in the catalogue!" << std::endl;
        remove("D:\\LASER-2\\DATA\\KAT_OBS.ROB");
        return 5;
    }
    remove("D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    rename("D:\\LASER-2\\DATA\\KAT_OBS.ROB", "D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    return 1;
}

int cota_obs(int nr)
{
    int	ampl, n, jj, ws, ws1, nsta, POLY, npoint, humid;
    int64_t nsat, secday, mjd;
    double	rrr1, sec, azym, elev, range, range1, TB, RB, RMS, clock_cor, temp, pres;
    char	c, namefe[9], name[33], kodst[3];

    if (nr == 0) {
        std::cout << "Error !!!" << std::endl;
        return 5;
    }
    std::ifstream obs_catalogue("D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    if (!obs_catalogue.is_open()) {
        std::cout << "Unable to open observations catalogue!" << std::endl;
        return 5;
    }

    n = 0;
    jj = 0;
    std::string temp_line;
    while (std::getline(obs_catalogue, temp_line)) {
        sscanf(temp_line.c_str(), datamodels::FormatKAT_OBS_R,
                                   &c, &ws, &ws1, &mjd, &secday, &nsat, &nsta, kodst, namefe, &npoint,
                                   &temp, &pres, &humid, &TB, &RB, &RMS, &POLY, &clock_cor);
        n++;
        if (n == nr) {
            jj = -1;
            break;
        }
    }
    obs_catalogue.close();

    if (jj == 0) {
        std::cout << "No such observation in the catalogue!" << std::endl;
        return 5;
    }
    if (ws1 == -1) {
        std::cout << "Please, do this on copy." << std::endl;
        return 5;
    }

    rrr1 = 0.; // 35.
    if (clock_cor != 0.) {
        printf("Clock correction was entered = %8.1lf  \n", clock_cor);
        printf("Program deleted old correction, and created new \n"
                "Do you want to enter new correction?    :\n");

        c = getch();
        if(c == 0x00)	c=getch();
        if(c != 0x0d)	return 1;
        fflush(stdin);
        printf(" Enter clock correction in microseconds :");
        scanf("%lf",&rrr1);
    }

    sprintf(name,"D:\\LASER-2\\DATA\\%8s.c%2s",namefe,kodst);
    std::ofstream copy("D:\\LASER-2\\DATA\\kopia.rob");
    if (!copy.is_open()) {
        std::cout << "Unable to write working copy of observation!" << std::endl;
        return 5;
    }
    std::ifstream obs_copy(name);
    if (!obs_copy.is_open()) {
        printf("Unable to open FILE %8s.c%2s !\n", namefe, kodst);
        return 5;
    }
    for (jj = 1; jj <= npoint; jj++) {
        std::getline(obs_copy, temp_line);
        sscanf(temp_line.c_str(), datamodels::FormatCGA_R, &sec, &azym, &elev, &range1, &range, &ampl);
        sec += (rrr1-clock_cor)*1.e-6;
        char buf[128];
        sprintf(buf, datamodels::FormatCGA_W, sec, azym, elev, range1, range, ampl);
        copy << buf << std::endl;
    }
    copy.close();
    obs_copy.close();

    obs_catalogue.open("D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    if (!obs_catalogue.is_open()) {
        std::cout << "Unable to open observations catalogue!" << std::endl;
        return 5;
    }
    std::ofstream obs_catalogue_copy("D:\\LASER-2\\DATA\\KAT_OBS.ROB");
    if (!obs_catalogue_copy.is_open()) {
        std::cout << "Unable to write observations catalogue copy!" << std::endl;
        return 5;
    }
    n = 0;

    while (std::getline(obs_catalogue, temp_line)) {
        n++;
        if (n == nr) {
            if (sscanf(temp_line.c_str(), datamodels::FormatKAT_OBS_R,
                        &c, &ws, &ws1, &mjd, &secday, &nsat, &nsta, kodst, namefe,
                        &npoint, &temp, &pres, &humid, &TB, &RB, &RMS, &POLY, &clock_cor)!=18) {
                std::cout << "\nERROR reading observations catalogue in cota_obs: line # " << n << std::endl;
                obs_catalogue.close();
                obs_catalogue_copy.close();
                remove("D:\\LASER-2\\DATA\\KAT_OBS.ROB");
                remove("D:\\LASER-2\\DATA\\kopia.rob");
                return 5;
            }
            clock_cor = rrr1;
            char buf[256];
            sprintf(buf, datamodels::FormatKAT_OBS_W,
                        ws, ws1, mjd, secday, nsat, nsta, kodst, namefe, npoint, temp, pres, humid, TB,
                        RB, RMS, POLY, clock_cor);
            obs_catalogue_copy << buf << std::endl;
        } else {
            obs_catalogue_copy << temp_line << std::endl;
        }
    }
    obs_catalogue.close();
    obs_catalogue_copy.close();
    if (remove("D:\\LASER-2\\DATA\\KAT_OBS.DIC") !=0 ) {
        std::cout << " Unable to delete old FILE  KAT_OBS.DIC" << std::endl;
        remove("D:\\LASER-2\\DATA\\kopia.rob");
        return 5;
    }
    if (rename("D:\\LASER-2\\DATA\\KAT_OBS.ROB", "D:\\LASER-2\\DATA\\KAT_OBS.DIC") !=0 ) {
        printf("ERROR in renaming FILE !!\n"
            "KAT_OBS.ROB   ­  : KAT_OBS.DIC\n");
        remove("D:\\LASER-2\\DATA\\kopia.rob");
        return 5;
    }
    remove(name);
    rename("D:\\LASER-2\\DATA\\kopia.rob", name);
    return 1;
}

int list_cal(int nr1, int nr2)
{
    if (nr1 == 0) nr1 = 1;
    if (nr2 == 0) nr2 = 2000;
    if (nr1 > nr2) {
        std::cout << " ERROR in list_cal: start number > finish number !!" << std::endl;
        return 5;
    }

    std::ifstream cal_catalogue("D:\\LASER-2\\DATA\\KAT_KAL.DIC");
    if (cal_catalogue.is_open()) {
        int n = 0, jj = 0;
        int	nsta, filtr, humid, nflash, nreturn, weather, y, m, d, hour, min, coef;
        int64_t mjd, numberpas, secday;
        double	meanval, rms, corect, temp, baro, skew, kurt, target;
        char c, nam[3], kodst[3], namcal[13];

        std::cout << std::endl << " NR  YEAR M D   H M  NF  AVER   ERROR CORR "
                  << " TEMP  PREAS HU NST NPO PAS  N" << std::endl;
        do {
            std::string temp_line;
            std::getline(cal_catalogue, temp_line);

            if (temp_line.empty()) {
                std::cout << "Calibrations catalogue is empty." << std::endl;
                cal_catalogue.close();
                return 1;
            }
            if (sscanf(temp_line.c_str(), datamodels::FormatKAT_KAL_R,
                       &c,&nsta,kodst,nam,&mjd,&secday,&filtr,&meanval,&rms,&corect,&temp,&baro,&humid,
                       &nflash, &nreturn,&weather,&numberpas,&coef,&skew,&kurt,&target,namcal) != 22) {
                std::cout << "ERROR in reading calibrations catalogue, line # " << n+1 << std::endl;
                cal_catalogue.close();
                return 5;
            }
            n++;
            if (n > nr2) break;

            if (jj == 20) {
                jj = 0;
                std::cout << "Continue (Enter)  " << std::endl;
                c = getch();
                if(c == 0x00)	c=getch();
                if(c != 0x0d)	break;
                std::cout << std::endl << " NR  YEAR M D   H M  NF  AVER   ERROR CORR "
                          << " TEMP  PREAS HU NST NPO PAS  N" << std::endl;
            }
            if (n >= nr1) {
                utility::mjd_dat(d,m,y,mjd);
                jj++;
                hour = static_cast<int>(secday/3600);
                min = static_cast<int>(secday-static_cast<int64_t>(hour)*3600)/60;

                printf("%3d %4d-%2d-%2d%3d:%2d%2d%9.3lf%5.1lf%7.2lf%5.1lf%7.1lf%3d%4d%4d%4lld%3d\n",
                n, y, m, d, hour, min, filtr, meanval, rms*15., corect, temp-273.15, baro, humid, nflash, nreturn, numberpas, coef);
            }
        } while (true);

        cal_catalogue.close();
        return 1;
    } else {
        std::cout << "Unable to open calibrations catalogue!" << std::endl;
        return 5;
    }
}

int list_obs(int nr1,int nr2)
{
    if (nr1 == 0) nr1 = 1;
    if (nr2 == 0) nr2 = 1000;
    if (nr1 > nr2) {
        std::cout << " ERROR in list_obs: start number > finish number !!" << std::endl;
        return 5;
    }

    std::ifstream obs_catalogue("D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    if (obs_catalogue.is_open()) {
        int n = 0, jj = 0;
        char c, namefe[9], kodst[3];
        int	nsta, ws, ws1, y, d, m, POLY, npoint, humid;
        int64_t mjd, secday, nsat, hour, min, sec;
        double	TB, RB, RMS, clock_cor, temp, pres;

        std::cout << std::endl << " NR  WS  STATION      SAT    DATA     TIME BEGIN.    QUANT. Ind " << std::endl;
        do {
            std::string temp_line;
            std::getline(obs_catalogue, temp_line);

            if (temp_line.empty()) {
                std::cout << "Observations catalogue is empty." << std::endl;
                obs_catalogue.close();
                return 1;
            }
            n++; // ????? maybe error -- early "++"

            if(n >= nr1 && n <= nr2) {
                if (jj == 20) {
                    jj = 0;
                    std::cout << "Continue (Enter)  " << std::endl;
                    c = getch();
                    if(c == 0x00)	c=getch();
                    if(c != 0x0d)	break;
                    std::cout << std::endl << " NR  WS  STATION      SAT    DATA     TIME BEGIN.    QUANT. Ind " << std::endl;
                }
                if (sscanf(temp_line.c_str(), datamodels::FormatKAT_OBS_R,
                           &c, &ws, &ws1, &mjd, &secday, &nsat, &nsta, kodst, namefe, &npoint,
                           &temp, &pres, &humid, &TB, &RB, &RMS, &POLY, &clock_cor) != 18) {
                    printf("%d %c %d %d %lld %lld %lld %d %2s %8s %d %lf %lf %d %lf %lf %lf %d %lf",
                           n, c, ws, ws1, mjd, secday, nsat, nsta, kodst, namefe, npoint,
                           temp, pres, humid, TB, RB, RMS, POLY, clock_cor);
                    std::cout << std::endl << "ERROR in reading OBS catalogue in list_obs, line # " << n+1 << std::endl;
                    obs_catalogue.close();
                    return 5;
                }

                utility::mjd_dat(d,m,y,mjd);
                hour = secday/3600LL;
                min = (secday-hour*3600LL)/60LL;
                sec = secday-hour*3600LL-min*60LL;
                printf("%3d%3d  %5d     %8s %4d-%2d-%2d %3lld:%2lld:%2lld%10d",
                       n, ws, nsta, namefe, y, m, d, hour, min, sec, npoint);

                if (ws1 == -1)
                    std::cout << "  Obs" << std::endl;
                else
                    std::cout << "  Copy" << std::endl;
            }
            if (n > nr2) break;
        } while (true);

        obs_catalogue.close();
        return 1;
    } else {
        std::cout << "Unable to open observations catalogue!" << std::endl;
        return 5;
    }
}

void distribc(double mean_val, double meanvall, double corectt, double rmss, int nret, int nflash,
              int maxplus, int maxmin, int i_step, int* ntab, double skew, double kurt)
{
    int i, jj, max;
    printf("  Averege = %9.3lf ns Error  = %7.3lf cm\n",meanvall,rmss);
    printf("  Correction   = %8.3lf ns     for %4d measurements\n", corectt, nret);
    printf("        Points > %4.1lf cm -   %5d\n", mean_val, maxplus);

    for(i = 0, jj = 0; i < 19; i++)
        if (ntab[i] > jj) jj = ntab[i];
    max = 1;
    if (jj > 65) max = jj/65+1;

    printf("       MAX = %3d   Skew = %7.3lf    Kurt = %7.3lf", max, skew, kurt);

    for(i = 0; i < 19; i++) {
        printf("\n  %4d I", (9-i)*i_step);
        if (ntab[i] != 0)
            for(jj = 0; jj < ntab[i]; jj += max)
                printf("X");
    }
    printf("\n        Points <    -%4.1lf cm -   %5d\n", mean_val, maxmin);

    fflush(stdin);
    printf("%3d%% Good. Ok?        : 0-exit, 1-Yes , 2-No(Cut),"
            " 3- 2. sigma: ", nret*100/nflash);
}

void distrib(std::string namex, double mean_val, int maxplus, int maxmin, int i_step, std::vector<int64_t> ntab, double skew, double kurt)
{
    int	i,jj,max;
    char buf[256];

    std::ofstream distrib_file(namex, std::ios::app);
    distrib_file << "\n\n                RESIDUALS DISTRIBUTION  \n";
    sprintf(buf,"\n       NUMBER OF POINTS > THAN %4.1lf CM  -%4d",mean_val,maxplus);
    distrib_file << buf;

    for(i=0,jj=0;i<19;i++)
        if(ntab[i]>jj)
            jj=ntab[i];
    max=1;
    if(jj>65)   max=jj/65+1;

    sprintf(buf,"\n       MAX = %3d        SKEW = %7.2lf        "
        " KURT = %7.2lf",max,skew,kurt);
    distrib_file << buf;
    for(i=0;i<19;i++) {
        sprintf(buf,"\n  %4d I",(9-i)*i_step);
        distrib_file << buf;
        if(ntab[i]!=0) {
            for(jj=0;jj<ntab[i];jj=jj+max)
                distrib_file << "X";
        }
    }

    sprintf(buf,"\n       NUMBER OF POINTS < THAN -%4.1lf CM  -%4d\n",mean_val,maxmin);
    distrib_file << buf;
    distrib_file.close();
}

int fit_cal(int nr)
{
    int      nsta, filtr, humid, nflash, nreturn, weather, n, jj, ntab[20], coef, maxplus, maxmin, nret, i_step, i;
    int64_t  mjd, numberpas, secday;
    double   meanval, rms, corect, mean_val, target, l,
            skew1, kurt1, temp, baro, step, mi2, mi3, mi4, y, x, skew, kurt;
    char     c, namcal[13], namfil[33], nam[3], kodst[3], _[5];

    double c_velocity = 299792458.;

    std::ifstream sys_conf("D:\\LASER-2\\DATA\\SYS_CONF");
    if (sys_conf.is_open()) {
        std::string temp_line;
        std::getline(sys_conf, temp_line);
        sscanf(temp_line.c_str(), datamodels::FormatSYS_CONF_R, &l, &_, &_, &_, &_, &_);
        sys_conf.close();
    } else {
        std::cout << "Unable to open configuration description file!" << std::endl;
        return 5;
    }

    l /= 1000.;
    double fl = 0.9650+0.0164/(l*l)+0.000228/(l*l*l*l);

    if (nr==0) {
        std::cout << "Error !!!" << std::endl;
        return 5;
    }

    std::ifstream cal_catalogue("D:\\LASER-2\\DATA\\KAT_KAL.DIC");
    if (cal_catalogue.is_open()) {
        n = 0;
        jj = 0;

        std::string temp_line;
        while (std::getline(cal_catalogue, temp_line)) {
            if (sscanf(temp_line.c_str(), datamodels::FormatKAT_KAL_R,
                       &c,&nsta,kodst,nam,&mjd,&secday,&filtr,&meanval,&rms,&corect,&temp,&baro,&humid,
                       &nflash, &nreturn,&weather,&numberpas,&coef,&skew,&kurt,&target,namcal) != 22) {
                std::cout << "ERROR in reading calibrations catalogue, line # " << n+1 << std::endl;
                cal_catalogue.close();
                return 5;
            }
            n++;
            if (n == nr) {
                jj = 1;
                break;
            }
        }
        cal_catalogue.close();
    } else {
        std::cout << "Unable to open calibrations catalogue!" << std::endl;
        return 5;
    }

    if (jj == 0) {
        std::cout << "No such calibration in the catalogue!" << std::endl;
        return 5;
    }
    if (nreturn > 1000) {
        std::cout << "MAX number of measurements = 1000 !!" << std::endl;
        return 5;
    }

    sprintf(namfil, "D:\\LASER-2\\DATA\\%s", namcal);
    std::ifstream data_file(namfil);

    std::vector<double> data;
    std::vector<int> icoef;
    if (data_file.is_open()) {
        for (int i=0; i < nreturn; i++) {
            double value;
            data_file >> value;
            data.push_back(value);
            icoef.push_back(1);
        }
        data_file.close();
    } else {
        std::cout << "Unable to open FILE: " << namfil << std::endl;
        return 5;
    }

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! NOT INCLUDED IN GUI VERSION

    std::ifstream target_cat("D:\\LASER-2\\DATA\\kat_tar.dic");
    if (target_cat.is_open()) {
        n = 0;
        i = -1;
        std::string temp_line;
        while (std::getline(target_cat, temp_line)) {
            temp_line = temp_line.substr(1);
            sscanf(temp_line.c_str(), datamodels::FormatKAT_TAR_R, kodst, &target);
            if (!strcmp(kodst, nam)) {
                i = 1;
                break;
            }
        }
        target_cat.close();
    } else {
        std::cout << "Unable to open target catalogue!" << std::endl;
        return 5;
    }

    double target1 = target;
    if (i == -1) {
        std::cout << "No such target!" << std::endl;
        return 5;
    }

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! NOT INCLUDED IN GUI VERSION

    double sx = 7.5*(temp-273.1)/(237.3-(temp-273.1));
    double e = static_cast<double>(humid)*1.e-2*6.11*pow(10.,sx);
    double gn = 80.343*fl*baro/temp-11.3*e/temp;
    double tkal=2.*1.e9*target*(1.+gn*1.e-6)/c_velocity;
    double rmss = rms, corectt = corect, meanvall = meanval;
    int jexi = 0;

    do {
        if (rmss > 0.6) {
            mean_val = 47.5;
            step = 0.5e-7*c_velocity/5.;
            i_step = 5;
        } else {
            mean_val = 19.;
            step = 0.5e-7*c_velocity/2.;
            i_step = 2;
        }
        if (rmss > 0.9) {
            mean_val = 95.;
            step = 0.5e-7*c_velocity/10.;
            i_step = 10;
        }

        for(n = 0; n < 19; n++) ntab[n] = 0;
        sx = 0.5/step;
        for(i = 0, maxplus = maxmin = nret = 0; i < nreturn; i++)
            if (icoef[i] == 1) {
                nret++;
                jj = 9-static_cast<int>(floor((data[i]+sx-meanvall)*step));
                if (jj < 0)
                    maxplus++;
                if (jj > 19)
                    maxmin++;
                if (jj>=0 && jj<=18)
                    ntab[jj]++;
            }

        for(i = 0,x = 0.;i < nreturn; i++)
            if(icoef[i]==1) x += data[i];
        x /= static_cast<double>(nret);

        for(i = 0, mi2 = 0., mi3 = 0., mi4 = 0.; i < nreturn; i++)
            if (icoef[i] == 1) {
                y = data[i] - x;
                mi2 += y*y;
                mi3 += y*y*y;
                mi4 += y*y*y*y;
            }
        mi2 /= static_cast<double>(nret);
        mi3 /= static_cast<double>(nret);
        mi4 /= static_cast<double>(nret);

        skew = (mi3 * mi3) / (mi2 * mi2 *mi2);
        skew1 = skew;
        kurt =  mi4 / (mi2 * mi2);
        kurt1 = kurt;

        sx = rmss*0.5e-7*c_velocity;
        distribc(mean_val, meanvall, corectt, sx, nret, nflash,
                 maxplus, maxmin, i_step, ntab, skew, kurt);

        fflush(stdin);
        std::cin >> jexi;
        if (jexi == 0) return 1;
        if (jexi == 2) {
            std::cout << "Enter accepted range +/- cm : ";
            std::cin >> sx;
            sx *= 2.e7/c_velocity;

            for (i = jj = 0, mean_val = 0; i < nreturn; i++) {
                if (fabs(data[i] - meanvall) <= sx) {
                    jj++;
                    mean_val += data[i];
                    icoef[i] = 1;
                } else {
                    icoef[i] = 0;
                }
            }
            if (jj < nret) {
                meanvall = mean_val / static_cast<double>(jj);
                mean_val = 0;
                nret = jj;
                for(i = 0; i< nreturn; i++)
                    if (icoef[i] == 1) {
                        rmss = data[i] - meanvall;
                        mean_val += rmss*rmss;
                    }
                rmss = sqrt(mean_val/static_cast<double>(jj-1));
                mean_val = meanvall;
                corectt = mean_val - tkal;
            }
        }
        if (jexi == 3) {
            sx = rmss*2.;

            for(i = jj = 0, mean_val = 0; i < nreturn; i++)
                if (icoef[i] == 1) {
                    if (fabs(data[i] - meanvall) <= sx) {
                        jj++;
                        mean_val += data[i];
                        icoef[i] = 1;
                    } else {
                        icoef[i] = 0;
                    }
                }
            if (jj < nret) {
                meanvall = mean_val / static_cast<double>(jj);
                mean_val = 0;
                nret = jj;
                for(i = 0; i< nreturn; i++)
                    if (icoef[i] == 1) {
                        rmss = data[i] - meanvall;
                        mean_val += rmss*rmss;
                    }
                rmss = sqrt(mean_val/static_cast<double>(jj-1));
                mean_val = meanvall;
                corectt = mean_val - tkal;
            }
        }
    } while (jexi != 1);

    cal_catalogue.open("D:\\LASER-2\\DATA\\KAT_KAL.DIC");
    if (!cal_catalogue.is_open()) {
        std::cout << "Unable to open calibrations catalogue!" << std::endl;
        return 5;
    }
    std::ofstream cal_catalogue_copy("D:\\LASER-2\\DATA\\KAT_KAL.ROB");
    if (!cal_catalogue_copy.is_open()) {
        std::cout << "Unable to write calibrations catalogue copy!" << std::endl;
        return 5;
    }

    n = 0;
    std::string temp_line;
    while (std::getline(cal_catalogue, temp_line)) {
        n++;

        if (n == nr) {
            if (sscanf(temp_line.c_str(), datamodels::FormatKAT_KAL_R,
                       &c,&nsta,kodst,nam,&mjd,&secday,&filtr,&meanval,&rms,&corect,&temp,&baro,&humid,
                       &nflash, &nreturn,&weather,&numberpas,&coef,&skew,&kurt,&target,namcal) != 22) {
                std::cout << "ERROR in reading calibrations catalogue, line # " << n+1 << std::endl;
                cal_catalogue.close();
                cal_catalogue_copy.close();
                return 5;
            }
            rms = rmss;
            meanval = meanvall;
            corect = corectt;
            nreturn = nret;

            std::ofstream temp_file("D:\\LASER-2\\DATA\\TEMP.TMP");
            if (!temp_file.is_open()) {
                std::cout << "Unable to open FILE TEMP.TMP" << std::endl;
                cal_catalogue_copy.close();
                return 5;
            } else {
                for(i = 0; i < nreturn; i++)
                    if (icoef[i] == 1) {
                        char _[64];
                        sprintf(_, "%9.3lf\n", data[i]);
                        temp_file << _;
                    }
                temp_file.close();
            }

            target = (target == 0.)?target1:target;
            char line[256];
            sprintf(line, datamodels::FormatKAT_KAL_W,
                     nsta, kodst, nam, mjd, secday, filtr, meanval, rms, corect, temp,baro,
                     humid, nflash, nreturn, weather, numberpas, coef, skew1, kurt1, target, namcal);
            temp_line.clear();
            temp_line.append(line);
        }
        cal_catalogue_copy << temp_line << std::endl;
    }

    cal_catalogue.close();
    cal_catalogue_copy.close();
    remove("D:\\LASER-2\\DATA\\KAT_KAL.DIC");
    rename("D:\\LASER-2\\DATA\\KAT_KAL.ROB",
        "D:\\LASER-2\\DATA\\KAT_KAL.DIC");
    remove(namfil);
    rename("D:\\LASER-2\\DATA\\TEMP.TMP",namfil);
    return 1;
}

void alarm()
{
    while (!kbhit()) {
        Beep(1000,150);
        Sleep(100);

        Beep(500,100);
        Sleep(100);

        Beep(2000,100);
        Sleep(100);

        Beep(300,100);
        Sleep(100);
    }
}

void sound5()
{
    while (!kbhit()) {
        Beep(1450,180);		Sleep(180);
        Beep(1560,180);		Sleep(180);
        Beep(1440,140);		Sleep(140);
        Beep(1240,100);		Sleep(100);
        Sleep(130);
        Beep(1188,180);		Sleep(180);
        Sleep(130);
        Beep(1168,200);		Sleep(200);
        Sleep(130);
    }
}

int stt_epaa(int64_t numpass)
{
    int j, nr, nr1, nr_obs, cal_before, cal_after;
    int ws, ws1, nsta, npoint, humid, step_poly;
    char namefe[9], c;
    int64_t nsat = 0, secday, mjd;
    double time_bias, range_bias, rms, clock_cor, temp, pres;

    while (true) {
        nr = -1;
        do {
            char input[31];
            std::cout << "Enter the number of observation." << std::endl
                      << " * l nr1, nr2 - list of observations, 0 - no observations :" << std::endl;
            fflush(stdin);
            gets(input);
            if (input[0] == 'l') {
                strtok(input, " ,:;");
                j = atoi(strtok(nullptr, " ,:;"));
                nr1 = atoi(strtok(nullptr, " ,:;"));
                if (list_obs(j,nr1)==5) return 5;
            } else {
                nr = atoi(input);
            }
        } while (nr==-1);
        if (nr==0) return 5;

        nr_obs = nr;
        std::ifstream obs_catalogue("D:\\LASER-2\\DATA\\KAT_OBS.DIC");
        double time_bias, range_bias, rms, clock_cor, temp, pres;
        bool isObsFound = false;
        char namefe[9], kodst[3];
        if (obs_catalogue.is_open()) {
            int n = 0;
            std::string temp_line;
            while (std::getline(obs_catalogue, temp_line)) {
                sscanf(temp_line.c_str(), datamodels::FormatKAT_OBS_R,
                                        &c, &ws, &ws1, &mjd, &secday, &nsat, &nsta, kodst, namefe,
                                        &npoint, &temp, &pres, &humid, &time_bias, &range_bias, &rms, &step_poly, &clock_cor);
                ++n;
                if (n==nr) {
                    isObsFound = true;
                    break;
                }
            }
            obs_catalogue.close();
        } else {
            std::cout << "Unable to open file with list of observations" << std::endl;
            return 5;
        }

        if (!isObsFound) {
            std::cout << "No such observation in the OBS catalogue!" << std::endl;
            return 5;
        }

        if (ws1==-1) {
            char name[35];
            sprintf(name, "D:\\LASER-2\\DATA\\%8s.c%2s", namefe, kodst);

            std::ifstream obsFile(name);
            if (!obsFile.is_open()) {
                if (copy_obs(nr)==5) return 5;
                nr_obs++;
            } else {
                std::cout << "Copy of this observation is present." << std::endl;
                obsFile.close();
                continue;
            }
        }
        break;
    }

    if (cota_obs(nr_obs)==5) return 5;

    while (true) {
        std::cout << "Enter the number of series (For Normal Points): " << std::endl;
        fflush(stdin);
        std::cin >> numpass;
        if (numpass <= 0L) {
            std::cout << "Is it real number of series?" << std::endl;
            fflush(stdin);
            char c = getch();
            if(c == 0x00)	c=getch();
            if(c != 0x0d)	continue;
        }
        break;
    }

    while (true) {
        nr = -1;
        do {
            char input[31];
            std::cout << "Enter the number of pre-observation calibration." << std::endl
                      << " * l nr1, nr2 - list of calibrations, 0 - no calibrations :" << std::endl;
            fflush(stdin);
            gets(input);
            if(input[0]!=0x0d) {
                if(input[0] == 'l') {
                    strtok(input, " ,:;");
                    j=atoi(strtok(nullptr, " ,:;"));
                    if(list_cal(j,0)==5) return 5;
                } else {
                    nr = atoi(input);
                }
            }
        } while (nr==-1);

        cal_before = nr;
        if (cal_before <= 0) {
            std::cout << "Is it real number of calibration?  " << cal_before << std::endl;
            fflush(stdin);
            char c = getch();
            if(c == 0x00)	c=getch();
            if(c != 0x0d)	continue;
        }
        break;
    }

    if(nr!=0)	if(fit_cal(nr)==5)	return 5;

    while (true) {
        nr = -1;
        do {
            char input[31];
            std::cout << "Enter the number of post-observation calibration." << std::endl
                      << " * l nr1, nr2 - list of calibrations, 0 - no calibrations :" << std::endl;
            fflush(stdin);
            gets(input);
            if(input[0] != 0x0d) {
                if(input[0] == 'l') {
                    strtok(input, " ,:;");
                    j=atoi(strtok(nullptr, " ,:;"));
                    if(list_cal(j,0)==5) return 5;
                } else {
                    nr = atoi(input);
                }
            }
        } while (nr==-1);

        cal_after = nr;
        if (cal_before <= 0) {
            std::cout << "Is it real number of calibration?  " << cal_after << std::endl;
            fflush(stdin);
            char c = getch();
            if(c == 0x00)	c=getch();
            if(c != 0x0d)	continue;
        }
        break;
    }

    if(nr!=0)	if(fit_cal(nr)==5)	return 5;

    std::ofstream epa_dat("D:\\LASER-2\\DATA\\EPA_AUTO.DAT");
    if (epa_dat.is_open()) {
        char buf[128];
        sprintf(buf, datamodels::FormatEPA_AUTO_W, nr_obs,numpass,cal_before,cal_after,nsat,namefe);
        epa_dat << buf << std::endl;
        epa_dat.close();
    } else {
        std::cout << "Unable to open file EPA_AUTO.DAT !" << std::endl;
        return 5;
    }

    return 1;
}

int ffit_obs(int nr)
{
    int	n, jj, ws, ws1, nsta, ampl, POLY, npoint, jjj, npointef, index, humid;
    int64_t mjd, secday, nsat, sss;
    double	sec, secc, azym, elev, range1, range, temp, pres, ss1, timeefe, stepefe, t10, t20, t21, tt0,
            tt1, tt2, re, secstart, secobs, omc, omcplus, omcminus, tbrb, czeb, TB, RB, RMS, clock_cor;
    char    c, namefe[9], name[33], nam[33], namm[33], kodst[3];
    std::vector<double> defefe0(4001), defefe1(4001);

    tbrb = 0.;
    czeb = 0.;
    std::ifstream obs_catalogue("D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    if (!obs_catalogue.is_open()) {
        std::cout << "Unable to open observations catalogue!" << std::endl;
        return 5;
    }
    n = 0;
    jj = 0;
    std::string temp_line;
    while (std::getline(obs_catalogue, temp_line)) {
        sscanf(temp_line.c_str() , datamodels::FormatKAT_OBS_R,
                                &c, &ws, &ws1, &mjd, &secday, &nsat, &nsta, kodst, namefe,
                                &npoint, &temp, &pres, &humid, &TB, &RB, &RMS, &POLY, &clock_cor);
        n++;
        if (n == nr) {
            jj = -1;
            break;
        }
    }
    obs_catalogue.close();
    if (jj == 0) {
        std::cout << "No such observation in the catalogue!" << std::endl;
        return 5;
    }
    if (ws == -1) {
        std::cout << "Please, do things on copy." << std::endl;
        return 5;
    }

    omcplus = 29999.;
    omcminus = -29999.;
    sprintf(name, "D:\\LASER-2\\DATA\\%8s.c%2s", namefe, kodst);
    sprintf(nam, "D:\\LASER-2\\DATA\\%8s.e%2s", namefe, kodst);
    sprintf(namm, "D:\\LASER-2\\DATA\\%8s.p%2s", namefe, kodst);
    std::ifstream efem_file(nam), obs_copy(name);
    if (!efem_file.is_open()) {
        std::cout << "Unable to open efemerides file: " << nam << std::endl;
        return 5;
    }
    if (!obs_copy.is_open()) {
        std::cout << "No copy of such observation: " << name << std::endl;
        return 5;
    }
    std::ofstream copy("D:\\LASER-2\\DATA\\copia.rob"), derivatives_file(namm);
    if (!copy.is_open()) {
        std::cout << "Unable to open working file!" << std::endl;
        return 5;
    }
    if (!derivatives_file.is_open()) {
        std::cout << "Unable to open file: " << namm << std::endl;
        return 5;
    }

    jj = 0;
    while (std::getline(efem_file, temp_line)) {
        jj++;
        sscanf(temp_line.c_str(), datamodels::FormatEGA_R, &c, &sss, &azym, &elev, &ss1);
        defefe0[jj] = static_cast<double>(sss);
        defefe1[jj] = ss1;
    }
    efem_file.close();
    npointef = jj;

    t10 = 0.;
    for (jjj = 2; jjj <= npointef; jjj++) {
        if (defefe0[jjj] < defefe0[jjj-1]) t10 = 86400.;
        defefe0[jjj] += t10;
    }
    timeefe = defefe0[1];
    stepefe = defefe0[2] - defefe0[1];

    tbrb=0.;
    czeb=0.;
    jjj=0;
    for (jj = 1; jj <= npoint; jj++) {
        std::getline(obs_copy, temp_line);
        sscanf(temp_line.c_str(), datamodels::FormatCGA_R, &sec, &azym, &elev, &range1, &range, &ampl);
        range = range1*0.5e-12*299792458.0;

        if (jj == 1) secobs = sec;
        secc = sec;
        if ((secc-timeefe) < 0.)	secc += 86400.;
        index = static_cast<int>((secc-timeefe)/stepefe);
        if(index<1)		index=1;
        if(index>=npointef-1)	index=npointef-2;
        t10 = defefe0[index+1]-defefe0[index];
        t20 = defefe0[index+2]-defefe0[index];
        t21 = defefe0[index+2]-defefe0[index+1];
        tt0 = secc-defefe0[index];
        tt1 = secc-defefe0[index+1];
        tt2 = secc-defefe0[index+2];

        re = tt1*tt2/(t10*t20)*defefe1[index]-
            tt0*tt2/(t10*t21)*defefe1[index+1]+
            tt0*tt1/(t20*t21)*defefe1[index+2];

        omc = (range-re)*1000.;
        if (omc >= omcminus && omc <= omcplus) {
            jjj++;
            if (jjj == 1) secstart = sec;
            char buf[256];
            sprintf(buf, datamodels::FormatCOPY_W, sec, azym, elev, range1, range, ampl);
            copy << buf << std::endl;
            sprintf(buf, datamodels::FormatPGA_W, sec, omc, tbrb, czeb);
            derivatives_file << buf << std::endl;
        }
    }
    obs_copy.close();
    copy.close();
    derivatives_file.close();
    remove(name);
    rename("D:\\LASER-2\\DATA\\copia.rob", name);
    printf("\n Removed : %5d points  Left : %6d points.", npoint-jjj, jjj);

    std::ofstream obs_catalogue_copy("D:\\LASER-2\\DATA\\KAT_OBS.ROB");
    if (!obs_catalogue_copy.is_open()) {
        std::cout << "Unable to write observations catalogue copy!" << std::endl;
        return 5;
    }
    obs_catalogue.open("D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    if (!obs_catalogue.is_open()) {
        std::cout << "Unable to open observations catalogue!" << std::endl;
        return 5;
    }

    n = 0;
    while (std::getline(obs_catalogue, temp_line)) {
        n++;
        if (n == nr) {
            if (sscanf(temp_line.c_str() , datamodels::FormatKAT_OBS_R,
                        &c, &ws, &ws1, &mjd, &secday, &nsat, &nsta, kodst, namefe,
                        &npoint, &temp, &pres, &humid, &TB, &RB, &RMS, &POLY, &clock_cor)!=18) {
                obs_catalogue.close();
                obs_catalogue_copy.close();
                std::cout << std::endl << "ERROR in reading OBS catalogue in copy_obs, line # " << n+1 << std::endl;

                std::ofstream result("D:\\LASER-2\\DATA\\RESULT.DAT");
                result << " 0";
                result.close();
                std::cout << "Line : " << temp_line << std::endl;
                return 5;
            }
            ws = 1;
            npoint = jjj;
            secday = static_cast<int64_t>(secstart);
            if(( fabs(secstart-secobs) ) > 80000.) mjd++;
            char buf[256];
            sprintf(buf, datamodels::FormatKAT_OBS_W,
                        ws, ws1, mjd, secday, nsat, nsta, kodst, namefe, npoint, temp, pres, humid, TB,
                        RB, RMS, POLY, clock_cor);
            obs_catalogue_copy << buf << std::endl;
        } else {
            obs_catalogue_copy << temp_line << std::endl;
        }
    }
    obs_catalogue.close();
    obs_catalogue_copy.close();
    remove("D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    rename("D:\\LASER-2\\DATA\\KAT_OBS.ROB", "D:\\LASER-2\\DATA\\KAT_OBS.DIC");

    std::ofstream result("D:\\LASER-2\\DATA\\RESULT.DAT");
    char buf[16];
    sprintf(buf, "%5d", jjj);
    result << buf;
    result.close();
    printf("\n END FILTERS");
    return 1;
}

int plo_fita(int nr)
{
    int      kk,n,jj,ws,ws1,nsta,x,y,POLY,points,humid,j;
    int	     day,month,year,hour,min,sek,jstep,jend,NSTA,normx,normy;
    int      OP[6],i,xe,ye,xb,yb,j9,h,r,kod,ss2;
    int64_t  mjd,secday,nsat,NSAT,secobs;
    double   sec,omc,omcplus,omcminus,tbrb,czeb,time,omcskala,timskala;
    double   temp,pres,TB,RB,RMS,clock_cor,x1,s2,s1;
    char     c,namefe[9],namm[33],kodst[3],NAMSTAC[12];
    char	    NAMSAT[12],NAMSYM[3],nam[3],name[33],linia[135];
    char     header1[81],header2[81],ss[20],msg[5],msc[5];

    tbrb = czeb = 0.;
    n = jj = 0;
    std::ofstream result("D:\\LASER-2\\DATA\\RESULT.DAT");
    std::ifstream obs_catalogue("D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    if (!obs_catalogue.is_open()) {
        std::cout << "Unable to open observations catalogue!" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    std::string temp_line;
    while (std::getline(obs_catalogue, temp_line)) {
        sscanf(temp_line.c_str() , datamodels::FormatKAT_OBS_R,
                                &c, &ws, &ws1, &mjd, &secday, &nsat, &nsta, kodst, namefe,
                                &points, &temp, &pres, &humid, &TB, &RB, &RMS, &POLY, &clock_cor);
        n++;
        if (n == nr) {
            jj = -1;
            break;
        }
    }
    obs_catalogue.close();
    if (jj == 0) {
        std::cout << "No such observation in the catalogue!" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    if (ws1 == -1) {
        std::cout << "Please, do this on copy." << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    if (ws1 == 0) {
        std::cout << "This copy hasn't been filtered yet!" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    if (points > 10001) {
        std::cout << "Too many points, MAX = 10000!" << std::endl;
        result << "0" << std::endl;
        return 5;
    }

    std::ifstream param_dat("D:\\LASER-2\\DATA\\PARAM.DAT");
    if (!param_dat.is_open()) {
        std::cout << "Unable to open PARAM.DAT!" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    param_dat >> kk;
    param_dat.close();
    if(kk == 2 && ws < 2) {
        std::cout << "Time and Range Bias haven't been calculated yet!" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    if(kk == 3 && ws != 3) {
        std::cout << "Polynomials haven't been calculated yet!" << std::endl;
        result << "0" << std::endl;
        return 5;
    }

    std::ifstream station("D:\\LASER-2\\DATA\\STATION");
    if (!station.is_open()) {
        std::cout << "Unable to open file station !" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    jj = 0;
    std::getline(station, temp_line);
    sscanf(temp_line.c_str(), "%s%d%d%d%d", &NAMSTAC,&NSTA,&ss2,&ss2,&ss2);
    station.close();
    if (nsta == NSTA) jj = 1;
    if (jj == 0) {
        std::cout << "\n NO STATION  : " << nsta << " IN THE STATION CATALOGUE !\n";
        result << "0" << std::endl;
        return 5;
    }

    std::ifstream satellit("D:\\LASER-2\\DATA\\SATELLIT");
    if (!satellit.is_open()) {
        std::cout << "Unable to open file satellit !" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    jj = 0;
    while (std::getline(satellit, temp_line)) {
        sscanf(temp_line.c_str(), datamodels::FormatSATELLIT_R, &NAMSAT,&NSAT,&ss2,&ss2,&ss2,&ss2,&ss2,&ss2);
        if (NSAT == nsat) {
            jj = 1;
            break;
        }
    }
    satellit.close();
    if (jj == 0) {
        std::cout << "\n NO SATELLIT : " << nsat << " IN THE CATALOGUE !\n";
        result << "0" << std::endl;
        return 5;
    }

    utility::mjd_dat(day, month, year, mjd);
    hour = static_cast<int>(secday / 3600LL);
    min  = static_cast<int>(secday % 3600LL / 60LL);
    sek  = static_cast<int>(secday % 3600LL % 60LL);



    sprintf(header1,"STATION: %+10s   SATELLITE: %-10s %4d-%02d-%02d%4d:%02d:%02d  %5d POINTS",
            NAMSTAC,NAMSAT,year,month,day,hour,min,sek,points);

    // .....
    if (namefe[3] == '\0') namefe[3] = '0';
    // .....

    sprintf(namm,"D:\\LASER-2\\DATA\\%8s.p%2s",namefe,kodst);
    std::ifstream derivatives_file(namm);
    if (!derivatives_file.is_open()) {
        printf("\n  Unable to open file %8s.p%2s !\n", namefe, kodst);
        result << "0" << std::endl;
        return 5;
    }

    QVector<double> X(points), Y(points);
    for (jj = 0; jj < points; ++jj) {
        std::getline(derivatives_file, temp_line);
        sscanf(temp_line.c_str(), datamodels::FormatPGA_R, &sec,&omc,&tbrb,&czeb);
        X[jj] = sec;
        if(kk == 1) Y[jj] = omc;
        if(kk == 2) Y[jj] = tbrb;
        if(kk == 3) Y[jj] = czeb;
    }
    derivatives_file.close();
    QVector<double> Xbackup(X);

    time = X.last() - X.first();
    if (time < 0.) time += 86400.;
    omcplus = omcminus = Y.first();
    for (jj = 1; jj < points; ++jj) {
        if(Y[jj] > omcplus)	 omcplus = Y[jj];
        if(Y[jj] < omcminus) 	 omcminus = Y[jj];
        if(X[jj] < X[0])   X[jj] += 86400.;
    }
    time = X.last() - X.first();
    omcplus =  fabs(omcplus);
    omcminus = fabs(omcminus);
    omc = (omcminus > omcplus) ? omcminus : omcplus;
    for (int i=1; i<X.size(); ++i)
        X[i] -= X[0];
    X[0] = 0;

    switch(kk) {
      case(1): {sprintf(header2," ");					break;}
      case(2): {
            sprintf(header2," ");
            sprintf(header2,"TIME BIAS : %6.1lf MS   RANGE BIAS"
                " : %6.1lf M ",TB,RB);					break;
    }
      case(3): {
            sprintf(header2," ");
            sprintf(header2,"TIME BIAS :%5.1lf MS  RANGE BIAS"
                " :%6.1lf M  STEP OF POLY :%3d  RMS :%5.2lf CM",
                    TB,RB,POLY,RMS*15.);				break;
    }
      default: {
            std::cout << "Incorrect draw option!!!" << std::endl;
            result << "0" << std::endl;
            return 5;
    }
    }

    Plot* p = new Plot();
    p->graph()->setData(X, Y);
    p->isPointDropped.fill(false, X.size());
    p->graph()->rescaleAxes();
    p->xAxis->setLabel(QString("TIME [ S ]"));
    p->yAxis->setLabel(QString("RESIDUALS [ M ]"));
    p->plotLayout()->insertRow(0);
    p->plotLayout()->addElement(0, 0, new QCPTextElement(p, QString(header1), QFont("sans", 12, QFont::Bold)));
    p->plotLayout()->insertRow(0);
    p->plotLayout()->addElement(0, 0, new QCPTextElement(p, QString(header2), QFont("sans", 12, QFont::Bold)));
    p->show();
    int res = qApp->exec();

    if (res == 1 || points == (*(p->graph()->data())).size()) {
        char buf[16];
        sprintf(buf, "%d", points);
        result << buf;
        result.close();
        printf("\n plo_fita finished its work. Points remaining - %d !\n",points);
        delete p;
        return 1;
    }

    int newPoints = 0;
    X.resize((*(p->graph()->data())).size());
    Y.resize((*(p->graph()->data())).size());
    for (auto point : *(p->graph()->data())) {
        X[newPoints] = point.key;
        Y[newPoints] = point.value;
        newPoints++;
    }

    sprintf(name,"D:\\LASER-2\\DATA\\%8s.c%2s",namefe,kodst);
    std::ifstream obs_copy(name);
    if (!obs_copy.is_open()) {
        printf("Unable to open FILE %8s.c%2s !\n", namefe, kodst);
        result << "0" << std::endl;
        delete p;
        return 5;
    }
    std::ofstream robcop("D:\\LASER-2\\DATA\\robcop.cop");
    if (!robcop.is_open()) {
        std::cout << "Unable to open file robcop.cop !" << std::endl;
        result << "0" << std::endl;
        delete p;
        return 5;
    }
    for (n=0, j=0; n < points; n++) {
        std::getline(obs_copy, temp_line);
        sscanf(temp_line.c_str(), datamodels::FormatCGA_R, &sec,&omc,&tbrb,&czeb,&s1,&i);
        Xbackup[n] = sec;
        if (!p->isPointDropped[n]) {
            j++;
            if (j == 1)	secobs = static_cast<int64_t>(sec);
            char buf[256];
            sprintf(buf, datamodels::FormatCOPY_W, sec,omc,tbrb,czeb,s1,i);
            robcop << buf << std::endl;
        }
    }
    obs_copy.close();
    robcop.close();

    sprintf(namm,"D:\\LASER-2\\DATA\\%8s.p%2s",namefe,kodst);
    derivatives_file.open(namm);
    if (!derivatives_file.is_open()) {
        printf("Unable to open FILE %8s.p%2s !\n", namefe, kodst);
        result << "0" << std::endl;
        delete p;
        return 5;
    }
    std::ofstream robkop("D:\\LASER-2\\DATA\\robkop.cop");
    if (!robkop.is_open()) {
        std::cout << "Unable to open file robkop.cop !" << std::endl;
        result << "0" << std::endl;
        delete p;
        return 5;
    }
    for (n=0; n < points; n++) {
        std::getline(derivatives_file, temp_line);
        sscanf(temp_line.c_str(), datamodels::FormatPGA_R, &sec,&omc,&tbrb,&czeb);
        if (!p->isPointDropped[n]) {
            char buf[256];
            sprintf(buf, datamodels::FormatPGA_W, Xbackup[n],omc,tbrb,czeb);
            robkop << buf << std::endl;
        }
    }
    obs_copy.close();
    robkop.close();
    delete p;

    obs_catalogue.open("D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    if (!obs_catalogue.is_open()) {
        std::cout << "Unable to open observations catalogue!" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    std::ofstream obs_catalogue_copy("D:\\LASER-2\\DATA\\KAT_OBS.ROB");
    if (!obs_catalogue_copy.is_open()) {
        std::cout << "Unable to write observations catalogue copy!" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    i = 0;
    while (std::getline(obs_catalogue, temp_line)) {
        i++;
        char buf[256];
        if (i == nr) {
            if (sscanf(temp_line.c_str(), datamodels::FormatKAT_OBS_R,
                       &c, &ws, &ws1, &mjd, &secday, &nsat, &nsta, kodst, namefe, &points,
                       &temp, &pres, &humid, &TB, &RB, &RMS, &POLY, &clock_cor) != 18) {
                std::cout << "\nERROR reading observations catalogue in pasint_a: line # " << n << std::endl;
                obs_catalogue.close();
                obs_catalogue_copy.close();
                result << "0" << std::endl;
                return 5;
            }
            points = j;
            if(secday > secobs)					mjd++;
            secday = secobs;
            sprintf(buf, datamodels::FormatKAT_OBS_W,
                        ws, ws1, mjd, secday, nsat, nsta, kodst, namefe, points, temp, pres, humid, TB,
                        RB, RMS, POLY, clock_cor);
            obs_catalogue_copy << buf << std::endl;
        } else {
            obs_catalogue_copy << temp_line << std::endl;
        }
    }
    obs_catalogue.close();
    obs_catalogue_copy.close();

    remove(name);
    rename("D:\\LASER-2\\DATA\\robcop.cop",name);
    rename("D:\\LASER-2\\DATA\\robkop.cop",namm);
    remove("D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    rename("D:\\LASER-2\\DATA\\KAT_OBS.ROB",
           "D:\\LASER-2\\DATA\\KAT_OBS.DIC");

    char buf[16];
    sprintf(buf, "%d", points);
    result << buf;
    result.close();
    printf("\n plo_fita finished its work. Points remaining - %d !\n",points);
    return 1;
}

int plo_nql(int nr)
{
    int	j,k,nsta,humid,npoint,day,month,year,day_y,l,jj,vx,vx1,
        npointt,n,ws,ws1,x,y,POLY,hour,min,sek,jstep,jend,NSTA,xx;
    int	licz,xx1,NOR_POINT;
    long int nsat,mjd,secday,NSAT;

    double	temp,pres,RMS,mean_range,time,tim,period,clock_cor,vsum,sum_time,
        end_time, v_mean,vv_mean,mean_time,sec,omc,omcplus,omcminus,tbrb,
        czeb,omcskala,timskala,TB,RB,x1,s2,s1,summa,timm[60],rmss[60],xy;
    char	c,name[33],kodst[3],ss[70];
    char	namefe[9],namm[33],NAMSTAC[12],
        NAMSAT[12],NAMSYM[3],nam[3];
    char	header1[81];
    char	header2[81];

    std::ifstream obs_catalogue("D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    if (!obs_catalogue.is_open()) {
        std::cout << "Unable to open observations catalogue!" << std::endl;
        return 5;
    }
    j = -1;
    n = 0;
    std::string temp_line;
    while (std::getline(obs_catalogue, temp_line)) {
        sscanf(temp_line.c_str() , datamodels::FormatKAT_OBS_R,
                                &c, &ws, &ws1, &mjd, &secday, &nsat, &nsta, kodst, namefe,
                                &npoint, &temp, &pres, &humid, &TB, &RB, &RMS, &POLY, &clock_cor);
        n++;
        if (n == nr) {
            j = 1;
            break;
        }
    }
    obs_catalogue.close();
    if (j == -1) {
        std::cout << "No such observation in the catalogue!" << std::endl;
        return 5;
    }
    if (ws1 == -1) {
        std::cout << "Please, do this on copy." << std::endl;
        return 5;
    }
    if (npoint > 5000) {
        std::cout << "Too many measurements (MAX = 5000)" << std::endl;
        return 5;
    }

    std::ifstream satellit("D:\\LASER-2\\DATA\\SATELLIT");
    if (!satellit.is_open()) {
        std::cout << "Unable to open file satellit !" << std::endl;
        return 5;
    }
    j = -1;
    while(std::getline(satellit, temp_line)) {
        sscanf(temp_line.c_str(), datamodels::FormatSATELLIT_R,&NAMSAT,&NSAT,&xx1,&xx1,&xx1,&xx1,&NOR_POINT,&day_y);
        if (NSAT==nsat) {
            j=1;
            break;
        }
    }
    satellit.close();
    if (j == -1) {
        std::cout << "No satellite in the catalogue: " << nsat << std::endl;
        return 5;
    }
    period = static_cast<double>(day_y);

    std::ifstream station("D:\\LASER-2\\DATA\\STATION");
    if (!station.is_open()) {
        std::cout << "Unable to open file station !" << std::endl;
        return 5;
    }
    j = -1;
    std::getline(station, temp_line);
    sscanf(temp_line.c_str(), "%s%d%d%d%d", &NAMSTAC,&NSTA,&xx1,&xx1,&xx1);
    station.close();
    if (nsta == NSTA) j = 1;
    if (j == -1) {
        std::cout << "\n NO STATION  : " << nsta << " IN THE STATION CATALOGUE !\n";
        return 5;
    }

    utility::mjd_dat(day, month, year, mjd);
    hour = static_cast<int>(secday / 3600LL);
    min  = static_cast<int>(secday % 3600LL / 60LL);
    sek  = static_cast<int>(secday % 3600LL % 60LL);
    sprintf(header1,"ST:%-4s SAT: %-5s %4d-%02d-%02d%4d:%02d:%02d %4d POINTS",
        NAMSTAC,NAMSAT,year,month,day,hour,min,sek,npoint);
    sprintf(header2,"TBIAS:%6.1lfMS RBIAS:%6.1lfM STEP :%2d RMS:%5.2lf",
        TB,RB,POLY,RMS*15.);

    sprintf(namm,"D:\\LASER-2\\DATA\\%8s.p%2s",namefe,kodst);
    std::ifstream derivatives_file(namm);
    if (!derivatives_file.is_open()) {
        printf("\n  Unable to open file %8s.p%2s !\n", namefe, kodst);
        return 5;
    }
    QVector<double> X(npoint), Y(npoint);
    for (jj = 0; jj<npoint; ++jj) {
        std::getline(derivatives_file, temp_line);
        sscanf(temp_line.c_str(), datamodels::FormatPGA_R, &sec,&omc,&tbrb,&czeb);
        X[jj] = sec;
        Y[jj] = czeb;
    }
    derivatives_file.close();

    time = X.last() - X.first();
    if (time < 0.) time += 86400.;
    omcplus = omcminus = Y.first();
    for (jj = 1; jj < npoint; ++jj) {
        if(Y[jj] > omcplus)	 omcplus = Y[jj];
        if(Y[jj] < omcminus) 	 omcminus = Y[jj];
        if(X[jj] < X[0])   X[jj] += 86400.;
    }
    time = X.last() - X.first();
    omcplus =  fabs(omcplus);
    omcminus = fabs(omcminus);
    omc = (omcminus > omcplus) ? omcminus : omcplus;

    QVector<double> tab_time(npoint), v(npoint);
    l = 0;
    vx1 = static_cast<int>(X.first()/period);
    tab_time[l] = sum_time = X.first();
    vsum = v[l] = Y.first();
    end_time=static_cast<double>(vx1+1)*period;
    npointt = npoint+1;
    licz=0;
    summa=0.;

    for (j = 1; j<npointt; ++j) {
        if (j == npoint)
            vx = -1;
        else
            vx = static_cast<int>(X[j]/period);

        if (X[j] <= end_time && vx==vx1) {
            l++;
            tab_time[l] = X[j];
            sum_time += X[j];
            vsum += Y[j];
            v[l] = Y[j];
        } else {
            if (l==0) {
                v_mean = v[l];
                mean_time = tab_time[0];
                mean_range = v[0];
            } else {
                mean_time = sum_time/static_cast<double>(l+1);
                vv_mean = vsum/static_cast<double>(l+1);
                for (jj=0, vsum=0.; jj<=l; ++jj)
                    vsum += (v[jj]-vv_mean)*(v[jj]-vv_mean);
                v_mean = sqrt(vsum/static_cast<double>(l));
                tim = fabs(mean_time-tab_time[0]);

                for (k=1, jj=0; k<=l; ++k) {
                    xx = fabs(mean_time-tab_time[k]);
                    if (xx < tim) {
                        jj = k;
                        tim = xx;
                    }
                }
                mean_time = tab_time[jj];
                mean_range = vv_mean;
            }

            licz++;
            summa += mean_range;
            timm[licz-1] = mean_time;
            rmss[licz-1] = mean_range;
            l = 0;
            vv_mean = 0.;
            vx1 = static_cast<int>(X[j]/period);
            tab_time[l] = sum_time = X[j];
            vsum = v[l] = Y[j];
            end_time=static_cast<double>(vx+1)*period;
        }
    }

    QVector<double> Xl, Yl, XL, YL;
    if (licz > 1) {
        summa /= static_cast<double>(licz);
        for (j=0, xy=0.; j<licz; ++j)
            xy += (summa-rmss[j])*(summa-rmss[j]);
        xy = 2.5*sqrt(xy/static_cast<double>(licz-1));

        for (j=0; j<licz; ++j)
            if (fabs(rmss[j]-summa) < xy) {         // < 2.5 sigma
                Xl.push_back(timm[j]-X.first());
                Yl.push_back(rmss[j]);
            } else {                                // > 2.5 sigma
                XL.push_back(timm[j]-X.first());
                YL.push_back(rmss[j]);
            }
    }


    for (int i=1; i<X.size(); ++i)
        X[i] -= X[0];
    X[0] = 0;
    QCustomPlot* p = new QCustomPlot();
    p->addGraph();
    p->graph()->setData(X,Y);
    p->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, Qt::blue, PIXEL_SIZE));
    p->graph()->setLineStyle(QCPGraph::lsNone);
    p->graph()->rescaleAxes();
    p->addGraph();
    p->graph()->setData(Xl,Yl);
    p->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::green, PIXEL_SIZE*2));
    p->graph()->setLineStyle(QCPGraph::lsNone);
    p->graph()->rescaleAxes();
    p->addGraph();
    p->graph()->setData(XL,YL);
    p->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::green, PIXEL_SIZE*3));
    p->graph()->setLineStyle(QCPGraph::lsNone);
    p->graph()->rescaleAxes();
    p->xAxis->setLabel(QString("TIME [ S ]"));
    p->yAxis->setLabel(QString("RESIDUALS [ M ]"));
    p->plotLayout()->insertRow(0);
    p->plotLayout()->addElement(0, 0, new QCPTextElement(p, QString(header1), QFont("sans", 11, QFont::Bold)));
    p->plotLayout()->insertRow(0);
    p->plotLayout()->addElement(0, 0, new QCPTextElement(p, QString(header2), QFont("sans", 11, QFont::Bold)));
    p->resize(700,400);
    p->show();
    qApp->exec();

    std::cout << std::endl << " END PROC. plo_nql." << std::endl;
    delete p;
    return 1;
}

int pasint_a(int nr)
{
    int	i, j, j1, y, m, d, npoint, npointef, nsta, NSTA, ws, ws1, n, POLY, ampl, stepefe, index, nk, humid;
    int64_t jj, mjd, secday, nsat, NSAT, MJD0, hour, min, sek, sss;
    double rmax, rmin, hmax, hmin, s1, s2, tx, x1, x2, ss1,
        TB, RB, RMS, azym, elev, sec, range1, range, timest, longpas,
        timeefe, t10, t20, t21, tt0, tt1, tt2, re, omc, zn, clock_cor, temp, pres;
    char c, nam[33], namm[33], name[33], namefe[9], NAMSYM[3], NAMSAT[12], NAMSTAC[12], kodst[3];
    std::vector<double> defefe0(4001), defefe1(4001);
    std::vector<double> defobs0(10001), defobs1(10001);

    std::ofstream result("D:\\LASER-2\\DATA\\RESULT.DAT");
    std::ifstream obs_catalogue("D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    if (!obs_catalogue.is_open()) {
        std::cout << "Unable to open observations catalogue!" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    jj = 0;
    n = 0;
    std::string temp_line;
    while (std::getline(obs_catalogue, temp_line)) {
        sscanf(temp_line.c_str() , datamodels::FormatKAT_OBS_R,
                                &c, &ws, &ws1, &mjd, &secday, &nsat, &nsta, kodst, namefe,
                                &npoint, &temp, &pres, &humid, &TB, &RB, &RMS, &POLY, &clock_cor);
        n++;
        if (n == nr) {
            jj = -1;
            break;
        }
    }
    obs_catalogue.close();
    if (jj == 0) {
        std::cout << "No such observation in the catalogue!" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    if (ws1 == -1) {
        std::cout << "Please, do this on copy." << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    if (npoint > 5000) {
        std::cout << "Too many measurements (MAX = 5000)" << std::endl;
        result << "0" << std::endl;
        return 5;
    }

    sprintf(name, "D:\\LASER-2\\DATA\\%8s.c%2s", namefe, kodst);
    std::ifstream obs_copy(name);
    if (!obs_copy.is_open()) {
        printf("Unable to open FILE %8s.c%2s !\n", namefe, kodst);
        result << "0" << std::endl;
        return 5;
    }
    std::getline(obs_copy, temp_line);
    sscanf(temp_line.c_str(), datamodels::FormatCGA_R, &sec, &azym, &elev, &range1, &range, &ampl);

    timest = sec;
    defobs0[1] = sec;
    defobs1[1] = range;
    rmax = rmin = range;
    hmax = hmin = elev;

    for (i = 2; i <= npoint; i++) {
        std::getline(obs_copy, temp_line);
        sscanf(temp_line.c_str(), datamodels::FormatCGA_R, &sec, &azym, &elev, &range1, &range, &ampl);
        if (defobs0[1] > sec) sec += 86400.;
        defobs0[i] = sec;
        defobs1[i] = range;
        if (rmax < range) rmax = range;
        if (rmin > range) rmin = range;
        if (hmax > elev) hmax = elev;
        if (hmin < elev) hmin = elev;
    }
    obs_copy.close();

    longpas = (sec-timest)/60.;
    if (longpas < 0.) longpas += 1440.;

    sprintf(nam, "D:\\LASER-2\\DATA\\%8s.e%2s", namefe, kodst);
    std::ifstream efem_file(nam);
    if (!efem_file.is_open()) {
        printf("\n  NO EFEMERIDE %8s.e%2s ! ! !\n", namefe, kodst);
        result << "0" << std::endl;
        return 5;
    }
    i = 0;
    while(std::getline(efem_file, temp_line)) {
        i++;
        sscanf(temp_line.c_str(), datamodels::FormatEGA_R, &c, &sss, &azym, &elev, &ss1);
        defefe0[i] = static_cast<double>(sss);
        defefe1[i] = ss1;
    }
    efem_file.close();

    npointef = i;
    for(i = 2, ss1 = 0.; i <= npointef; i++) {
        if(defefe0[i] < defefe0[i-1])	ss1 = 86400.;
        defefe0[i] += ss1;
    }
    timeefe = defefe0[1];
    stepefe = 1;
    s1=0.; s2=0.;

    for(jj = 1; jj <= npoint; jj++)
    {
        sec = defobs0[jj];
        if((sec-timeefe) < 0.)	sec += 86400.;
        range = defobs1[jj];
        index = (static_cast<int>(sec-timeefe))/stepefe;
        if(index < 1) index=1;
        if(index >= npointef-1)	index = npointef-2;

        t10 = defefe0[index+1]-defefe0[index];
        t20 = defefe0[index+2]-defefe0[index];
        t21 = defefe0[index+2]-defefe0[index+1];
        tt0 = sec-defefe0[index];
        tt1 = sec-defefe0[index+1];
        tt2 = sec-defefe0[index+2];
        re = tt1*tt2/(t10*t20)*defefe1[index]-
             tt0*tt2/(t10*t21)*defefe1[index+1]+
             tt0*tt1/(t20*t21)*defefe1[index+2];
        omc = (range-re)*1000.;
        s1 += omc;
        s2 += omc*omc;
    }
    s2 -= s1*s1/static_cast<double>(npoint);

    j1 = 0;
    tx = 0.;
    for(j = j1; j <= 4; j++)
    {
        tx += utility::pot(10., -j);
        nk = 0;

e503:	nk++;
        x1 = 0.;
        x2 = 0.;
        for(jj = 1; jj <= npoint; jj++)
        {
            sec = defobs0[jj];
            if((sec-timeefe) < 0.)	sec += 86400.;
            range = defobs1[jj];
            index = (static_cast<int>(sec-timeefe))/stepefe;
            if(index < 1) index=1;
            if(index >= npointef-1)	index = npointef-2;

            t10 = defefe0[index+1]-defefe0[index];
            t20 = defefe0[index+2]-defefe0[index];
            t21 = defefe0[index+2]-defefe0[index+1];
            tt0 = sec+tx-defefe0[index];
            tt1 = sec+tx-defefe0[index+1];
            tt2 = sec+tx-defefe0[index+2];
            re = tt1*tt2/(t10*t20)*defefe1[index]-
                 tt0*tt2/(t10*t21)*defefe1[index+1]+
                 tt0*tt1/(t20*t21)*defefe1[index+2];
            omc = (range-re)*1000.;
            x1 += omc;
            x2 += omc*omc;
        }

        x1 /= static_cast<double>(npoint);
        x2 -= x1*x1*static_cast<double>(npoint);

        if(nk > 1)  goto e501;
        if(x2 < s2)	goto e502;
        zn = -1.;
        tx -= 2.*utility::pot(10., -j);
        goto e503;
e502:	zn = 1.;
e501:	if(x2 <= s2) goto e504;
        tx -= zn*utility::pot(10., -j);
        goto e303;
e504:	s1 = x1;
        s2 = x2;
        tx += zn*utility::pot(10., -j);
        goto e503;
e303:	continue;
    }

    tx = -0.3643/1000.;
    s1 = 32.7503;

    sprintf(namm, "D:\\LASER-2\\DATA\\%8s.p%2s", namefe, kodst);
    std::ofstream derivatives_file(namm);
    if (!derivatives_file.is_open()) {
        printf("\n  Unable to open file %8s.p%2s !\n", namefe, kodst);
        result << "0" << std::endl;
        return 5;
    }
    for(jj = 1; jj <= npoint; jj++)
    {
        sec = defobs0[jj];
        if((sec-timeefe) < 0.)	sec += 86400.;
        range = defobs1[jj];
        index = (static_cast<int>(sec-timeefe))/stepefe;
        if(index < 1) index=1;
        if(index >= npointef-1)	index = npointef-2;

        t10 = defefe0[index+1]-defefe0[index];
        t20 = defefe0[index+2]-defefe0[index];
        t21 = defefe0[index+2]-defefe0[index+1];
        tt0 = sec+tx-defefe0[index];
        tt1 = sec+tx-defefe0[index+1];
        tt2 = sec+tx-defefe0[index+2];
        re = tt1*tt2/(t10*t20)*defefe1[index]-
             tt0*tt2/(t10*t21)*defefe1[index+1]+
             tt0*tt1/(t20*t21)*defefe1[index+2];
        omc = (range-re)*1000.;
        x1 = omc - s1;
        tt0 = sec-defefe0[index];
        tt1 = sec-defefe0[index+1];
        tt2 = sec-defefe0[index+2];
        re = tt1*tt2/(t10*t20)*defefe1[index]-
             tt0*tt2/(t10*t21)*defefe1[index+1]+
             tt0*tt1/(t20*t21)*defefe1[index+2];
        omc = (range-re)*1000.;

        if (sec > 86400.) sec -= 86400.;
        char buf[256];
        sprintf(buf, datamodels::FormatPGA_W, sec, omc, x1, 0.);
        derivatives_file << buf << std::endl;
    }
    derivatives_file.close();

    obs_catalogue.open("D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    if (!obs_catalogue.is_open()) {
        std::cout << "Unable to open observations catalogue!" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    std::ofstream obs_catalogue_copy("D:\\LASER-2\\DATA\\KAT_OBS.ROB");
    if (!obs_catalogue_copy.is_open()) {
        std::cout << "Unable to write observations catalogue copy!" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    n = 0;
    while (std::getline(obs_catalogue, temp_line)) {
        n++;
        char buf[256];
        if (n == nr) {
            if (sscanf(temp_line.c_str(), datamodels::FormatKAT_OBS_R,
                       &c, &ws, &ws1, &mjd, &secday, &nsat, &nsta, kodst, namefe, &npoint,
                       &temp, &pres, &humid, &TB, &RB, &RMS, &POLY, &clock_cor) != 18) {
                std::cout << "\nERROR reading observations catalogue in pasint_a: line # " << n << std::endl;
                obs_catalogue.close();
                obs_catalogue_copy.close();
                result << "0" << std::endl;
                return 5;
            }
            ws = 2;
            TB = tx*1000.;
            RB = s1;
            sprintf(buf, datamodels::FormatKAT_OBS_W,
                        ws, ws1, mjd, secday, nsat, nsta, kodst, namefe, npoint, temp, pres, humid, TB,
                        RB, RMS, POLY, clock_cor);
            obs_catalogue_copy << buf << std::endl;
        } else {
            obs_catalogue_copy << temp_line << std::endl;
        }
    }
    obs_catalogue.close();
    obs_catalogue_copy.close();

    if (remove("D:\\LASER-2\\DATA\\KAT_OBS.DIC") !=0 ) {
        std::cout << " Unable to delete old FILE  KAT_OBS.DIC" << std::endl;
        return 5;
    }
    if (rename("D:\\LASER-2\\DATA\\KAT_OBS.ROB", "D:\\LASER-2\\DATA\\KAT_OBS.DIC") !=0 ) {
        printf("ERROR in renaming FILE !!\n"
            "KAT_OBS.ROB   ­  : KAT_OBS.DIC\n");
        return 5;
    }
    result << npointef << std::endl;
    return 1;
}

int pol_epaa(int nr)
{
    int	Q[22]={0,30,31,33,35,37,39,41,43,45,47,49,52,
        57,62,67,74,90,112,137,175,300};
    double FO1[51]=
        {0.0L,161.L,18.51L,10.13L,7.71L,6.61L,5.99L,5.39L,5.32L,5.12L,
         4.96L,4.84L,4.75L,4.67L,4.6L,4.54L,4.49L,
         4.45L,4.41L,4.38L,4.35L,4.32L,4.3L,4.28L,4.26L,4.24L,4.22L,
         4.21L,4.2L,4.18L,4.17L,4.15L,4.13L,4.11L,4.1L,4.08L,4.07L,
         4.06L,4.05L,4.04L,4.03L,4.02L,4.L,3.99L,3.98L,3.96L,3.94L,
         3.92L,3.91L,3.89L,3.86L};

    int	n,l1,i,k,j,j1,m,mx,k1,jx[16],wss[16],jj,ws,ws1,nsta,
        POLY,npoint,humid,kod;
    int64_t secday,mjd,nsat;
    double sl,sm,sa,ddd,xy,c[16],d1[16],omc[16],clock_cor,temp,pres;
    double TB,RB,RMS,timepas,xx,a[16],ss1,ss2;
    double jc,js,jy,jt,jf,j_c,j_s,j_y,j_t,j_f,j__c,j__s,j__y,j__t,j__f;
    char	_,namefe[9],name[33],kodst[3];
    double	c_vel=299792458.0L;

    std::vector<double> def0(2001);
    std::vector<double> def1(2001);
    double array[2001][15];
    double jjx[2001][15];

    for(i=0;i<16;i++)
    {
        wss[i]=-1;
        a[i]=c[i]=omc[i]=d1[i]=0.L;
    }

    std::ofstream result("D:\\LASER-2\\DATA\\RESULT.DAT");
    if (!result.is_open()) {
        std::cout << "Unable to open RESULT.DAT file for write!" << std::endl;
        return 5;
    }
    std::ifstream obs_catalogue("D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    if (!obs_catalogue.is_open()) {
        std::cout << "Unable to open observations catalogue!" << std::endl;
        result << "0" << std::endl;
        return 5;
    }

    jj = 0;
    n = 0;
    std::string temp_line;
    while (std::getline(obs_catalogue, temp_line)) {
        sscanf(temp_line.c_str(), datamodels::FormatKAT_OBS_R,
                                &_, &ws, &ws1, &mjd, &secday, &nsat, &nsta, kodst, namefe,
                                &npoint, &temp, &pres, &humid, &TB, &RB, &RMS, &POLY, &clock_cor);
        n++;
        if (n == nr) {
            jj = -1;
            break;
        }
    }

    printf("TB/RB - %lf/%lf \n", TB, RB);

    obs_catalogue.close();
    if (jj == 0) {
        std::cout << "No such observation in the catalogue!" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    if (ws1 == -1) {
        std::cout << "Please, do this on copy." << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    if (ws < 2) {
        std::cout << "This copy is not pre-processed." << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    if (npoint > 2000) {
        std::cout << "Too many measurements (MAX = 2000)" << std::endl;
        result << "0" << std::endl;
        return 5;
    }

    sprintf(name, "D:\\LASER-2\\DATA\\%8s.p%2s", namefe, kodst);
    std::ifstream derivatives_file(name);
    if (!derivatives_file.is_open()) {
        std::cout << "Unable to open file: " << name << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    for(i=1;i<=npoint;i++)
    {
        std::getline(derivatives_file, temp_line);
        sscanf(temp_line.c_str(),datamodels::FormatPGA_R,&ss1,&sa,&ss2,&ddd);
        def0[i]=ss1;
        def1[i]=ss2;
    }

    sl=0.;
    timepas=def0[npoint]-def0[1];
    if (timepas<0.) timepas += 86400.;
    sa=def0[1];
    js=jc=0.0;
    for(i=1;i<=npoint;i++)
    {
        sl += def1[i];
        xy=def0[i]-sa;
        if (fabs(xy)>80000.)	xy=def0[i]-sa+86400.;
        def0[i]=array[i][0]=xy/timepas;
        jy=jc+def1[i];
        jt=js+jy;
        jf=0.0;
        if(jy*js>0.0)
            jf=0.46*jt;
        jc=js-jt+jy;
        js=jt;
    }
    sl=js+jc;
    c[1]=sl/static_cast<double>(npoint);
    for(i=1;i<=npoint;i++)
    {
        xx=(def1[i]-c[1])*10000.;
        if(fabs(xx)>32222.)
            jjx[i][0]=32222;
        else
            jjx[i][0]=static_cast<int>(xx);
    }
    l1=npoint-1;
    if(l1>15)l1=15;

    for(j=2;j<=l1;j++)
    {
        j1=j-1;
        jx[j]=j1;
        if(j1!=1) {
            for(i=1;i<=npoint;i++)
            array[i][0]=def0[i]*array[i][0];
        }
        for(k=1;k<=j1;k++)
        {
            sl=sm=0.;
            jc=js=j_s=j_c=0.0;
            for(i=1;i<=npoint;i++)
            {
                sa=array[i][k-1];
                if (k==1) sa=1.L;
                sl+=sa*array[i][0];
                sm+=sa*sa;

                jy=jc+sa*sa;
                jt=js+jy;
                jf=0.0;
                if(jy*js>0.0)
                    jf=0.46*jt;
                jc=js-jt+jy;
                js=jt;

                j_y=j_c+sa*array[i][0];
                j_t=j_s+j_y;
                j_f=0.0;
                if(j_y*j_s>0.0)
                    j_f=0.46*j_t;
                j_c=j_s-j_t+j_y;
                j_s=j_t;

            }
            sm=js+jc;
            sl=j_s+j_c;
            a[k]=sl/sm;
        }
        j__s=j__c=j_c=j_s=0.0;

        for(i=1,sa=0.,sm=0.;i<=npoint;i++)
        {
            js=jc=0.0;
            for(m=1,sl=0.;m<=j1;m++)
            {
                xx=array[i][m-1];
                if (m==1) xx=1.L;
                sl+=a[m]*xx;
                jy=jc+a[m]*xx;
                jt=js+jy;
                jf=0.0;
                if(jy*js>0.0)
                    jf=0.46*jt;
                jc=js-jt+jy;
                js=jt;

            }
            sl=js+jc;
            xx=array[i][0];
            xx-=sl;
            sa+=def1[i]*xx;
            sm+=xx*xx;
            array[i][j-1]=xx;
            j__y=j__c+def1[i]*xx;
            j__t=j__s+j__y;
            j__f=0.0;
            if(j__y*j__s>0.0)
                j__f=0.46*j__t;
            j__c=j__s-j__t+j__y;
            j__s=j__t;
            j_y=j_c+xx*xx;
            j_t=j_s+j_y;
            j_f=0.0;
            if(j_y*j_s>0.0)
                j_f=0.46*j_t;
            j_c=j_s-j_t+j_y;
            j_s=j_t;
        }

        sa=j__s+j__c;
        sm=j_s+j_c;
        c[j]=sa/sm;

        j__s=j__c=0.0;
        for(i=1,sl=0.;i<=npoint;i++)
        {
            jc=js=0.0;
            for(k=1,sa=0.;k<=j;k++)
            {
                xx=array[i][k-1];
                if (k==1) xx=1.;
                sa=sa+c[k]*xx;
                jy=jc+c[k]*xx;
                jt=js+jy;
                jf=0.0;
                if(jy*js>0.0)
                    jf=0.46*jt;
                jc=js-jt+jy;
                js=jt;
            }
            sa=js+jc;
            sm=def1[i]-sa;
            sl+=sm*sm;
            j__y=j__c+sm*sm;
            j__t=j__s+j__y;
            j__f=0.0;
            if(j__y*j__s>0.0)
                j__f=0.46*j__t;
            j__c=j__s-j__t+j__y;
            j__s=j__t;

            xx=sm*10000.;
            if(fabs(xx)>32222.)
                jjx[i][j-1]=32222;
            else
                jjx[i][j-1]=static_cast<int>(xx);
        }
        sl=j__s+j__c;
        k=npoint-j;

        if(k <= 0) {
            std::cout << std::endl << "ERROR Number of points less than have been set." << std::endl;
            result << "0" << std::endl;
            return 5;
        }
        sm=static_cast<double>(k);
        d1[j]=sl/sm;
        if(j==2)	goto e50;
        k1=k;
        if(k<=30)	goto e21;
        if(k<=300)	goto e22;
        k1=50;
        goto e21;
    e22:	for(i=2;i<=21;i++)
        {
            k1=i;
            if(k>Q[i-1]&&k<=Q[i])	break;
        }
    e24:	k1+=28;
    e21:	if((d1[j-1]*(sm+1.)*sm/sl-sm)<=FO1[k1])	wss[j]=1;
        if(wss[j]==1&&wss[j-1]==1)	goto e25;

    e50:	continue;
    }
    e25:	m=j1+1;

    k=2;
    if (m>9) k=m-7;
    printf("\n            Residuals for polynomial step  - cm  \n");
    printf("k=%d m=%d\n",k,m);

    printf("\nNO/POLY ");
    for(j=k;j<=m;j++)
        printf("%8d",jx[j]);

    printf("\n VARIAN.  ");
    for(j=k;j<=m;j++)
        printf("%8.1lf",d1[j]*1.e4);

    printf("\n ST.DEV.  ");
    for(j=k;j<=m;j++)
        printf("%8.1lf",sqrt(static_cast<double>(d1[j]))*1.e2);

    printf("\n WS.      ");
    for(j=k;j<=m;j++)
        printf("%8d",wss[j]);
    printf("\n");

    for(j=k;j<=m;j++)
    {
        mx=m;
        if(wss[j]==1&&wss[j+1]==1)	mx=j-1;
        if(j+1==m)	break;
    }

    derivatives_file.clear();
    derivatives_file.seekg(0, std::ios_base::beg);
    std::ofstream derivatives_copy("D:\\LASER-2\\DATA\\copia.wie");
    if (!derivatives_copy.is_open()) {
        std::cout << "Unable to open working catalogue!" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    for(i=1;i<=npoint;i++)
    {
        std::getline(derivatives_file, temp_line);
        sscanf(temp_line.c_str(), datamodels::FormatPGA_R, &ss1,&sl,&ss2,&ddd);
        char buf[256];
        sprintf(buf,datamodels::FormatPGA_W,ss1,sl,ss2,static_cast<double>(jjx[i][mx-1])*1.e-4);
        derivatives_copy << buf << std::endl;
    }
    derivatives_copy.close();
    derivatives_file.close();

    RMS=sqrt(d1[mx]);
    POLY=mx-1;

    obs_catalogue.open("D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    if (!obs_catalogue.is_open()) {
        std::cout << "Unable to open observations catalogue!" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    std::ofstream obs_catalogue_copy("D:\\LASER-2\\DATA\\KAT_OBS.ROB");
    if (!obs_catalogue_copy.is_open()) {
        std::cout << "Unable to write observations catalogue copy!" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    n = 0;
    while (std::getline(obs_catalogue, temp_line)) {
        n++;
        char buf[256];
        if (n == nr) {
            if (sscanf(temp_line.c_str(), datamodels::FormatKAT_OBS_R,
                       &_, &ws, &ws1, &mjd, &secday, &nsat, &nsta, kodst, namefe, &npoint,
                       &temp, &pres, &humid, &TB, &RB, &sl, &jj, &clock_cor) != 18) {
                std::cout << "\nERROR reading observations catalogue in pol_epaa: line # " << n << std::endl;
                obs_catalogue.close();
                obs_catalogue_copy.close();
                result << "0" << std::endl;
                remove("D:\\LASER-2\\DATA\\KAT_OBS.ROB");
                remove("D:\\LASER-2\\DATA\\copia.wie");
                return 5;
            }
            ws = 3;
            sl=RMS*2.e+9/c_vel;
            jj=POLY;
            sprintf(buf, datamodels::FormatKAT_OBS_W,
                        ws, ws1, mjd, secday, nsat, nsta, kodst, namefe, npoint, temp, pres, humid, TB,
                        RB, sl, jj, clock_cor);
            obs_catalogue_copy << buf << std::endl;
        } else {
            obs_catalogue_copy << temp_line << std::endl;
        }
    }
    obs_catalogue.close();
    obs_catalogue_copy.close();

    remove(name);
    rename("D:\\LASER-2\\DATA\\copia.wie",name);

    remove("D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    rename("D:\\LASER-2\\DATA\\KAT_OBS.ROB",
        "D:\\LASER-2\\DATA\\KAT_OBS.DIC");

    result << npoint;
    return(1);
}

int graf_ep0(int nr)
{
    int	n,i,jj,ws,ws1,nsta,POLY,npoint,ampl,humid,
        maxplus,maxmin,i_step, np;
    std::vector<int64_t> ntab(19);
    int64_t nsat,secday,mjd;
    double	TB,RB,RMS,rms,mean_val,omc1,omc2,omc3,time,sec,azym,elev,temp,pres,
        range1,range,clock_cor,secobs,step,x,y,mi2,mi3,mi4,skew,kurt,mean;
    char	c,namefe[9],name[35],nam[35],kodst[3],namex[35];
    double	c_vel=299792458.0;
    std::vector<double> plo(5000);

    for(n=0;n<19;n++)  ntab[n]=0;

    std::ofstream result("D:\\LASER-2\\DATA\\RESULT.DAT");
    std::ofstream tmp("D:\\LASER-2\\DATA\\SKEW_KUR.DAT");
    if (!tmp.is_open()) {
        std::cout << "Unable to create file SKEW_KUR.DAT !" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    std::ifstream obs_catalogue("D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    if (!obs_catalogue.is_open()) {
        std::cout << "Unable to open observations catalogue!" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    jj = 0;
    n = 0;
    std::string temp_line;
    while (std::getline(obs_catalogue, temp_line)) {
        sscanf(temp_line.c_str() , datamodels::FormatKAT_OBS_R,
                                &c, &ws, &ws1, &mjd, &secday, &nsat, &nsta, kodst, namefe,
                                &npoint, &temp, &pres, &humid, &TB, &RB, &RMS, &POLY, &clock_cor);
        n++;
        if (n == nr) {
            jj = -1;
            break;
        }
    }
    obs_catalogue.close();
    if (jj == 0) {
        std::cout << "No such observation in the catalogue!" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    if (ws != 3) {
        std::cout << "This observation is not pre-processed!" << std::endl;
        result << "0" << std::endl;
        return 5;
    }

    sprintf(name, "D:\\LASER-2\\DATA\\%8s.p%2s", namefe, kodst);
    std::ifstream derivatives_file(name);
    if (!derivatives_file.is_open()) {
        std::cout << "Unable to open file: " << name << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    rms=RMS*2.5e-9*c_vel/2.;
    for(n = 1, np = 0; n <= npoint; n++)
    {
        std::getline(derivatives_file, temp_line);
        sscanf(temp_line.c_str(),datamodels::FormatPGA_R,&time,&omc1,&omc2,&omc3);
        plo[n] = omc3;
        if (fabs(omc3) <= rms) np++;
    }
    derivatives_file.close();

    printf("rms = %lf, left/total - %d/%d \n", rms, np, npoint);

    if (np < npoint) goto L_9;
    if(rms > 0.6) {	mean_val=47.5;	step=20.;	i_step=5;	}
    else {	mean_val=19.;	step=50.;	i_step=2;	}
    if(rms > 0.9) {	mean_val=95.0;	step=10.;	i_step=10;	}

    x=0.5/step;
    for(i=1 , maxplus=maxmin=0 ;i<=npoint; i++) {
        jj=9-static_cast<int>((plo[i]+x)*step);
        if(jj<0)
            maxplus=maxplus+1;
        else
            if(jj>19)
                maxmin=maxmin+1;
            else
                ntab[jj]=ntab[jj]+1;
    }

    for(i = 1,x = 0.;i <= npoint; i++)
        x += plo[i];
    x = x/static_cast<double>(npoint);
    for( i = 1, mi2 =0., mi3 =0., mi4 =0.; i<= npoint; i++) {
        y = plo[i] - x;
        mi2 += y * y;
        mi3 += y * y * y;
        mi4 += y * y * y * y;
    }
    mi2 = mi2/static_cast<double>(npoint);
    mi3 = mi3/static_cast<double>(npoint);
    mi4 = mi4/static_cast<double>(npoint);

    skew = ( mi3 *mi3 ) /( mi2 * mi2 *mi2 );
    kurt =  mi4 / ( mi2 * mi2 );

    sound5();
    c=getch();
    if(c==0x00)
        c=getch();

L_8:
    sprintf(namex,"CON");
    distrib(namex,mean_val,maxplus,maxmin,i_step,ntab,skew ,kurt);

    fflush(stdin);
    printf(" ACCEPT?  1 - YES , 2 - NO  , 3 -  2. sigma : ");
    fflush(stdin);
    scanf("%d",&jj);

    rms=RMS*2.e-9*c_vel/2.;
    if( jj==3 )	jj=1;
    if( jj==1 ) {
        result << " -99" << std::endl;
        tmp << skew << "  " << kurt << std::endl;
        return 1;
    }
    if( jj==2 ) {
        printf(" Define the range for good points +/- cm :  \n");
        fflush(stdin);
        scanf("%lf",&rms);
        rms*=0.01;
    }

L_9:
    sprintf(nam,"D:\\LASER-2\\DATA\\%8s.c%2s",namefe,kodst);
    derivatives_file.open(name);
    std::ifstream obs_copy(nam);
    if (!obs_copy.is_open()) {
        std::cout << "Unable to open file: " << nam << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    std::ofstream robcop("D:\\LASER-2\\DATA\\robcop.cop");
    if (!robcop.is_open()) {
        std::cout << "Unable to open file robcop.cop !" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    std::ofstream robcze("D:\\LASER-2\\DATA\\robcze.cze");
    if (!robcze.is_open()) {
        std::cout << "Unable to open file robcze.cze !" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    jj=0;	mean=0.0;
    for(i=1;i<=npoint;i++) {
        std::getline(derivatives_file, temp_line);
        sscanf(temp_line.c_str(),datamodels::FormatPGA_R,&time,&omc1,&omc2,&omc3);
        std::getline(obs_copy, temp_line);
        sscanf(temp_line.c_str(),datamodels::FormatCGA_R,&sec,&azym,&elev,&range1,&range,&ampl);

        if(fabs(omc3)<=rms) {
            jj++;
            if(jj == 1) secobs = sec;
            mean+=omc3*omc3;
            char buf[256];
            sprintf(buf,datamodels::FormatCOPY_W,sec,azym,elev,range1,range,ampl);
            robcop << buf << std::endl;
            sprintf(buf,datamodels::FormatPGA_W,sec,omc1,omc2,omc3);
            robcze << buf << std::endl;
        }
    }

    printf("rms = %lf, left/total - %d/%d \n", rms, jj, npoint);

    derivatives_file.close();
    obs_copy.close();
    robcop.close();
    robcze.close();

    if(jj==npoint) {
        remove("D:\\LASER-2\\DATA\\robcop.cop");
        remove("D:\\LASER-2\\DATA\\robcze.cze");
        printf("\n Last point wasn't deleted !\n");
        goto L_8;
    }
    printf("\n Deleted :%4d points Left : %4d points !\n",npoint-jj,jj);

    obs_catalogue.open("D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    if (!obs_catalogue.is_open()) {
        std::cout << "Unable to open observations catalogue!" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    std::ofstream obs_catalogue_copy("D:\\LASER-2\\DATA\\KAT_OBS.ROB");
    if (!obs_catalogue_copy.is_open()) {
        std::cout << "Unable to write observations catalogue copy!" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    n = 0;
    while (std::getline(obs_catalogue, temp_line)) {
        n++;
        char buf[256];
        if (n == nr) {
            if (sscanf(temp_line.c_str(), datamodels::FormatKAT_OBS_R,
                       &c, &ws, &ws1, &mjd, &secday, &nsat, &nsta, kodst, namefe, &npoint,
                       &temp, &pres, &humid, &TB, &RB, &RMS, &POLY, &clock_cor) != 18) {
                std::cout << "\nERROR reading observations catalogue in graf_ep0: line # " << n << std::endl;
                obs_catalogue.close();
                obs_catalogue_copy.close();
                result << "0" << std::endl;
                remove("D:\\LASER-2\\DATA\\KAT_OBS.ROB");
                remove("D:\\LASER-2\\DATA\\robcop.cop");
                remove("D:\\LASER-2\\DATA\\robcze.cze");
                return 5;
            }
            npoint=jj;
            if(( fabs(static_cast<double>(secday)-secobs)) > 80000.) mjd++;
            secday = static_cast<int64_t>(secobs);
            RMS=sqrt(mean/static_cast<double>(npoint-POLY))*2.e+9/c_vel;

            sprintf(buf, datamodels::FormatKAT_OBS_W,
                        ws, ws1, mjd, secday, nsat, nsta, kodst, namefe, npoint, temp, pres, humid, TB,
                        RB, RMS, POLY, clock_cor);
            obs_catalogue_copy << buf << std::endl;
        } else {
            obs_catalogue_copy << temp_line << std::endl;
        }
    }
    obs_catalogue.close();
    obs_catalogue_copy.close();

    remove("D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    rename("D:\\LASER-2\\DATA\\KAT_OBS.ROB",
        "D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    remove(nam);
    remove(name);
    rename("D:\\LASER-2\\DATA\\robcop.cop",nam);
    rename("D:\\LASER-2\\DATA\\robcze.cze",name);

    result << jj;
    return(1);

}

int del_npt(int nr)
{
    int	i,j,k,nsta,humid,npoint,day,month,year,day_y,l,jj,vx,vx1,
        npointt,n,ws,ws1,POLY,hour,min,sek,NSTA,xx;
    int	licz,ampl,ss1;
    int64_t nsat,mjd,secday,NSAT,secc;

    double	temp,pres,RMS,mean_range,tim,period,clock_cor,vsum,sum_time,timeb,
        timek,end_time,v_mean,vv_mean,mean_time,sec,omc1,omc2,omc3,TB,RB,x1,
        s2,s1,summa,azym,elev,range,range1,timm[60],rmss[60],xy,timdel[10];
    char	c,name[33],kodst[3],linia[130];
    char	namefe[9],namm[33],NAMSTAC[12],
        NAMSAT[12],NAMSYM[3],nam[33];

    std::vector<double> tab_time(5000), v(5000), plo0(10001), plo1(10001);
    std::vector<int> ind(10001);

    std::ofstream result("D:\\LASER-2\\DATA\\RESULT.DAT");
    if (!result.is_open()) {
        std::cout << "Unable to open RESULT.DAT file for write!" << std::endl;
        return 5;
    }
    std::ifstream obs_catalogue("D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    if (!obs_catalogue.is_open()) {
        std::cout << "Unable to open observations catalogue!" << std::endl;
        return 5;
    }
    j = -1;
    n = 0;
    std::string temp_line;
    while (std::getline(obs_catalogue, temp_line)) {
        sscanf(temp_line.c_str(), datamodels::FormatKAT_OBS_R,
                                &c, &ws, &ws1, &mjd, &secday, &nsat, &nsta, kodst, namefe,
                                &npoint, &temp, &pres, &humid, &TB, &RB, &RMS, &POLY, &clock_cor);
        n++;
        if (n == nr) {
            j = 1;
            break;
        }
    }
    obs_catalogue.close();

    if (j == -1) {
        std::cout << "No such observation in the catalogue!" << std::endl;
        return 5;
    }
    if (ws1 == -1) {
        std::cout << "Do it on copy!!" << std::endl;
        return 5;
    }
    if (ws < 3) {
        std::cout << "This copy is not ready!" << std::endl;
        return 5;
    }
    if (npoint > 10001) {
        std::cout << "Too many points!! MAX: 10000" << std::endl;
        return 5;
    }

    std::ifstream satellit("D:\\LASER-2\\DATA\\SATELLIT");
    if (!satellit.is_open()) {
        std::cout << "Unable to open file satellit !" << std::endl;
        return 5;
    }
    j = -1;
    while(std::getline(satellit, temp_line)) {
        sscanf(temp_line.c_str(), datamodels::FormatSATELLIT_R,&NAMSAT,&NSAT,&ss1,&ss1,&ss1,&ss1,&ss1,&day_y);
        if (NSAT==nsat) {
            j=1;
            break;
        }
    }
    satellit.close();

    if (j == -1) {
        std::cout << "No satellite in the catalogue: " << nsat << std::endl;
        return 5;
    }

    period = static_cast<double>(day_y);
    utility::mjd_dat(day,month,year,mjd);
    hour=static_cast<int>(secday/3600L);
    sek=static_cast<int>(secday-hour*3600L);
    min=sek/60;
    sek-=min*60;


    sprintf(namm,"D:\\LASER-2\\DATA\\%8s.p%2s",namefe,kodst);
    std::ifstream derivatives_file(namm);
    if (!derivatives_file.is_open()) {
        std::cout << "Unable to open file: " << namm << std::endl;
        return 5;
    }
    for(jj=1;jj<=npoint;jj++)
    {
        std::getline(derivatives_file, temp_line);
        sscanf(temp_line.c_str(),datamodels::FormatPGA_R,&sec,&omc1,&omc2,&omc3);
        plo0[jj]=sec;
        plo1[jj]=omc3;
    }
    derivatives_file.close();

    for(jj=2;jj<=npoint;jj++) {
        if(plo0[jj]<plo0[1])    plo0[jj]+=86400.;
    }

    l=1;
    vx1=static_cast<int>(plo0[1]/period);
    tab_time[l]=sum_time=plo0[1];
    vsum=v[l]=plo1[1];
    end_time=static_cast<double>(vx1+1)*period;
    npointt=npoint+1;
    licz=0;
    summa=0.;

    for(j=2;j<=npointt;j++) {
        if(j==npointt)
            vx=-1;
        else
            vx=static_cast<int>(plo0[j]/period);

        if(plo0[j]<=end_time && vx==vx1) {
            l++;
            tab_time[l]=plo0[j];
            sum_time+=plo0[j];
            vsum+=plo1[j];
            v[l]=plo1[j];
        } else {
            if(l==1) {
                v_mean=v[l];
                mean_time=tab_time[1];
                mean_range=v[1];
            } else {
                mean_time=sum_time/static_cast<double>(l);
                vv_mean=vsum/static_cast<double>(l);
                for(jj=1,vsum=0.;jj<=l;jj++)
                    vsum+=(v[jj]-vv_mean)*(v[jj]-vv_mean);

                v_mean=sqrt(vsum/static_cast<double>(l-1));
                tim=fabs(mean_time-tab_time[1]);

                for(k=2,jj=1;k<=l;k++) {
                    xx=fabs(mean_time-tab_time[k]);
                    if(xx<tim)
                    {	jj=k;	tim=xx;	}
                }
                mean_time=tab_time[jj];
                mean_range=vv_mean;
            }
            licz++;
            summa+=mean_range;
            timm[licz]=mean_time;
            rmss[licz]=mean_range;

            l=1;
            vv_mean=0.;
            vx1=static_cast<int>(plo0[j]/period);
            tab_time[l]=sum_time=plo0[j];
            vsum=v[l]=plo1[j];
            end_time=static_cast<double>(vx+1)*period;
        }
    }
    if(licz>1)
    {
        summa/=static_cast<double>(licz);
        for(j=1,xy=0.;j<=licz;j++)
            xy+=(summa-rmss[j])*(summa-rmss[j]);

        xy=2.*sqrt(xy/static_cast<double>(licz-1));
        for(j=1,k=0;j<=licz;j++)
            if(fabs(rmss[j]) > xy ) {
                k++;
                printf("\n Removed normal point %12.2lf %7.2lf ?\n",
                    timm[j],rmss[j]*100.);
                timdel[k]=timm[j];
            }
    }
    if (k == 0) {
        result << "-99";
        std::cout << "End of del_npt." << std::endl;
        return 1;
    }
    licz=k;

    sprintf(namm,"D:\\LASER-2\\DATA\\%8s.p%2s",namefe,kodst);
    sprintf(nam,"D:\\LASER-2\\DATA\\%8s.c%2s",namefe,kodst);
    derivatives_file.open(namm);
    if (!derivatives_file.is_open()) {
        std::cout << "Unable to open file: " << namm << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    std::ifstream obs_copy(nam);
    if (!obs_copy.is_open()) {
        printf("Unable to open FILE %8s.c%2s !\n", namefe, kodst);
        result << "0" << std::endl;
        return 5;
    }
    std::ofstream robcop("D:\\LASER-2\\DATA\\robcop.cop");
    if (!robcop.is_open()) {
        std::cout << "Unable to open file robcop.cop !" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    std::ofstream robcze("D:\\LASER-2\\DATA\\robcze.cze");
    if (!robcze.is_open()) {
        std::cout << "Unable to open file robcze.cze !" << std::endl;
        result << "0" << std::endl;
        return 5;
    }

    for(n=1;n<=npoint;n++)
        ind[n]=1;
    for(k=1;k<=licz;k++) {
        timeb=static_cast<double>(static_cast<int>(timdel[k]/period))*period;
        timek=timeb+period;
        for(n=1;n<=npoint;n++)
            if(plo0[n] > timeb && plo0[n] <= timek ) ind[n]=0;
    }

    for(i=1,jj=0;i<=npoint;i++) {
        std::getline(derivatives_file, temp_line);
        sscanf(temp_line.c_str(),datamodels::FormatPGA_R,&tim,&omc1,&omc2,&omc3);
        std::getline(obs_copy, temp_line);
        sscanf(temp_line.c_str(),datamodels::FormatCGA_R,&sec,&azym,&elev,&range1,&range,&ampl);

        if( ind[i] == 1) {
            jj++;
            if(jj == 1) secc = static_cast<int64_t>(sec);

            char buf[256];
            sprintf(buf,"%15.7lf%9.4lf%8.4lf%14.3lf%15.7lf%5d\n",
                sec,azym,elev,range1,range,ampl);
            robcop << buf;
            sprintf(buf,"%15.7lf%12.4lf%12.4lf%12.4lf\n",
                sec,omc1,omc2,omc3);
            robcze << buf;
        }
    }
    printf("\n Removed :%4d points  Left: %4d points !\n", npoint-jj,jj);

    obs_catalogue.open("D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    if (!obs_catalogue.is_open()) {
        std::cout << "Unable to open observations catalogue!" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    std::ofstream obs_catalogue_copy("D:\\LASER-2\\DATA\\KAT_OBS.ROB");
    if (!obs_catalogue_copy.is_open()) {
        std::cout << "Unable to write observations catalogue copy!" << std::endl;
        result << "0" << std::endl;
        return 5;
    }
    n = 0;
    while (std::getline(obs_catalogue, temp_line)) {
        n++;
        char buf[256];
        if (n == nr) {
            if (sscanf(temp_line.c_str(), datamodels::FormatKAT_OBS_R,
                       &c, &ws, &ws1, &mjd, &secday, &nsat, &nsta, kodst, namefe, &npoint,
                       &temp, &pres, &humid, &TB, &RB, &RMS, &POLY, &clock_cor) != 18) {
                std::cout << "\nERROR reading observations catalogue in graf_ep0: line # " << n << std::endl;
                obs_catalogue.close();
                obs_catalogue_copy.close();
                result << "0" << std::endl;
                remove("D:\\LASER-2\\DATA\\KAT_OBS.ROB");
                remove("D:\\LASER-2\\DATA\\robcop.cop");
                remove("D:\\LASER-2\\DATA\\robcze.cze");
                return 5;
            }
            npoint=jj;
            if( std::abs(secday-secc) > 80000L) mjd++;
            secday = secc;
            RMS=0.;
            POLY=0;
            ws=2;
            sprintf(buf, datamodels::FormatKAT_OBS_W,
                        ws, ws1, mjd, secday, nsat, nsta, kodst, namefe, npoint, temp, pres, humid, TB,
                        RB, RMS, POLY, clock_cor);
            obs_catalogue_copy << buf << std::endl;
        } else {
            obs_catalogue_copy << temp_line << std::endl;
        }
    }
    obs_catalogue.close();
    obs_catalogue_copy.close();
    remove(nam);
    remove(namm);
    rename("D:\\LASER-2\\DATA\\robcop.cop",nam);
    rename("D:\\LASER-2\\DATA\\robcze.cze",namm);
    remove("D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    rename("D:\\LASER-2\\DATA\\KAT_OBS.ROB",
        "D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    result << jj;
    return 1;
}

int cat_kboa(int64_t numpass)
{
    int	j,k,n,nsta,humid,npoint,sky,ws,ws1,rang_st,rang,rang_end,
        weather,elev_st,elev,elev_end,step_poly,good_flash2,
        good_flash1,flash1,flash2,filter1,filter2,
        cal_after,cal_before,ampl,nr,humid1,coef,dup;

    int64_t nsat,mjd,secday,numberpas,mjd1,tim_st_cal,tim_end_cal;

    double	max_o_c,min_o_c,time_bias,range_bias,clock_cor,dist_target1,
        long_pass,dist_target2,target,skew,kurt;
    double	temp,pres,rms,sk1,sk2,corect1,corect2,rmsk1,rmsk2,sec,
        omc,tbrb,czeb,azym,elevation,range1,range,time_start,
        time_end,temp1,press1;

    char	c,kodst[3],s[10],nam[3],namefe[9],namobs[33],nff[9],
        cc[3],namcop[33],nampol[33],namcal[13],namlog[33];

    double c_velocity=299792458.0;

    std::ifstream interimData("D:\\LASER-2\\DATA\\EPA_AUTO.DAT");
    if (!interimData.is_open()) {
        std::cout << "Unable to open file EPA_AUTO.DAT !" << std::endl;
        return 5;
    }
    std::string temp_line;
    std::getline(interimData, temp_line);
    sscanf(temp_line.c_str(), "%4d%6ld%4d%4d", &nr, &numpass, &cal_before, &cal_after);
    interimData.close();

    std::ifstream obs_catalogue("D:\\LASER-2\\DATA\\KAT_OBS.DIC");
    if (!obs_catalogue.is_open()) {
        std::cout << "Unable to open observations catalogue!" << std::endl;
        return 5;
    }
    sky = -1;
    n = 0;
    while (std::getline(obs_catalogue, temp_line)) {
        sscanf(temp_line.c_str(), datamodels::FormatKAT_OBS_R,
                                &c, &ws, &ws1, &mjd, &secday, &nsat, &nsta, kodst, namefe,
                                &npoint, &temp, &pres, &humid, &time_bias, &range_bias, &rms, &step_poly, &clock_cor);
        n++;
        if (n == nr) {
            sky = 1;
            break;
        }
    }
    obs_catalogue.close();
    strcpy(nff,namefe);
    if (sky == -1) {
        std::cout << "No such observation in the observation catalogue!" << std::endl;
        return 5;
    }
    if (ws1 == -1) {
        std::cout << "This is original data!" << std::endl;
        return 5;
    }
    if (ws < 3) {
        std::cout << "This observation is not ready for catalogization!" << std::endl;
        return 5;
    }

    sky = 0;
    sprintf(namobs,"D:\\LASER-2\\DATA\\%8s.k%2s",namefe,kodst);
    sprintf(namcop,"D:\\LASER-2\\DATA\\%8s.c%2s",namefe,kodst);
    sprintf(nampol,"D:\\LASER-2\\DATA\\%8s.p%2s",namefe,kodst);

    std::fstream k_obs(namobs, std::ios_base::in);
    if (k_obs.is_open()) {
        printf("\n File with name %s presented in the catalogue !\n",namobs);
        return 5;
    }
    k_obs.close();
    k_obs.open(namobs, std::ios_base::out);
    if (!k_obs.is_open()) {
        printf("Unable to open FILE %8s.k%2s !\n", namefe, kodst);
        return 5;
    }
    std::ifstream derivatives_file(nampol);
    if (!derivatives_file.is_open()) {
        printf("Unable to open FILE %8s.p%2s !\n", namefe, kodst);
        return 5;
    }
    std::ifstream obs_copy(namcop);
    if (!obs_copy.is_open()) {
        printf("Unable to open FILE %8s.c%2s !\n", namefe, kodst);
        return 5;
    }

    std::getline(derivatives_file, temp_line);
    sscanf(temp_line.c_str(),datamodels::FormatPGA_R,&sec,&omc,&tbrb,&czeb);
    std::getline(obs_copy, temp_line);
    sscanf(temp_line.c_str(),datamodels::FormatCGA_R,&sec,&azym,&elevation,&range,&range1,&ampl);
    elev_st=elev=static_cast<int>(elevation);
    rang_st=rang=static_cast<int>(range1);
    time_start=sec;
    max_o_c=static_cast<int>(omc);
    min_o_c=static_cast<int>(omc);
    char buf[256];
    sprintf(buf,"%20.12lf%9.4lf%8.4lf%14.3lf%5d%8.4lf\n",sec,azym,elevation,range,ampl,czeb*2.e9/c_velocity);
    k_obs << buf;
    for(k=2;k<=npoint;k++) {
        std::getline(derivatives_file, temp_line);
        sscanf(temp_line.c_str(),datamodels::FormatPGA_R,&sec,&omc,&tbrb,&czeb);
        std::getline(obs_copy, temp_line);
        sscanf(temp_line.c_str(),datamodels::FormatCGA_R,&sec,&azym,&elevation,&range,&range1,&ampl);

        if(static_cast<int>(elevation)>elev) elev=static_cast<int>(elevation);
        if(static_cast<int>(range1)<rang)    rang=static_cast<int>(range1);
        if(static_cast<int>(omc)>max_o_c)    max_o_c=static_cast<int>(omc);
        if(static_cast<int>(omc)<min_o_c)    min_o_c=static_cast<int>(omc);
        sprintf(buf,"%20.12lf%9.4lf%8.4lf%14.3lf%5d%8.4lf\n",sec,azym,elevation,range,ampl,czeb*2.e9/c_velocity);
        k_obs << buf;
    }
    k_obs.close();

    elev_end=static_cast<int>(elevation);
    rang_end=static_cast<int>(range1);
    time_end=sec;
    if(time_end<time_start) time_end+=86400.;
    long_pass=(time_end-time_start)/60.;

    if(cal_before==0) {
        tim_st_cal=0L;
        dist_target1=0.;
        flash1=good_flash1=0;
        filter1=0;
        sk1=0.;
        corect1=0.;
        rmsk1=0.;
    } else {
        std::ifstream cal_catalogue("D:\\LASER-2\\DATA\\KAT_KAL.DIC");
        if (!cal_catalogue.is_open()) {
            std::cout << "Unable to open file KAT_KAL.DIC !" << std::endl;
            remove(namobs);
            return 5;
        }
        nr=0;
        j=-1;
        while(std::getline(cal_catalogue, temp_line)) {
            sscanf(temp_line.c_str(), datamodels::FormatKAT_KAL_R, &c,&nsta,kodst,nam,&mjd1,&tim_st_cal,&filter1,&sk1,
                   &rmsk1,&corect1,&temp1,&press1,&humid1,&flash1,
                   &good_flash1,&weather,&numberpas,&coef,&skew,&kurt,&target,namcal);
            nr++;
            if (nr == cal_before) {
                j=1;
                break;
            }
        }
        cal_catalogue.close();

        if(j==-1) {
            printf(" No such calibration is presented ! ! ! \n");
            remove(namobs);
            return 5;
        }
    }
    dist_target1 = target;

    if(cal_after==0) {
        tim_end_cal=0L;
        dist_target2=0.;
        flash2=good_flash2=0;
        filter2=0;
        sk2=0.;
        corect2=0.;
        rmsk2=0.;
    } else {
        std::ifstream cal_catalogue("D:\\LASER-2\\DATA\\KAT_KAL.DIC");
        if (!cal_catalogue.is_open()) {
            std::cout << "Unable to open file KAT_KAL.DIC !" << std::endl;
            remove(namobs);
            return 5;
        }
        nr=0;
        j=-1;
        while(std::getline(cal_catalogue, temp_line)) {
            sscanf(temp_line.c_str(), datamodels::FormatKAT_KAL_R, &c,&nsta,kodst,nam,&mjd1,&tim_st_cal,&filter1,&sk1,
                   &rmsk1,&corect1,&temp1,&press1,&humid1,&flash1,
                   &good_flash1,&weather,&numberpas,&coef,&skew,&kurt,&target,namcal);
            nr++;
            if (nr == cal_before) {
                j=1;
                break;
            }
        }
        cal_catalogue.close();

        if(j==-1) {
            printf(" No such calibration is presented ! ! ! \n");
            remove(namobs);
            return 5;
        }
    }
    strcpy(namefe,nff);
    dist_target2 = target;

    sprintf(namlog,"D:\\LASER-2\\DATA\\%8s.l%2s",namefe,kodst);
    std::ofstream obs_log(namlog, std::ios_base::app);
    if (!obs_log.is_open()) {
        printf("Unable to open FILE %8s.l%2s !\n", namefe, kodst);
        remove(namobs);
        return 5;
    }
    sprintf(buf,"%6d\n",elev_st); obs_log << buf;
    sprintf(buf,"%6d\n",elev); obs_log << buf;
    sprintf(buf,"%6d\n",elev_end); obs_log << buf;
    sprintf(buf,"%7d\n",rang_st); obs_log << buf;
    sprintf(buf,"%7d\n",rang); obs_log << buf;
    sprintf(buf,"%7d\n",rang_end); obs_log << buf;
    sprintf(buf,"%7.1lf\n",long_pass); obs_log << buf;
    sprintf(buf,"%8.2lf\n",time_bias); obs_log << buf;
    sprintf(buf,"%8.2lf\n",range_bias); obs_log << buf;
    sprintf(buf,"%8.2lf\n",max_o_c); obs_log << buf;
    sprintf(buf,"%8.2lf\n",min_o_c); obs_log << buf;
    sprintf(buf,"%4d\n",step_poly); obs_log << buf;
    sprintf(buf,"%8.2lf\n",dist_target1); obs_log << buf;
    sprintf(buf,"%6lld\n",tim_st_cal); obs_log << buf;
    sprintf(buf,"%6d\n",good_flash1); obs_log << buf;
    sprintf(buf,"%6d\n",flash1); obs_log << buf;
    sprintf(buf,"%6d\n",filter1); obs_log << buf;
    sprintf(buf,"%8.2lf\n",dist_target2); obs_log << buf;
    sprintf(buf,"%6lld\n",tim_end_cal); obs_log << buf;
    sprintf(buf,"%6d\n",good_flash2); obs_log << buf;
    sprintf(buf,"%6d\n",flash2); obs_log << buf;
    sprintf(buf,"%6d\n",filter2); obs_log << buf;
    obs_log.close();

    std::ofstream kbo_catalogue("D:\\LASER-2\\DATA\\KAT_KBO.DIC", std::ios_base::app);
    if (!kbo_catalogue.is_open()) {
        printf("Unable to open FILE KAT_KBO.DIC !\n");
        remove(namobs);
        return 5;
    }
    sprintf(buf, datamodels::FormatKAT_KBO_W,
            numpass,nsat,nsta,kodst,mjd,secday,temp,pres,humid,
            npoint,rms,sky,cal_before,cal_after,namefe,clock_cor);
    kbo_catalogue << buf << std::endl;
    kbo_catalogue.close();
    return 1;
}

int npt_kbod(int64_t nr)
{
    int	i,j,k,coef,nsta,humid,npoint,weather,day,month,year,day_y,amplit,
        l,jj,vx,vx1,npointt,godz,min,sum_l,cal_before,cal_after,
        SYS_NUM,OCU_SEQ,NOR_POINT,EPO_EVENT,
        TIM_SCALE,ANGL_IND,TROP_CORR,MAS_COR_IND,AMPL_CORR,SYS_CALIB,
        CAL_INDIC,SYS_FLAG,FOR_REVIS,RELA_FLAG,ss1;
    int64_t numpass,nsat,mjd,secday,nr1,slong;

    char	c,s[3],kodst[3],namefill[13],namfil[35],ss[70];
    double temp,pres,rms,sk1,sk2,corect,corecta,corectb,rmsk1,rmsk2,
        time,tim,azym,elev,range,o_c,np_rms,mean,sig,npsig,sum_v,sum_vv,
        sum_np,fi,H,LAS_WAV,period,vsum,sum_time,end_time,np_l,
        mean_range,v_mean,vv_mean,mean_time,xx,sek,c_velocity,clock_cor;
    c_velocity=299792458.0;
    std::vector<double> tab_time(5000), v(5000), rang(5000);

    if (nr == 0) return 5; // exit(1) ??
    std::ofstream npt_data("D:\\LASER-2\\DATA\\NPT.DAT");
    std::ifstream kbo_catalogue("D:\\LASER-2\\DATA\\KAT_KBO.DIC");
    if (!kbo_catalogue.is_open()) {
        printf("Unable to open FILE KAT_KBO.DIC !\n");
        return 5;
    }
    j = -1;
    std::string temp_line;
    while(std::getline(kbo_catalogue, temp_line)) {
        sscanf(temp_line.c_str(), datamodels::FormatKAT_KBO_R,
               &c,&numpass,&nsat,&nsta,kodst,&mjd,&secday,&temp,&pres,&humid,
               &npoint,&rms,&weather,&cal_before,&cal_after,namefill,&clock_cor);

        if (nr == numpass) {
            j=1;
            break;
        }
    }
    kbo_catalogue.close();
    if (j==-1) {
        std::cout << "No such observation in the catalogue:  " << nr << std::endl;
        return 5;
    }

    coef = nsta;
    std::ifstream satellit("D:\\LASER-2\\DATA\\SATELLIT");
    if (!satellit.is_open()) {
        std::cout << "Unable to open file satellit !" << std::endl;
        return 5;
    }
    j = -1;
    while(std::getline(satellit, temp_line)) {
        sscanf(temp_line.c_str(), datamodels::FormatSATELLIT_R,&namfil,&nr1,&ss1,&ss1,&ss1,&ss1,&NOR_POINT,&day_y);
        if (nr1==nsat) {
            j=1;
            break;
        }
    }
    satellit.close();
    if (j==-1) {
        std::cout << "No such satellite in the catalogue:  " << namfil << std::endl;
        return 5;
    }

    utility::mjd_dat(day,month,year,mjd);
    printf("                    N O R M A L  P O I N T S  D A T A\n\n");
    printf("                     %s   %4d %02d %02d    %5d",namfil,year,month,day,numpass);
    printf("\n");
    printf("  NR       EPOCH              RANGE        FR     RMS     MRMS      N\n");
    printf("                               km          cm      cm      cm\n");

    char buf[256];
    sprintf(buf,"                    N O R M A L  P O I N T S  D A T A\n\n");                    npt_data << buf;
    sprintf(buf,"                     %s   %4d %02d %02d    %5d",namfil,year,month,day,numpass); npt_data << buf;
    sprintf(buf,"\n\n");                                                                         npt_data << buf;
    sprintf(buf,"  NR       EPOCH              RANGE        FR     RMS     MRMS      N\n");      npt_data << buf;
    sprintf(buf,"                               km          cm      cm      cm\n");              npt_data << buf;

    period = static_cast<double>(day_y);
    day_y = utility::day_year(day,month,year);
    corect=(corectb+corecta)*0.5;
    coef=1;
    if(corecta==0.)coef=-1;
    if(corectb==0.)coef=-1;

    sprintf(namfil,"D:\\LASER-2\\DATA\\%8s.k%2s",namefill,kodst);
    std::ifstream k_obs(namfil);
    if (!k_obs.is_open()) {
        std::cout << "Unable to open FILE " << namefill << std::endl;
        return 5;
    }
    i=0;
    sum_v=0.;
    sum_vv=0.;
    sum_np=0.;
    sum_l=0;
    std::getline(k_obs, temp_line);
    sscanf(temp_line.c_str(), "%lf%lf%lf%lf%d%lf", &time,&azym,&elev,&range,&amplit,&o_c);
    vx1=static_cast<int>(time/period);
    l=1;
    tab_time[l]=sum_time=time;
    vsum=v[l]=o_c;
    rang[l]=range;
    end_time=static_cast<double>(vx1+1)*period;
    npointt=npoint+1;
    if (static_cast<double>(secday) > time) day_y++;

    for(j=2;j<=npointt;j++) {
        if(j==npointt)
            vx=-1;
        else {
            std::getline(k_obs, temp_line);
            sscanf(temp_line.c_str(),"%lf%lf%lf%lf%d%lf",&time,&azym,&elev,&range,&amplit,&o_c);
            vx=static_cast<int>(time/period);
        }
        if(time<=end_time&&vx==vx1) {
            l++;
            tab_time[l]=time;
            sum_time=sum_time+time;
            vsum=vsum+o_c;
            v[l]=o_c;
            rang[l]=range;
        } else {
            if(l==1) {
               v_mean=rms;
               mean_time=tab_time[1];
               mean_range=rang[1]-corect;
               vv_mean=vsum;
            } else {
                mean_time=sum_time/static_cast<double>(l);
                vv_mean=vsum/static_cast<double>(l);
                for(jj=1,vsum=0.;jj<=l;jj++)
                    vsum+=(v[jj]-vv_mean)*(v[jj]-vv_mean);
                v_mean=sqrt(vsum/static_cast<double>(l));
                tim=fabs(mean_time-tab_time[1]);
                jj=1;
                for(k=2;k<=l;k++) {
                    xx=fabs(mean_time-tab_time[k]);
                    if(xx<tim)
                        {jj=k; tim=xx;}
                }
                mean_time=tab_time[jj];
                mean_range=rang[jj]-corect-(v[jj]-vv_mean);
            }
            mean_range *= 1.E-9*c_velocity*1.E-3/2.;
            v_mean *= 1.E-9*c_velocity*1.E2/2.;
            vv_mean *= 1.E-9*c_velocity*1.E2/2.;
            np_rms=v_mean/sqrt(static_cast<double>(l));
            utility::sgms(godz,min,sek,mean_time);
            i++;
            sum_v += vv_mean;
            sum_vv += vv_mean*vv_mean;
            sum_np += np_rms*np_rms;
            sum_l += l;
            sprintf(ss,"  %2d   %02d:%02d:%010.7lf   %12.6lf   %5.2lf   %5.2lf   %5.2lf   %4d",
            i,godz,min,sek,mean_range,vv_mean,v_mean,np_rms,l);
            std::cout << ss << std::endl;
            npt_data << ss << std::endl;

            l=1;
            vv_mean=0.;
            vx1=static_cast<int>(time/period);
            tab_time[l]=sum_time=time;
            vsum=v[l]=o_c;
            rang[l]=range;
            end_time=static_cast<double>(vx+1)*period;
        }
    }

    np_l=static_cast<double>(sum_l)/static_cast<double>(i);
    mean=sum_v/static_cast<double>(i);
    sig=sqrt((sum_vv-sum_v*sum_v/i)/static_cast<double>(i-1));
    npsig=sqrt(sum_np/static_cast<double>(i));
    rms *= 1.E-9*c_velocity*1.E2/2.;
    printf("\n  %2d                              MEAN   %5.2lf   %5.2lf   %5.2lf    %5.1lf",
        i,mean,rms,npsig,np_l);
    printf("\n%4d                            FR RMS   %5.2lf",sum_l,sig);
    sprintf(buf,"\n  %2d                              MEAN   %5.2lf   %5.2lf   %5.2lf    %5.1lf",
        i,mean,rms,npsig,np_l);
    npt_data << buf;
    sprintf(buf,"\n%4d                            FR RMS   %5.2lf",sum_l,sig);
    npt_data << buf;
    if(nr1==7603901L||nr1==9207002L) {
        sprintf(buf,"\n\n\n\n\n\n                    UTX/CSR LAGEOS NORMAL POINTS ANALYSIS\n"); npt_data << buf;
        sprintf(buf,"\n\n      EDITED POINTS  :...........................................");   npt_data << buf;
        sprintf(buf,"\n\n      PRECISION (cm) :...........................................");   npt_data << buf;
        sprintf(buf,"\n\n      RAW RMS (cm)   :...........................................");   npt_data << buf;
        sprintf(buf,"\n\n      RANGE BIAS (cm):...........................................");   npt_data << buf;
        sprintf(buf,"\n\n      TIME BIAS (us) :...........................................");   npt_data << buf;
    }
    npt_data << std::endl << std::endl;
    return 1;
}

void nql_kbo(int64_t nr)
{
    int	i,j,k,coef,nsta,humid,npoint,weather,day,month,year,day_y,
        amplit,icoef,check_sum,num_obs,l,jj,vx,vx1,npointt,year_cent,
        SYS_NUM,OCU_SEQ,NOR_POINT,EPO_EVENT,TIM_SCALE,ANGL_IND,TROP_CORR,
        MAS_COR_IND,AMPL_CORR,SYS_CALIB,CAL_INDIC,SYS_FLAG,FOR_REVIS,
        RELA_FLAG,flaglog=1,cal_before,cal_after,flag_cal,gf;
    int64_t ne[40],numpass,nsat,mjd,secday,nr1,sat_id,lenth;
    double	temp,pres,rms,sk1,sk2,corect,corecta,corectb,rmsk1,rmsk2,
        rms_cal,time1,tim,azym,elev,range,o_c,shift,fi,
        H,LAS_WAV,period,vsum,sum_time,end_time,time_x,
        mean_range,v_mean,vv_mean,mean_time,xx,clock_cor;
    double  mi1,mi2,mi3,mi4,mi_tim[5000],mean_peak,mean_rang;
    int	num[60],numer,CAL_IND;
    double timm[60],ran[60],rmss[60],summa,rmes[60],speed_light,peak;
    char	c,name[35],s[3],kodst[3],namefill[9],namfil[35],ss[70],ss1[2];
    char    s1_crd[48],shor[4],satn[11],cal_str[6];
    char name1[41],name2[41],name3[41],name4[41],namkal[9],nam[3],name5[63],name6[63];
    int st_num,st_sys,st_occ,st_tim,trop_corr,cmas_corr,ampl_corr,sys_del,hz,range_type;
    int hour,minut,rr,cospar,sic,transponder,i_flag,rang_typ,dig,sign,ii,ik;
    char st_name[5],*str_num,*sss,str2[22],sat_nam[11];
    int  data_qv,m_relis,day_s,month_s,year_s,day_f,month_f,year_f,N_pulse;
    int64_t norad,mjd_f,mjd_s;
    double tim1,tim2,hour1,hour2,sec1,sec2,minut1,minut2;
    double tr_wave,p_wave,fire_rate,pulse_width,target_c1,target_c2;
    float beam,a_w,qv,d_c,o_p,s_f1,tr_s,s_f2,e_delay,p_energy,volt,k_urt,s_kew;
    char sys_conf[5],a_conf[5],b_conf[5],c_conf[5],d_conf[5],laser_type[11],d_type[11],o_type[11],esp[11];
    char t_source[21],f_sourse[21],timer[21],s_num[21],namcal[13],str_name[9];
    double time2,time3,cal_dist,skew1,skew2,kurt1,kurt2,cal_peak1,cal_peak2,rmsk_c1,rmsk_c2,temp_c,press_c,humid_c,sk_c1,sk_c2;
    float las_en, feu_volt;
    int cal_type,cal_n1,cal_n2,cal_shift,np_window,filter1,filter2,flag;
    int64_t mjd_c1,mjd_c2,tim_st_cal1,tim_st_cal2;
    int cal_type_ind,cal_shift_ind,det_canal,flash1,good_flash1,flash2,good_flash2,namberpass,coef_c;
    double tab_time[5000],v[5000],rang[5000];
    std::string temp_line;
    ne[0] = nr;
    speed_light = 299792458.;
    num_obs = k = 0;

    sprintf(name,"D:\\LASER-2\\DATA\\SEND\\EDC.NPT");
    std::ofstream edc_npt(name);
    if (!edc_npt.is_open()) {
        std::cout << "Unable to open FILE D:\\LASER-2\\DATA\\SEND\\EDC.NPT !" << std::endl;
        return; // exit(1)
    }
    std::ifstream satellit("D:\\LASER-2\\DATA\\SATELLIT");
    if (!satellit.is_open()) {
        std::cout << "Unable to open file satellit !" << std::endl;
        return; // exit(1)
    }
    std::ifstream station("D:\\LASER-2\\DATA\\STATION");
    if (!station.is_open()) {
        std::cout << "Unable to open file station !" << std::endl;
        return; // exit(1)
    }
    flag = 1;
    std::ifstream skew_kur("D:\\LASER-2\\DATA\\SKEW_KUR.DAT");
    if (!skew_kur.is_open()) {
        std::cout << "Unable to open FILE SKEW_KUR.DAT" << std::endl;
        flag = 0;
    }
    if (flag) {
        std::getline(skew_kur, temp_line);
        sscanf(temp_line.c_str(), "%f %f",&s_kew,&k_urt);
        skew_kur.close();
    }

    std::getline(station, temp_line);
    sscanf(temp_line.c_str(),"%4s%d%d%d%d",&st_name,&st_num,&st_sys,&st_occ,&st_tim);
    std::getline(station, temp_line);
    sscanf(temp_line.c_str(),"%d%d%d%d%d%d%d",&trop_corr,&cmas_corr,&ampl_corr,&sys_del,&hz,&rang_typ,&data_qv);
    str_num = ecvt(static_cast<double>(st_num),4,&dig,&sign);
    sprintf(name1,"D:\\LASER-2\\DATA\\SEND\\%4s_crd.npt",str_num);
    sprintf(name2,"D:\\LASER-2\\DATA\\SEND\\%4s_crd.frd",str_num);
    std::ofstream npt_crd(name1);
    if (!npt_crd.is_open()) {
        std::cout << "Unable to write the file " << name1 << std::endl;
        return; // exit(1)
    }
    std::ofstream frd_crd(name2);
    if (!frd_crd.is_open()) {
        std::cout << "Unable to write the file " << name2 << std::endl;
        return; // exit(1)
    }
    std::ifstream las_feu("D:\\LASER-2\\DATA\\las_feu.dat");
    if (!las_feu.is_open()) {
        std::cout << "Unable to open FILE D:\\LASER-2\\DATA\\las_feu.dat !" << std::endl;
        return; // exit(1)
    }
    std::ofstream tmp1("D:\\LASER-2\\DATA\\1.TMP"), tmp2("D:\\LASER-2\\DATA\\2.TMP");

    SYS_NUM=st_sys;
    OCU_SEQ=st_occ;
    std::ifstream s_conf("D:\\LASER-2\\DATA\\SYS_CONF");
    if (!s_conf.is_open()) {
        std::cout << "Unable to open configuration description file SYS_CONF!" << std::endl;
        return; // exit(1)
    }
    std::getline(s_conf, temp_line);
    sscanf(temp_line.c_str(),"%lf%s%s%s%s%s",&tr_wave,&sys_conf,&a_conf,&b_conf,&c_conf,&d_conf);
    std::getline(s_conf, temp_line);
    sscanf(temp_line.c_str(),"%s%lf%lf%lf%f%d",&laser_type,&p_wave,&fire_rate,&pulse_width,&beam,&N_pulse);
    std::getline(s_conf, temp_line);
    sscanf(temp_line.c_str(),"%s %f %f %f %s %f %f %f %f %s",&d_type,&a_w,&qv,&d_c,&o_type,&o_p,&s_f1,&tr_s,&s_f2,&esp);
    std::getline(s_conf, temp_line);
    sscanf(temp_line.c_str(),"%s %s %s %s %f",&t_source,&f_sourse,&timer,&s_num,&e_delay);
    LAS_WAV=tr_wave/1000.0;

    printf("\n FILE was created :%13s\n",name);
    printf("\n FILE was created :%13s\n",name1);
    printf("\n FILE was created :%13s\n",name2);

    time_t t = time(nullptr);
    struct tm tm = *localtime(&t);
    char buf[256];
    sprintf(buf,"H1 CRD 01 %04d %02d %02d %02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour);
    npt_crd << buf << std::endl;
    frd_crd << buf << std::endl;
    sprintf(buf,"H2 %10s %04d %02d %02d %02d",st_name,st_num,st_sys,st_occ,st_tim);
    npt_crd << buf << std::endl;
    frd_crd << buf << std::endl;


    for(i=0;i<=num_obs;i++) {
        satellit.clear();
        satellit.seekg(0, std::ios_base::beg);
        nr = ne[i];
        std::ifstream kbo_catalogue("D:\\LASER-2\\DATA\\KAT_KBO.DIC");
        if (!kbo_catalogue.is_open()) {
            printf("Unable to open FILE KAT_KBO.DIC !\n");
            return; // exit(1)
        }
        j=-1;
        while(std::getline(kbo_catalogue, temp_line)) {
            sscanf(temp_line.c_str(), datamodels::FormatKAT_KBO_R,
                   &c,&numpass,&nsat,&nsta,kodst,&mjd,&secday,&temp,&pres,&humid,
                   &npoint,&rms,&weather,&cal_before,&cal_after,namefill,&clock_cor);
            if (nr == numpass) {
                j=1;
                break;
            }
        }
        kbo_catalogue.close();

        sk1=sk2=rmsk1=rmsk2=corecta=corectb=0.0;
        flag_cal=0;
        if(cal_before!=0) flag_cal++;
        if(cal_after!=0) flag_cal++;

        if(j==-1) {
            printf("\nNo such observation in the catalogue:%7lld",nr);
            tmp1 << "0" << std::endl;
            return; // exit(1)
        }
        sprintf(buf,"%8s",namefill);
        tmp1 << buf << std::endl;
        tmp1.close();

        if(flag_cal!=0) {
            std::ifstream cal_catalogue("D:\\LASER-2\\DATA\\KAT_KAL.DIC");
            if (!cal_catalogue.is_open()) {
                std::cout << "Unable to open calibrations catalogue!" << std::endl;
                return; // exit(1)
            }
            j=-1;
            for(nr=1;nr<32000;nr++) {
                std::getline(cal_catalogue, temp_line);
                sscanf(temp_line.c_str(),datamodels::FormatKAT_KAL_R,
                    &c,&nsta,kodst,nam,&mjd_c1,&tim_st_cal1,&filter1,&sk1,
                    &rmsk1,&corecta,&temp_c,&press_c,&humid_c,&flash1,
                    &gf,&weather,&namberpass,&coef_c,&skew1,&kurt1,&target_c1,namcal);
                    good_flash1=gf;
                    if(nr==cal_before) {
                        j=1;
                        break;
                    }
            }
            strcpy(s,kodst);
            if(j==-1) {
                printf("Kalibration %4d not found in kat_kal.dic\n",cal_before);
                return; // exit(1)
            }

            cal_catalogue.clear();
            cal_catalogue.seekg(0, std::ios_base::beg);
            nr=0;
            j=-1;
            for(nr=1;nr<32000;nr++) {
                std::getline(cal_catalogue, temp_line);
                sscanf(temp_line.c_str(),datamodels::FormatKAT_KAL_R,
                &c,&nsta,kodst,nam,&mjd_c2,&tim_st_cal2,&filter2,&sk2,
                &rmsk2,&corectb,&temp_c,&press_c,&humid_c,&flash2,
                &gf,&weather,&namberpass,&coef_c,&skew2,&kurt2,&target_c2,namcal);
                good_flash2=gf;
                if(nr==cal_after) {
                    j=1;
                    break;
                }
            }
            if(j==-1) {
                printf("Kalibration %4d not found in kat_kal.dic\n",cal_after);
                return; // exit(1)
            }
        }
        coef=nsta;

        j=-1;
        while(std::getline(satellit, temp_line)) {
            sscanf(temp_line.c_str(), datamodels::FormatSATELLIT_R,&sat_nam,&sat_id,
                   &sic,&norad,&transponder,&i_flag,&NOR_POINT,&day_y,&shor);
            np_window = day_y;
            if (sat_id == nsat) {
                j=1;
                break;
            }
        }
        satellit.clear();
        satellit.seekg(0, std::ios_base::beg);
        if(j==-1) {
            std::cout << "No satellite # " << nsat << " in file SATELLIT !" << std::endl;
            return; // exit(1)
        }

        period=static_cast<double>(day_y);
        edc_npt << "99999" << std::endl;
        sprintf(buf,"H3 %10s % 8lld %04d %08lld %1d %1d",sat_nam,sat_id,sic,norad,transponder,i_flag);
        npt_crd << buf << std::endl;
        frd_crd << buf << std::endl;


        utility::mjd_dat(day,month,year,mjd);
        day_y = utility::day_year(day,month,year);        /*dzien roku  */
        year_cent=year-(year/100)*100;
        sk_c1=sk1; sk_c2=sk2;
        rmsk_c1=rmsk1; rmsk_c2=rmsk2;

        if(flag_cal==2) {
            shift=fabs(sk1-sk2);
            corect=(corectb+corecta)*0.5;
            rms_cal=(rmsk1+rmsk2)*0.5;
        } else {
            shift=0;
        }
        if(flag_cal==1) {
          corect=corectb+corecta;
          rms_cal=rmsk1+rmsk2;
        }
        if(flag_cal==0) {
            corect=0;
            rms_cal=0;
        }

        TIM_SCALE=7; SYS_CALIB=0;CAL_INDIC=2; SYS_FLAG=2;
        sprintf(ss,"%.7lld%02d%03d%4d%02d%02d%4.0lf%08.0lf%06.0lf"
            "%04.0lf%1d%1d%1d%1d%1d%04.0lf0",
            nsat,year_cent,day_y,nsta,SYS_NUM,OCU_SEQ,
            LAS_WAV*1.e4,corect*1000.,shift*1000.,rms_cal*1000.,
            NOR_POINT,TIM_SCALE,SYS_CALIB,CAL_INDIC,
            SYS_FLAG,rms*1000.);

        check_sum=0;
        vv_mean=0.;
        for(j=0;j<=51;j++) {
            ss1[0]=ss[j];
            check_sum += atoi(ss1);
        }

        sprintf(buf,"%s%02d%d",ss,check_sum%100,flaglog);
        edc_npt << buf << std::endl;
        j=-1;
        las_feu.clear();
        las_feu.seekg(0, std::ios_base::beg);
        while (std::getline(las_feu, temp_line)) {
            sscanf(temp_line.c_str(),"%9s %f %f\n",&str_name,&las_en,&feu_volt);
            if(strncmp(str_name,namefill,9)==0) {
                j=1;
                break;
            }
        }
        if(j==-1) {
            printf("\n Energy laser and feu volage not found for pass %s",namefill);
            printf("\n Please, input laser energy");
            std::cin >> las_en;
            printf("\n Please, input FEU voltage");
            std::cin >> feu_volt;
        }

        sprintf(namfil,"D:\\LASER-2\\DATA\\%8s.k%2s",namefill,kodst);
        std::ifstream k_obs(namfil);
        if (!k_obs.is_open()) {
            std::cout << "Unable to open FILE" << namfil << std::endl;
            return; // exit(1)
        }
        std::getline(k_obs, temp_line);
        sscanf(temp_line.c_str(),"%lf%lf%lf%lf%d%lf",&time_x,&azym,&elev,&range,&amplit,&o_c);
        npointt=npoint;
        tim1=time_x;
        sprintf(buf,"time_x start=%lf",time_x);
        tmp2 << buf << std::endl;

        for(j=2;j<=npointt;j++) {
            std::getline(k_obs, temp_line);
            sscanf(temp_line.c_str(),"%lf%lf%lf%lf%d%lf",&time_x,&azym,&elev,&range,&amplit,&o_c);
        }
        tim2=time_x+1;
        sprintf(buf,"time_x finish=%lf",time_x);
        tmp2 << buf << std::endl;
        mjd_f=mjd_s=mjd;
        if(tim2<tim1) mjd_f++;
        utility::mjd_dat(day_s,month_s,year_s,mjd_s);
        utility::mjd_dat(day_f,month_f,year_f,mjd_f);
        sprintf(buf,"tim1=%.lf  tim2=%.lf",tim1,tim2);
        tmp2 << buf << std::endl;

        hour1=floor(tim1/3600);
        tim1 -= hour1*3600;
        minut1=floor(tim1/60);
        tim1 -= minut1*60;
        sec1=floor(tim1);
        hour2=floor(tim2/3600);
        tim2 -= hour2*3600;
        minut2=floor(tim2/60);
        tim2 -= minut2*60;
        sec2=floor(tim2);
        m_relis=0;

        sprintf(buf,"H4 01 %4d %02d %02d %2.lf %2.lf %2.lf %04d %02d %02d %2.lf %2.lf %2.lf %02d %1d %1d %1d %1d %1d %1d %1d",
                 year_s,month_s,day_s,hour1,minut1,sec1,year_f,month_f,day_f,hour2,minut2,sec2,m_relis,
                 trop_corr,cmas_corr,ampl_corr,sys_del,hz,rang_typ,data_qv);
        npt_crd << buf << std::endl;
        sprintf(buf,"H4 00 %4d %02d %02d %2.lf %2.lf %2.lf %04d %02d %02d %2.lf %2.lf %2.lf %02d %1d %1d %1d %1d %1d %1d %1d",
                 year_s,month_s,day_s,hour1,minut1,sec1,year_f,month_f,day_f,hour2,minut2,sec2,m_relis,
                 trop_corr,cmas_corr,ampl_corr,sys_del,hz,rang_typ,data_qv);
        frd_crd << buf << std::endl;

        sprintf(buf,"mjd=%lld  start=%lld   finish=%lld",mjd,mjd_s,mjd_f); tmp2 << buf << std::endl;
        sprintf(buf,"start day is %d  %d  %d",year_s,month_s,day_s);    tmp2 << buf << std::endl;
        sprintf(buf,"start in %.lf %.lf %.lf",hour1,minut1,sec1);       tmp2 << buf << std::endl;
        sprintf(buf,"finish day is %d  %d  %d",year_f,month_f,day_f);   tmp2 << buf << std::endl;
        sprintf(buf,"finish time %.lf  %.lf  %.lf",hour2,minut2,sec2);  tmp2 << buf << std::endl;

        sprintf(buf,"C0 0 %10.3lf %4s %4s %4s %4s %4s",tr_wave,sys_conf,a_conf,b_conf,c_conf,d_conf);
        npt_crd << buf << std::endl;
        frd_crd << buf << std::endl;
        sprintf(buf,"C1 0 %4s %10s %10.2lf %10.2lf %10.2f %6.1lf %5.2f %4d",a_conf,laser_type,p_wave,fire_rate,las_en,pulse_width,beam,N_pulse);
        npt_crd << buf << std::endl;
        frd_crd << buf << std::endl;
        sprintf(buf,"C2 0 %4s %10s %10.3f %6.2f %5.1f %5.1f %10s %7.1f %5.2f %5.1f %5.1f %10s",b_conf,d_type,a_w,qv,feu_volt,d_c,o_type,o_p,s_f1,tr_s,s_f2,esp);
        npt_crd << buf << std::endl;
        frd_crd << buf << std::endl;
        sprintf(buf,"C3 0 %4s %20s %20s %20s %20s %6.1f",c_conf,t_source,f_sourse,timer,s_num,e_delay);
        npt_crd << buf << std::endl;
        frd_crd << buf << std::endl;

        cal_type=0;
        cal_type_ind=2;
        cal_shift_ind=0;
        det_canal=0;
        cal_shift=1;
        cal_peak1=sk_c1*1000.0;
        cal_peak2=sk_c2*1000.0;
        if(flag_cal!=0) {
            sprintf(buf,"40 %18.12lf %1d %4s %8d %8d %7.3lf %10.1lf %8.1lf %6.1f %7.3f %7.3f %10.1lf %1d %1d %1d",
                      static_cast<double>(tim_st_cal1),cal_type,sys_conf,flash1,good_flash1,target_c1,sk_c1*1000.0,shift*1000.0,rmsk_c1*1000.0,skew1,kurt1,cal_peak1,
                      cal_type_ind,cal_shift_ind,det_canal);
            npt_crd << buf << std::endl;
            frd_crd << buf << std::endl;
        }

        k_obs.clear();
        k_obs.seekg(0, std::ios_base::beg);
        std::getline(k_obs, temp_line);
        sscanf(temp_line.c_str(),"%lf%lf%lf%lf%d%lf",&time_x,&azym,&elev,&range,&amplit,&o_c);
        sprintf(buf,"20 %18.12lf %7.2f %6.2f %4d 1",time_x,pres,temp,humid);
        npt_crd << buf << std::endl;
        frd_crd << buf << std::endl;
        sprintf(buf,"10 %18.12f %18.12f %4s 2 2 0 0 %5d",time_x,range/1000000000.0,sys_conf,amplit);
        frd_crd << buf << std::endl;

        vx1=static_cast<int>(time_x/period);
        l=1;
        tab_time[l]=sum_time=time_x;
        vsum=v[l]=o_c;
        rang[l]=range;
        end_time=static_cast<double>(vx1+1)*period;
        npointt=npoint+1;
        numer=0;
        summa=0.;
        for(j=2;j<=npointt;j++) {
            if(j==npointt)
                vx=-1;
            else {
                std::getline(k_obs, temp_line);
                sscanf(temp_line.c_str(),"%lf%lf%lf%lf%d%lf",&time_x,&azym,&elev,&range,&amplit,&o_c);
                vx=static_cast<int>(time_x/period);
                sprintf(buf,"10 %18.12f %18.12f %4s 2 2 0 0 %5d",time_x,range/1000000000.0,sys_conf,amplit);
                frd_crd << buf << std::endl;
            }
            if(time_x<=end_time&&vx==vx1) {
                l++;
                tab_time[l]=time_x;
                sum_time += time_x;
                vsum += o_c;
                v[l]=o_c;
                rang[l]=range;
            } else {
                if(l>5000) printf("\n Warning!!! \n Program works corectly up to 3000 points per 1 normal point\n now Q=%d\n",l);
                if(l==1) {
                    v_mean=rms;
                    mean_time=tab_time[1];
                    mean_range=rang[1]-corect;
                    vv_mean=v[1];
                    if(flag==0) {k_urt=s_kew=0;}
                } else {
                    mean_time=sum_time/static_cast<double>(l);
                    vv_mean=vsum/static_cast<double>(l);
                    mi1=0;
                    for(jj=1,vsum=0.;jj<=l;jj++) {
                       vsum += (v[jj]-vv_mean)*(v[jj]-vv_mean);
                       mi_tim[jj]=(rang[jj]-corect)*1000.0;// v picosecundah
                       mi1 += mi_tim[jj];
                    }
                    mi1 /= static_cast<double>(l);
                    v_mean=sqrt(vsum/static_cast<double>(l-1));
                    tim=fabs(mean_time-tab_time[1]);

                    mi2=mi3=mi4=0;
                    for(jj=1;jj<=l;jj++) {
                       mi2+=(mi_tim[jj]-mi1)*(mi_tim[jj]-mi1);
                       mi3+=(mi_tim[jj]-mi1)*(mi_tim[jj]-mi1)*(mi_tim[jj]-mi1);
                       mi4+=(mi_tim[jj]-mi1)*(mi_tim[jj]-mi1)*(mi_tim[jj]-mi1)*(mi_tim[jj]-mi1);
                    }
                    mi2=mi2/static_cast<double>(l);
                    mi3=mi3/static_cast<double>(l);
                    mi4=mi4/static_cast<double>(l);
                    if(flag==0) {
                        s_kew=(mi3*mi3)/(mi2*mi2*mi2);
                        k_urt=(mi4)/(mi2*mi2);
                    }

                    jj=1;
                    for(k=2;k<=l;k++) {
                        xx=fabs(mean_time-tab_time[k]);
                        if(xx<tim)
                        {	jj=k; tim=xx;}
                    }
                    mean_time=tab_time[jj];
                    mean_range=rang[jj]-corect-(v[jj]-vv_mean);
                }

                sprintf(ss,"%012.0lf%012.0lf%07.0lf%05.0lf%04.0lf%03d%04d00000",
                mean_time*1.e7,mean_range*1000.,v_mean*1000.,pres*10.,temp*10.,humid,l);
                check_sum=0;
                sprintf(buf,"11 %18.12lf %18.12lf %4s 2 %6.1f %6d %9.1f %7.3f %7.3f %9.1f %5.1f 0",
                         mean_time,mean_range/1000000000.0,sys_conf,static_cast<double>(np_window),l,v_mean*1000.0,s_kew,k_urt,v_mean*500.0,
                        static_cast<double>(l*10/np_window));
                npt_crd << buf << std::endl;

                numer++;
                timm[numer]=mean_time;
                ran[numer]=mean_range;
                rmes[numer]=vv_mean;
                rmss[numer]=v_mean;
                num[numer]=l;
                summa+=vv_mean;
                for(jj=0;jj<=51;jj++) {
                    ss1[0]=ss[jj];
                    check_sum += atoi(ss1);
                }
                sprintf(buf,"%s%02d",ss,check_sum%100);
                edc_npt << buf << std::endl;

                l=1;
                vv_mean=0.;
                vx1=static_cast<int>(time_x/period);
                tab_time[l]=sum_time=time_x;
                vsum=v[l]=o_c;
                rang[l]=range;
                end_time=static_cast<double>(vx+1)*period;
            }
        }

        icoef=0;
        if(icoef==1) {
            k_obs.clear();
            k_obs.seekg(0, std::ios_base::beg);
            edc_npt << "88888" << std::endl;
            jj=0;
            sprintf(ss,"%7lld%02d%03d%4d%02d%02d%4.0lf%08.0lf"
                "%06.0lf%04.0lf%1d%1d%1d%1d%1d%04.0lf0",
            nsat,year_cent,day_y,nsta,SYS_NUM,OCU_SEQ,
            LAS_WAV*1.e4,corect,shift,rms_cal,jj,TIM_SCALE,
            SYS_CALIB,CAL_INDIC,SYS_FLAG,rms*1000.);
            check_sum=0;
            for(j=0;j<=51;j++) {
                ss1[0]=ss[j];
                check_sum +=atoi(ss1);
            }
            sprintf(buf,"%s%02d",ss,check_sum%100);
            edc_npt << buf << std::endl;
            xx=50./static_cast<double>(npoint);
            if(npoint<50)	xx=1.;

            jj=1;
            for(j=1;j<=npoint;j++) {
                std::getline(k_obs, temp_line);
                sscanf(temp_line.c_str(),"%lf%lf%lf%lf%d%lf",&time_x,&azym,&elev,&range,&amplit,&o_c);
                l=static_cast<int>(static_cast<double>(j)*xx+0.1);
                if(l==jj) {
                    range -= corect;
                    sprintf(ss,"%012.0lf%012.0lf%05.0lf%04.0lf"
                    "%03d00000000%04d%01d%07.0lf%06.0lf00000",
                    time_x*1.e7,range*1000.,pres*10.,temp*10.,
                    humid,amplit,ANGL_IND,azym*1.e4,elev*1.e4);
                    check_sum=0;
                    for(jj=0;jj<=66;jj++) {
                        ss1[0]=ss[jj];
                        check_sum += atoi(ss1);
                    }
                    sprintf(buf,"%s%02d",ss,check_sum%100);
                    edc_npt << buf << std::endl;
                    jj=l+1;
                }
            }
        }
        k_obs.close();

        if(cal_after!=cal_before)
        {
            sprintf(buf,"40 %18.12lf %1d %4s %8d %8d %7.3lf %10.1lf %8.1lf %6.1f %7.3f %7.3f %10.1lf %1d %1d %1d",
                  static_cast<double>(tim_st_cal2),cal_type,sys_conf,flash2,good_flash2,target_c2,sk_c2*1000.0,shift*1000.0,rmsk_c2*1000.0,skew2,kurt2,cal_peak2,
                  cal_type_ind,cal_shift_ind,det_canal);
            npt_crd << buf << std::endl;
            frd_crd << buf << std::endl;
        }
        mean_rang=0;
        for(i=1;i<=numer;i++) {
            mean_rang+=ran[i];
            k=1;
            for(j=i+1;j<=numer;j++) {
                num[i]=0;
                if(ran[i]==ran[j]) {
                    k++;
                    num[i]=k;
                }
            }
        }
        mean_rang /= numer;
        k=1;
        for(i=1;i<=numer;i++) {
            if(k<num[i]) {
                k=num[i];
                peak=ran[i];
            }
        }
        mean_peak=mean_rang;
        range=fabs(mean_rang-ran[1]);
        if(k==1) {
            for(i=1;i<=numer;i++) {
                if(range>fabs(mean_rang-ran[i])) {
                    range=fabs(mean_rang-ran[i]);
                    mean_peak=ran[i];
                }
            }
            mean_peak=fabs(mean_peak-mean_rang)*1000.0;
        }
        printf("RMS=%f  mean_peak=%f\n",rms,mean_peak);
        if(mean_peak>9999.9) mean_peak=0.0;

        sprintf(buf,"50 %4s %6.1f %7.3lf %7.3lf %6.1lf 1",sys_conf,rms*1000,s_kew,k_urt,mean_peak);
        npt_crd << buf << std::endl;
        sprintf(buf,"H8");
        npt_crd << buf << std::endl;
        frd_crd << buf << std::endl;
    }
    sprintf(buf,"H9");
    npt_crd << buf << std::endl;
    frd_crd << buf << std::endl;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    Plot* p = new Plot();
//    QVector<double> x(1000),y(1000);
//    QRandomGenerator rd;
//    for (int i=0; i<1000; ++i) {
//        x[i] = rd.generateDouble();
//        y[i] = rd.generateDouble();
//    }
//    p->graph()->setData(x,y);
//    p->isPointDropped.fill(false, x.size());
//    p->graph()->rescaleAxes();
//    p->xAxis->setLabel(QString("TIME [ S ]"));
//    p->yAxis->setLabel(QString("RESIDUALS [ M ]"));
//    p->plotLayout()->insertRow(0);
//    p->plotLayout()->addElement(0, 0, new QCPTextElement(p, QString("header1"), QFont("sans", 12, QFont::Bold)));
//    p->show();
//    return a.exec();

    AOPConsoleWindow w;
    w.show();
    return a.exec();

    alarm();
    if (stt_epaa(0) == 5) {
        std::cout << "Error in stt_epaa !! Exiting.." << std::endl;
        alarm();
        return 1;
    }

    std::ifstream interimData("D:\\LASER-2\\DATA\\EPA_AUTO.DAT");
    int nr, cal_before, cal_after;
    int64_t numpass, nr_sat;
    if (interimData.is_open()) {
        std::string temp_line;
        std::getline(interimData, temp_line);
        sscanf(temp_line.c_str(), datamodels::FormatEPA_AUTO_R, &nr, &numpass, &cal_before, &cal_after, &nr_sat);
        interimData.close();
    } else {
        std::cout << "Unable to open file EPA_AUTO.DAT !" << std::endl;
        return 1;
    }

    std::cout << "Observation nr " << nr << " was taken." << std::endl;

    if (ffit_obs(nr) == 5) {
        std::cout << "Error in ffit_obs !! Exiting.." << std::endl;
        alarm();
        return 1;
    }

    std::ifstream result_dat("D:\\LASER-2\\DATA\\RESULT.DAT");
    if (!result_dat.is_open()) {
        std::cout << "Unable to open RESULT.DAT file!" << std::endl;
        return 1;
    }
    int npoint;
    result_dat >> npoint;
    result_dat.close();
    if (npoint == 0) {
        std::cout << "No measurements in fit_obs !!" << std::endl;
        alarm();
        return 1;
    }

    std::ofstream param_dat("D:\\LASER-2\\DATA\\PARAM.DAT");
    if (!param_dat.is_open()) {
        std::cout << "Unable to open PARAM.DAT file!" << std::endl;
        return 1;
    }
    int kk = 1;
    char buf[16];
    sprintf(buf, "%4d\n", kk);
    param_dat << buf;
    param_dat.close();

    do {
        if (plo_fita(nr) == 5) {
            std::cout << "Error in plo_fit !! Exiting.." << std::endl;
            alarm();
            return 1;
        }

        result_dat.open("D:\\LASER-2\\DATA\\RESULT.DAT");
        if (!result_dat.is_open()) {
            std::cout << "Unable to open RESULT.DAT file!" << std::endl;
            return 1;
        }
        result_dat >> npoint;
        result_dat.close();
        if (npoint == 0) {
            std::cout << "No measurements in plo_fit !!" << std::endl;
            alarm();
            return 1;
        }

        std::cout << "Accept result (Enter) :" << std::endl;
        char c = getch();
        if(c == 0x00)
            c = getch();
        if(c == 0x0d)	break;
    } while (true);

    std::cout << "In result.dat " << npoint << " points." << std::endl;
    param_dat.open("D:\\LASER-2\\DATA\\PARAM.DAT");
    if (!param_dat.is_open()) {
        std::cout << "Unable to open PARAM.DAT file!" << std::endl;
        return 1;
    }
    kk = 2;
    sprintf(buf, "%4d\n", kk);
    param_dat << buf;
    param_dat.close();

    do {
        if (pasint_a(nr) == 5) {
            std::cout << "Error in pasint_a !! Exiting.." << std::endl;
            alarm();
            return 1;
        }
        int nns;
        result_dat.open("D:\\LASER-2\\DATA\\RESULT.DAT");
        result_dat >> nns;
        result_dat.close();
        if (nns == 0) {
            std::cout << "The number of measurments in RESULT.DAT is 0" << std::endl;
            alarm();
            return 1;
        }

        if (plo_fita(nr) == 5) {
            std::cout << "Error in plo_fit !! Exiting.." << std::endl;
            alarm();
            return 1;
        }
        result_dat.open("D:\\LASER-2\\DATA\\RESULT.DAT");
        result_dat >> npoint;
        result_dat.close();
        if (npoint == 0) {
            std::cout << "The number of measurments in RESULT.DAT is 0" << std::endl;
            alarm();
            return 1;
        }

        std::cout << "Accept result (Enter) :" << std::endl;
        char c = getch();
        if(c == 0x00)
            c = getch();
        if(c == 0x0d)	break;
    } while (true);

    int nss = 0;
    do {
        int npoint_a = npoint;
        if (pasint_a(nr) == 5) {
            std::cout << "Error in pasint_a !! Exiting.." << std::endl;
            alarm();
            return 1;
        }
        result_dat.open("D:\\LASER-2\\DATA\\RESULT.DAT");
        result_dat >> npoint;
        result_dat.close();
        if (npoint == 0) {
            std::cout << "The number of measurments in RESULT.DAT is 0" << std::endl;
            alarm();
            return 1;
        }

        if (pol_epaa(nr) == 5) {
            std::cout << "Error in pol_epaa !! Exiting.." << std::endl;
            alarm();
            return 1;
        }
        result_dat.open("D:\\LASER-2\\DATA\\RESULT.DAT");
        result_dat >> npoint;
        result_dat.close();
        if (kbhit()!=0) {
            char c = getch();
            if (c == 0x00) c=getch();
            if (c == 0x1b) return 1;
        }
        if (npoint == 0) {
            std::cout << "The number of measurments in RESULT.DAT is 0" << std::endl;
            alarm();
            return 1;
        }

        if (graf_ep0(nr) == 5) {
            std::cout << "Error in graf_ep0 !! Exiting.." << std::endl;
            alarm();
            return 1;
        }
        result_dat.open("D:\\LASER-2\\DATA\\RESULT.DAT");
        result_dat >> npoint;
        result_dat.close();
        if (npoint == 0) {
            std::cout << "The number of measurments in RESULT.DAT is 0" << std::endl;
            alarm();
            return 1;
        }
        if (kbhit()!=0) {
            char c = getch();
            if (c == 0x00) c=getch();
            if (c == 0x0d) return 1;
        }

        if (npoint_a == npoint) nss++;
        if (npoint == -99) {
            if (del_npt(nr) == 5) {
                std::cout << "Error in del_npt !! Exiting.." << std::endl;
                alarm();
                return 1;
            }
            result_dat.open("D:\\LASER-2\\DATA\\RESULT.DAT");
            result_dat >> npoint;
            result_dat.close();
            if (npoint == 0) {
                std::cout << "The number of measurments in RESULT.DAT is 0" << std::endl;
                alarm();
                return 1;
            }
        }
        if (npoint == -99) break;
    } while (true);

    if (plo_nql(nr) == 5) {
        std::cout << "Error in plo_nql !! Exiting.." << std::endl;
        alarm();
        return 1;
    }

    int cat = 0;
    std::cout << "Save observation to KBO?" << std::endl;
    fflush(stdin);
    char c = getch();
    if (c == 0x00) c=getch();
    if (c == 0x0d) {
        if (cat_kboa(numpass) == 5) {
            std::cout << "Error in cat_kboa !! Exiting.." << std::endl;
            alarm();
            return 1;
        }
        cat = 1;

        std::ofstream print_catalogue("D:\\LASER-2\\DATA\\WYDRUK.DIC", std::ios_base::app);
        if (!print_catalogue.is_open()) {
            std::cout << "Unable to open printing catalogue" << std::endl;
        } else {
            time_t t = time(nullptr);
            struct tm tm = *localtime(&t);
            int64_t mjd = utility::dat_mjd(tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
            int64_t secday = 3600*tm.tm_hour+60*tm.tm_min+tm.tm_sec;
            char buf[256];
            sprintf(buf, "e 0 %8lld %8lld %8lld %8lld", numpass, nr_sat, mjd, secday);
            print_catalogue << buf << std::endl;
        }
    }

    if (cat == 1) {
        if (npt_kbod(numpass) == 5) {
            std::cout << "Error in npt_kbod !! Exiting.." << std::endl;
            alarm();
            return 1;
        }
        nql_kbo(numpass);
    }

    std::cout << "END of the AOP!" << std::endl;
    fflush(stdin);

    return a.exec();
}
