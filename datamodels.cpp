#include "datamodels.h"

bool datamodels::ObsCatalogue::read(const std::string& fname)
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
        datamodels::ObsCatElem r;
        if (sscanf(temp_line.c_str(), FormatKAT_OBS_R,
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

bool datamodels::ObsCatalogue::write(const std::string& fname)
{
    std::ofstream out(fname);
    if (!out.is_open()) {
        std::cout << "Unable to write file " << fname << std::endl;
        return false;
    }
    char buf[256];
    for (ObsCatElem r: cat) {
        sprintf(buf, FormatKAT_OBS_W,
                r.ws, r.ws1, r.mjd, r.secday, r.nsat, r.nsta, r.kodst, r.namefe, r.npoint,
                r.temp, r.pres, r.humid, r.TB, r.RB, r.RMS, r.POLY, r.clock_cor);
        out << buf << std::endl;
    }
    return true;
}


bool datamodels::CalCatalogue::read(const std::string& fname)
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
        datamodels::CalCatElem r;
        if (sscanf(temp_line.c_str(), FormatKAT_KAL_R,
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

bool datamodels::CalCatalogue::write(const std::string& fname)
{
    std::ofstream out(fname);
    if (!out.is_open()) {
        std::cout << "Unable to write file " << fname << std::endl;
        return false;
    }
    char buf[256];
    for (CalCatElem r: cat) {
        sprintf(buf, FormatKAT_KAL_W,
                r.nsta, r.kodst, r.nam, r.mjd, r.secday, r.filtr, r.meanval, r.rms, r.corect,
                r.temp, r.baro, r.humid, r.nflash, r.nreturn, r.weather, r.numberpas, r.coef, r.skew, r.kurt, r.target, r.namcal);
        out << buf << std::endl;
    }
    return true;
}

bool datamodels::SatellitCatalogue::read(const std::string& fname)
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
        datamodels::SatellitLine r;
        if (sscanf(temp_line.c_str(), FormatSATELLIT_R,
            r.name, &r.id, &r.sic, &r.norad, &r.transponder,
            &r.i_flag, &r.NOR_POINT, &r.np_window, r.sname) != 9) {

            std::cout << "Read error in SATELLIT, Line " << cat.size()+1 << std::endl;
            return false;
        }
        cat.push_back(r);
    }
    return true;
}

datamodels::SatellitLine datamodels::SatellitCatalogue::getSatById(int64_t sat_id)
{
    for (SatellitLine r: cat) {
        if (r.id == sat_id) return r;
    }
    return SatellitLine{"",0,0,0,0,0,0,0,""};
}

bool datamodels::CopyObs::read(const std::string& fname)
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
        datamodels::CopyObsElem r;
        if (sscanf(temp_line.c_str(), FormatCGA_R,
            &r.sec, &r.azym, &r.elev, &r.range, &r.range1, &r.ampl) != 6) {

            std::cout << "Read error in obs copy " << fname << ", Line " << data.size()+1 << std::endl;
            return false;
        }
        data.push_back(r);
    }
    return true;
}

bool datamodels::CopyObs::write(const std::string& fname)
{
    std::ofstream out(fname);
    if (!out.is_open()) {
        std::cout << "Unable to write file " << fname << std::endl;
        return false;
    }
    char buf[256];
    for (CopyObsElem r: data) {
        sprintf(buf, FormatCGA_W,
                r.sec, r.azym, r.elev, r.range, r.range1, r.ampl);
        out << buf << std::endl;
    }
    return true;
}

bool datamodels::Obs::read(const std::string& fname)
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
        datamodels::ObsElem r;
        if (sscanf(temp_line.c_str(), FormatOGA_R,
            &r.sec, &r.azym, &r.elev, &r.range, &r.ampl) != 5) {

            std::cout << "Read error in obs " << fname << ", Line " << data.size()+1 << std::endl;
            return false;
        }
        data.push_back(r);
    }
    return true;
}

bool datamodels::Efem::read(const std::string& fname)
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
        datamodels::EfemElem r;
        if (sscanf(temp_line.c_str(), FormatEGA_R,
            &r.c, &r.sec, &r.azym, &r.elev, &r.range) != 5) {

            std::cout << "Read error in efem file " << fname << ", Line " << data.size()+1 << std::endl;
            return false;
        }
        data.push_back(r);
    }
    return true;
}

bool datamodels::Dif::read(const std::string& fname)
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
        datamodels::DifElem r;
        if (sscanf(temp_line.c_str(), FormatPGA_R,
            &r.sec, &r.omc, &r.tbrb, &r.czeb) != 4) {

            std::cout << "Read error in derivatives file " << fname << ", Line " << data.size()+1 << std::endl;
            return false;
        }
        data.push_back(r);
    }
    return true;
}

bool datamodels::Dif::write(const std::string& fname)
{
    std::ofstream out(fname);
    if (!out.is_open()) {
        std::cout << "Unable to write file " << fname << std::endl;
        return false;
    }
    char buf[256];
    for (DifElem r: data) {
        sprintf(buf, FormatPGA_W,
                r.sec, r.omc, r.tbrb, r.czeb);
        out << buf << std::endl;
    }
    return true;
}

bool datamodels::Kobs::read(const std::string& fname)
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
        datamodels::KobsElem r;
        if (sscanf(temp_line.c_str(), FormatKGA_R,
            &r.sec, &r.azym, &r.elev, &r.range, &r.ampl, &r.czeb1) != 6) {

            std::cout << "Read error in k** file " << fname << ", Line " << data.size()+1 << std::endl;
            return false;
        }
        data.push_back(r);
    }
    return true;
}

bool datamodels::Kobs::write(const std::string& fname)
{
    std::ofstream out(fname);
    if (!out.is_open()) {
        std::cout << "Unable to write file " << fname << std::endl;
        return false;
    }
    char buf[256];
    for (KobsElem r: data) {
        sprintf(buf, FormatKGA_W,
                r.sec, r.azym, r.elev, r.range, r.ampl, r.czeb1);
        out << buf << std::endl;
    }
    return true;
}
