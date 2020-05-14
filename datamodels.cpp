#include "datamodels.h"

using namespace datamodels;

bool ObsCatalogue::read(const std::string& fname)
{
    std::ifstream in(fname);
    if (!in.is_open()) {
        std::cout << "Unable to read file " << fname << std::endl;
        return false;
    }

    std::string temp_line;
    cat.clear();
    int n = 1;
    while (std::getline(in, temp_line)) {
        if (temp_line.empty()) continue;
        ObsCatElem r;
        if (sscanf(temp_line.c_str(), datamodels::FormatKAT_OBS_R,
            &r.c, &r.ws, &r.ws1, &r.mjd, &r.secday, &r.nsat, &r.nsta, r.kodst, r.namefe,
            &r.npoint, &r.temp, &r.pres, &r.humid, &r.TB, &r.RB, &r.RMS, &r.POLY, &r.clock_cor) != 18) {

            std::cout << "Read error in KAT_OBS, Line " << cat.size()+1 << std::endl;
            return false;
        }
        r.nr = n;
        cat.push_back(r);
        ++n;
    }
    return true;
}

bool ObsCatalogue::write(const std::string& fname)
{
    std::ofstream out(fname);
    if (!out.is_open()) {
        std::cout << "Unable to write file " << fname << std::endl;
        return false;
    }
    char buf[256];
    for (ObsCatElem r: cat) {
        sprintf(buf, datamodels::FormatKAT_OBS_W,
                r.ws, r.ws1, r.mjd, r.secday, r.nsat, r.nsta, r.kodst, r.namefe, r.npoint,
                r.temp, r.pres, r.humid, r.TB, r.RB, r.RMS, r.POLY, r.clock_cor);
        out << buf << std::endl;
    }
    return true;
}


bool CalCatalogue::read(const std::string& fname)
{
    std::ifstream in(fname);
    if (!in.is_open()) {
        std::cout << "Unable to read file " << fname << std::endl;
        return false;
    }

    std::string temp_line;
    cat.clear();
    int n = 1;
    while (std::getline(in, temp_line)) {
        if (temp_line.empty()) continue;
        CalCatElem r;
        if (sscanf(temp_line.c_str(), datamodels::FormatKAT_KAL_R,
            &r.c, &r.nsta, r.kodst, r.nam, &r.mjd, &r.secday, &r.filtr, &r.meanval, &r.rms, &r.corect, &r.temp, &r.baro,
            &r.humid, &r.nflash, &r.nreturn, &r.weather, &r.numberpas, &r.coef, &r.skew, &r.kurt, &r.target, r.namcal) != 22) {

            std::cout << "Read error in KAT_OBS, Line " << cat.size()+1 << std::endl;
            return false;
        }
        r.nr = n;
        cat.push_back(r);
        ++n;
    }
    return true;
}

bool CalCatalogue::write(const std::string& fname)
{
    std::ofstream out(fname);
    if (!out.is_open()) {
        std::cout << "Unable to write file " << fname << std::endl;
        return false;
    }
    char buf[256];
    for (CalCatElem r: cat) {
        sprintf(buf, datamodels::FormatKAT_KAL_W,
                r.nsta, r.kodst, r.nam, r.mjd, r.secday, r.filtr, r.meanval, r.rms, r.corect,
                r.temp, r.baro, r.humid, r.nflash, r.nreturn, r.weather, r.numberpas, r.coef, r.skew, r.kurt, r.target, r.namcal);
        out << buf << std::endl;
    }
    return true;
}

bool SatellitCatalogue::read(const std::string& fname)
{
    std::ifstream in(fname);
    if (!in.is_open()) {
        std::cout << "Unable to read file " << fname << std::endl;
        return false;
    }

    std::string temp_line;
    cat.clear();
    while (std::getline(in, temp_line)) {
        if (temp_line.empty()) continue;
        SatellitLine r;
        if (sscanf(temp_line.c_str(), datamodels::FormatSATELLIT_R,
            r.name, &r.id, &r.sic, &r.norad, &r.transponder,
            &r.i_flag, &r.NOR_POINT, &r.np_window, r.sname) != 9) {

            std::cout << "Read error in SATELLIT, Line " << cat.size()+1 << std::endl;
            return false;
        }
        cat.push_back(r);
    }
    return true;
}

SatellitLine SatellitCatalogue::getSatById(int64_t sat_id)
{
    for (SatellitLine r: cat) {
        if (r.id == sat_id) return r;
    }
    return SatellitLine{"",0,0,0,0,0,0,0,""};
}

