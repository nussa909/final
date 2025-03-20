#include "textoutput.h"
#include "qapplication.h"
#include <QTextTable>
#include <sstream>

FragmentProccessor::FragmentProccessor(std::shared_ptr<tResult>& p, const tNumType& pg, const tNumType& cur)
    : ptr(p)
{
    page = pg;
    curIdx = cur;

    if(ptr)
    {
        size = ptr->data.size();

        startIdx = curIdx * page;
        endIdx = std::min(curIdx * page + page, size);
    }
}

QTextDocument* FragmentProccessor::createFragment()
{
    qDebug() << "TextOutput::processNextFragment(" << startIdx << "," << endIdx << ")";
    QTextDocument* textDoc = new QTextDocument();

    QTextCursor cursor (textDoc);
    cursor.movePosition(QTextCursor::Start);

    const tNumType rowCount = endIdx - startIdx;
    const tNumType colCount = 2;
    QTextTableFormat tf;
    QTextTable* table = cursor.insertTable(rowCount,colCount,tf);
    for(tNumType i= 0; i< rowCount; i++)
    {
        QString valueStr = QString::number(ptr->data[i+startIdx].value);
        QString chainLenStr = (ptr->data[i+startIdx].isFailed.load() == false) ? QString::number(ptr->data[i+startIdx].chainLen) : "failed";
        table->cellAt(i,0 ).firstCursorPosition().insertText(valueStr);
        table->cellAt(i,1).firstCursorPosition().insertText(chainLenStr);
    }
    return textDoc;
}
void FragmentProccessor::nextFragment()
{
    startIdx = curIdx * page;
    endIdx = std::min(curIdx * page + page, size);
    curIdx++;
}
bool FragmentProccessor::isFinished() const {
    return curIdx > size / page;
}

TextEditUpdate::TextEditUpdate()
    : QObject()
{}

void TextEditUpdate::calculationStarted(std::shared_ptr<tResult> res)
{
    qDebug() << "TextEditUpdate::calculationStarted";
    mRes = res;
}

void TextEditUpdate::calculationFinished()
{
    qDebug() << "TextEditUpdate::calculationFinished";
    if(!mRes)
    {
        setRunningState(false);
        emit finished();
        return;
    }
    std::stringstream ss;
    ss << "Value " << mRes->max->value.load() << " has the longest chain: " << mRes->max->chainLen.load()<< ".\nExecution time:" << mRes->timeDuration<<"ms";
    emit textLabelUpdate(QString(ss.str().data()));

    mFragmentProcPtr = std::make_unique<FragmentProccessor>(mRes,100,0);
    processNextFragment();
}

void TextEditUpdate::processNextFragment()
{
    if(!mFragmentProcPtr || mFragmentProcPtr->isFinished() || !isRunning())
    {
        qDebug() << "TextOutput::processNextFragment finished";
        mRes.reset();
        mFragmentProcPtr.reset();
        setRunningState(false);
        emit finished();
        return;
    }

    QTextDocument* textDoc = mFragmentProcPtr->createFragment();
    mFragmentProcPtr->nextFragment();
    textDoc->moveToThread(QApplication::instance()->thread());
    emit textEditFragmentUpdate(textDoc);
}

void TextEditUpdate::start(tNumType value, int threadNumber)
{
    qDebug() << "TextOutput::start";
    setRunningState(true);
}

void TextEditUpdate::stop()
{
    qDebug() << "TextOutput::stop";
    setRunningState(false);
}

bool TextEditUpdate::isRunning()
{
    std::lock_guard<std::mutex> lock(mMux);
    return mIsRunning;
}

void TextEditUpdate::setRunningState(bool isRunning)
{
    std::lock_guard<std::mutex> lock(mMux);
    mIsRunning = isRunning;
}
