#include "centralwidget.h"
#include "config.h"
#include <QCoreApplication>

using namespace centralwidget;

CatTableItem::CatTableItem(const QString &text, int type) : QTableWidgetItem(text, type)
{
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    setFont(QFont("Courier New", 9, QFont::Medium));
}
CatTableItem::~CatTableItem() {}

CustomTitle::CustomTitle(const QString& text, QWidget *parent) : QLabel(text, parent)
{
    setFont(QFont("Courier New", 12, QFont::Bold));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}
CustomTitle::~CustomTitle() {}


CentralWidget::CentralWidget(QWidget *parent) : QWidget(parent),
    calBefore_plot(nullptr),
    calAfter_plot(nullptr),
    countCalSelected(0),
    omc_plot(nullptr),
    residDist_plot(nullptr),
    nql_plot(nullptr)
{
    setup1();
    reset1();
}

CentralWidget::~CentralWidget() {}

// fixate current highlightment
void CentralWidget::clearHighlightConnections()
{
    for (auto con : highlightConnections)
        disconnect(con);
    highlightConnections.clear();
}

void CentralWidget::setup1()
{
    selObsBtn = new QPushButton(this);
    selObsBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    switchObsViewBtn = new QPushButton("Table/Tree view", this);
    checkOrig = new QCheckBox(this);
    checkNew = new QCheckBox(this);
    catObsTableView = new QTableWidget(this);
    catObsTableView->setVisible(false);
    catObsTreeView = new QTreeWidget(this);
    catObsTreeView->setVisible(false);

    next1btn = new QPushButton("NEXT \n STEP", this);
    next1btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    reset1btn = new QPushButton("RESET", this);
    reset1btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    reset1btn->setStyleSheet("background-color: red; color: white; font: bold 14px;");

    grid1 = new QGridLayout;
    grid1->addWidget(new CustomTitle("Observation selection, creation of copy"), 0, 1, 1, 5);
    grid1->addWidget(reset1btn, 1, 0, 6, 1);
    grid1->addWidget(selObsBtn, 1, 1, 1, 5);
    grid1->addWidget(switchObsViewBtn, 2, 1);
    grid1->addWidget(checkOrig, 2, 2);
    grid1->addWidget(new QLabel("Show only original records"), 2, 3);
    grid1->addWidget(checkNew, 2, 4);
    grid1->addWidget(new QLabel("Show only unprocessed records"), 2, 5);

    Config::SelObsView sov = Config::Instance()->defaultSelObsView();
    if (sov == Config::SelObsView::Table || sov == Config::SelObsView::TableNew) {
        catObsTableView->setVisible(true);
        grid1->addWidget(catObsTableView, 3, 1, 4, 5);
    } else {
        catObsTreeView->setVisible(true);
        grid1->addWidget(catObsTreeView, 3, 1, 4, 5);
    }

    grid1->addWidget(next1btn, 1, 6, 6, 1);
    setLayout(grid1);

    connect(switchObsViewBtn, &QPushButton::clicked, this, [&](){
        if (catObsTableView->isVisible()) {
            grid1->replaceWidget(catObsTableView, catObsTreeView);
            catObsTableView->setVisible(false);
            catObsTreeView->setVisible(true);
        } else {
            grid1->replaceWidget(catObsTreeView, catObsTableView);
            catObsTableView->setVisible(true);
            catObsTreeView->setVisible(false);
        }
    });
    connect(reset1btn, &QPushButton::clicked, this, &CentralWidget::reset1);
    connect(next1btn, &QPushButton::clicked, this, [&](){
        setup2();
        reset2();
    });
}

void CentralWidget::reset1()
{
    selObsBtn->setText("Press on table to select OBS");
    selObsBtn->setDisabled(true);
    next1btn->setDisabled(true);
    next1btn->setStyleSheet("");    // that's how you reset appearence

    highlightConnections.push_back(connect(catObsTableView, &QTableWidget::cellClicked, this, &CentralWidget::obsHighlighted));
    updateCatObsTable(UniqueKAT_OBS::Instance()->cat, catObsTableView);
    updateCatObsTreeView(UniqueKAT_OBS::Instance()->cat);
}

