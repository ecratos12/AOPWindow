#include "plot.h"
#include <QPainter>

Plot::Plot(QWidget *parent) : QCustomPlot(parent),
    _h1(nullptr),
    _h2(nullptr)
{
    isReadyForFiltering = false;
    trb_evaluated = false;

    addGraph();
    addGraph();
    graph()->setLineStyle(QCPGraph::lsNone);
    graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, Qt::blue, PIXEL_SIZE));
    graph(0)->setLineStyle(QCPGraph::lsNone);
    graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::green, PIXEL_SIZE*5/2));

    setBackground(QBrush(Qt::gray));

    // two header lines
    plotLayout()->insertRow(0);
    plotLayout()->insertRow(0);
}

void Plot::setEfemeris(const QVector<double> &et, const QVector<double> &er)
{
    _et = et;
    _er = er;
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

    Qt::KeyboardModifiers kbm = QApplication::keyboardModifiers();
    if (kbm.testFlag(Qt::ControlModifier)) {
        QPointF pointAreaLeftBottom = QPointF(xAxis->pixelToCoord(event->pos().x() - PIXEL_SIZE), yAxis->pixelToCoord(event->pos().y() + PIXEL_SIZE)),
                pointAreaRightTop   = QPointF(xAxis->pixelToCoord(event->pos().x() + PIXEL_SIZE), yAxis->pixelToCoord(event->pos().y() - PIXEL_SIZE));

        for (int i=0, fi = 0; i<isPointDropped.size(); i++) {
            if (!isPointDropped[i]) {
                QCPGraphData point = *graph()->data()->at(fi);
                if (point.key > pointAreaLeftBottom.x() && point.key < pointAreaRightTop.x())
                    if (point.value > pointAreaLeftBottom.y() && point.value < pointAreaRightTop.y()) {
                        selSignalPoints.push_back(point);
                        graph(0)->data()->add(point);
                    }
                ++fi;
            }
        }
    }
}

void Plot::evaluateTRB()
{
    double sxx = 0., sx = 0., syx = 0., sy = 0., v, s = selSignalPoints.size();
    if (s < 2) return; // not enough points to do least squares

    // solve "ysig = vsig*TB + RB" by least squares
    int stepefe = 1, index;
    for (QCPGraphData p : selSignalPoints) {
        index = (static_cast<int>(p.key - _et[0]))/stepefe;
        if (index < 1) index = 1;
        if (index > _et.size()-2) index = _et.size()-2;
        --index;

        v = (_er[index+1] - _er[index-1])/2;
        sx += v;
        sxx += v*v;
        sy += p.value;
        syx += p.value*v;
    }
    double det = s*sxx-sx*sx;
    TB = (s*syx-sx*sy)/det;
    RB = (sxx*sy-sx*syx)/det;
    eTB = sqrt(s/det);
    eRB = sqrt(sxx/det);

    // rebuild plots, applying TB and RB
    auto oldData = *(graph()->data());
    dataHistory.push_back(qMakePair(oldData,isPointDropped));
    QVector<double> xall,yall;
    for (int i=0, fi = 0; i<isPointDropped.size(); i++) {
        if (!isPointDropped[i]) {
            QCPGraphData point = *oldData.at(fi);
            xall.push_back(point.key);

            index = (static_cast<int>(point.key - _et[0]))/stepefe;
            if (index < 1) index = 1;
            if (index > _et.size()-2) index = _et.size()-2;
            --index;
            v = (_er[index+1] - _er[index-1])/2;
            yall.push_back(point.value - TB*v - RB);

            ++fi;
        }
    }
    QVector<double> xsig,ysig;
    for (QCPGraphData p : selSignalPoints) {
        xsig.push_back(p.key);

        index = (static_cast<int>(p.key - _et[0]))/stepefe;
        if (index < 1) index = 1;
        if (index > _et.size()-2) index = _et.size()-2;
        --index;
        v = (_er[index+1] - _er[index-1])/2;
        ysig.push_back(p.value - TB*v - RB);
    }

    graph()->setData(xall,yall);
    graph(0)->setData(xsig,ysig);
    graph()->rescaleAxes();
    setTitles(_h1->text(), QString("RB = " + QString::number(RB, 'f', 4) + " +- " + QString::number(eRB, 'f', 4) + " m, " +
                                   "TB = " + QString::number(TB*1000., 'f', 4) + " +- " + QString::number(eTB*1000., 'f', 4) + " ms"));
    trb_evaluated = true;
    replot();
    setFocus();
}

