#include <crd2.h>
#include <string.h>

CRD_FILE::CRD_FILE(DataType dt, std::string fname) : type(dt)
{
    file.open(fname);
}

CRD_FILE::CRD_FILE(DataType dt, char st_name[], char sat_nam[], int yyyy, int mm, int dd, double hh, int version) : type(dt)
{
    char fname[256];
    sprintf(fname,"D:\\LASER-2\\DATA\\SEND\\%4s_%1.10s_crd_%04d%02d%02d_%02.lf_%02d",
            st_name, sat_nam, yyyy, mm, dd, hh, version);
    if (type == DataType::FullRate) strncat(fname, ".frd", 4);
    if (type == DataType::NormalPoint) strncat(fname, ".npt", 4);
    file.open(fname);
}

std::string CRD_FILE::getAsString()
{
    return content;
}

/*!
 * \brief Write to CRD file Format header H1
 * \param year Year of file production
 * \param month Month of file production
 * \param day Day of file production
 * \param hour Hour of file production (UTC)
 */
void CRD_FILE::write_H1(int year, int month, int day, int hour)
{
    sprintf(buf, "H1 CRD 02 %4d %2d %2d %2d\n",
            year, month, day, hour);
    file << buf;
    content.append(buf,strlen(buf));
}

/*!
 * \brief Write to CRD file Station header H2
 * \param st_name Station name from official list (e.g., "MOB7 ", "MLRS ")
 * \param st_num System identifier: Crustal Dynamics Project (CDP) Pad Identifier for SLR
 * \param st_sys System number: Crustal Dynamics Project (CDP) 2-digit system number for SLR
 * \param st_occ System occupancy: Crustal Dynamics Project (CDP) 2-digit occupancy sequence number for SLR
 * \param st_tim Station Epoch Time Scale - indicates the time scale reference.
 */
void CRD_FILE::write_H2(char *st_name, int st_num, int st_sys, int st_occ, int st_tim)
{
    sprintf(buf, "H2 %10s %4d %2d %2d %2d EUROLAS\n",
            st_name, st_num, st_sys, st_occ, st_tim);
    file << buf;
    content.append(buf,strlen(buf));
}

/*!
 * \brief Write to CRD file Target header H3
 * \param sat_nam Target name from official list (e.g., "ajisai", "gps35")
 * \param sat_id ILRS Satellite Identifier (Based on the COSPAR ID)
 * \param sic SIC (Satellite Identification Code) (Provided by ILRS; set to “na” for non-ILRS targets without a SIC)
 * \param norad NORAD ID (also known as  “Satellite Catalog Number”)
 * \param transp Spacecraft Epoch Time Scale (transponders only)
 * \param tclass Target class
 * \param targetloc Target location/dynamics
 */
void CRD_FILE::write_H3(char *sat_nam, int64_t sat_id, int sic, int64_t norad, int transp, int tclass, int targetloc)
{
    if (targetloc == -1)
        sprintf(buf, "H3 %10s %7lld %4d %5lld %1d %1d na\n",
                sat_nam, sat_id, sic, norad, transp, tclass);
    else
        sprintf(buf, "H3 %10s %7lld %4d %5lld %1d %1d %1d\n",
                sat_nam, sat_id, sic, norad, transp, tclass, targetloc);
    file << buf;
    content.append(buf,strlen(buf));
}

/*!
 * \brief Write to NPT file Session (Pass/Pass segment) Header H4
 * \param year1 Starting Year
 * \param month1 Starting Month
 * \param day1 Starting Day
 * \param hour1 Starting Hour (UTC)
 * \param min1 Starting Minute (UTC)
 * \param sec1 Starting Second (UTC)
 * \param year2 Ending Year
 * \param month2 Ending Month
 * \param day2 Ending Day
 * \param hour2 Ending Hour (UTC)
 * \param min2 Ending Minute (UTC)
 * \param sec2 Ending Second (UTC)
 * \param release A flag to indicate the data release
 * \param trop_cor Tropospheric refraction correction applied indicator
 * \param com_cor Center of mass correction applied indicator
 * \param ampl_cor Receive amplitude correction applied indicator
 * \param sys_del_station Station system delay applied indicator
 * \param sys_del_spacecr Spacecraft system delay applied (transponders) indicator
 * \param range_type Range type indicator
 * \param data_qlty Data quality alert indicator
 */
