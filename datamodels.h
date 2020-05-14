#ifndef DATAMODELS_H
#define DATAMODELS_H

#include <fstream>
#include <string>
#include <vector>
#include <iostream>

namespace datamodels {

const char* const FormatKAT_OBS_R = "%c%d%d%lld%lld%lld%d%2s%8s%d%lf%lf%d%lf%lf%lf%d%lf";
const char* const FormatKAT_OBS_W = "e%3d%3d%6lld%6lld%8lld%5d%3s%9s%6d%6.1lf%7.1lf%3d%8.3lf%10.2lf%6.3lf%3d%7.1lf";

const char* const FormatKAT_KBO_R = "%c%lld%lld%d%3s%lld%lld%lf%lf%d%d%lf%d%d%d%8s%lf";
const char* const FormatKAT_KBO_W = "e%7lld%8lld%5d%3s%6lld%6lld%6.1lf%7.1lf%3d%6d%6.3lf%3d%4d%4d% 9s%7.1lf";

const char* const FormatOGA_R = "%lf%lf%lf%lf%d";
const char* const FormatCGA_W = " %20.12lf%10.4lf%8.4lf%14.3lf%15.7lf%5d";
const char* const FormatCGA_R = " %lf%lf%lf%lf%lf%d";
const char* const FormatEGA_R = "%c%lld%lf%lf%lf";
const char* const FormatPGA_W = " %20.12lf%12.4lf%12.4lf%12.4lf";
const char* const FormatPGA_R = "%lf%lf%lf%lf";
const char* const FormatKGA_R = "%lf%lf%lf%lf%d%lf";
const char* const FormatKGA_W = "%20.12lf%9.4lf%8.4lf%14.3lf%5d%8.4lf";

const char* const FormatKAT_KAL_R = "%c%d%2s%2s%lld%lld%d%lf%lf%lf%lf%lf%d%d%d%d%lld%d%lf%lf%lf%12s";
const char* const FormatKAT_KAL_W = "e %5d %2s %2s%6lld%6lld%4d%9.3lf%6.3lf%9.3lf%6.1lf%7.1lf%3d%5d%4d%3d%7lld%3d%7.3lf%7.3lf%12.4lf % 12s";

const char* const FormatSYS_CONF_R = "%lf%s%s%s%s%s";
const char* const FormatKAT_TAR_R = "%*d%*2s%2s%lf%*lf%*lf%*20s";
const char* const FormatEPA_AUTO_W = "%4d%6lld%4d%4d%8lld %10s";
const char* const FormatEPA_AUTO_R = "%d%lld%d%d%lld";

const char* const FormatCOPY_W = "%20.12lf%9.4lf%8.4lf%14.3lf%15.7lf%5d";

const char* const FormatSATELLIT_R = "%10s%lld%d%lld%d%d%d%d%s";


template <typename T>
class Singleton {
public:
    static T* Instance() {
        if (!m_instance)
            m_instance = new T();
        return m_instance;
    }
    Singleton(const Singleton&) = delete;
    Singleton& operator= (const Singleton&) = delete;
private:
    static T* m_instance;
};

template <typename T>
T* Singleton<T>::m_instance = nullptr;


// following structure and class represents KAT_OBS data.
struct ObsCatElem {
    int nr;
    char c;
    int ws, ws1;
    int64_t mjd, secday;
    int64_t nsat;
    int nsta;
    char kodst[3], namefe[9];
    int npoint;
    double temp, pres;
    int humid;
    double TB, RB, RMS;
    int POLY;
    double clock_cor;
};
class ObsCatalogue {
public:
    ObsCatalogue()=default;
    ~ObsCatalogue()=default;
    bool read (const std::string& fname);
    bool write(const std::string& fname);
    std::vector<ObsCatElem> cat;
};


// following structure and class represents KAT_KAL data.
struct CalCatElem {
    int nr;
    char c;
    int nsta;
    char kodst[3], nam[3];
    int64_t mjd, secday;
    int filtr;
    double meanval, rms, corect, temp, baro;
    int humid, nflash, nreturn, weather;
    int64_t numberpas;
    int coef;
    double skew, kurt, target;
    char namcal[13];
};
class CalCatalogue {
public:
    CalCatalogue()=default;
    ~CalCatalogue()=default;
    bool read (const std::string& fname);
    bool write(const std::string& fname);
    std::vector<CalCatElem> cat;
};


// following structure and class represents SATELLIT file's data.
struct SatellitLine {
    char name[11];
    int64_t id;
    int sic;
    int64_t norad;
    int transponder, i_flag, NOR_POINT, np_window;
    char sname[4];
};
class SatellitCatalogue {
public:
    SatellitCatalogue()=default;
    ~SatellitCatalogue()=default;
    bool read (const std::string& fname);
    // if sat not found by sat_id, returns blank: SatellitLine{"",0,0,0,0,0,0,0,""}
    SatellitLine getSatById(int64_t sat_id);
    std::vector<SatellitLine> cat;
};


// .cXX files data.
struct CopyObsElem {
    double sec, azym, elev, range1, range;
    int ampl;
};
class CopyObs {
public:
    CopyObs()=default;
    ~CopyObs()=default;
    bool read (const std::string& fname);
    bool write(const std::string& fname);
    std::vector<CopyObsElem> data;
};


// .oXX files data.
struct ObsElem {
    double sec, azym, elev, range;
    int ampl;
};
class Obs {
public:
    Obs()=default;
    ~Obs()=default;
    bool read (const std::string& fname);
    std::vector<ObsElem> data;
};


// .eXX files data.
struct EfemElem {
    char c;
    double azym, elev, range;
    int64_t sec;
};
class Efem {
public:
    Efem()=default;
    ~Efem()=default;
    bool read (const std::string& fname);
    std::vector<EfemElem> data;
};


// .pXX files data.
struct DifElem {
    double sec, omc, tbrb, czeb;
};
class Dif {
public:
    Dif()=default;
    ~Dif()=default;
    bool read (const std::string& fname);
    bool write(const std::string& fname);
    std::vector<DifElem> data;
};

// .kXX files data.
struct KobsElem {
    double sec, azym, elev, range, czeb1;
    int ampl;
};
class Kobs {
public:
    Kobs()=default;
    ~Kobs()=default;
    bool read (const std::string& fname);
    bool write(const std::string& fname);
    std::vector<KobsElem> data;
};

}

typedef datamodels::Singleton<datamodels::ObsCatalogue> UniqueKAT_OBS;
typedef datamodels::Singleton<datamodels::CalCatalogue> UniqueKAT_CAL;
typedef datamodels::Singleton<datamodels::SatellitCatalogue> UniqueKAT_SAT;


#endif // DATAMODELS_H