bool CopyObs::read(const std::string& fname)
{
    std::ifstream in(fname);
    if (!in.is_open()) {
        std::cout << "Unable to read file " << fname << std::endl;
        return false;
    }

    std::string temp_line;
    data.clear();
    while (std::getline(in, temp_line)) {
        if (temp_line.empty()) continue;
        CopyObsElem r;
        if (sscanf(temp_line.c_str(), datamodels::FormatCGA_R,
            &r.sec, &r.azym, &r.elev, &r.range, &r.range1, &r.ampl) != 6) {

            std::cout << "Read error in obs copy " << fname << ", Line " << data.size()+1 << std::endl;
            return false;
        }
        data.push_back(r);
    }
    return true;
}

bool CopyObs::write(const std::string& fname)
{
    std::ofstream out(fname);
    if (!out.is_open()) {
        std::cout << "Unable to write file " << fname << std::endl;
        return false;
    }
    char buf[256];
    for (CopyObsElem r: data) {
        sprintf(buf, datamodels::FormatCGA_W,
                r.sec, r.azym, r.elev, r.range, r.range1, r.ampl);
        out << buf << std::endl;
    }
    return true;
}

bool Obs::read(const std::string& fname)
{
    std::ifstream in(fname);
    if (!in.is_open()) {
        std::cout << "Unable to read file " << fname << std::endl;
        return false;
    }

    std::string temp_line;
    data.clear();
    while (std::getline(in, temp_line)) {
        if (temp_line.empty()) continue;
        ObsElem r;
        if (sscanf(temp_line.c_str(), datamodels::FormatOGA_R,
            &r.sec, &r.azym, &r.elev, &r.range, &r.ampl) != 5) {

            std::cout << "Read error in obs " << fname << ", Line " << data.size()+1 << std::endl;
            return false;
        }
        data.push_back(r);
    }
    return true;
}

bool Efem::read(const std::string& fname)
{
    std::ifstream in(fname);
    if (!in.is_open()) {
        std::cout << "Unable to read file " << fname << std::endl;
        return false;
    }

    std::string temp_line;
    data.clear();
    while (std::getline(in, temp_line)) {
        if (temp_line.empty()) continue;
        EfemElem r;
        if (sscanf(temp_line.c_str(), datamodels::FormatEGA_R,
            &r.c, &r.sec, &r.azym, &r.elev, &r.range) != 5) {

            std::cout << "Read error in efem file " << fname << ", Line " << data.size()+1 << std::endl;
            return false;
        }
        data.push_back(r);
    }
    return true;
}

bool Dif::read(const std::string& fname)
{
    std::ifstream in(fname);
    if (!in.is_open()) {
        std::cout << "Unable to read file " << fname << std::endl;
        return false;
    }

    std::string temp_line;
    data.clear();
    while (std::getline(in, temp_line)) {
        if (temp_line.empty()) continue;
        DifElem r;
        if (sscanf(temp_line.c_str(), datamodels::FormatPGA_R,
            &r.sec, &r.omc, &r.tbrb, &r.czeb) != 4) {

            std::cout << "Read error in derivatives file " << fname << ", Line " << data.size()+1 << std::endl;
            return false;
        }
        data.push_back(r);
    }
    return true;
}

bool Dif::write(const std::string& fname)
{
    std::ofstream out(fname);
    if (!out.is_open()) {
        std::cout << "Unable to write file " << fname << std::endl;
        return false;
    }
    char buf[256];
    for (DifElem r: data) {
        sprintf(buf, datamodels::FormatPGA_W,
                r.sec, r.omc, r.tbrb, r.czeb);
        out << buf << std::endl;
    }
    return true;
}

bool Kobs::read(const std::string& fname)
{
    std::ifstream in(fname);
    if (!in.is_open()) {
        std::cout << "Unable to read file " << fname << std::endl;
        return false;
    }

    std::string temp_line;
    data.clear();
    while (std::getline(in, temp_line)) {
        if (temp_line.empty()) continue;
        KobsElem r;
        if (sscanf(temp_line.c_str(), datamodels::FormatKGA_R,
            &r.sec, &r.azym, &r.elev, &r.range, &r.ampl, &r.czeb1) != 6) {

            std::cout << "Read error in k** file " << fname << ", Line " << data.size()+1 << std::endl;
            return false;
        }
        data.push_back(r);
    }
    return true;
}

bool Kobs::write(const std::string& fname)
{
    std::ofstream out(fname);
    if (!out.is_open()) {
        std::cout << "Unable to write file " << fname << std::endl;
        return false;
    }
    char buf[256];
    for (KobsElem r: data) {
        sprintf(buf, datamodels::FormatKGA_W,
                r.sec, r.azym, r.elev, r.range, r.ampl, r.czeb1);
        out << buf << std::endl;
    }
    return true;
}
