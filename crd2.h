#ifndef CRD2_H
#define CRD2_H

#include <fstream>
#include <string>

enum class DataType
{
    FullRate = 1,
    NormalPoint = 2
};

class CRD_FILE
{
protected:
    std::ofstream file;
    char buf[256];
    std::string content;
    DataType type;
public:
    CRD_FILE()=delete;
    CRD_FILE(DataType dt, std::string fname);
    CRD_FILE(DataType dt, char st_name[5], char sat_nam[11], int yyyy, int mm, int dd, double hh, int version=0);
    ~CRD_FILE()=default;

    std::string getAsString();

    void write_H1(int year, int month, int day, int hour);
    void write_H2(char st_name[5], int st_num, int st_sys, int st_occ, int st_tim);
    void write_H3(char sat_nam[11], int64_t sat_id, int sic, int64_t norad, int transp, int tclass, int targetloc);
    void write_H4(int year1, int month1, int day1, int hour1, int min1, int sec1,
                  int year2, int month2, int day2, int hour2, int min2, int sec2,
                  int release, int trop_cor, int com_cor, int ampl_cor, int sys_del_station, int sys_del_spacecr,
                  int range_type, int data_qlty);
    void write_H5();
    void write_H8();
    void write_H9();

    void write_C0(double wavelength_nm, char sys_conf[5], char a_conf[5], char b_conf[5], char c_conf[5], char d_conf[5], char e_conf[5], char f_conf[5], char g_conf[5]);
    void write_C1(char a_conf[5], char laser_type[11], double pr_wavelen_nm, double fire_rate, double penergy_mJ, double pwidth_ps, double beam_angle, int Npulse);
    void write_C2(char b_conf[5], char det_type[11], double app_wavelen_nm, double qe, double feu_volt, double dark_count, char op_type[11], double op_width, double spfilt_nm, double spfilt_trPersent, double spart_filt, char esp[11], double ampl_gain, double ampl_bw, int ampl_use);
    void write_C3(char c_conf[5], char time_src[21], char frq_src[21], char timer[21], char timer_snum[21], double epoch_delay_mcs);
    void write_C4();
    void write_C5();
    void write_C6();
    void write_C7();

    void write_10(double sec, double range_insec, char sys_conf[5], int event_type, int filt_flag, int det_ch_type, int stop_num, int RxAmpl, int TxAmpl);
    void write_11(double sec, double range_insec, char sys_conf[5], int event_type, double np_window, int rangenum_per_np, double rms_ps, double skew, double kurt, double peak, double ret_rate, int det_ch, double SNR);
    void write_12();
    void write_20(double sec, double pres, double temp, double humid, int src_type);
    void write_21();
    void write_30(double sec, double az, double el, int dir_flag, int angle_src, int refr_cor, double azRate_dps, double elRate_dps);
    void write_40(double sec, int cal_type, char sys_conf[5], int nflash, int nret, double targ_m, double delay_ps, double shift_ps, double rms_ps, double skew, double kurt, double peak_ps, int type_ind, int shift_ind, int det_ch, int span, double ret_rate);
    void write_41(double sec, int cal_type, char sys_conf[5], int nflash, int nret, double targ_m, double delay_ps, double shift_ps, double rms_ps, double skew, double kurt, double peak_ps, int type_ind, int shift_ind, int det_ch, int span, double ret_rate);
    void write_42();
    void write_50(char sys_conf[5], double rms_ps, double skew, double kurt, double peak_ps, int data_qlt_type);
};

#endif // CRD2_H