void CRD_FILE::write_H4(int year1, int month1, int day1, int hour1, int min1, int sec1, int year2, int month2, int day2, int hour2, int min2, int sec2, int release, int trop_cor, int com_cor, int ampl_cor, int sys_del_station, int sys_del_spacecr, int range_type, int data_qlty)
{
    int dt;
    if (type == DataType::FullRate) dt = 0;
    if (type == DataType::NormalPoint) dt = 1;

    sprintf(buf, "H4 %1d %4d %2d %2d %2d %2d %2d %4d %2d %2d %2d %2d %2d %2d %1d %1d %1d %1d %1d %1d %1d\n",
            dt,year1,month1,day1,hour1,min1,sec1,year2,month2,day2,hour2,min2,sec2,
            release,trop_cor,com_cor,ampl_cor,sys_del_station,sys_del_spacecr,range_type,data_qlty);
    file << buf;
    content.append(buf,strlen(buf));
}

void CRD_FILE::write_H5() {}

/*!
 * \brief Write to CRD file End of Session (EOS) Footer H8
 */
void CRD_FILE::write_H8()
{
    file << "H8" << std::endl;
    content.append("H8\n");
}

/*!
 * \brief Write to CRD file End of File (EOF) Footer
 */
void CRD_FILE::write_H9()
{
    file << "H9" << std::endl;
    content.append("H9\n");
    file.close();
}

/*!
 * \brief Write to CRD file System Configuration Record C0
 * \param wavelength_nm Transmit Wavelength (nanometers)
 * \param sys_conf System configuration ID (unique within the file)
 * \param a_conf Component A configuration ID (e.g., laser configuration ID)
 * \param b_conf Component B configuration ID (e.g., detector configuration ID)
 * \param c_conf Component C configuration ID (e.g., local timing system configuration ID)
 * \param d_conf Component D configuration ID (e.g., transponder configuration ID)
 * \param e_conf Component E configuration ID (e.g., software configuration ID)
 * \param f_conf Component F configuration ID (e.g., meteorological configuration ID)
 * \param g_conf Component G configuration ID (e.g., calibration target configuration ID)
 */
void CRD_FILE::write_C0(double wavelength_nm, char *sys_conf, char *a_conf, char *b_conf, char *c_conf, char *d_conf, char *e_conf, char *f_conf, char *g_conf)
{
    sprintf(buf, "C0 0 %10.3lf %4s %4s %4s %4s %4s %4s %4s %4s\n",
            wavelength_nm,sys_conf,a_conf,b_conf,c_conf,d_conf,e_conf,f_conf,g_conf);
    file << buf;
    content.append(buf,strlen(buf));
}

/*!
 * \brief Write to CRD file Laser Configuration Record C1
 * \param a_conf Laser Configuration ID (unique within the file)
 * \param laser_type Laser Type (e.g., “Nd-Yag”)
 * \param pr_wavelen_nm Primary Wavelength (nm)
 * \param fire_rate Nominal Fire Rate (Hz)
 * \param penergy_mJ Pulse Energy (mJ)
 * \param pwidth_ps Pulse Width (FWHM in ps)
 * \param beam_angle Beam Divergence (full angle, arcseconds)
 * \param Npulse Number of pulses in outgoing semi-train
 */
