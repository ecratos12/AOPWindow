#include "config.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

Config* Config::pInstance = nullptr;

Config::~Config() {if (pInstance) delete pInstance;}

Config* Config::Instance()
{
    if (!pInstance)
        pInstance = new Config;

    return pInstance;
}

bool Config::read(const QString &configFileName)
{
    if (!QFile::exists(configFileName))
        return false;

    QFile configFile;
    configFile.setFileName(configFileName);
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit fatal("Cannot open config file! -- " + configFileName);
        return false;
    }

    QJsonObject json = QJsonDocument::fromJson(configFile.readAll()).object();
    configFile.close();

    // read default obs selection view
    QString value_defaultSelObsView = json["Default obs selection view"].toString();
    if (value_defaultSelObsView == "Tree (all)") _defaultSelObsView = SelObsView::Tree;
    if (value_defaultSelObsView == "Table (all)") _defaultSelObsView = SelObsView::Table;
    if (value_defaultSelObsView == "Tree (only new)") _defaultSelObsView = SelObsView::TreeNew;
    if (value_defaultSelObsView == "Table (only new)") _defaultSelObsView = SelObsView::TableNew;

    // read ftp destinations data
    QJsonArray ftpDestinations = json["Ftp Destinations"].toArray();
    _ftpDestinationList.clear();
    QJsonObject o;
    FtpSendForm f;
    for (QJsonValue dest : ftpDestinations) {
        o = dest.toObject();
        f.address = o["Address"].toString();
        f.login = o["Login"].toString();
        f.passw = o["Password"].toString();
        f.enabled = o["Enabled"].toBool();
        _ftpDestinationList.push_back(f);
    }

    // read achive folder
    _archiveDir = json["Archive Folder"].toString();

    // read filter cal view flag
    _doFilterCalView = json["Show cal only within 2 hours"].toBool();

    return true;
}

bool Config::write(const QString &configFileName)
{
    QFile configFile;
    configFile.setFileName(configFileName);
    if (!configFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit fatal("Cannot write config file! -- " + configFileName);
        return false;
    }

    QJsonObject json;

    // write default obs selection view
    if (_defaultSelObsView == SelObsView::Tree) json["Default obs selection view"] = "Tree (all)";
    if (_defaultSelObsView == SelObsView::Table) json["Default obs selection view"] = "Table (all)";
    if (_defaultSelObsView == SelObsView::TreeNew) json["Default obs selection view"] = "Tree (only new)";
    if (_defaultSelObsView == SelObsView::TableNew) json["Default obs selection view"] = "Table (only new)";

    // write ftp destinations data
    QJsonArray ftpDestinations;
    QJsonObject o;
    for (FtpSendForm f : _ftpDestinationList) {
        o["Address"] = f.address;
        o["Login"] = f.login;
        o["Password"] = f.passw;
        o["Enabled"] = f.enabled;
        ftpDestinations.push_back(static_cast<QJsonValue>(o));
    }
    json["Ftp Destinations"] = ftpDestinations;

    // write archive folder
    json["Archive Folder"] = _archiveDir;

    // write filter cal view flag
    json["Show cal only within 2 hours"] = _doFilterCalView;

    QJsonDocument d(json);
    configFile.write(d.toJson());
    configFile.close();

    return true;
}

Config::SelObsView Config::defaultSelObsView() const
{
    return _defaultSelObsView;
}

std::vector<Config::FtpSendForm> Config::ftpDestinationList() const
{
    return _ftpDestinationList;
}

QString Config::archiveDir() const
{
    return _archiveDir;
}

bool Config::doFilterCalView() const
{
    return _doFilterCalView;
}

void Config::setDefaultSelObsView(const Config::SelObsView &a)
{
    _defaultSelObsView = a;
}

void Config::setFtpDestinationList(const std::vector<Config::FtpSendForm> &a)
{
    _ftpDestinationList = a;
}

void Config::clearFtpDestinationList()
{
    _ftpDestinationList.clear();
}

void Config::addFtpDestination(const Config::FtpSendForm &a)
{
    _ftpDestinationList.push_back(a);
}

void Config::setArchiveDir(const QString &a)
{
    _archiveDir = a;
}

void Config::setDoFilterCalView(bool a)
{
    _doFilterCalView = a;
}


