#ifndef COLLATZCALCULATOR_H
#define COLLATZCALCULATOR_H

#include <QObject>
#include "common.h"

class CollatzCalculator : public QObject
{
    Q_OBJECT
public:
    explicit CollatzCalculator(QObject *parent = nullptr);

    bool isRunning();

signals:
    void finished();
    void runningStateChanged(bool state);
    void calculationFinished(std::shared_ptr<tResult> res);

public slots:
    void start(tNumType value, int threadNumber);
    void stop();

private:
    void setRunningState(bool state);
    void startParCalulation(tNumType value, int threadNum, std::shared_ptr<tResult> resPtr);
    void startCalcPerThread( std::shared_ptr<tResult> resPtr, tNumType startIdx, tNumType endIdx, tNumType size, tNode*& max);
    tNumType calculateChain(std::vector<tNode>& res, const tNumType& size, tNumType n);

    std::atomic<bool> mIsRunning{false};
};

#endif // COLLATZCALCULATOR_H