void CRD_FILE::write_C1(char *a_conf, char *laser_type, double pr_wavelen_nm, double fire_rate, double penergy_mJ, double pwidth_ps, double beam_angle, int Npulse)
{
    sprintf(buf, "C1 0 %4s %10s %10.2lf %10.2lf %10.2lf %6.1lf %5.2lf %4d\n",
            a_conf,laser_type,pr_wavelen_nm,fire_rate,penergy_mJ,pwidth_ps,beam_angle,Npulse);
    file << buf;
    content.append(buf,strlen(buf));
}

/*!
 * \brief Write to CRD file Detector Configuration Record C2
 * \param b_conf Detector Configuration ID (unique within the file)
 * \param det_type Detector Type (e.g., ”SPAD”,  “CSPAD”, “MCP”, “APD”, “GeDiode”, … )
 * \param app_wavelen_nm Applicable Wavelength (nm)
 * \param qe Quantum Efficiency at applicable wavelength (%)
 * \param feu_volt Applied Voltage (V)
 * \param dark_count Dark Count (kHz)
 * \param op_type Output Pulse Type (ECL, TTL, photon-dependent, …)
 * \param op_width Output Pulse Width (ps)
 * \param spfilt_nm Spectral Filter (nm)
 * \param spfilt_tr Persent Transmission of Spectral Filter
 * \param spart_filt Spatial Filter (arcsec)
 * \param esp External Signal Processing
 * \param ampl_gain Amplifier Gain
 * \param ampl_bw Amplifier Bandwidth (kHz)
 * \param ampl_use Amplifier In Use
 */
void CRD_FILE::write_C2(char *b_conf, char *det_type, double app_wavelen_nm, double qe, double feu_volt, double dark_count, char *op_type, double op_width, double spfilt_nm, double spfilt_trPersent, double spart_filt, char *esp, double ampl_gain, double ampl_bw, int ampl_use)
{
    sprintf(buf, "C2 0 %4s %10s %10.3lf %6.2lf %5.1lf %5.1lf %10s %5.1lf %5.2lf %5.1lf %5.1lf %10s %6.1lf %6.1lf",
            b_conf,det_type,app_wavelen_nm,qe,feu_volt,dark_count,op_type,op_width,spfilt_nm,spfilt_trPersent,spart_filt,esp,ampl_gain,ampl_bw);
    if (ampl_use == -1)
        sprintf(buf+strlen(buf), " na\n");
    else
        sprintf(buf+strlen(buf), " %2d\n", ampl_use);

    file << buf;
    content.append(buf,strlen(buf));
}

/*!
 * \brief Write to CRD file Timing System Configuration Record C3
 * \param c_conf Timing System Configuration ID (unique within the file)
 * \param time_src Time Source (e.g., “Truetime_XLi”,  “Truetime_XL-SD”, “Datum_9390”, “HP_58503A”, “TAC”, ...)
 * \param frq_src Frequency Source (e.g., “Truetime_OCXO”, “CS-4000”, …)
 * \param timer Timer (e.g., “MRCS”, “SR620”, “HP5370B”, “Dassault”, ”Other”, … )
 * \param timer_snum Timer Serial Number (for multiple timers of the same model)
 * \param epoch_delay_mcs Epoch Delay Correction (μs)
 */
void CRD_FILE::write_C3(char *c_conf, char *time_src, char *frq_src, char *timer, char *timer_snum, double epoch_delay_mcs)
{
    sprintf(buf, "C3 0 %4s %20s %20s %20s %20s %6.1lf\n",
            c_conf,time_src,frq_src,timer,timer_snum,epoch_delay_mcs);
    file << buf;
    content.append(buf,strlen(buf));
}

void CRD_FILE::write_C4() {}
void CRD_FILE::write_C5() {}
void CRD_FILE::write_C6() {}
void CRD_FILE::write_C7() {}

