#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QTreeWidget>
#include <QCheckBox>
#include <QComboBox>
#include <qcustomplot/qcustomplot.h>
#include <plot.h>
#include <QVector>
#include <datamodels.h>

typedef std::vector< std::pair< std::string, std::vector<datamodels::ObsCatElem> > > CatObsTree;

class CentralWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CentralWidget(QWidget *parent = nullptr);
    ~CentralWidget();

    void clearHighlightConnections();

private:
    QGridLayout *grid1, *grid2, *grid3, *grid4, *grid5, *grid6;
    QPushButton *next1btn, *next2btn, *next3btn, *next4btn, *next5btn;
    QPushButton *reset1btn, *reset2btn, *reset3btn, *reset4btn;

    // step 1
    QTableWidget *catObsTableView;
    QTreeWidget  *catObsTreeView;
    QPushButton *selObsBtn, *switchObsViewBtn;
    QCheckBox *checkOrig, *checkNew;
    QVector<QMetaObject::Connection> highlightConnections;
    int nrObs;

    // step 2
    QTableWidget *catCalTableView;
    QPushButton *calBeforeBtn, *calAfterBtn;
    QComboBox *calBeforeBox, *calAfterBox;
    QCustomPlot *calBefore_plot, *calAfter_plot;
    int countCalSelected, calSelected;

    // step 3
    QPushButton *esc_pBtn, *undo_pBtn, *up_pBtn, *down_pBtn, *filtBtn;
    Plot *omc_plot;

    // step 4
    QPushButton *recalcTRB_pBtn;

    // step 5
    QTextEdit *polyTable;
    QCustomPlot *residDist_plot, *nql_plot;

    // step 6
    QTextEdit *npt_content, *frd_content;
    QLabel *nptf_label, *frdf_label;
    QPushButton *send;


signals:
    void plotOmcResult(bool save, DropFlags df, int npointsleft);

public slots:
    void setup1();
    void reset1();

    void setup2();
    void checkReady2();
    void reset2();

    void setup3();
    void reset3();

    void setup4();
    void reset4();

    void setup5();

    void setup6();

    void updateCatCalTable(std::vector<datamodels::CalCatElem> &cat, QTableWidget* table);
    void updateCatObsTable(std::vector<datamodels::ObsCatElem> &cat, QTableWidget *table);
    void updateCatObsTreeView(std::vector<datamodels::ObsCatElem>& cat);
    void obsHighlighted(int,int);

    void plotOmc(QVector<double> X, QVector<double> Y, QString h1, QString h2);
    void onPlotOmcFinished(bool save);
    void showPolyFitTable(QString content);
};

namespace centralwidget {

// ------------Classes useful for CentralWidget------------------
// this class derivered from QTableWidgetItem for unique customization of table cells
class CatTableItem : public QTableWidgetItem
{
public:
    explicit CatTableItem(const QString &text, int type = Type);
    virtual ~CatTableItem();
};

// this class derivered from QLabel for unique customization of step titles
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

}

#endif // CENTRALWIDGET_H
