#include "collatzcalculator.h"
#include "qdebug.h"
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
        startParCalulation(value, threadNumber, resPtr);
        if(isRunning())
        {
            setRunningState(false);
            emit calculationSuccess(resPtr);
        }
        emit finished();

    }catch(std::exception& ex)
    {
        qDebug() << ex.what();
        setRunningState(false);
        emit calculationFailed(ex.what());
        emit finished();
    }
}

void CollatzCalculator::stop()
{
    qDebug() << "Stop calculation\n";
    setRunningState(false);
}

void CollatzCalculator::setRunningState(bool state)
{
    mIsRunning.store(state);
}

bool CollatzCalculator::isRunning()
{
    return mIsRunning.load();
}

void CollatzCalculator::startParCalulation(tNumType value, int threadNum, std::shared_ptr<tResult> resPtr)
{
    qDebug() << "CollatzCalculator::startParCalulation(" << value << "," << threadNum <<")";
    auto start = std::chrono::high_resolution_clock::now();

    tResult& res = *resPtr;
    res.data.reserve(value);
    res.max = &res.data[1];

    tNumType startIdx = 0;
    tNumType endIdx = 0;
    std::vector<std::thread> threads;
    threads.reserve(threadNum);
    std::vector<tNode*> maxValues(threadNum, nullptr);
    std::vector<std::vector<tNumType>> errorList(threadNum);

    const tNumType size = res.data.size();
    const tNumType interval = size / threadNum;
    for (int  tIdx = 0; tIdx < threadNum; tIdx++)
    {
        startIdx = tIdx * interval;
        endIdx = tIdx * interval + interval;
        maxValues[tIdx] = &res.data[startIdx];
        errorList[tIdx].reserve(25);
        threads.emplace_back(std::thread(&CollatzCalculator::startCalcPerThread,
                                         this,
                                         resPtr,
                                         startIdx,
                                         endIdx,
                                         size,
                                         std::ref(maxValues[tIdx]),
                                         std::ref(errorList[tIdx])));
    }

    for (auto& t : threads)
    {
        t.join();
    }

    res.max = *std::max_element(maxValues.begin(), maxValues.end(), []( tNode* left, tNode* right)
    {
        return (left->chainLen.load() < right->chainLen.load());
    });

    int errorsCount = 0;
    std::for_each(errorList.begin(), errorList.end(),[&errorsCount](const std::vector<tNumType>& v){
        errorsCount += v.size();
    });
    if(errorsCount > 0)
    {
        res.errorList.reserve(errorsCount);
        for(auto& errorsVec : errorList)
        {
            std::copy(errorsVec.begin(),errorsVec.end(), res.errorList.end());
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    res.timeDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    qDebug() << "Execution time:" << res.timeDuration << "ms";
    qDebug() << "Result: val " << res.max->value << ",len:" << res.max->chainLen;
    qDebug() << "Error count:" << errorsCount;
}

void CollatzCalculator::startCalcPerThread( std::shared_ptr<tResult> resPtr, tNumType startIdx, tNumType endIdx, tNumType size, tNode*& max, std::vector<tNumType>& errors)
{
    tResult& res = *resPtr;
    for (tNumType j = startIdx; j < endIdx; j++)
    {
        if(!isRunning())
        {
            return;
        }
        tNumType chainLen = calculateChain(res.data, size, j+1);
        res.data[j].value.store(j + 1);
        res.data[j].chainLen.store(chainLen);
        if (max->chainLen.load() < res.data[j].chainLen.load())
        {
            max = &res.data[j];
        }
        if(chainLen == 0)
        {
            errors.push_back(j+1);
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
                res = 0;
                break;
            }
            n = 3 * n + 1;
        }
        res++;
    }
    return res;
}