void CentralWidget::setup2()
{
    calBeforeBtn = new QPushButton(this);
    calAfterBtn = new QPushButton(this);
    catCalTableView = new QTableWidget(this);
    calBeforeBox = new QComboBox(this);
    calAfterBox = new QComboBox(this);

    next2btn = new QPushButton("NEXT \n STEP", this);
    next2btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    reset2btn = new QPushButton("RESET", this);
    reset2btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    reset2btn->setStyleSheet("background-color: red; color: white; font: bold 14px;");

    grid2 = new QGridLayout;
    grid2->addWidget(new CustomTitle("Calibration selection and processing"), 0, 1, 1, 4);
    grid2->addWidget(calBeforeBtn, 1, 1);
    grid2->addWidget(calBeforeBox, 1, 2);
    grid2->addWidget(calAfterBtn, 1, 3);
    grid2->addWidget(calAfterBox, 1, 4);
    grid2->addWidget(catCalTableView, 2, 1, 1, 4);
    grid2->addWidget(next2btn, 1, 5, 3, 1);
    grid2->addWidget(reset2btn, 1, 0, 3, 1);
    setLayout(grid2);

    connect(reset2btn, &QPushButton::clicked, this, &CentralWidget::reset2);
}

void CentralWidget::checkReady2()
{
    countCalSelected++;
    next2btn->setDisabled(false);
    next2btn->setStyleSheet("background-color: green; color: white; font: bold 14px;");
}

void CentralWidget::reset2()
{
    calBeforeBtn->setText("Press to choose pre-observation CAL");
    calBeforeBtn->setDisabled(false);
    calAfterBtn->setText("Press to choose post-observation CAL");
    calAfterBtn->setDisabled(false);

    next2btn->setDisabled(true);
    next2btn->setStyleSheet("");    // that's how you reset appearence

    if (calBefore_plot != nullptr) {
        grid2->removeWidget(calBefore_plot);
        delete calBefore_plot;
    }
    if (calAfter_plot != nullptr) {
        grid2->removeWidget(calAfter_plot);
        delete calAfter_plot;
    }
}

void CentralWidget::setup3()
{
    omc_plot = new Plot(this);
    emit omc_plot->user_finished(true);
    connect(omc_plot, &Plot::user_finished, this, &CentralWidget::onPlotOmcFinished);
    esc_pBtn = new QPushButton("Exit (ESC)", this);
    undo_pBtn = new QPushButton("Undo (CTRL+Z)", this);
    up_pBtn = new QPushButton("Clear above (Key_Up)", this);
    down_pBtn = new QPushButton("Clear below (Key_Down)", this);
    filtBtn = new QPushButton("Auto-Filter", this);

    next3btn = new QPushButton("NEXT \n STEP", this);
    next3btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    reset3btn = new QPushButton("RESET", this);
    reset3btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    reset3btn->setStyleSheet("background-color: red; color: white; font: bold 14px;");

    grid3 = new QGridLayout;
    grid3->addWidget(new CustomTitle("Observation fitting, OMC plotting and filtering"), 0, 1, 1, 5);
    grid3->addWidget(reset3btn, 1, 0, 2, 1);
    grid3->addWidget(esc_pBtn, 1, 1);
    grid3->addWidget(undo_pBtn, 1, 2);
    grid3->addWidget(filtBtn, 1, 3);
    grid3->addWidget(up_pBtn, 1, 4);
    grid3->addWidget(down_pBtn, 1, 5);
    grid3->addWidget(omc_plot, 2, 1, 1, 5);
    grid3->addWidget(next3btn, 1, 6, 2, 1);
    setLayout(grid3);

    connect(esc_pBtn, &QPushButton::clicked, this, [&](){
        QKeyEvent *e = new QKeyEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
        QCoreApplication::postEvent(omc_plot, e);
    });
    connect(undo_pBtn, &QPushButton::clicked, this, [&](){
        QKeyEvent *e = new QKeyEvent(QEvent::KeyPress, Qt::Key_Z, Qt::ControlModifier);
        QCoreApplication::postEvent(omc_plot, e);
    });
    connect(up_pBtn, &QPushButton::clicked, this, [&](){
        QKeyEvent *e = new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
        QCoreApplication::postEvent(omc_plot, e);
    });
    connect(down_pBtn, &QPushButton::clicked, this, [&](){
        QKeyEvent *e = new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
        QCoreApplication::postEvent(omc_plot, e);
    });
    connect(next3btn, &QPushButton::clicked, this, [&](){
        if (omc_plot != nullptr) {
            grid3->removeWidget(omc_plot);
            delete omc_plot;
        }
        setup4();
        reset4();
    });
}

void CentralWidget::reset3()
{
    next3btn->setDisabled(true);
    reset3btn->setDisabled(true);
}

