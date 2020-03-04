#include "engine.h"

const double C_VEL = 299792458.0;
#include <time.h>
#include <cmath>
#include <QFile>
#include <valarray>
#include "utility.h"

// user react tools
#include <QMessageBox>
#include <QPushButton>
#include <QInputDialog>

Engine::Engine(QObject *parent) : QObject(parent)
{

}

void Engine::prepareObsCopy(int nr)
{
    nrObs = nr;
    selectedObs = UniqueKAT_OBS::Instance()->cat[nrObs-1];
    satelliteInfo = UniqueKAT_SAT::Instance()->getSatById(selectedObs.nsat);
    if (satelliteInfo.id == 0) {
        emit searchError("Satellite id in KAT_OBS record dont match any satellite in SATELLIT file ! (id="
                         + QString::number(selectedObs.nsat) + ")");
        return;
    }
    sprintf(fnameobs, "D:\\LASER-2\\DATA\\%8s.o%2s", selectedObs.namefe, selectedObs.kodst);
    sprintf(fnamecopy, "D:\\LASER-2\\DATA\\%8s.c%2s", selectedObs.namefe, selectedObs.kodst);
    sprintf(fnameefem, "D:\\LASER-2\\DATA\\%8s.e%2s", selectedObs.namefe, selectedObs.kodst);
    sprintf(fnamederiv, "D:\\LASER-2\\DATA\\%8s.p%2s", selectedObs.namefe, selectedObs.kodst);
    sprintf(fnamekobs, "D:\\LASER-2\\DATA\\%8s.k%2s", selectedObs.namefe, selectedObs.kodst);
    sprintf(fnamelog, "D:\\LASER-2\\DATA\\%8s.l%2s", selectedObs.namefe, selectedObs.kodst);

    if (!sObs.read(fnameobs)) {
        emit accessError("Unable to open efemerids FILE : " + QString(fnameobs) + "!\n Provide the file or select other OBS and press OK, otherwise press Cancel to exit.");
        return;
    }
    if (!sEfem.read(fnameefem)) {
        emit accessError("Unable to open efemerids FILE : " + QString(fnameefem) + "!\n Provide the file or select other OBS and press OK, otherwise press Cancel to exit.");
        return;
    }

    if (selectedObs.ws1 == -1) { // if it was selected original observation -- find or create it's copy

        if (QFile::exists(fnamecopy)) {

            QMessageBox copyPresented_react;
            copyPresented_react.setText("Original observation was selected, though it's copy is present.");
            QAbstractButton* b0 = copyPresented_react.addButton(tr("Select presented copy"), QMessageBox::ButtonRole::NoRole);
            QAbstractButton* b1 = copyPresented_react.addButton(tr("Re-create clear copy"), QMessageBox::ButtonRole::NoRole);
            copyPresented_react.exec();
            QAbstractButton* b = copyPresented_react.clickedButton();

            if (b == b0) nrObs++;
            if (b == b1) {
                datamodels::CopyObsElem e;
                e.sec = sObs.data[0].sec;
                e.azym = sObs.data[0].azym;
                e.elev = sObs.data[0].elev;
                e.range = sObs.data[0].range;
                e.range1 = e.range * 0.5e-12*C_VEL;
                e.ampl = sObs.data[0].ampl;
                sCopy.data.push_back(e);

                for (int i=1; i<selectedObs.npoint; ++i)
                    if (sObs.data[i].sec > sObs.data[i-1].sec) {
                        e.sec = sObs.data[i].sec;
                        e.azym = sObs.data[i].azym;
                        e.elev = sObs.data[i].elev;
                        e.range = sObs.data[i].range;
                        e.range1 = e.range * 0.5e-12*C_VEL;
                        e.ampl = sObs.data[i].ampl;
                        sCopy.data.push_back(e);
                    }
                sCopy.write(fnamecopy);

                selectedObs.npoint = sCopy.data.size();
                selectedObs.ws1 = 1;
                selectedObs.nr++;
                if (sObs.data[0].sec < selectedObs.secday)
                    ++selectedObs.mjd;

                UniqueKAT_OBS::Instance()->cat[nrObs] = selectedObs; // re-write copy record in KAT_OBS
                ++nrObs; // switch to newly created and inserted copy
            }
        } else {
            datamodels::CopyObsElem e;
            e.sec = sObs.data[0].sec;
            e.azym = sObs.data[0].azym;
            e.elev = sObs.data[0].elev;
            e.range = sObs.data[0].range;
            e.range1 = e.range * 0.5e-12*C_VEL;
            e.ampl = sObs.data[0].ampl;
            sCopy.data.push_back(e);

            for (int i=1; i<selectedObs.npoint; ++i)
                if (sObs.data[i].sec > sObs.data[i-1].sec) {
                    e.sec = sObs.data[i].sec;
                    e.azym = sObs.data[i].azym;
                    e.elev = sObs.data[i].elev;
                    e.range = sObs.data[i].range;
                    e.range1 = e.range * 0.5e-12*C_VEL;
                    e.ampl = sObs.data[i].ampl;
                    sCopy.data.push_back(e);
                }
            sCopy.write(fnamecopy);

            selectedObs.npoint = sCopy.data.size();
            selectedObs.ws1 = 1;
            selectedObs.nr++;
            if (sObs.data[0].sec < selectedObs.secday)
                ++selectedObs.mjd;

            auto it = UniqueKAT_OBS::Instance()->cat.begin();
            UniqueKAT_OBS::Instance()->cat.insert(it+nrObs, selectedObs); // insert new record for copy in KAT_OBS
            ++nrObs; // switch to newly created and inserted copy

            for (size_t i = nrObs; i<UniqueKAT_OBS::Instance()->cat.size(); ++i)
                UniqueKAT_OBS::Instance()->cat[i].nr++; // shift enumeration as new line has been inserted
        }
    }

    selectedObs = UniqueKAT_OBS::Instance()->cat[nrObs-1];
    do {
        double rrr1 = 0.;
        if (selectedObs.clock_cor != 0.) {
            QMessageBox mb;
            mb.setText("Clock correction was entered = " + QString::number(selectedObs.clock_cor, 'f', 4)
                       + "\nProgram deleted old correction, and created new one\n"
                       + "Do you want to enter new correction?");
            mb.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            int code = mb.exec();
            if (code == QMessageBox::Yes) {
                bool ok;
                while (true) {
                    rrr1 = QInputDialog::getDouble(nullptr, "", "Enter clock correction in microseconds :", 0., -1e10, 1e10, 4, &ok);
                    if (ok) break;
                }
            }
        }

        sCopy.read(fnamecopy);
        for (datamodels::CopyObsElem e: sCopy.data)
            e.sec += (rrr1-selectedObs.clock_cor)*1.e-6;
        sCopy.write(fnamecopy);

        selectedObs.clock_cor = rrr1;
        UniqueKAT_OBS::Instance()->cat[nrObs-1] = selectedObs;
    } while (false);
}

