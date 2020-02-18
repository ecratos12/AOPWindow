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
                    int bandLength, double bandWidth, double snr, double sigmaRate);

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
    QList<QPair<QCPGraphDataContainer, DropFlags>> dataHistory;
//    QVector<double> Xinit, Yinit;
};

#endif // PLOT_H
