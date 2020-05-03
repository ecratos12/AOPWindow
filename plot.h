#ifndef PLOT_H
#define PLOT_H

#include <utility.h>
#include <qcustomplot/qcustomplot.h>

typedef QVector<bool> DropFlags;
#define PIXEL_SIZE 4

class Plot : public QCustomPlot
{
    Q_OBJECT
public:
    Plot(QWidget *parent = nullptr);

    DropFlags isPointDropped;
//    inline void setInitData(const QVector<double>& x, const QVector<double>& y) {Xinit = x; Yinit = y;}
//    inline QVector<double> initX() {return Xinit;}
//    inline QVector<double> initY() {return Yinit;}

public slots:
    void clear();
    void autoFilter(const QVector<double> &et, const QVector<double> &er,
                    const int& CARRIER_MIN_PTS, const double& CARRIER_MIN_TIME, double bandWidth, double sigmaRate);
    void setTitles(QString h1, QString h2);
    void setEfemeris(const QVector<double> &et, const QVector<double> &er);

    void evaluateTRB();

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;

protected slots:
    void clearAbove();
    void clearBelow();

signals:
    void user_finished(bool save);
    void filtered(int pointsRemain);

protected:
    QPoint start, stop;      // pixel values
    QPointF xyStart, xyStop; // graph values
    bool isReadyForFiltering;

    bool trb_evaluated;
    double TB, RB, eTB, eRB; // roughly evaluated
    QVector<double> _et, _er;

    QList<QPair<QCPGraphDataContainer, DropFlags>> dataHistory;
    QCPTextElement *_h1, *_h2;
    QVector<QCPGraphData> selSignalPoints;
//    QVector<double> Xinit, Yinit;
};

#endif // PLOT_H