/*!
 * \brief Write to CRD file Range Record (Full rate, Sampled Engineering/Quicklook) 10
 * \param sec Seconds of day (typically to 100 ns precision for SLR/Lunar Laser Ranging (LLR) or 1 picosecond for transponder/time transfer). For transponders and time transfer, station clock correction may be applied.
 * \param range_insec Time-of-flight in seconds (none, one-, or two-way depending on range type indicator); or (for Epoch Event 5) spacecraft receive time in units of the spacecraft master clock, or seconds if “Spacecraft offset and drift applied indicator” is true. Time-of-flight may be corrected for station system delay; receive time may be corrected for spacecraft system delay and/or clock correction.
 * \param sys_conf System configuration ID
 * \param event_type Epoch Event - indicates the time event reference
 * \param filt_flag Filter flag
 * \param det_ch_type Detector channel
 * \param stop_num Stop number (in multiple-stop system)
 * \param RxAmpl Receive Amplitude - a positive linear scale value
 * \param TxAmpl Transmit Amplitude - a positive linear scale value
 */
void CRD_FILE::write_10(double sec, double range_insec, char *sys_conf, int event_type, int filt_flag, int det_ch_type, int stop_num, int RxAmpl, int TxAmpl)
{
    if (TxAmpl == -1)
        sprintf(buf, "10 %18.12lf %18.12lf %4s %1d %1d %1d %1d %5d na\n",
                sec,range_insec,sys_conf,event_type,filt_flag,det_ch_type,stop_num,RxAmpl);
    else
        sprintf(buf, "10 %18.12lf %18.12lf %4s %1d %1d %1d %1d %5d %5d\n",
                sec,range_insec,sys_conf,event_type,filt_flag,det_ch_type,stop_num,RxAmpl,TxAmpl);
    file << buf;
    content.append(buf,strlen(buf));
}

/*!
 * \brief Write to CRD file Range Record (Normal Point) 11
 * \param sec Seconds of day (typically to < 100 ns precision for SLR/LLR or < 1 ps for transponders/time transfer). Station clock corrections should be applied for all targets.
 * \param range_insec Time-of-flight in seconds (none, one-, or two-way depending on range type indicator); or (for Epoch Event = 5) spacecraft receive time in units of the spacecraft master clock, or seconds if “Spacecraft offset and drift applied indicator” is true. Time-of-flight should be corrected for station system delay; receive time may be corrected for spacecraft system delay and/or clock correction.
 * \param sys_conf System configuration ID
 * \param event_type Epoch Event - indicates the time event reference
 * \param np_window Normal point window length (seconds)
 * \param rangenum_per_np Number of raw ranges (after editing) compressed into the normal point
 * \param rms_ps Bin RMS from the mean of raw accepted time-of-flight values minus the trend function (ps)
 * \param skew Bin skew from the mean of raw accepted time-of-flight values minus the trend function
 * \param kurt Bin kurtosis from the mean of raw accepted time-of-flight values minus the trend function
 * \param peak Bin peak – mean value (ps)
 * \param ret_rate Return rate (persents)
 * \param det_ch Detector channel
 * \param SNR Signal to noise ratio (S:N)
 */
void CRD_FILE::write_11(double sec, double range_insec, char *sys_conf, int event_type, double np_window, int rangenum_per_np, double rms_ps, double skew, double kurt, double peak, double ret_rate, int det_ch, double SNR)
{
    if (SNR < 0)
        sprintf(buf, "11 %18.12lf %18.12lf %4s %1d %6.1lf %6d %9.1lf %7.3lf %7.3lf %9.1lf %5.1lf %1d na\n",
                sec,range_insec,sys_conf,event_type,np_window,rangenum_per_np,rms_ps,skew,kurt,peak,ret_rate,det_ch);
    else
        sprintf(buf, "11 %18.12lf %18.12lf %4s %1d %6.1lf %6d %9.1lf %7.3lf %7.3lf %9.1lf %5.1lf %1d %5.1lf\n",
                sec,range_insec,sys_conf,event_type,np_window,rangenum_per_np,rms_ps,skew,kurt,peak,ret_rate,det_ch,SNR);
    file << buf;
    content.append(buf,strlen(buf));
}

