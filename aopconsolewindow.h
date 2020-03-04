#ifndef AOPCONSOLEWINDOW_H
#define AOPCONSOLEWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QLineEdit>
#include <QGridLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QInputDialog>
#include <QNetworkReply>
#include <QSlider>

#include "datamodels.h"
#include "plot.h"
#include "utility.h"
#include "config.h"

class CustomSlider;

class AOPConsoleWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit AOPConsoleWindow(QWidget *parent = nullptr);
    ~AOPConsoleWindow();

    // for selection view specification
    void updateCatCalTable(std::vector<datamodels::CalCatElem> &cat, QTableWidget* table);
    void updateCatObsTable(std::vector<datamodels::ObsCatElem> &cat, QTableWidget *table);
    void updateCatObsTreeView(std::vector<datamodels::ObsCatElem> &cat);

    // engine
    void prepareObsCopy();
    void fit_calibration(bool is_before);
    void fit_observation();
    void plot_omc(int type);

    void fanFilter(QVector<double> &x, QVector<double> &y);

    void calc_TimeRangeBias();
    void polynom_fit();
    bool resid_distrib_accepted();
    bool filterPolyResiduals(double rms);

    bool validationNP();
    void plot_NP();

    void writeKobs_Log();
    void prepareResults();

private:
    // basic structure
    QWidget *central1_, *central2_, *central3_, *central4_, *central5_, *central6_;
    QGridLayout *grid1, *grid2, *grid3, *grid4, *grid5, *grid6;

    // user interface
    QPushButton *complete1Btn, *reset1Btn;
    QPushButton *complete2Btn, *reset2Btn;
    QPushButton *complete3Btn, *reset3Btn;
    QPushButton *complete4Btn, *reset4Btn;
    QPushButton *complete5Btn;

    QPushButton *exitBtn;

    QTableWidget *catObsTableView;
    QTableWidget *catCalTableView;
    QTreeWidget  *catObsTreeView;
    QPushButton *selObsBtn, *switchObsViewBtn;
    QCheckBox *checkOrig, *checkNew;
    std::vector<QMetaObject::Connection> highlightConnections;

    QPushButton *calBeforeBtn, *calAfterBtn;
    QComboBox *calBeforeBox, *calAfterBox;
    QPushButton *send;

    QTextEdit *polyTable;
    QTextEdit *npt_content, *frd_content;
    QLabel *nptf_label, *frdf_label;

    // plot and graphs
    QCustomPlot *calBefore_plot, *calAfter_plot, *residDist_plot;
    Plot *omc_plot;
    QCustomPlot *nql_plot;
    QPushButton *esc_pBtn, *undo_pBtn, *up_pBtn, *down_pBtn, *filtBtn, *recalcTRB_pBtn, *applyTRB_pBtn, *saveexit_pBtn;
    CustomSlider *bandLengthVal, *bandWidthVal, *snrVal, *sigmaVal;

    // settings
    SettingsDialog *sd;

    // misc
    int numberCalSelected;

    QString fnameNPT, fnameFRD;
    int uploadsFinishedCount, totalUploads;
    std::vector<QNetworkReply*> reps1, reps2;
    QNetworkReply *rep1, *rep2;
    QString replyMsgs;

    double cSkew, cKurt;

    // data
    int nrObs, calBefore, calAfter, calSelected; // enumeration from 1, as rows in files
    char fnameobs[50], fnamecopy[50], fnameefem[50], fnamederiv[50], fnamekobs[50], fnamelog[50];
    datamodels::Obs sObs;      // currently selected obs .oxx
    datamodels::CopyObs sCopy; // currently selected copy obs .cxx
    datamodels::Efem sEfem;    // corresponding efem .exx
    datamodels::Dif sDeriv;    // corresponding omc data .pxx
    datamodels::Kobs sKobs;    // corresponding .kxx data file
    std::vector< std::pair< std::string, std::vector<datamodels::ObsCatElem> > > catObsTree;
    datamodels::SatellitLine satelliteInfo;
    QVector<double> X,Y; // plot data

    void createMenu();
    void createDock();

private slots:
    void error(QString);
    void info(QString);
    void fatal(QString);

signals:

public slots:
    // misc
    void obsHighlighted(int,int);
    void onOmc_plotFinish(bool);
    void onOMC_Filtering(int);

    // select obs and create copy - step 1
    void setup1();
    void reset1();

    // calibration fitting - step 2
    void setup2();
    void checkReady2();
    void showFitCalHist(bool is_before, const int& scale, const QVector<double>& hist, const double& meanvall, const double& rmss, const double& corectt, const int& nret, const double& mean_val, const int& maxplus, const int& maxmin, const double& skew, const double& kurt, const int& nflash);
    void reset2();

    // observation fitting, omc plotting and filtering
    void setup3();
    void reset3();

    // time & range bias calculation, second omc plot
    void setup4();
    void reset4();

    // polynomial fitting, extraction of normal points
    void setup5();
    void showResidHist(const double& mean_val, const int& maxplus, const int& maxmin, const int& scale, const QVector<double>& hist, const double& skew, const double& kurt);

    // write CRD files, save & send them via FTP
    void setup6();
};


// this class derivered from QTableWidgetItem for unique customization of table cell
class CatTableItem : public QTableWidgetItem
{
public:
    explicit CatTableItem(const QString &text, int type = Type);
    virtual ~CatTableItem();
};


class CustomTitle : public QLabel
{
public:
    explicit CustomTitle(const QString& text, QWidget* parent=nullptr);
    virtual ~CustomTitle();
};


// this class represents button serving to expand/collapse nested content in tree
class QtCategoryButton : public QPushButton
{
    Q_OBJECT
public:
    QtCategoryButton(const QString& a_Text, QTreeWidget* a_pParent, QTreeWidgetItem* a_pItem)
        : QPushButton(a_Text, a_pParent), m_pItem(a_pItem)
    {
        connect(this, &QtCategoryButton::pressed, this, [&](){
            m_pItem->setExpanded( !m_pItem->isExpanded() );
        });
    }

private:
    QTreeWidgetItem* m_pItem;
};

class CustomSlider : public QWidget
{
public:
    explicit CustomSlider(Qt::Orientation o, double unit, QString name, QWidget* parent=nullptr);
    virtual ~CustomSlider();

    void setRange(int min, int max);
    double value();
    QSlider *slider;
private:
    double _unit;
    QLabel *minVal, *maxVal, *curVal, *title;
    QGridLayout *grid;
};

#endif // AOPCONSOLEWINDOW_H
