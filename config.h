#ifndef CONFIG_H
#define CONFIG_H

#include <vector>
#include <fstream>

#include <QObject>
#include <QString>
#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QTableWidget>
#include <QGridLayout>
#include <QFileDialog>
#include <QFrame>
#include <QCheckBox>

class Config : public QObject
{
private:
    Q_OBJECT
    Config(){}
    Config(Config const&);
    Config& operator=(Config const&);
    static Config* pInstance;

public:

    enum class SelObsView {
        Tree = 0,
        Table,
        TreeNew, //
        TableNew //  only newly added (unprocessed yet)
    };

    struct FtpSendForm {
        QString address;
        QString login;
        QString passw;
        bool enabled;
    };

    virtual ~Config();
    static Config* Instance();

    // config file is presented as readable JSON file on disk
    bool read(const QString &configFileName);
    bool write(const QString &configFileName);

    SelObsView defaultSelObsView() const;
    std::vector<FtpSendForm> ftpDestinationList() const;
    QString archiveDir() const;
    bool doFilterCalView() const;
    bool tropoCorrection() const;

    void setDefaultSelObsView(const SelObsView& a);
    void setFtpDestinationList(const std::vector<FtpSendForm>& a);
    void clearFtpDestinationList();
    void addFtpDestination(const FtpSendForm& a);
    void setArchiveDir(const QString& a);
    void setDoFilterCalView(bool a);
    void setTropoCorrection(bool a);

signals:
    void fatal(QString errmsg);

private:
    SelObsView _defaultSelObsView;
    std::vector<FtpSendForm> _ftpDestinationList;
    QString _archiveDir;
    bool _doFilterCalView; // if true - show only calibrations within 2 hours before/after selected obs
    bool _tropoCorrection;
};


class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    SettingsDialog(bool settingsFileExists, QWidget *parent = nullptr);

private:
    QComboBox *defaultSelObsView_cb;
    QGroupBox *ftpDestinations_box;
    QTableWidget *ftpDestinationTable;
    QPushButton *addFtpDest;
    QLineEdit *archiveDirLine;
    QPushButton *archiveDirBtn;
    QCheckBox *doFilterCalViewBox, *tropoCorrBox;
    QDialogButtonBox *decisionBtns;

    QFrame *totalVisualLine;
};


#endif // CONFIG_H