void CentralWidget::setup4()
{
    omc_plot = new Plot(this);
    connect(omc_plot, &Plot::user_finished, this, &CentralWidget::onPlotOmcFinished);
    recalcTRB_pBtn = new QPushButton("Recalculate TB and RB", this);

    next4btn = new QPushButton("NEXT \n STEP", this);
    next4btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    reset4btn = new QPushButton("RESET", this);
    reset4btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    reset4btn->setStyleSheet("background-color: red; color: white; font: bold 14px;");

    grid4 = new QGridLayout;
    grid4->addWidget(new CustomTitle("Time and Range Bias calculation, OMC plotting and filtering"), 0, 1, 1, 5);
    grid4->addWidget(reset4btn, 1, 0, 2, 1);
    grid4->addWidget(esc_pBtn, 1, 1);
    grid4->addWidget(undo_pBtn, 1, 2);
    grid4->addWidget(recalcTRB_pBtn, 1, 3);
    grid4->addWidget(up_pBtn, 1, 4);
    grid4->addWidget(down_pBtn, 1, 5);
    grid4->addWidget(omc_plot, 2, 1, 1, 5);
    grid4->addWidget(next4btn, 1, 6, 2, 1);
    grid4->setRowStretch(1,1);
    grid4->setRowStretch(2,2);
    setLayout(grid4);

    connect(next4btn, &QPushButton::clicked, this, [&]() {
        if (omc_plot != nullptr) {
            grid4->removeWidget(omc_plot);
            delete omc_plot;
        }
        setup5();
    });
}

void CentralWidget::reset4()
{
    reset4btn->setDisabled(true);
    next4btn->setDisabled(true);
}

void CentralWidget::setup5()
{
    polyTable = new QTextEdit(this);
    nql_plot = new QCustomPlot(this);
    residDist_plot = new QCustomPlot(this);

    next5btn = new QPushButton("NEXT \n STEP", this);
    next5btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    next5btn->setDisabled(true);

    grid5 = new QGridLayout;
    grid5->addWidget(new CustomTitle("Polynomial fitting. Normal point extraction."), 0, 0, 1, 2);
    grid5->addWidget(polyTable, 1, 0);
    grid5->addWidget(next5btn, 1, 2, 2, 1);
    setLayout(grid5);

    connect(next5btn, &QPushButton::clicked, this, [&]() {
        if (nql_plot != nullptr) {
            grid5->removeWidget(nql_plot);
            delete nql_plot;
        }
        if (residDist_plot != nullptr) {
            grid5->removeWidget(residDist_plot);
            delete residDist_plot;
        }
        setup6();
    });
}

void CentralWidget::setup6()
{
    send = new QPushButton("SEND FILES TO ILRS", this);
    send->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    send->setDisabled(true);
    send->setStyleSheet("");
    npt_content = new QTextEdit(this);
    npt_content->setFont(QFont("Courier New",9,QFont::Monospace));
    frd_content = new QTextEdit(this);
    frd_content->setFont(QFont("Courier New",9,QFont::Monospace));
    nptf_label = new QLabel(this);
    frdf_label = new QLabel(this);

    grid6 = new QGridLayout;
    grid6->addWidget(new CustomTitle("Write CRD files and send them to ILRS"), 0, 0, 1, 2);
    grid6->addWidget(nptf_label, 1, 0);
    grid6->addWidget(frdf_label, 1, 1);
    grid6->addWidget(npt_content, 2, 0);
    grid6->addWidget(frd_content, 2, 1);
    grid6->addWidget(send, 3, 0, 1, 2);
    setLayout(grid6);
}