void Engine::fit_calibration(bool is_before)
{
    int nr = is_before ? calBefore : calAfter;
    datamodels::CalCatElem selectedCal = UniqueKAT_CAL::Instance()->cat[nr-1];
    double l;
    char _[5], namecal[33];
    sprintf(namecal, "D:\\LASER-2\\DATA\\%s", selectedCal.namcal);

    std::string temp_line;
    std::ifstream sys_conf("D:\\LASER-2\\DATA\\SYS_CONF");
    if (sys_conf.is_open()) {
        std::getline(sys_conf, temp_line);
        sscanf(temp_line.c_str(), datamodels::FormatSYS_CONF_R, &l, &_, &_, &_, &_, &_);
        sys_conf.close();
    } else {
        emit accessError("Unable to open system configuration file D:\\LASER-2\\DATA\\SYS_CONF! Provide it or press Cancel.");
        return;
    }

    l /= 1000.;
    double fl = 0.9650+0.0164/(l*l)+0.000228/(l*l*l*l);

    std::valarray<double> cal_data(selectedCal.nreturn);
    std::valarray<bool> icoef(selectedCal.nreturn);
    std::fstream cal(namecal, std::ios_base::in);
    if (cal.is_open()) {
        for (int i=0; i<selectedCal.nreturn; ++i) {
            cal >> cal_data[i];
            icoef[i] = true;
        }
        cal.close();
    } else {
        emit accessError("Unable to open file " + QString(namecal) + " ! Provide it or press Cancel.");
        return;
    }

    double sx = 7.5*(selectedCal.temp-273.1)/(237.3-(selectedCal.temp-273.1));
    double e = static_cast<double>(selectedCal.humid)*1.e-2*6.11*pow(10.,sx);
    double gn = 80.343*fl*selectedCal.baro/selectedCal.temp-11.3*e/selectedCal.temp;
    double tkal=2.*1.e9*selectedCal.target*(1.+gn*1.e-6)/C_VEL;
    double rmss = selectedCal.rms, corectt = selectedCal.corect, meanvall = selectedCal.meanval;
    double mean_val, step;
    int scale, nret;
}

void Engine::fit_observation()
{
    selectedObs.TB = selectedObs.RB = 0;
    int efemPoints = sEfem.data.size();
    std::vector<double> defe_t(efemPoints, 0.), defe_r(efemPoints, 0.);

    for (int i=0; i<efemPoints; ++i) {
        defe_t[i] = sEfem.data[i].sec;
        defe_r[i] = sEfem.data[i].range;
    }

    double omclimit = 29999., t10 = 0.;
    for (int i=0; i<efemPoints-1; ++i) {
        if (defe_t[i+1] < defe_t[i]) t10 = 86400.;
        defe_t[i+1] += t10;
    }

    double t0_efe = defe_t[0], dt_efe = defe_t[1]-defe_t[0];
    double secobs = sCopy.data[0].sec;
    double t20, t21, tt0, tt1, tt2, secstart = 0.;
    int derivCount = 0;

    datamodels::CopyObs copy;
    for (datamodels::CopyObsElem ce : sCopy.data) {
        double secc = ce.sec;
        if (secc < t0_efe) secc += 86400.;
        int index = static_cast<int>((secc-t0_efe)/dt_efe);
        if (index < 1) index=1;
        if (index >= efemPoints-1) index = efemPoints-2;
        --index;

        t10 = defe_t[index+1]-defe_t[index];
        t20 = defe_t[index+2]-defe_t[index];
        t21 = defe_t[index+2]-defe_t[index+1];
        tt0 = secc-defe_t[index];
        tt1 = secc-defe_t[index+1];
        tt2 = secc-defe_t[index+2];

        double re = tt1*tt2/(t10*t20)*defe_r[index]-
                    tt0*tt2/(t10*t21)*defe_r[index+1]+
                    tt0*tt1/(t20*t21)*defe_r[index+2];
        double omc = (ce.range1 - re)*1000.;


        if (fabs(omc) <= omclimit) {
            ++derivCount;
            if (derivCount==1) secstart = ce.sec;
            datamodels::DifElem e;
            e.sec = ce.sec;
            e.omc = omc;
            e.tbrb = e.czeb = 0.;
            sDeriv.data.push_back(e);
            copy.data.push_back(ce);
        }
    }
    sCopy = copy;

    if (derivCount != 0) {
        selectedObs.ws = 1;
        selectedObs.npoint = derivCount;
        selectedObs.secday = static_cast<int64_t>(secstart);
        if(( fabs(secstart-secobs) ) > 80000.) ++selectedObs.mjd;

        UniqueKAT_OBS::Instance()->cat[nrObs-1] = selectedObs;
        sCopy.write(fnamecopy);
        sDeriv.write(fnamederiv);
    }
}

void Engine::prepareOmcPlot(int type)
{
    if (type==2 && selectedObs.ws<2) {
        emit logicError("Time and Range Bias haven't been calculated yet!");
        return;
    }
    if (type==3 && selectedObs.ws!=3) {
        emit logicError("Polynomials haven't been calculated yet!");
        return;
    }

    std::ifstream station("D:\\LASER-2\\DATA\\STATION");
    std::string temp_line;
    if (!station.is_open()) {
        emit accessError("Unable to open STATION file! Provide it or press Cancel.");
        return;
    }
    std::getline(station, temp_line);
    char NAMSTAC[12];
    int NSTA;
    sscanf(temp_line.c_str(), "%s%d%*d%*d%*d", NAMSTAC,&NSTA);
    station.close();
    if (selectedObs.nsta != NSTA) {
        emit searchError("Different station numbers provided! In catalogue: " + QString::number(selectedObs.nsta) + ", in STATION file: " + QString::number(NSTA));
        return;
    }

    int dd,mth,yy,hh,min,ss;
    char header1[81], header2[81];
    utility::mjd_dat(dd,mth,yy, selectedObs.mjd);
    hh  = static_cast<int>(selectedObs.secday / 3600LL);
    min = static_cast<int>(selectedObs.secday % 3600LL / 60LL);
    ss  = static_cast<int>(selectedObs.secday % 3600LL % 60LL);
    sprintf(header1,"STATION: %+10s   SATELLITE: %-10s %4d-%02d-%02d%4d:%02d:%02d  %5d POINTS",
            NAMSTAC,satelliteInfo.name,yy,mth,dd,hh,min,ss,selectedObs.npoint);

    QVector<double> X,Y;
    X.resize(selectedObs.npoint);
    Y.resize(selectedObs.npoint);
    for (int i=0; i<selectedObs.npoint; ++i) {
        X[i] = sDeriv.data[i].sec;
        if (type==1) Y[i] = sDeriv.data[i].omc;
        if (type==2) Y[i] = sDeriv.data[i].tbrb;
        if (type==3) Y[i] = sDeriv.data[i].czeb;
    }
    for (int i=1; i<selectedObs.npoint; ++i) {
        if(X[i] < X[0])   X[i] += 86400.;
        X[i] -= X[0];
    }
    X[0] = 0;

    if (type == 1)
        sprintf(header2," ");
    if (type == 2) {
        sprintf(header2," ");
        sprintf(header2,"TIME BIAS : %6.6lf MS   RANGE BIAS"
            " : %6.6lf M ",selectedObs.TB,selectedObs.RB);
    }
    if (type == 3) {
        sprintf(header2,"TIME BIAS :%5.3lf MS  RANGE BIAS"
            " :%6.1lf M  STEP OF POLY :%3d  RMS :%5.2lf CM",
                selectedObs.TB,selectedObs.RB,selectedObs.POLY,selectedObs.RMS*15.);
    }
    emit plotOmcData(X, Y, QString(header1), QString(header2));
}

void Engine::onPlotOmcResult(bool save, QVector<bool> dropFlags, int npointsLeft)
{
    if (selectedObs.npoint == npointsLeft || !save) {
        return;
    }

    int64_t secobs = 0;
    datamodels::CopyObs copy;
    datamodels::Dif deriv;
    for (int i=0, j=0; i<selectedObs.npoint; ++i) {
        if (!dropFlags[i]) {
            ++j;
            if (j==1) secobs = static_cast<int64_t>(sCopy.data[i].sec);
            copy.data.push_back(sCopy.data[i]);
            sDeriv.data[i].sec = sCopy.data[i].sec;
            deriv.data.push_back(sDeriv.data[i]);
        }
    }
    sCopy = copy;
    sDeriv = deriv;

    selectedObs.secday = secobs;
    selectedObs.npoint = npointsLeft;
    UniqueKAT_OBS::Instance()->cat[nrObs-1] = selectedObs;
    sCopy.write(fnamecopy);
    sDeriv.write(fnamederiv);
}

