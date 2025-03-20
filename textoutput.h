#ifndef TEXTOUTPUT_H
#define TEXTOUTPUT_H

#include <QObject>
#include <QTextDocument>
#include <mutex>
#include "common.h"

class FragmentProccessor
{
public:
    explicit FragmentProccessor(std::shared_ptr<tResult>& d, const tNumType& page, const tNumType& curIdx);

    void nextFragment();
    QTextDocument* createFragment();
    bool isFinished() const;

private:
    std::shared_ptr<tResult> ptr;

    tNumType startIdx{0};
    tNumType endIdx{0};
    tNumType size{0};
    tNumType page{0};
    tNumType curIdx{0};
};


class TextEditUpdate : public QObject
{
    Q_OBJECT
public:
    explicit TextEditUpdate();

    bool isRunning();

signals:
    void textEditFragmentUpdate(QTextDocument* textDoc);
    void textLabelUpdate(QString text);
    void finished();

public slots:
    void calculationStarted(std::shared_ptr<tResult> res);
    void calculationFinished();
    void processNextFragment();
    void start(tNumType value, int threadNumber);
    void stop();

private:
    void setRunningState(bool isRunning);

    std::shared_ptr<tResult> mRes{nullptr};
    std::unique_ptr<FragmentProccessor> mFragmentProcPtr{nullptr};

    std::mutex mMux;
    bool mIsRunning {false};
};

#endif // TEXTOUTPUT_H