SettingsDialog::SettingsDialog(bool settingsFileExists, QWidget *parent)
    : QDialog(parent)
{
    defaultSelObsView_cb = new QComboBox(this);
    defaultSelObsView_cb->addItem("Table (all entries)");
    defaultSelObsView_cb->addItem("Tree (all entries)");
    defaultSelObsView_cb->addItem("Table (unprocessed)");
    defaultSelObsView_cb->addItem("Tree (unprocessed)"); // default !
    if (settingsFileExists) {
        Config::SelObsView sov = Config::Instance()->defaultSelObsView();
        if (sov == Config::SelObsView::Table)    defaultSelObsView_cb->setCurrentIndex(0);
        if (sov == Config::SelObsView::Tree)     defaultSelObsView_cb->setCurrentIndex(1);
        if (sov == Config::SelObsView::TableNew) defaultSelObsView_cb->setCurrentIndex(2);
        if (sov == Config::SelObsView::TreeNew)  defaultSelObsView_cb->setCurrentIndex(3);
    } else {
        defaultSelObsView_cb->setCurrentIndex(3);
    }

    ftpDestinationTable = new QTableWidget;
    ftpDestinationTable->setColumnCount(4);
    ftpDestinationTable->setRowCount(0);
    ftpDestinationTable->setHorizontalHeaderLabels({"Address", "Login", "Password", "Enabled"});
    if (settingsFileExists) {
        std::vector<Config::FtpSendForm> fs = Config::Instance()->ftpDestinationList();
        ftpDestinationTable->setRowCount(fs.size());
        for (int i=0; i<fs.size(); ++i) {
            ftpDestinationTable->setItem(i, 0, new QTableWidgetItem(fs[i].address));
            ftpDestinationTable->setItem(i, 1, new QTableWidgetItem(fs[i].login));
            ftpDestinationTable->setItem(i, 2, new QTableWidgetItem(fs[i].passw));

            QComboBox *enabled_cb = new QComboBox(ftpDestinationTable);
            enabled_cb->addItems({"Disabled", "Enabled"});
            enabled_cb->setCurrentIndex(fs[i].enabled);
            ftpDestinationTable->setCellWidget(i, 3, enabled_cb);
        }
        ftpDestinationTable->resizeColumnsToContents();
    }

    addFtpDest = new QPushButton("Add new destination");
    connect(addFtpDest, &QPushButton::clicked, this, [&](){
        int row = ftpDestinationTable->rowCount();
        ftpDestinationTable->setRowCount(row+1);

        QComboBox *enabled_cb = new QComboBox(ftpDestinationTable);
        enabled_cb->addItems({"Disabled", "Enabled"});
        enabled_cb->setCurrentIndex(1);

        ftpDestinationTable->setItem(row, 0, new QTableWidgetItem("ftp://example"));
        ftpDestinationTable->setItem(row, 1, new QTableWidgetItem);
        ftpDestinationTable->setItem(row, 2, new QTableWidgetItem);
        ftpDestinationTable->setCellWidget(row, 3, enabled_cb);
    });


    archiveDirLine = new QLineEdit(this);
    if (settingsFileExists) archiveDirLine->setText(Config::Instance()->archiveDir());
    archiveDirBtn = new QPushButton("Select Folder", this);
    connect(archiveDirBtn, &QPushButton::clicked, this, [&](){
        archiveDirLine->setText(QFileDialog::getExistingDirectory(this, tr("Select Directory"), "",
                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
    });

    doFilterCalViewBox = new QCheckBox(this);
    if (settingsFileExists) doFilterCalViewBox->setChecked(Config::Instance()->doFilterCalView());
    else doFilterCalViewBox->setChecked(true);

    totalVisualLine = new QFrame(this);
    totalVisualLine->setFrameShape(QFrame::HLine);
    totalVisualLine->setFrameShadow(QFrame::Sunken);

    decisionBtns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    ftpDestinations_box = new QGroupBox("Table of FTP destinations for CRD files", this);
    QVBoxLayout *ftpDestLayout = new QVBoxLayout(ftpDestinations_box);
    ftpDestLayout->addWidget(ftpDestinationTable);
    ftpDestLayout->addWidget(addFtpDest);

    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->addWidget(new QLabel("Default view for selecting an observation:", this), 0, 0);
    mainLayout->addWidget(defaultSelObsView_cb, 0, 1);
    mainLayout->addWidget(ftpDestinations_box, 1, 0, 1, 3);
    mainLayout->addWidget(new QLabel("Archive folder:", this), 2, 0);
    mainLayout->addWidget(archiveDirLine, 2, 1);
    mainLayout->addWidget(archiveDirBtn, 2, 2);
    mainLayout->addWidget(new QLabel("Show calibrations ONLY within 2 hours before/after selected obs", this), 3, 0, 1, 2);
    mainLayout->addWidget(doFilterCalViewBox, 3, 2);
    mainLayout->addWidget(totalVisualLine, 4, 0, 1, 3);
    mainLayout->addWidget(decisionBtns, 5, 0, 1, 3);


    connect(decisionBtns, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(decisionBtns, &QDialogButtonBox::accepted, this, [&](){
        // save settings to Config and setting.json

        int m = defaultSelObsView_cb->currentIndex();
        if (m == 0) Config::Instance()->setDefaultSelObsView(Config::SelObsView::Table);
        if (m == 1) Config::Instance()->setDefaultSelObsView(Config::SelObsView::Tree);
        if (m == 2) Config::Instance()->setDefaultSelObsView(Config::SelObsView::TableNew);
        if (m == 3) Config::Instance()->setDefaultSelObsView(Config::SelObsView::TreeNew);

        Config::Instance()->clearFtpDestinationList();
        for (int i=0; i<ftpDestinationTable->rowCount(); ++i) {
            Config::FtpSendForm f;
            f.address = ftpDestinationTable->item(i,0)->text();
            f.login = ftpDestinationTable->item(i,1)->text();
            f.passw = ftpDestinationTable->item(i,2)->text();
            f.enabled = reinterpret_cast<QComboBox*>(ftpDestinationTable->cellWidget(i,3))->currentIndex();
            Config::Instance()->addFtpDestination(f);
        }

        Config::Instance()->setArchiveDir(archiveDirLine->text());
        Config::Instance()->setDoFilterCalView(doFilterCalViewBox->isChecked());

        Config::Instance()->write("settings.json");
        accept();
    });

    setWindowTitle(tr("AOP window settings"));
    resize(650, 400);
}
