#ifndef ENGINE_H
#define ENGINE_H

#include <QObject>
#include <QVector>
#include <QString>
#include "datamodels.h"

class Engine : public QObject
{
    Q_OBJECT
public:
    explicit Engine(QObject *parent = nullptr);

signals:
    void accessError(QString);
    void searchError(QString);
    void logicError(QString);
    void alreadyDoneError(QString);
    void info(QString);

    // send data & results
    void plotOmcData(QVector<double> X, QVector<double> Y, QString h1, QString h2);
    void polyFitTable(QString content);
    void plotNqlData(QVector<double> X, QVector<double> Y, QVector<double> Xl, QVector<double> Yl,
                     QVector<double> XL, QVector<double> YL, QString h1, QString h2);

public slots:
    void prepareObsCopy(int nr);
    void fit_calibration(bool is_before);
    void fit_observation();
    void prepareOmcPlot(int type);
    void calc_TimeRangeBias();
    void polynom_fit();
//    void prepareResidHist();
    bool filterPolyResiduals(double rms);
    bool validationNP();
    void prepareNPPlot();
    void writeKobs_Log();
    void prepareCRD();

    void onPlotOmcResult(bool save, QVector<bool> dropFlags, int npointsLeft);

private:
    double cSkew, cKurt;
    int nrObs, calBefore, calAfter; // enumeration from 1, as rows in files
    char fnameobs[50], fnamecopy[50], fnameefem[50], fnamederiv[50], fnamekobs[50], fnamelog[50];
    QString fnameNPT, fnameFRD;
    datamodels::ObsCatElem selectedObs; // currently selected obs line in KAT_OBS
    datamodels::Obs sObs;      // currently selected obs .oxx
    datamodels::CopyObs sCopy; // currently selected copy obs .cxx
    datamodels::Efem sEfem;    // corresponding efem .exx
    datamodels::Dif sDeriv;    // corresponding omc data .pxx
    datamodels::Kobs sKobs;    // corresponding .kxx data file
    datamodels::SatellitLine satelliteInfo; // info about located target in selected obs

};

#endif // ENGINE_H