void Plot::mouseReleaseEvent(QMouseEvent *event)
{
    QCustomPlot::mouseReleaseEvent(event);
    stop = event->pos();
    update();
    replot();

    isReadyForFiltering = true;
    xyStart = QPointF(xAxis->pixelToCoord(start.x()), yAxis->pixelToCoord(start.y()));
    xyStop  = QPointF(xAxis->pixelToCoord(stop.x()), yAxis->pixelToCoord(stop.y()));
    if (xyStart.x() > xyStop.x())
        std::swap(xyStart, xyStop);
}

void Plot::mouseMoveEvent(QMouseEvent *event)
{
    QCustomPlot::mouseMoveEvent(event);
    QToolTip::showText(event->pos(), "Double-click to remove, CTRL-click to select as signal");
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
            case QMessageBox::Save:    {emit user_finished(true); /*qApp->exit(0);*/ break;}
            case QMessageBox::Discard: {emit user_finished(false);/*qApp->exit(1);*/ break;}
            case QMessageBox::Cancel:  {                                        break;}
        }
    }
    if (event->matches(QKeySequence::Undo)) {
        selSignalPoints.clear();
        graph(0)->data()->clear();
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
    double minR,maxR,maxAcc,minV,maxV;
    double maxTB = 1e-1; // robust assumption to calculate max possible acceleration -> max OMC curve's slope

    QVector<double> acc(et.size()-2);
    for (int i=0; i<acc.size(); ++i) // calculate velocity in efemeris
        acc[i] = utility::inter_parabola(et[i], er[i], et[i+1], er[i+1], et[i+2], er[i+2], et[i]+1e-1) - er[i];
    minV = *std::min_element(acc.begin(), acc.end())/maxTB;
    maxV = *std::max_element(acc.begin(), acc.end())/maxTB;

    for (int i=0; i<acc.size()-2; ++i) // calculate acceleration in efemeris
        acc[i] = utility::inter_parabola(et[i], acc[i], et[i+1], acc[i+1], et[i+2], acc[i+2], et[i]+1e-1) - acc[i];
    acc.resize(acc.size()-2);


    maxAcc = 0; // calculate max possible acceleration
    for (int i=0; i<acc.size(); ++i) {
        double facc = fabs(acc[i]);
        if (facc > maxAcc) maxAcc = facc;
    }

    // filter sigma criteria
    auto oldData = *(graph()->data());
    dataHistory.push_back(qMakePair(oldData,isPointDropped));

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

    if (trb_evaluated) {
        minR = minV*eTB - eRB - bandWidth;
        maxR = maxV*eTB + eRB + bandWidth;
        maxAcc *= eTB/maxTB; // scale maxAcc
    } else {
        minR = (*std::min_element(oldData.begin(), oldData.end(),[](QCPGraphData const &a, QCPGraphData const &b){
            return a.value < b.value;
        })).value;
        maxR = (*std::max_element(oldData.begin(), oldData.end(),[](QCPGraphData const &a, QCPGraphData const &b){
            return a.value < b.value;
        })).value;
    }

    // variables for algo
    int i,segmentFinishPos,inPoints;
    double k,b,yBot,yTop,y,x0;
    bool moveBandFlag = false;

    // re-assamble drop flags due to adding data by algorithm, not removing
    QVector<int> pdropIndexes(oldData.size());
    for (int i=0, fi=0; i<isPointDropped.size(); ++i)
        if (!isPointDropped[i]) {
            pdropIndexes[fi] = i;
            fi++;
        }
    isPointDropped.fill(true);
    graph()->data()->clear();

    // perform fan search
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
    selSignalPoints.clear();
    graph(0)->data()->clear();
    graph()->data()->clear();
    replot();
}

void Plot::setTitles(QString h1, QString h2)
{
    if (!_h1) {
        _h1 = new QCPTextElement(this, h1, QFont("sans", 12, QFont::Bold));
        this->plotLayout()->addElement(0,0,_h1);
    } else
        _h1->setText(h1);

    if (!_h2) {
        _h2 = new QCPTextElement(this, h2, QFont("sans", 12, QFont::Bold));
        this->plotLayout()->addElement(1,0,_h2);
    } else
        _h2->setText(h2);
}
