#include "collatzcalculator.h"
#include "qdebug.h"
#include <mutex>
#include <thread>
#include <chrono>

CollatzCalculator::CollatzCalculator(QObject *parent)
    : QObject{parent}
{}

void CollatzCalculator::start(tNumType value, int threadNumber)
{
    qDebug() << "Start calculation(" << value<< "," << threadNumber << ")\n";
    try{
        setRunningState(true);

        std::shared_ptr<tResult> resPtr = std::make_shared<tResult>(value);
        emit calculationStarted(resPtr);

        startParCalulation(value, threadNumber, resPtr);
        setRunningState(false);
        emit calculationFinished();
        emit finished();

    }catch(std::exception& ex)
    {
        qDebug() << ex.what();
        setRunningState(false);
        emit finished();
    }
}

void CollatzCalculator::stop()
{
    qDebug() << "Stop calculation\n";

    setRunningState(false);
    emit finished();
}

void CollatzCalculator::setRunningState(bool state)
{
    std::unique_lock lock(mMux);
    mIsRunning = state;
}

bool CollatzCalculator::isRunning()
{
    std::shared_lock lock(mMux);
    return mIsRunning;
}


void CollatzCalculator::startParCalulation(tNumType value, int threadNum, std::shared_ptr<tResult> resPtr)
{
    qDebug() << "CollatzCalculator::runThreads(" << value << "," << threadNum <<")";
    auto start = std::chrono::high_resolution_clock::now();

    tResult& res = *resPtr;
    res.data.reserve(value);
    res.max = &res.data[1];

    tNumType startIdx = 0;
    tNumType endIdx = 0;
    std::vector<std::thread> threads;
    threads.reserve(threadNum);
    std::vector<tNode*> maxValues(threadNum, nullptr);

    const tNumType size = res.data.size();
    const tNumType interval = size / threadNum;
    for (int  tIdx = 0; tIdx < threadNum; tIdx++)
    {
        startIdx = tIdx * interval;
        endIdx = tIdx * interval + interval;
        maxValues[tIdx] = &res.data[startIdx];
        threads.emplace_back(std::thread(&CollatzCalculator::startCalcPerThread, this, resPtr, startIdx, endIdx, size, std::ref(maxValues[tIdx]) ));
    }

    for (auto& t : threads)
    {
        t.join();
    }

    res.max = *std::max_element(maxValues.begin(), maxValues.end(), []( tNode* left, tNode* right)
    {
        return (left->chainLen.load() < right->chainLen.load());
    });

    auto end = std::chrono::high_resolution_clock::now();
    res.timeDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    qDebug() << "Time for variant:" << res.timeDuration << "ms";
    qDebug() << "Result: val " << res.max->value << ",len:" << res.max->chainLen;
}

void CollatzCalculator::startCalcPerThread( std::shared_ptr<tResult> resPtr, tNumType startIdx, tNumType endIdx, tNumType size, tNode*& max)
{
    tResult& res = *resPtr;
    for (tNumType j = startIdx; j < endIdx; j++)
    {
        //if( !isRunning() ) return;
        tNumType chainLen = calculateChain(res.data, size, j+1);
        res.data[j].value.store(j + 1);
        res.data[j].chainLen.store(chainLen);
        if (max->chainLen < res.data[j].chainLen)
        {
            max = &res.data[j];
        }
    }
}

tNumType CollatzCalculator::calculateChain(std::vector<tNode>& v, const tNumType& size, tNumType n)
{
    tNumType res = 1;
    while (n != 1)
    {
        if (n < size && v[n - 1].chainLen != 0)
        {
            res += v[n - 1].chainLen-1;
            break;
        }
        if (n % 2 == 0)
        {
            n = n / 2;
        }
        else
        {
            if(n > cValue_MaxLimit )
            {
                v[n-1].isFailed.store(true);
                res = 0;
                break;
            }
            n = 3 * n + 1;
        }
        res++;
    }
    return res;
}

