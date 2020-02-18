#include "plot.h"
#include <QPainter>

Plot::Plot(QWidget *parent) : QCustomPlot(parent)
{
    isReadyForFiltering = false;

    addGraph();
    graph()->setLineStyle(QCPGraph::lsNone);
    graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, Qt::blue, PIXEL_SIZE));
    setBackground(QBrush(Qt::gray));

    // two header lines
    plotLayout()->insertRow(0);
    plotLayout()->insertRow(0);
}

void Plot::closeEvent(QCloseEvent *event)
{
    event->ignore();
}

void Plot::mousePressEvent(QMouseEvent *event)
{
    QCustomPlot::mousePressEvent(event);
    isReadyForFiltering = false;
    start = event->pos();
}

void Plot::mouseReleaseEvent(QMouseEvent *event)
{
    QCustomPlot::mouseReleaseEvent(event);
    stop = event->pos();
    update();

    isReadyForFiltering = true;
    xyStart = QPointF(xAxis->pixelToCoord(start.x()), yAxis->pixelToCoord(start.y()));
    xyStop  = QPointF(xAxis->pixelToCoord(stop.x()), yAxis->pixelToCoord(stop.y()));
    if (xyStart.x() > xyStop.x())
        std::swap(xyStart, xyStop);
}

void Plot::mouseMoveEvent(QMouseEvent *event)
{
    QCustomPlot::mouseMoveEvent(event);
    auto btns = event->buttons();
    if (btns == Qt::LeftButton || btns == Qt::RightButton) {
        stop = event->pos();
        update();
    }
}

void Plot::mouseDoubleClickEvent(QMouseEvent *event)
{
    QCustomPlot::mouseDoubleClickEvent(event);
    QPointF pointAreaLeftBottom = QPointF(xAxis->pixelToCoord(event->pos().x() - PIXEL_SIZE), yAxis->pixelToCoord(event->pos().y() + PIXEL_SIZE)),
            pointAreaRightTop   = QPointF(xAxis->pixelToCoord(event->pos().x() + PIXEL_SIZE), yAxis->pixelToCoord(event->pos().y() - PIXEL_SIZE));

    auto oldData = *(graph()->data());
    dataHistory.push_back(qMakePair(oldData,isPointDropped));

    for (int i=0, fi = 0; i<isPointDropped.size(); i++) {
        if (!isPointDropped[i]) {
            QCPGraphData point = *oldData.at(fi);
            if (point.key > pointAreaLeftBottom.x() && point.key < pointAreaRightTop.x())
                if (point.value > pointAreaLeftBottom.y() && point.value < pointAreaRightTop.y()) {
                    graph()->data()->remove(point.key);
                    isPointDropped[i] = true;
                }
            ++fi;
        }
    }
    graph()->rescaleAxes();
    replot();
}

void Plot::paintEvent(QPaintEvent *pe)
{
    QCustomPlot::paintEvent(pe);
    if (start == stop)
        return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(Qt::red));
    p.drawLine(start, stop);
}

void Plot::keyPressEvent(QKeyEvent *event)
{
    // remove line
    start = stop;
    update();

    QWidget::keyPressEvent(event);
    if (isReadyForFiltering) {
        if (event->key() == Qt::Key_Up)
            clearAbove();
        if (event->key() == Qt::Key_Down)
            clearBelow();
        isReadyForFiltering = false;
    }
    if (event->key() == Qt::Key_Escape) {
        QMessageBox mb;
        mb.setText("Accept changes and remove " + QString::number(isPointDropped.count(true)) + " points?");
        mb.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        int code = mb.exec();
        switch (code) {
            case QMessageBox::Save:    {emit user_finished(true);  break;}
            case QMessageBox::Discard: {emit user_finished(false); break;}
            case QMessageBox::Cancel:  {                           break;}
        }
    }
    if (event->matches(QKeySequence::Undo)) {
        if (!dataHistory.isEmpty()) {
            auto t = dataHistory.takeLast();
            graph()->data()->set(t.first);
            isPointDropped = t.second;
            graph()->rescaleAxes();
            replot();
            emit filtered(graph()->data()->size());
        }
    }
    if (event->key() == Qt::Key_P) {
        // printing
    }
}

void Plot::clearAbove()
{
    auto oldData = *(graph()->data());
    dataHistory.push_back(qMakePair(oldData,isPointDropped));

    for (int i=0, fi = 0; i<isPointDropped.size(); i++) {
        if (!isPointDropped[i]) {
            QCPGraphData point = *oldData.at(fi);
            if (point.key > xyStart.x() && point.key < xyStop.x()) {
                double yBorder = (xyStop.y() - xyStart.y())/(xyStop.x() - xyStart.x())*(point.key - xyStart.x()) + xyStart.y();
                if (point.value > yBorder) {
                    graph()->data()->remove(point.key);
                    isPointDropped[i] = true;
                }
            }
            ++fi;
        }
    }
    graph()->rescaleAxes();
    replot();
    emit filtered(graph()->data()->size());
}