void CentralWidget::updateCatCalTable(std::vector<datamodels::CalCatElem> &cat, QTableWidget *table)
{
    table->setColumnCount(22);
    table->setHorizontalHeaderLabels({"nr" ,"nsta", "kodst", "nam", "MJD", "secday", "filtr", "MEAN", "RMS", "Corect", "temp", "pres", "humid", "nflash", "nreturn", "weather", "numpass", "coef", "SKEW", "KURT", "target", "name"});
    table->showGrid();
    bool showAllCals = !(Config::Instance()->doFilterCalView());
    int r = 0;

    for (size_t i = 0; i < cat.size(); ++i) {
        int64_t startObs = 86400*UniqueKAT_OBS::Instance()->cat[nrObs-1].mjd + UniqueKAT_OBS::Instance()->cat[nrObs-1].secday;
        int64_t startCal = 86400*cat[i].mjd + cat[i].secday;

        if (fabs(startObs-startCal) < 7200. || showAllCals) {
            table->setRowCount(r+1);
            table->setItem(r, 0, new CatTableItem(QString::number(cat[i].nr)));
            table->setItem(r, 1, new CatTableItem(QString::number(cat[i].nsta)));
            table->setItem(r, 2, new CatTableItem(QString(cat[i].kodst)));
            table->setItem(r, 3, new CatTableItem(QString(cat[i].nam)));
            table->setItem(r, 4, new CatTableItem(QString::number(cat[i].mjd)));
            table->setItem(r, 5, new CatTableItem(QString::number(cat[i].secday)));
            table->setItem(r, 6, new CatTableItem(QString::number(cat[i].filtr)));
            table->setItem(r, 7, new CatTableItem(QString::number(cat[i].meanval)));
            table->setItem(r, 8, new CatTableItem(QString::number(cat[i].rms)));
            table->setItem(r, 9, new CatTableItem(QString::number(cat[i].corect)));
            table->setItem(r,10, new CatTableItem(QString::number(cat[i].temp)));
            table->setItem(r,11, new CatTableItem(QString::number(cat[i].baro)));
            table->setItem(r,12, new CatTableItem(QString::number(cat[i].humid)));
            table->setItem(r,13, new CatTableItem(QString::number(cat[i].nflash)));
            table->setItem(r,14, new CatTableItem(QString::number(cat[i].nreturn)));
            table->setItem(r,15, new CatTableItem(QString::number(cat[i].weather)));
            table->setItem(r,16, new CatTableItem(QString::number(cat[i].numberpas)));
            table->setItem(r,17, new CatTableItem(QString::number(cat[i].coef)));
            table->setItem(r,18, new CatTableItem(QString::number(cat[i].skew)));
            table->setItem(r,19, new CatTableItem(QString::number(cat[i].kurt)));
            table->setItem(r,20, new CatTableItem(QString::number(cat[i].target)));
            table->setItem(r,21, new CatTableItem(QString(cat[i].namcal)));

            calBeforeBox->addItem("(" + QString::number(i+1) + ")  " + cat[i].namcal);
            calAfterBox ->addItem("(" + QString::number(i+1) + ")  " + cat[i].namcal);

            if (fabs(startObs - startCal) < 7200.) {  // if calibration was taken within 2 hours before/after the obs,
                for (int c=0; c<22; ++c)              // highlight it
                    table->item(r,c)->setBackground(QBrush(QColor(200,16,64,48)));
            }
            ++r;
        }
    }
    table->resizeColumnsToContents();
    table->resizeRowsToContents();
    table->verticalHeader()->setVisible(false);
    table->setCurrentCell(table->rowCount()-1, 0);
}

void CentralWidget::updateCatObsTable(std::vector<datamodels::ObsCatElem> &cat, QTableWidget *table)
{
    table->setColumnCount(18);
    table->setHorizontalHeaderLabels({"nr", "ws", "ws1", "MJD", "secday", "nsat", "nsta", "kodst", "name", "npoint", "temp", "pres", "humid", "TB", "RB", "RMS", "POLY", "clock_cor", "SKEW", "KURT", "target", "name"});
    table->showGrid();

    table->setRowCount(cat.size());
    for (int i = 0; i < table->rowCount(); ++i) {
        table->setItem(i, 0, new CatTableItem(QString::number(cat[i].nr)));
        table->setItem(i, 1, new CatTableItem(QString::number(cat[i].ws)));
        if (cat[i].ws1 == -1)
            table->setItem(i, 2, new CatTableItem("orig"));
        else
            table->setItem(i, 2, new CatTableItem("copy"));
        table->setItem(i, 3, new CatTableItem(QString::number(cat[i].mjd)));
        table->setItem(i, 4, new CatTableItem(QString::number(cat[i].secday)));
        table->setItem(i, 5, new CatTableItem(QString::number(cat[i].nsat)));
        table->setItem(i, 6, new CatTableItem(QString::number(cat[i].nsta)));
        table->setItem(i, 7, new CatTableItem(QString(cat[i].kodst)));
        table->setItem(i, 8, new CatTableItem(QString(cat[i].namefe)));
        table->setItem(i, 9, new CatTableItem(QString::number(cat[i].npoint)));
        table->setItem(i,10, new CatTableItem(QString::number(cat[i].temp)));
        table->setItem(i,11, new CatTableItem(QString::number(cat[i].pres)));
        table->setItem(i,12, new CatTableItem(QString::number(cat[i].humid)));
        table->setItem(i,13, new CatTableItem(QString::number(cat[i].TB)));
        table->setItem(i,14, new CatTableItem(QString::number(cat[i].RB)));
        table->setItem(i,15, new CatTableItem(QString::number(cat[i].RMS)));
        table->setItem(i,16, new CatTableItem(QString::number(cat[i].POLY)));
        table->setItem(i,17, new CatTableItem(QString::number(cat[i].clock_cor)));
    }
    table->resizeColumnsToContents();
    table->resizeRowsToContents();
    table->verticalHeader()->setVisible(false);
    table->setCurrentCell(table->rowCount()-1, 0);
}