void CRD_FILE::write_12() {}

/*!
 * \brief Write to CRD file Meteorological Record 20
 * \param sec Seconds of day (typically to 1 milllisec precision).
 * \param pres Surface pressure (millibar)
 * \param temp Surface temperature in degrees Kelvin
 * \param humid Relative humidity at the surface in %
 * \param src_type Origin of values
 */
void CRD_FILE::write_20(double sec, double pres, double temp, double humid, int src_type)
{
    sprintf(buf, "20 %18.12lf %7.2lf %6.2lf %4.0lf %1d\n",
            sec,pres,temp,humid,src_type);
    file << buf;
    content.append(buf,strlen(buf));
}

void CRD_FILE::write_21() {}

/*!
 * \brief Write to CRD file Pointing Angle Record 30
 * \param sec Seconds of day (typically to 1 milllisec precision).
 * \param az Azimuth in degrees
 * \param el Elevation in degrees
 * \param dir_flag Direction flag
 * \param angle_src Angle origin indicator
 * \param refr_cor Refraction corrected
 * \param azRate_dps Azimuth Rate in degrees/second
 * \param elRate_dps Elevation Rate in degrees/second
 */
void CRD_FILE::write_30(double sec, double az, double el, int dir_flag, int angle_src, int refr_cor, double azRate_dps, double elRate_dps)
{
    sprintf(buf, "30 %18.12lf %8.4lf %8.4lf %1d %1d %1d %10.7lf %10.7lf\n",
            sec,az,el,dir_flag,angle_src,refr_cor,azRate_dps,elRate_dps);
    file << buf;
    content.append(buf,strlen(buf));
}

/*!
 * \brief Write to CRD file Calibration Record 40
 * \param sec Seconds of day (typically to < 100 ns precision for SLR/LLR, or <1 ps for transponder ranging). Station clock corrections should be applied for all targets. This should be the time at the middle of the pass segment.
 * \param cal_type Type of data
 * \param sys_conf System configuration ID
 * \param nflash Number of data points recorded (= “na” if no information)
 * \param nret Number of data points used (= “na” if no information)
 * \param targ_m One-way target distance (meters, nominal) (= “na” if no information)
 * \param delay_ps Calibration System Delay (picoseconds)
 * \param shift_ps Calibration Delay Shift - a measure of calibration stability (picoseconds)
 * \param rms_ps RMS of raw system delay (ps). If pre- and post- pass calibrations are made, use the mean of the two RMS values, or the RMS of the combined data set.
 * \param skew Skew of raw system delay values from the mean. If pre- and post- pass calibrations are made, use the mean of the two skew values, or the skew of the combined data set.
 * \param kurt Kurtosis of raw system delay values from the mean. If pre- and post- pass calibrations are made, use the mean of the two kurtosis values, or the kurtosis of the combined data set.
 * \param peak_ps System delay peak – mean value (ps). If pre- and post- pass calibrations are made, use the mean of the two peak-mean values, or the peak-mean of the combined data set.
 * \param type_ind Calibration Type Indicator
 * \param shift_ind Calibration Shift Type Indicator
 * \param det_ch Detector Channel
 * \param span Calibration Span
 * \param ret_rate Return Rate (persents)
 */
void CRD_FILE::write_40(double sec, int cal_type, char *sys_conf, int nflash, int nret, double targ_m, double delay_ps, double shift_ps, double rms_ps, double skew, double kurt, double peak_ps, int type_ind, int shift_ind, int det_ch, int span, double ret_rate)
{
    sprintf(buf, "40 %18.12lf %1d %4s %8d %8d %7.3lf %10.1lf %8.1lf %6.1f %7.3lf %7.3lf %6.1lf %1d %1d %1d %1d %5.1lf\n",
            sec,cal_type,sys_conf,nflash,nret,targ_m,delay_ps,shift_ps,rms_ps,skew,kurt,peak_ps,type_ind,shift_ind,det_ch,span,ret_rate);
    file << buf;
    content.append(buf,strlen(buf));
}