void Plot::clearBelow()
{
    auto oldData = *(graph()->data());
    dataHistory.push_back(qMakePair(oldData,isPointDropped));

    for (int i=0, fi = 0; i<isPointDropped.size(); i++) {
        if (!isPointDropped[i]) {
            QCPGraphData point = *oldData.at(fi);
            if (point.key > xyStart.x() && point.key < xyStop.x()) {
                double yBorder = (xyStop.y() - xyStart.y())/(xyStop.x() - xyStart.x())*(point.key - xyStart.x()) + xyStart.y();
                if (point.value < yBorder) {
                    graph()->data()->remove(point.key);
                    isPointDropped[i] = true;
                }
            }
            ++fi;
        }
    }
    graph()->rescaleAxes();
    replot();
    emit filtered(graph()->data()->size());
}

void Plot::autoFilter(const QVector<double> &et, const QVector<double> &er,
                      int bandLength, double bandWidth, double snr, double sigmaRate)
{
    auto oldData = *(graph()->data());
    dataHistory.push_back(qMakePair(oldData,isPointDropped));

    QVector<double> acc(et.size()-2);
    for (int i=0; i<acc.size(); ++i) // calculate velocity in efemeris
        acc[i] = utility::inter_parabola(et[i], er[i], et[i+1], er[i+1], et[i+2], er[i+2], et[i]+1e-1) - er[i];
    for (int i=0; i<acc.size()-2; ++i) // calculate acceleration in efemeris
        acc[i] = utility::inter_parabola(et[i], acc[i], et[i+1], acc[i+1], et[i+2], acc[i+2], et[i]+1e-1) - acc[i];
    acc.resize(acc.size()-2);

    double maxAcc = 0; // calculate max acceleration
    for (int i=0; i<acc.size(); ++i) {
        double facc = fabs(acc[i]);
        if (facc > maxAcc) maxAcc = facc;
    }

    // filter sigma criteria
    double gates = 0., aver = 0.;
    for (auto point : oldData) aver += point.value;
    aver /= oldData.size();
    for (auto point : oldData) gates += (point.value-aver)*(point.value-aver);
    gates = sigmaRate*sqrt(gates/oldData.size());
    for (int i=0, fi = 0; i<isPointDropped.size(); i++)
        if (!isPointDropped[i]) {
            QCPGraphData point = *oldData.at(fi);
            if (fabs(point.value-aver) > gates) {
                graph()->data()->remove(point.key);
                isPointDropped[i] = true;
            }
            fi++;
        }
    oldData = *(graph()->data());

    double minR = (*std::min_element(oldData.begin(), oldData.end(),[](QCPGraphData const &a, QCPGraphData const &b){
        return a.value < b.value;
    })).value;
    double maxR = (*std::max_element(oldData.begin(), oldData.end(),[](QCPGraphData const &a, QCPGraphData const &b){
        return a.value < b.value;
    })).value;

    // variables for algo
    int i,segmentFinishPos,inPoints;
    double k,b,yBot,yTop,y,x0;
    bool moveBandFlag;

    QVector<int> pdropIndexes(oldData.size());
    for (int i=0, fi=0; i<isPointDropped.size(); ++i)
        if (!isPointDropped[i]) {
            pdropIndexes[fi] = i;
            fi++;
        }
    isPointDropped.fill(true);
    graph()->data()->clear();

    for (int pos=bandLength; pos<=oldData.size(); ++pos) { // move band end position
        for (k = -maxAcc; k < maxAcc+1e-10; k += maxAcc/50) {
            for (b = minR; b < maxR+1e-10; b += bandWidth) { // create band based on r=kt+b
                inPoints = 0;
                moveBandFlag = false;
                x0 = oldData.at(pos-bandLength)->key;
                for (i=bandLength; i>0; --i) {
                    yBot = k*(oldData.at(pos-i)->key - x0) + b - bandWidth/2;
                    yTop = yBot + bandWidth;
                    y = oldData.at(pos-i)->value;
                    if (y > yBot && y < yTop)
                        ++inPoints;
                }

                if (inPoints > snr*bandLength) {
                    segmentFinishPos = 0;
                    for (i=bandLength; i>0; --i) {
                        yBot = k*(oldData.at(pos-i)->key - x0) + b - bandWidth/2;
                        yTop = yBot + bandWidth;
                        y = oldData.at(pos-i)->value;
                        if (y > yBot && y < yTop) {
                            graph()->addData(oldData.at(pos-i)->key, y);
                            isPointDropped[ pdropIndexes[pos-i] ] = false;
                            segmentFinishPos = pos-i;
                        }
                    }
                    pos = segmentFinishPos+bandLength+1;
                    moveBandFlag = true;
                    break;
                }
            }
            if (moveBandFlag) break;
        }
    }


    graph()->rescaleAxes();
    replot();
    emit filtered(graph()->data()->size());
}

void Plot::clear()
{
    isReadyForFiltering = false;
    dataHistory.clear();
//    Xinit.clear();
//    Yinit.clear();
}