void CentralWidget::updateCatObsTreeView(std::vector<datamodels::ObsCatElem> &cat)
{
    // clear tree view
    QTreeWidgetItem *item = catObsTreeView->takeTopLevelItem(0);
    while (item) {
        delete item;
        item = catObsTreeView->takeTopLevelItem(0);
    }

    // transform list data -> tree data
    CatObsTree data;

    std::vector<std::string> obsNames; // unique passage names ("SATmmdd#")
    for (datamodels::ObsCatElem e: cat)
        obsNames.push_back(std::string(e.namefe));

    auto it = std::unique(obsNames.begin(), obsNames.end());
    obsNames.erase(it, obsNames.end());

    // map a unique passage name on it's records in KAT_OBS
    for (size_t i=0; i<obsNames.size(); ++i) {
        std::vector<datamodels::ObsCatElem> obsSession;
        for (datamodels::ObsCatElem e: cat)
            if (obsNames[i].compare(std::string(e.namefe)) == 0)
                obsSession.push_back(e);
        data.push_back(std::make_pair(obsNames[i], obsSession));
    }


    // fill the view with provided tree data
    for (auto session : data) {
        // add tree view item pSingleSession - expanding button with session name - at top level
        QTreeWidgetItem *pSingleSession = new QTreeWidgetItem();
        catObsTreeView->addTopLevelItem(pSingleSession);
        catObsTreeView->setItemWidget(pSingleSession, 0,
                                      new QtCategoryButton(QString::fromStdString(session.first), catObsTreeView, pSingleSession));

        // add table based on records of a unique session from KAT_OBS at bottom level of tree view
        QTableWidget *sessionRecords = new QTableWidget(this);
        updateCatObsTable(session.second, sessionRecords);
        highlightConnections.push_back(connect(sessionRecords, &QTableWidget::cellClicked, this, &CentralWidget::obsHighlighted));

        QTreeWidgetItem *pSingleSessionTbl = new QTreeWidgetItem();
        pSingleSession->addChild(pSingleSessionTbl);
        catObsTreeView->setItemWidget(pSingleSessionTbl, 0, sessionRecords);
    }
}

void CentralWidget::obsHighlighted(int row, int column)
{
    QTableWidget *table = (QTableWidget*) sender(); // specify what table was highlighted
    int nr = table->item(row, 0)->text().toInt(); // get "nr" value from record in table

    selObsBtn->setText("Press to select " + QString(UniqueKAT_OBS::Instance()->cat[nr-1].namefe) + " (nr=" + QString::number(nr) + ")");
    selObsBtn->setDisabled(false);
    for (int i=0; i<table->rowCount(); ++i)
        if (i != row) {
            for (int c=0; c<18; ++c)
                table->item(i,c)->setBackground(QBrush(QColor(255,255,255,255)));
        } else {
            for (int c=0; c<18; ++c)
                table->item(i,c)->setBackground(QBrush(QColor(0,255,0,48)));
        }
    nrObs = nr;
}

void CentralWidget::plotOmc(QVector<double> X, QVector<double> Y, QString h1, QString h2)
{
    omc_plot->clear();
    omc_plot->graph()->setData(X, Y);
    omc_plot->isPointDropped.fill(false, X.size());

    omc_plot->xAxis->setLabel(QString("TIME [ S ]"));
    omc_plot->yAxis->setLabel(QString("RESIDUALS [ M ]"));

    omc_plot->plotLayout()->addElement(0, 0, new QCPTextElement(omc_plot, h1, QFont("sans", 12, QFont::Bold)));
    omc_plot->plotLayout()->addElement(1, 0, new QCPTextElement(omc_plot, h2, QFont("sans", 12, QFont::Bold)));

    omc_plot->graph()->rescaleAxes();
    omc_plot->replot();
    omc_plot->setFocus();
}

void CentralWidget::onPlotOmcFinished(bool save)
{
    emit plotOmcResult(save, omc_plot->isPointDropped, (*(omc_plot->graph()->data())).size());
}

void CentralWidget::showPolyFitTable(QString content)
{
    polyTable->setText(content);
    polyTable->setFont(QFont("Courier New",9,QFont::Monospace));
    polyTable->setReadOnly(true);
}