/*!
 * \brief Write to CRD file Calibration Detail Record 41
 * \param sec Seconds of day (typically to < 100 ns precision for SLR/LLR, or <1 ps for transponder ranging). Station clock corrections should be applied for all targets. This should be the time at the middle of the pass segment.
 * \param cal_type Type of data
 * \param sys_conf System configuration ID
 * \param nflash Number of data points recorded (= “na” if no information)
 * \param nret Number of data points used (= “na” if no information)
 * \param targ_m One-way target distance (meters, nominal) (= “na” if no information)
 * \param delay_ps Calibration System Delay (picoseconds)
 * \param shift_ps Calibration Delay Shift - a measure of calibration stability (picoseconds)
 * \param rms_ps RMS of raw system delay (ps). If pre- and post- pass calibrations are made, use the mean of the two RMS values, or the RMS of the combined data set.
 * \param skew Skew of raw system delay values from the mean. If pre- and post- pass calibrations are made, use the mean of the two skew values, or the skew of the combined data set.
 * \param kurt Kurtosis of raw system delay values from the mean. If pre- and post- pass calibrations are made, use the mean of the two kurtosis values, or the kurtosis of the combined data set.
 * \param peak_ps System delay peak – mean value (ps). If pre- and post- pass calibrations are made, use the mean of the two peak-mean values, or the peak-mean of the combined data set.
 * \param type_ind Calibration Type Indicator
 * \param shift_ind Calibration Shift Type Indicator
 * \param det_ch Detector Channel
 * \param span Calibration Span
 * \param ret_rate Return Rate (persents)
 */
void CRD_FILE::write_41(double sec, int cal_type, char *sys_conf, int nflash, int nret, double targ_m, double delay_ps, double shift_ps, double rms_ps, double skew, double kurt, double peak_ps, int type_ind, int shift_ind, int det_ch, int span, double ret_rate)
{
    sprintf(buf, "41 %18.12lf %1d %4s %8d %8d %7.3lf %10.1lf %8.1lf %6.1f %7.3lf %7.3lf %6.1lf %1d %1d %1d %1d %5.1lf\n",
            sec,cal_type,sys_conf,nflash,nret,targ_m,delay_ps,shift_ps,rms_ps,skew,kurt,peak_ps,type_ind,shift_ind,det_ch,span,ret_rate);
    file << buf;
    content.append(buf,strlen(buf));
}

void CRD_FILE::write_42() {}

/*!
 * \brief Write to CRD file Session (Pass) Statistics Record 50
 * \param sys_conf System configuration ID
 * \param rms_ps Session RMS from the mean of raw accepted time-of-flight values minus the trend function (ps)
 * \param skew Session skewness from the mean of raw accepted time-of-flight values minus the trend function
 * \param kurt Session kurtosis from the mean of raw accepted time-of-flight values minus the trend function
 * \param peak_ps Session peak – mean value (ps)
 * \param data_qlt_type Data quality assessment indicator. For SLR and LLR data: 0=undefined or no comment 1=clear, easily filtered data, with little or no noise 2= clear data with some noise; filtering is slightly compromised by noise level 3=clear data with a significant amount of noise, or weak data with little noise. Data are certainly present, but filtering is difficult. 4=unclear data; data appear marginally to be present, but are very difficult to separate from noise during filtering. Signal to noise ratio can be less than 1:1. 5=no data apparent
 */
void CRD_FILE::write_50(char *sys_conf, double rms_ps, double skew, double kurt, double peak_ps, int data_qlt_type)
{
    sprintf(buf, "50 %4s %6.1lf %7.3lf %7.3lf %6.1lf %1d\n",
            sys_conf,rms_ps,skew,kurt,peak_ps,data_qlt_type);
    file << buf;
    content.append(buf,strlen(buf));
}