void Engine::calc_TimeRangeBias()
{
    int efemPoints = sEfem.data.size();
    std::vector<double> defe_t(efemPoints), defe_r(efemPoints);
    std::vector<double> dobs_t(selectedObs.npoint), dobs_r(selectedObs.npoint);

    for (int i=0; i<selectedObs.npoint; ++i) {
        dobs_t[i] = sCopy.data[i].sec;
        dobs_r[i] = sCopy.data[i].range1;
        if (dobs_t[i] < dobs_t[0]) dobs_t[i]+=86400.;
    }

    double longpas = (sCopy.data.back().sec - sCopy.data.front().sec)/60.;
    if (longpas < 0.) longpas += 1440.;

    for (int i=0; i<efemPoints; ++i) {
        defe_t[i] = sEfem.data[i].sec;
        defe_r[i] = sEfem.data[i].range;
        if (defe_t[i] < defe_t[0]) defe_t[i]+=86400.;
    }

    int stepefe = 1;
    double s1 = 0., s2 = 0.;
    for (int i=0; i<selectedObs.npoint; ++i) {
        double t = dobs_t[i];
        if (t < defe_t[0]) t+=86400.;
        int index = (static_cast<int>(t-defe_t[0]))/stepefe;
        if(index < 1) index=1;
        if(index >= efemPoints-1)	index = efemPoints-2;
        --index;

        double t10,t20,t21,tt0,tt1,tt2,re,omc;
        t10 = defe_t[index+1]-defe_t[index];
        t20 = defe_t[index+2]-defe_t[index];
        t21 = defe_t[index+2]-defe_t[index+1];
        tt0 = t-defe_t[index];
        tt1 = t-defe_t[index+1];
        tt2 = t-defe_t[index+2];
        re = tt1*tt2/(t10*t20)*defe_r[index]-
             tt0*tt2/(t10*t21)*defe_r[index+1]+
             tt0*tt1/(t20*t21)*defe_r[index+2];
        omc = (dobs_r[i] - re)*1000.;
        s1 += omc;
        s2 += omc*omc;
    }
    s1 /= static_cast<double>(selectedObs.npoint);
    s2 -= s1*s1*static_cast<double>(selectedObs.npoint);

    double tx = 0., zn = 0., nk;
    for(int j=0; j <= 8; ++j)
    {
        tx += utility::pot(10., -j);
        nk = 0;

e503:	nk++;
        double x1 = 0.;
        double x2 = 0.;
        for(int i=0; i<selectedObs.npoint; ++i)
        {
            double t = dobs_t[i];
            if (t < defe_t[0]) t+=86400.;
            int index = (static_cast<int>(t-defe_t[0]))/stepefe;
            if(index < 1) index=1;
            if(index >= efemPoints-1)	index = efemPoints-2;
            --index;

            double t10,t20,t21,tt0,tt1,tt2,re,omc;
            t10 = defe_t[index+1]-defe_t[index];
            t20 = defe_t[index+2]-defe_t[index];
            t21 = defe_t[index+2]-defe_t[index+1];
            tt0 = t+tx-defe_t[index];
            tt1 = t+tx-defe_t[index+1];
            tt2 = t+tx-defe_t[index+2];
            re = tt1*tt2/(t10*t20)*defe_r[index]-
                 tt0*tt2/(t10*t21)*defe_r[index+1]+
                 tt0*tt1/(t20*t21)*defe_r[index+2];
            omc = (dobs_r[i] - re)*1000.;
            x1 += omc;
            x2 += omc*omc;
        }

        x1 /= static_cast<double>(selectedObs.npoint);
        x2 -= x1*x1*static_cast<double>(selectedObs.npoint);

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

    for(int i=0; i<selectedObs.npoint; ++i)
    {
        double t = dobs_t[i];
        if (t < defe_t[0]) t+=86400.;
        int index = (static_cast<int>(t-defe_t[0]))/stepefe;
        if(index < 1) index=1;
        if(index >= efemPoints-1)	index = efemPoints-2;
        --index;

        double t10,t20,t21,tt0,tt1,tt2,re,omc;
        t10 = defe_t[index+1]-defe_t[index];
        t20 = defe_t[index+2]-defe_t[index];
        t21 = defe_t[index+2]-defe_t[index+1];
        tt0 = t+tx-defe_t[index];
        tt1 = t+tx-defe_t[index+1];
        tt2 = t+tx-defe_t[index+2];
        re = tt1*tt2/(t10*t20)*defe_r[index]-
             tt0*tt2/(t10*t21)*defe_r[index+1]+
             tt0*tt1/(t20*t21)*defe_r[index+2];
        omc = (dobs_r[i] - re)*1000.;
        sDeriv.data[i].tbrb = omc - s1;

        tt0 = t-defe_t[index];
        tt1 = t-defe_t[index+1];
        tt2 = t-defe_t[index+2];
        re = tt1*tt2/(t10*t20)*defe_r[index]-
             tt0*tt2/(t10*t21)*defe_r[index+1]+
             tt0*tt1/(t20*t21)*defe_r[index+2];
        omc = (dobs_r[i]-re)*1000.;

        if (t > 86400.) t -= 86400.;
        sDeriv.data[i].sec = t;
        sDeriv.data[i].omc = omc;
        sDeriv.data[i].czeb = 0.;
    }

    selectedObs.ws = 2;
    selectedObs.RB = s1;
    selectedObs.TB = tx*1000.;
    UniqueKAT_OBS::Instance()->cat[nrObs-1] = selectedObs;
}

void Engine::polynom_fit()
{
    if (selectedObs.ws < 2) {
        emit logicError("This copy has not been pre-processed!");
        return;
    }

    std::vector<int> Q = {0,30,31,33,35,37,39,41,43,45,47,49,52,
        57,62,67,74,90,112,137,175,300};
    std::vector<double> FO1 =
        {0.0,161.,18.51,10.13,7.71,6.61,5.99,5.39,5.32,5.12,
         4.96,4.84,4.75,4.67,4.6,4.54,4.49,
         4.45,4.41,4.38,4.35,4.32,4.3,4.28,4.26,4.24,4.22,
         4.21,4.2,4.18,4.17,4.15,4.13,4.11,4.1,4.08,4.07,
         4.06,4.05,4.04,4.03,4.02,4.,3.99,3.98,3.96,3.94,
         3.92,3.91,3.89,3.86};
    std::vector<double> dder_t(selectedObs.npoint+1), dder_b(selectedObs.npoint+1);


    double** array = new double*[10001];
    double** jjx = new double*[10001];
    for (int i=0; i<10001; ++i) {
        array[i] = new double[15];
        jjx[i] = new double[15];
    }

    int	l1,k,i,j,j1,m,mx,k1,jx[16],wss[16];
    double sl,sm,sa,xy,c[16],d1[16],omc[16];
    double timepas,xx,a[16];
    double jc,js,jy,jt,jf,j_c,j_s,j_y,j_t,j_f,j__c,j__s,j__y,j__t,j__f;

    for(i=0;i<16;i++) {
        wss[i]=-1;
        a[i]=c[i]=omc[i]=d1[i]=0.;
    }
    for (int i=0; i<selectedObs.npoint; ++i) {
        dder_t[i+1] = sDeriv.data[i].sec;
        dder_b[i+1] = sDeriv.data[i].tbrb;
    }

    sl=0.;
    timepas=dder_t[selectedObs.npoint]-dder_t[1];
    if (timepas<0.) timepas += 86400.;
    sa=dder_t[1];
    js=jc=0.0;
    for(i=1;i<=selectedObs.npoint;i++)
    {
        sl += dder_b[i];
        xy=dder_t[i]-sa;
        if (fabs(xy)>80000.)	xy=dder_t[i]-sa+86400.;
        dder_t[i]=array[i][0]=xy/timepas;
        jy=jc+dder_b[i];
        jt=js+jy;
        jf=0.0;
        if(jy*js>0.0)
            jf=0.46*jt;
        jc=js-jt+jy;
        js=jt;
    }
    sl=js+jc;
    c[1]=sl/static_cast<double>(selectedObs.npoint);
    for(i=1;i<=selectedObs.npoint;i++)
    {
        xx=(dder_b[i]-c[1])*10000.;
        if(fabs(xx)>32222.)
            jjx[i][0]=32222;
        else
            jjx[i][0]=static_cast<int>(xx);
    }
    l1=selectedObs.npoint-1;
    if(l1>15)l1=15;

    for(j=2;j<=l1;j++)
    {
        j1=j-1;
        jx[j]=j1;
        if(j1!=1) {
            for(i=1;i<=selectedObs.npoint;i++)
            array[i][0]=dder_t[i]*array[i][0];
        }
        for(k=1;k<=j1;k++)
        {
            sl=sm=0.;
            jc=js=j_s=j_c=0.0;
            for(i=1;i<=selectedObs.npoint;i++)
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

        for(i=1,sa=0.,sm=0.;i<=selectedObs.npoint;i++)
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
            sa+=dder_b[i]*xx;
            sm+=xx*xx;
            array[i][j-1]=xx;
            j__y=j__c+dder_b[i]*xx;
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
        for(i=1,sl=0.;i<=selectedObs.npoint;i++)
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
            sm=dder_b[i]-sa;
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
        k=selectedObs.npoint-j;

        if(k <= 0) {
            emit logicError("Number of points less than have been set!");
            delete[] array;
            delete[] jjx;
            return;
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
        k1+=28;
    e21:	if((d1[j-1]*(sm+1.)*sm/sl-sm)<=FO1[k1])	wss[j]=1;
        if(wss[j]==1&&wss[j-1]==1)	goto e25;

    e50:	continue;
    }
    e25:	m=j1+1;

    k=2;
    if (m>9) k=m-7;

    for(j=k;j<=m;j++) {
        mx=m;
        if(wss[j]==1&&wss[j+1]==1)	mx=j-1;
        if(j+1==m)	break;
    }

    for (i=0; i<selectedObs.npoint; ++i)
        sDeriv.data[i].czeb = static_cast<double>(jjx[i][mx-1])*1.e-4;

    selectedObs.RMS = sqrt(d1[mx])*2.e+9/C_VEL;
    selectedObs.POLY = mx-1;
    selectedObs.ws = 3;
    UniqueKAT_OBS::Instance()->cat[nrObs-1] = selectedObs;
    sDeriv.write(fnamederiv);

    char table[512];
    sprintf(table, "\n            Residuals for polynomial step  - cm  \n");
    sprintf(table + strlen(table), "k=%d m=%d\n",k,m);
    sprintf(table + strlen(table), "\nNO/POLY ");
    for(j=k;j<=m;j++)
        sprintf(table + strlen(table), "%8d",jx[j]);
    sprintf(table + strlen(table), "\n VARIAN.  ");
    for(j=k;j<=m;j++)
        sprintf(table + strlen(table), "%8.1lf",d1[j]*1.e4);
    sprintf(table + strlen(table), "\n ST.DEV.  ");
    for(j=k;j<=m;j++)
        sprintf(table + strlen(table), "%8.1lf",sqrt(static_cast<double>(d1[j]))*1.e2);
    sprintf(table + strlen(table), "\n WS.      ");
    for(j=k;j<=m;j++)
        sprintf(table + strlen(table), "%8d",wss[j]);
    sprintf(table + strlen(table), "\n");

    emit polyFitTable(QString::fromLatin1(table));

    delete[] array;
    delete[] jjx;
}



// returns false if "bad" points by omc criteria existed and was filtered out, and true otherwise
bool Engine::filterPolyResiduals(double rms)
{
    double mean = 0.;
    int np = 0;
    datamodels::CopyObs copy;
    datamodels::Dif deriv;

    for (int i=0; i<selectedObs.npoint; ++i)
        if (fabs(sDeriv.data[i].czeb) <= rms) {
            ++np;
            if (np==1) selectedObs.secday = static_cast<int64_t>(sCopy.data[i].sec);
            mean += sDeriv.data[i].czeb * sDeriv.data[i].czeb;
            copy.data.push_back(sCopy.data[i]);
            deriv.data.push_back(sDeriv.data[i]);
        }

    if (np == selectedObs.npoint) return true;

    selectedObs.RMS = sqrt(mean/static_cast<double>(np-selectedObs.POLY))*2.e+9/C_VEL;
    selectedObs.npoint = np;

    UniqueKAT_OBS::Instance()->cat[nrObs-1] = selectedObs;
    sCopy = copy;
    sDeriv = deriv;
    sCopy.write(fnamecopy);
    sDeriv.write(fnamederiv);
    return false;
}

// returns true if all Normal Points has been validated -- none of them was dropped
bool Engine::validationNP()
{
    std::vector<double> tab_time(5000), v(5000), plo0(10001), plo1(10001);
    std::vector<bool> ind(10001);

    int period = satelliteInfo.np_window;

    int dd, mt, yy;
    utility::mjd_dat(dd,mt,yy, selectedObs.mjd);
    int hh = selectedObs.secday/3600;
    int ss = selectedObs.secday - hh*3600;
    int mn = ss/60;
    ss -= mn*60;

    for (int i=1; i<=selectedObs.npoint; ++i) {
        plo0[i] = sDeriv.data[i].sec;
        plo1[i] = sDeriv.data[i].czeb;
        if (plo0[i] < plo0[1]) plo0[i]+=86400.;
    }

    int l = 1;
    int vx1 = static_cast<int>(plo0[1]/period);
    double sum_time = tab_time[1] = plo0[1];
    double vsum = v[1] = plo1[1];
    double end_time = static_cast<double>(vx1+1)*period;
    int licz = 0;
    double summa = 0.;
    double mean_time, mean_range, vv_mean, tim, timm[60], rmss[60], timdel[10];
    int jj, k = 0;

    for (int j=2; j<=selectedObs.npoint+1; ++j) {
        int vx;
        if (j == selectedObs.npoint+1) vx = -1;
        else vx = static_cast<int>(plo0[j]/period);

        if (plo0[j]<=end_time && vx==vx1) {
            ++l;
            tab_time[l] = plo0[j];
            sum_time += plo0[j];
            vsum += plo1[j];
            v[l] = plo1[j];
        } else {
            if (l==1) {
                mean_time = tab_time[1];
                mean_range = v[1];
            } else {
                mean_time = sum_time/l;
                vv_mean = vsum/l;
                vsum = 0.;
                for (jj=1; jj<=l; ++jj)
                    vsum += (v[jj]-vv_mean)*(v[jj]-vv_mean);

                tim = fabs(mean_time - tab_time[1]);
                jj = 1;
                for(k=2; k<=l; ++k) {
                    double xx=fabs(mean_time-tab_time[k]);
                    if (xx<tim)
                    {	jj=k;	tim=xx;	}
                }
                mean_time = tab_time[jj];
                mean_range = vv_mean;
            }
            ++licz;
            summa += mean_range;
            timm[licz] = mean_time;
            rmss[licz] = mean_range;

            l = 1;
            vv_mean = 0.;
            vx1 = static_cast<int>(plo0[j]/period);
            tab_time[l] = sum_time = plo0[j];
            vsum = v[l] = plo1[j];
            end_time = (vx+1)*period;
        }
    }

    if (licz > 1) {
        summa /= licz;
        double xy = 0.;
        for (int i=1; i<=licz; ++i)
            xy += (summa-rmss[i])*(summa-rmss[i]);
        xy = 2*sqrt(xy/(licz-1));

        k = 0;
        for (int i=1; i<=licz; ++i)
            if (fabs(rmss[i]) > xy) {
                ++k;
                timdel[k] = timm[i];
            }
    }
    if (k == 0) return true;

    licz = k;
    for (int i=1; i<=selectedObs.npoint; ++i)
        ind[i] = true;
    for (k=1; k<=licz; ++k) {
        double timeb = static_cast<double>(static_cast<int>(timdel[k]/period))*period;
        double timek = timeb + period;
        for (int i=1; i<=selectedObs.npoint; ++i)
            if (plo0[i] > timeb && plo0[i] <= timek) ind[i] = false;
    }

    jj = 0;
    int64_t tstart = 0;
    datamodels::CopyObs copy;
    datamodels::Dif deriv;
    for (int i=1; i<=selectedObs.npoint; ++i)
       if (ind[i]) {
           ++jj;
           if (jj == 1) tstart = static_cast<int64_t>(sCopy.data[i-1].sec);
           copy.data.push_back(sCopy.data[i-1]);
           deriv.data.push_back(sDeriv.data[i-1]);
           deriv.data.back().sec = copy.data.back().sec;
       }

    selectedObs.npoint = jj;
    if( std::abs(selectedObs.secday-tstart) > 80000L) selectedObs.mjd++;
    selectedObs.secday = tstart;
    selectedObs.RMS = selectedObs.POLY = 0.;
    selectedObs.ws = 2;

    UniqueKAT_OBS::Instance()->cat[nrObs-1] = selectedObs;
    sCopy = copy;
    sDeriv = deriv;
    sCopy.write(fnamecopy);
    sDeriv.write(fnamederiv);
    return false;
}

void Engine::prepareNPPlot()
{
    int period = satelliteInfo.np_window;

    std::ifstream station("D:\\LASER-2\\DATA\\STATION");
    if (!station.is_open()) {
        emit accessError("Unable to open STATION file! Provide it or press Cancel.");
        return;
    }
    std::string temp_line;
    std::getline(station, temp_line);
    char NAMSTAC[12];
    int NSTA;
    sscanf(temp_line.c_str(), "%s%d%*d%*d%*d", NAMSTAC,&NSTA);
    station.close();
    if (selectedObs.nsta != NSTA) {
        emit searchError("Different station numbers provided! In catalogue: " + QString::number(selectedObs.nsta) + ", in STATION file: " + QString::number(NSTA));
        return;
    }

    int dd,mth,yy,hh,min,ss;
    char header1[81], header2[81];
    utility::mjd_dat(dd,mth,yy, selectedObs.mjd);
    hh  = static_cast<int>(selectedObs.secday / 3600LL);
    min = static_cast<int>(selectedObs.secday % 3600LL / 60LL);
    ss  = static_cast<int>(selectedObs.secday % 3600LL % 60LL);
    sprintf(header1,"ST:%-4s SAT: %-5s %4d-%02d-%02d%4d:%02d:%02d %4d POINTS",
            NAMSTAC,satelliteInfo.name,yy,mth,dd,hh,min,ss,selectedObs.npoint);
    sprintf(header2,"TBIAS:%6.1lfMS RBIAS:%6.3lfM STEP :%2d RMS:%5.2lf",
        selectedObs.TB,selectedObs.RB,selectedObs.POLY,selectedObs.RMS*15.);

    QVector<double> X(selectedObs.npoint), Y(selectedObs.npoint); // plot data
    for (int i=0; i<selectedObs.npoint; ++i) {
        X[i] = sDeriv.data[i].sec;
        Y[i] = sDeriv.data[i].czeb;
        if (X[i] < X[0]) X[i] += 86400.;
    }

    QVector<double> tab_time(selectedObs.npoint), v(selectedObs.npoint);
    int l = 0, licz = 0;
    int vx1 = static_cast<int>(X[0]/period);
    double sum_time = tab_time[0] = X[0];
    double vsum = v[0] = Y[0];
    double end_time = static_cast<double>(vx1+1)*period;
    double summa = 0., timm[60], rmss[60];
    double mean_time, mean_range, vv_mean;

    for (int j=1; j<selectedObs.npoint; ++j) {

        int vx = static_cast<int>(X[j]/period);

        if (X[j] <= end_time && vx==vx1) {
            l++;
            tab_time[l] = X[j];
            sum_time += X[j];
            vsum += Y[j];
            v[l] = Y[j];
        } else {
            if (l==0) {
                mean_time = tab_time[0];
                mean_range = v[0];
            } else {
                mean_time = sum_time/static_cast<double>(l+1);
                vv_mean = vsum/static_cast<double>(l+1);
                vsum = 0.;
                for (int jj=0; jj<=l; ++jj)
                    vsum += (v[jj]-vv_mean)*(v[jj]-vv_mean);
                double tim = fabs(mean_time-tab_time[0]);

                int jj = 0;
                for (int k=1; k<=l; ++k) {
                    double xx = fabs(mean_time-tab_time[k]);
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
        double xy = 0.;
        for (int j=0; j<licz; ++j)
            xy += (summa-rmss[j])*(summa-rmss[j]);
        xy = 2.5*sqrt(xy/static_cast<double>(licz-1));

        for (int j=0; j<licz; ++j)
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

    emit plotNqlData(X,Y,Xl,Yl,XL,YL, QString(header1), QString(header2));
}

void Engine::writeKobs_Log()
{
    datamodels::CalCatElem selectedKal1 = UniqueKAT_CAL::Instance()->cat[calBefore-1];
    datamodels::CalCatElem selectedKal2 = UniqueKAT_CAL::Instance()->cat[calAfter-1];
    if (selectedObs.ws < 3) {
        emit logicError("This observation is not ready for catalogization!");
        return;
    }

    if (sKobs.read(fnamekobs)) {
        emit alreadyDoneError("File " + QString(fnamekobs) + " has already created!");
        return;
    }

    int elev_max = INT_MIN, range1_min = INT_MAX;
    double omc_min = 9e10, omc_max = -9e10;
    for (int i=0; i<selectedObs.npoint; ++i) {
        datamodels::KobsElem t;
        t.sec = sCopy.data[i].sec;
        t.azym = sCopy.data[i].azym;
        t.elev = sCopy.data[i].elev;
        t.range = sCopy.data[i].range;
        t.ampl = sCopy.data[i].ampl;
        t.czeb1 = sDeriv.data[i].czeb * 2.e9/C_VEL;
        sKobs.data.push_back(t);

        if (sCopy.data[i].elev > elev_max) elev_max = static_cast<int>(sCopy.data[i].elev);
        if (sCopy.data[i].range1 < range1_min) range1_min = static_cast<int>(sCopy.data[i].range1);
        if (sDeriv.data[i].omc > omc_max) omc_max = sDeriv.data[i].omc;
        if (sDeriv.data[i].omc < omc_min) omc_min = sDeriv.data[i].omc;
    }
    sKobs.write(fnamekobs);

    std::ofstream log(fnamelog);
    if (!log.is_open()) {
        emit accessError("Unable to open file " + QString(fnamelog) + " !");
        remove(fnamekobs);
        return;
    }
    char buf[64];
    sprintf(buf, "%6d\n", static_cast<int>(sCopy.data[0].elev)); log << buf;
    sprintf(buf, "%6d\n", elev_max); log << buf;
    sprintf(buf, "%6d\n", static_cast<int>(sCopy.data.back().elev)); log << buf;

    sprintf(buf, "%6d\n", static_cast<int>(sCopy.data[0].range1)); log << buf;
    sprintf(buf, "%6d\n", range1_min); log << buf;
    sprintf(buf, "%6d\n", static_cast<int>(sCopy.data.back().range1)); log << buf;

    double sec_pass = (sCopy.data[0].sec > sCopy.data.back().sec)?
                (sCopy.data.back().sec - sCopy.data[0].sec + 86400):
                (sCopy.data.back().sec - sCopy.data[0].sec);

    sprintf(buf, "%7.1lf\n", sec_pass/60); log << buf;
    sprintf(buf, "%8.3lf\n", selectedObs.TB); log << buf;
    sprintf(buf, "%8.3lf\n", selectedObs.RB); log << buf;
    sprintf(buf, "%8.2lf\n", omc_max); log << buf;
    sprintf(buf, "%8.2lf\n", omc_min); log << buf;
    sprintf(buf, "%4d\n", selectedObs.POLY); log << buf;

    sprintf(buf, "%8.2lf\n", selectedKal1.target); log << buf;
    sprintf(buf, "%6lld\n", selectedKal1.secday); log << buf;
    sprintf(buf, "%6d\n", selectedKal1.nreturn); log << buf;
    sprintf(buf, "%6d\n", selectedKal1.nflash); log << buf;
    sprintf(buf, "%6d\n", selectedKal1.filtr); log << buf;
    sprintf(buf, "%8.2lf\n", selectedKal2.target); log << buf;
    sprintf(buf, "%6lld\n", selectedKal2.secday); log << buf;
    sprintf(buf, "%6d\n", selectedKal2.nreturn); log << buf;
    sprintf(buf, "%6d\n", selectedKal2.nflash); log << buf;
    sprintf(buf, "%6d\n", selectedKal2.filtr); log << buf;
    log.close();
}

void Engine::prepareCRD()
{
    int	i,j,k,day,month,year,day_y,check_sum,l,jj,vx,vx1,year_cent,SYS_NUM,
        OCU_SEQ,TIM_SCALE,SYS_CALIB,CAL_INDIC,SYS_FLAG,flaglog=1,flag_cal;
    double	corect,corecta,corectb,rmsk1,rmsk2,rms_cal,tim,range,shift,LAS_WAV,
            period,vsum,sum_time,end_time,mean_range,v_mean,vv_mean,mean_time,xx,
            mi1,mi2,mi3,mi4,mi_tim[5000],mean_peak,mean_rang;
    int	num[60],numer;
    double timm[60],ran[60],rmss[60],summa,rmes[60];
    char	ss[70],ss1[2],name1[128],name2[128];
    int st_num,st_sys,st_occ,st_tim,trop_corr,cmas_corr,ampl_corr,sys_del,hz;
    int rang_typ,dig,sign;
    char st_name[5],*str_num;
    int  data_qv,m_relis,day_s,month_s,year_s,day_f,month_f,year_f,N_pulse;
    int64_t mjd_f,mjd_s;
    double tim1,tim2,hour1,hour2,sec1,sec2,minut1,minut2;
    double tr_wave,p_wave,fire_rate,pulse_width;
    double beam,a_w,qv,d_c,o_p,s_f1,tr_s,s_f2,e_delay;
    char sys_conf[5],a_conf[5],b_conf[5],c_conf[5],d_conf[5],laser_type[11],d_type[11],o_type[11],esp[11];
    char t_source[21],f_sourse[21],timer[21],s_num[21],str_name[9];
    double cal_peak1,cal_peak2;
    double las_en, feu_volt;
    int cal_type,cal_shift;
    int cal_type_ind,cal_shift_ind,det_canal;
    double tab_time[5000],v[5000],rang[5000];
    std::string temp_line;
    char buf[256];
    k = 0;

    datamodels::CalCatElem selectedKal1 = UniqueKAT_CAL::Instance()->cat[calBefore-1];
    datamodels::CalCatElem selectedKal2 = UniqueKAT_CAL::Instance()->cat[calAfter-1];

    std::ofstream edc_npt("D:\\LASER-2\\DATA\\SEND\\EDC.NPT");
    if (!edc_npt.is_open()) {
        emit accessError("Unable to open FILE D:\\LASER-2\\DATA\\SEND\\EDC.NPT !");
        return;
    }

    std::ifstream station("D:\\LASER-2\\DATA\\STATION");
    if (!station.is_open()) {
        emit accessError("Unable to open STATION file! Provide it or press Cancel.");
        return;
    }
    std::getline(station, temp_line);
    sscanf(temp_line.c_str(),"%4s%d%d%d%d",&st_name,&st_num,&st_sys,&st_occ,&st_tim);
    std::getline(station, temp_line);
    sscanf(temp_line.c_str(),"%d%d%d%d%d%d%d",&trop_corr,&cmas_corr,&ampl_corr,&sys_del,&hz,&rang_typ,&data_qv);
    str_num = ecvt(static_cast<double>(st_num),4,&dig,&sign);

    std::ofstream tmp1("D:\\LASER-2\\DATA\\1.TMP"), tmp2("D:\\LASER-2\\DATA\\2.TMP");

    tim1 = sKobs.data[0].sec;
    tim2 = sKobs.data.back().sec+1;
    sprintf(buf,"time_x start=%lf",tim1);
    tmp2 << buf << std::endl;
    sprintf(buf,"time_x finish=%lf",tim2);
    tmp2 << buf << std::endl;

    mjd_f = mjd_s = selectedObs.mjd;
    if (tim2 < tim1) ++mjd_f;
    utility::mjd_dat(day_s,month_s,year_s,mjd_s);
    utility::mjd_dat(day_f,month_f,year_f,mjd_f);

    sprintf(buf,"tim1=%.lf  tim2=%.lf", tim1, tim2);
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

    sprintf(name1,"D:\\LASER-2\\DATA\\SEND\\%4s_%1.10s_crd_%04d%02d%02d_%02.lf_00.npt",str_num, satelliteInfo.name, year_s, month_s, day_s, hour1);
    sprintf(name2,"D:\\LASER-2\\DATA\\SEND\\%4s_%1.10s_crd_%04d%02d%02d_%02.lf_00.frd",str_num, satelliteInfo.name, year_s, month_s, day_s, hour1);
    fnameNPT = QString(name1);
    fnameFRD = QString(name2);

    std::ofstream NPT_CRD(name1);
    if (!NPT_CRD.is_open()) {
        emit accessError("Unable to write the NPT file " + fnameNPT + " !");
        return;
    }
    std::ofstream FRD_CRD(name2);
    if (!FRD_CRD.is_open()) {
        emit accessError("Unable to write the FRD file " + fnameFRD + " !");
        return;
    }
    std::ifstream las_feu("D:\\LASER-2\\DATA\\las_feu.dat");
    if (!las_feu.is_open()) {
        emit accessError("Unable to open FILE D:\\LASER-2\\DATA\\las_feu.dat !");
        return;
    }

    SYS_NUM=st_sys;
    OCU_SEQ=st_occ;
    std::ifstream s_conf("D:\\LASER-2\\DATA\\SYS_CONF");
    if (!s_conf.is_open()) {
        emit accessError("Unable to open configuration description file SYS_CONF!");
        return;
    }
    std::getline(s_conf, temp_line);
    sscanf(temp_line.c_str(),"%lf%s%s%s%s%s",&tr_wave,&sys_conf,&a_conf,&b_conf,&c_conf,&d_conf);
    std::getline(s_conf, temp_line);
    sscanf(temp_line.c_str(),"%s%lf%lf%lf%lf%d",&laser_type,&p_wave,&fire_rate,&pulse_width,&beam,&N_pulse);
    std::getline(s_conf, temp_line);
    sscanf(temp_line.c_str(),"%s %lf %lf %lf %s %lf %lf %lf %lf %s",&d_type,&a_w,&qv,&d_c,&o_type,&o_p,&s_f1,&tr_s,&s_f2,&esp);
    std::getline(s_conf, temp_line);
    sscanf(temp_line.c_str(),"%s %s %s %s %lf",&t_source,&f_sourse,&timer,&s_num,&e_delay);
    LAS_WAV=tr_wave/1000.0;

    double det_amp_gain = 35.0;
    double det_amp_bw = 500000.0; // kHz

    time_t t = time(nullptr);
    struct tm tm = *localtime(&t);
    sprintf(buf,"H1 CRD 02 %04d %02d %02d %02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour);
    NPT_CRD << buf << std::endl;
    FRD_CRD << buf << std::endl;
    sprintf(buf,"H2 %10s %04d %02d %02d %02d EUROLAS",st_name,st_num,st_sys,st_occ,st_tim);
    NPT_CRD << buf << std::endl;
    FRD_CRD << buf << std::endl;

    rmsk1=rmsk2=corecta=corectb=0.0;
    flag_cal=0;
    if(calBefore!=0) flag_cal++;
    if(calAfter!=0) flag_cal++;

    sprintf(buf,"%8s",selectedObs.namefe);
    tmp1 << buf << std::endl;
    tmp1.close();

    period=static_cast<double>(satelliteInfo.np_window);
    edc_npt << "99999" << std::endl;
    sprintf(buf,"H3 %10s % 8lld %04d %08lld %1d %1d 1",satelliteInfo.name,satelliteInfo.id,satelliteInfo.sic,
            satelliteInfo.norad,satelliteInfo.transponder,satelliteInfo.i_flag);
    NPT_CRD << buf << std::endl;
    FRD_CRD << buf << std::endl;

    utility::mjd_dat(day,month,year,selectedObs.mjd);
    day_y = utility::day_year(day,month,year);
    year_cent=year-(year/100)*100;

    if(flag_cal==2) {
        shift=fabs(selectedKal1.meanval - selectedKal2.meanval);
        corect=(selectedKal1.corect + selectedKal2.corect)*0.5;
        rms_cal=(selectedKal1.rms + selectedKal2.rms)*0.5;
    } else {
        shift=0;
    }
    if(flag_cal==1) {
      corect=selectedKal1.corect + selectedKal2.corect;
      rms_cal=selectedKal1.rms + selectedKal2.rms;
    }
    if(flag_cal==0) {
        corect=0;
        rms_cal=0;
    }

    TIM_SCALE=7; SYS_CALIB=0;CAL_INDIC=2; SYS_FLAG=2;
    sprintf(ss,"%.7lld%02d%03d%4d%02d%02d%4.0lf%08.0lf%06.0lf"
        "%04.0lf%1d%1d%1d%1d%1d%04.0lf0",
        selectedObs.nsat,year_cent,day_y,selectedObs.nsta,SYS_NUM,OCU_SEQ,
        LAS_WAV*1.e4,corect*1000.,shift*1000.,rms_cal*1000.,
        satelliteInfo.NOR_POINT,TIM_SCALE,SYS_CALIB,CAL_INDIC,
        SYS_FLAG,selectedObs.RMS*1000.);
    check_sum=0;
    vv_mean=0.;
    for(j=0;j<=51;j++) {
        ss1[0]=ss[j];
        check_sum += atoi(ss1);
    }
    sprintf(buf,"%s%02d%d",ss,check_sum%100,flaglog);
    edc_npt << buf << std::endl;

    bool laserConfFound = false;
    las_feu.clear();
    las_feu.seekg(0, std::ios_base::beg);
    while (std::getline(las_feu, temp_line)) {
        sscanf(temp_line.c_str(),"%9s %lf %lf\n",&str_name,&las_en,&feu_volt);
        if(strncmp(str_name,selectedObs.namefe,9)==0) {
            laserConfFound = true;
            break;
        }
    }
    if(!laserConfFound) {
        emit searchError("Energy laser and feu volage not found for pass " + QString(selectedObs.namefe));
        las_en = QInputDialog::getDouble(nullptr, "", "Please, input laser energy", 0, -1e10, 1e10, 1);
        feu_volt = QInputDialog::getDouble(nullptr, "", "Please, input FEU voltage", 0, -1e10, 1e10, 1);
    }

    sprintf(buf,"H4 01 %4d %02d %02d %2.lf %2.lf %2.lf %04d %02d %02d %2.lf %2.lf %2.lf %02d %1d %1d %1d %1d %1d %1d %1d",
             year_s,month_s,day_s,hour1,minut1,sec1,year_f,month_f,day_f,hour2,minut2,sec2,m_relis,
             trop_corr,cmas_corr,ampl_corr,sys_del,hz,rang_typ,data_qv);
    NPT_CRD << buf << std::endl;
    sprintf(buf,"H4 00 %4d %02d %02d %2.lf %2.lf %2.lf %04d %02d %02d %2.lf %2.lf %2.lf %02d %1d %1d %1d %1d %1d %1d %1d",
             year_s,month_s,day_s,hour1,minut1,sec1,year_f,month_f,day_f,hour2,minut2,sec2,m_relis,
             trop_corr,cmas_corr,ampl_corr,sys_del,hz,rang_typ,data_qv);
    FRD_CRD << buf << std::endl;

    sprintf(buf,"mjd=%lld  start=%lld   finish=%lld",selectedObs.mjd,mjd_s,mjd_f); tmp2 << buf << std::endl;
    sprintf(buf,"start day is %d  %d  %d",year_s,month_s,day_s);    tmp2 << buf << std::endl;
    sprintf(buf,"start in %.lf %.lf %.lf",hour1,minut1,sec1);       tmp2 << buf << std::endl;
    sprintf(buf,"finish day is %d  %d  %d",year_f,month_f,day_f);   tmp2 << buf << std::endl;
    sprintf(buf,"finish time %.lf  %.lf  %.lf",hour2,minut2,sec2);  tmp2 << buf << std::endl;


    strcpy(laser_type, "    Nd-Yag");

    sprintf(buf,"C0 0 %10.3lf %4s %4s %4s %4s %4s",tr_wave,sys_conf,a_conf,b_conf,c_conf,d_conf);
    NPT_CRD << buf << std::endl;
    FRD_CRD << buf << std::endl;
    sprintf(buf,"C1 0 %4s %10s %10.2lf %10.2lf %10.2lf %6.1lf %5.2lf %4d",a_conf,laser_type,p_wave,fire_rate,las_en,pulse_width,beam,N_pulse);
    NPT_CRD << buf << std::endl;
    FRD_CRD << buf << std::endl;
    sprintf(buf,"C2 0 %4s %10s %10.3lf %6.2lf %5.1lf %5.1lf %10s %5.1lf %5.2lf %5.1lf %5.1lf %10s %6.1lf %6.1lf 1",b_conf,d_type,a_w,qv,feu_volt,d_c,o_type,o_p,s_f1,tr_s,s_f2,esp,det_amp_gain,det_amp_bw);
    NPT_CRD << buf << std::endl;
    FRD_CRD << buf << std::endl;
    sprintf(buf,"C3 0 %4s %20s %20s %20s %20s %6.1lf",c_conf,t_source,f_sourse,timer,s_num,e_delay);
    NPT_CRD << buf << std::endl;
    FRD_CRD << buf << std::endl;

    cal_type=0;
    cal_type_ind=2;
    cal_shift_ind=0;
    det_canal=0;
    int cal_span = 3; // Combined (pre- and post-calibrations or multiple)
    cal_shift=1;
    cal_peak1=selectedKal1.meanval*1000.0;
    cal_peak2=selectedKal2.meanval*1000.0;

    double ret_rate_40 = static_cast<double>(selectedKal1.nreturn+selectedKal2.nreturn)/(selectedKal1.nflash+selectedKal2.nflash) * 100,
           ret_rate_41b= static_cast<double>(selectedKal1.nreturn)/selectedKal1.nflash * 100,
           ret_rate_41a= static_cast<double>(selectedKal2.nreturn)/selectedKal2.nflash * 100;

    sprintf(buf,"40 %18.12lf %1d %4s %8d %8d %7.3lf %10.1lf %8.1lf %6.1lf %7.3lf %7.3lf %6.1lf %1d %1d %1d %1d %5.1lf",
              static_cast<double>(selectedKal1.secday),cal_type,sys_conf,(selectedKal1.nflash+selectedKal2.nflash)/2,(selectedKal1.nreturn+selectedKal2.nreturn)/2,
              (selectedKal1.target+selectedKal2.target)/2,(cal_peak1+cal_peak2)/2,shift*1000.0,rms_cal*1000.0,(selectedKal1.skew+selectedKal2.skew)/2,(selectedKal1.kurt+selectedKal2.kurt)/2,
              0.0,cal_type_ind,cal_shift_ind,det_canal,cal_span, ret_rate_40);
    NPT_CRD << buf << std::endl;
    FRD_CRD << buf << std::endl;

    sprintf(buf,"41 %18.12lf %1d %4s %8d %8d %7.3lf %10.1lf %8.1lf %6.1lf %7.3lf %7.3lf %6.1lf %1d %1d %1d %1d %5.1lf",
              static_cast<double>(selectedKal1.secday),cal_type,sys_conf,selectedKal1.nflash,selectedKal1.nreturn,
              selectedKal1.target,cal_peak1,shift*1000.0,selectedKal1.rms*1000.0,selectedKal1.skew,selectedKal1.kurt,0.0,
              cal_type_ind,cal_shift_ind,det_canal,1,ret_rate_41b);
    NPT_CRD << buf << std::endl;
    FRD_CRD << buf << std::endl;

    sprintf(buf,"41 %18.12lf %1d %4s %8d %8d %7.3lf %10.1lf %8.1lf %6.1lf %7.3lf %7.3lf %6.1lf %1d %1d %1d %1d %5.1lf",
              static_cast<double>(selectedKal2.secday),cal_type,sys_conf,selectedKal2.nflash,selectedKal2.nreturn,
              selectedKal2.target,cal_peak2,shift*1000.0,selectedKal2.rms*1000.0,selectedKal2.skew,selectedKal2.kurt,0.0,
              cal_type_ind,cal_shift_ind,det_canal,2,ret_rate_41a);
    NPT_CRD << buf << std::endl;
    FRD_CRD << buf << std::endl;


//    if(flag_cal!=0) {
//        sprintf(buf,"40 %18.12lf %1d %4s %8d %8d %7.3lf %10.1lf %8.1lf %6.1lf %7.3lf %7.3lf %10.1lf %1d %1d %1d",
//                  static_cast<double>(selectedKal1.secday),cal_type,sys_conf,selectedKal1.nflash,selectedKal1.nreturn,
//                  selectedKal1.target,cal_peak1,shift*1000.0,selectedKal1.rms*1000.0,selectedKal1.skew,selectedKal1.kurt,cal_peak1,
//                  cal_type_ind,cal_shift_ind,det_canal);
//        NPT_CRD << buf << std::endl;
//        FRD_CRD << buf << std::endl;
//    }

    tim1 = sKobs.data[0].sec;
    sprintf(buf,"20 %18.12lf %7.2lf %6.2lf %4d 1", tim1, selectedObs.pres,selectedObs.temp,selectedObs.humid);
    NPT_CRD << buf << std::endl;
    FRD_CRD << buf << std::endl;
    sprintf(buf,"10 %18.12lf %18.12lf %4s 2 2 0 0 %5d na", tim1, sKobs.data[0].range/1000000000.0, sys_conf, sKobs.data[0].ampl);
    FRD_CRD << buf << std::endl;

    vx1=static_cast<int>(tim1/period);
    l=1;
    tab_time[l]=sum_time=tim1;
    vsum=v[l]=sKobs.data[0].czeb1;
    rang[l]=sKobs.data[0].range;
    end_time=static_cast<double>(vx1+1)*period;
    numer=0;
    summa=0.;
    for (j=1; j<=selectedObs.npoint; ++j) {
        if (j == selectedObs.npoint)
            vx = -1;
        else {
            vx = static_cast<int>(sKobs.data[j].sec/period);
            sprintf(buf,"10 %18.12lf %18.12lf %4s 2 2 0 0 %5d na", sKobs.data[j].sec, sKobs.data[j].range/1000000000.0, sys_conf, sKobs.data[j].ampl);
            FRD_CRD << buf << std::endl;
        }
        if (sKobs.data[j].sec <= end_time && vx==vx1) {
            ++l;
            tab_time[l] = sKobs.data[j].sec;
            sum_time += sKobs.data[j].sec;
            vsum += sKobs.data[j].czeb1;
            v[l] = sKobs.data[j].czeb1;
            rang[l] = sKobs.data[j].range;
        } else {
            if (l == 1) {
                v_mean = selectedObs.RMS;
                mean_time = tab_time[1];
                mean_range = rang[1] - corect;
                vv_mean = v[1];
                if (flag_cal == 0) {cSkew = cKurt = 0;}
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
                if(flag_cal==0) {
                    cSkew=(mi3*mi3)/(mi2*mi2*mi2);
                    cKurt=(mi4)/(mi2*mi2);
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
            mean_time*1.e7,mean_range*1000.,v_mean*1000.,selectedObs.pres*10.,selectedObs.temp*10.,selectedObs.humid,l);
            check_sum=0;
            sprintf(buf,"11 %18.12lf %18.12lf %4s 2 %6.1lf %6d %9.1lf %7.3lf %7.3lf %9.1lf %5.1lf 0 na",
                     mean_time,mean_range/1000000000.0,sys_conf,static_cast<double>(satelliteInfo.np_window),l,v_mean*1000.0,cSkew,cKurt,v_mean*500.0,
                    static_cast<double>(l*10/satelliteInfo.np_window));
            NPT_CRD << buf << std::endl;

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
            vx1=static_cast<int>(sKobs.data[j].sec/period);
            tab_time[l]=sum_time=sKobs.data[j].sec;
            vsum=v[l]=sKobs.data[j].czeb1;
            rang[l]=sKobs.data[j].range;
            end_time=static_cast<double>(vx+1)*period;
        }
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
        if(k<num[i])
            k=num[i];
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
    emit info("RMS=" + QString::number(selectedObs.RMS, 'f', 3) + ",  mean_peak=" + QString::number(mean_peak, 'f', 3));
    if(mean_peak>9999.9) mean_peak=0.0;

    sprintf(buf,"50 %4s %6.1lf %7.3lf %7.3lf %6.1lf 1",sys_conf,selectedObs.RMS*1000,cSkew,cKurt,mean_peak);
    NPT_CRD << buf << std::endl;
    sprintf(buf,"H8");
    NPT_CRD << buf << std::endl;
    FRD_CRD << buf << std::endl;

    sprintf(buf,"H9");
    NPT_CRD << buf << std::endl;
    FRD_CRD << buf << std::endl;
}
