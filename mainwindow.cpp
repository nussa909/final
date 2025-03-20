#include "mainwindow.h"
#include <QTextDocumentFragment>
#include "textoutput.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , mTOThread()
    , mCCThread()
    , mUiPtr(new Ui::MainWindow)
    , mTextOut()
    , mCalcCol()
{
    mUiPtr->setupUi(this);

    setupButtons(true);
    setupSlider();

    connect(mUiPtr->buttonStart, &QPushButton::clicked, this, &MainWindow::startPressed);
    connect(mUiPtr->buttonStop,  &QPushButton::clicked, this, &MainWindow::stopPressed);

    connect(this, &MainWindow::startCalled, &mCalcCol, &CollatzCalculator::start );
    connect(this, &MainWindow::startCalled, &mTextOut, &TextEditUpdate::start );

    connect(this, &MainWindow::stopCalled, &mCalcCol, &CollatzCalculator::stop );
    connect(this, &MainWindow::stopCalled, &mTextOut, &TextEditUpdate::stop );

    connect(&mTextOut, &TextEditUpdate::finished, &mTOThread, &QThread::quit);
    connect(&mCalcCol, &CollatzCalculator::finished, &mCCThread, &QThread::quit);

    connect(&mTextOut, &TextEditUpdate::finished, this, &MainWindow::calculationFinished);
    connect(&mCalcCol, &CollatzCalculator::finished, this, &MainWindow::calculationFinished);

    connect(&mCalcCol, &CollatzCalculator::calculationStarted, &mTextOut, &TextEditUpdate::calculationStarted );
    connect(&mCalcCol, &CollatzCalculator::calculationFinished, &mTextOut, &TextEditUpdate::calculationFinished );

    connect(&mTextOut, &TextEditUpdate::textEditFragmentUpdate, this, &MainWindow::textEditUpdated );
    connect(&mTextOut, &TextEditUpdate::textLabelUpdate, this, &MainWindow::labelResultUpdated );
    connect(this, &MainWindow::requestNextFragment, &mTextOut, &TextEditUpdate::processNextFragment);

    mTextOut.moveToThread(&mTOThread);
    mCalcCol.moveToThread(&mCCThread);
}

MainWindow::~MainWindow()
{
    mTOThread.quit();
    mCCThread.quit();
    delete mUiPtr;
}

int MainWindow::getThreadNumber() const
{
    return  mUiPtr->horizontalSlider->sliderPosition();
}

tNumType MainWindow::getInputValue() const
{
    return mUiPtr->spinBox->value();
}

void MainWindow::startPressed()
{
    qDebug() << "StartPressed\n";
    setupButtons(false);

    mUiPtr->textEdit->clear();
    mUiPtr->labelResult->clear();

    mCCThread.start();
    mTOThread.start();
    emit startCalled( getInputValue(), getThreadNumber());
}

void MainWindow::stopPressed()
{
    qDebug() << "StopPressed\n";
    setupButtons(true);
    emit stopCalled();
}

void MainWindow::setupButtons(bool isRunning)
{
    mUiPtr->buttonStart->setEnabled(isRunning);
    mUiPtr->buttonStart->setChecked(isRunning);
    mUiPtr->buttonStop->setEnabled(!isRunning);
    mUiPtr->buttonStop->setChecked(!isRunning);
}

void MainWindow::setupSlider()
{
    mUiPtr->horizontalSlider->setMinimum(1);
    mUiPtr->horizontalSlider->setMaximum(std::thread::hardware_concurrency());
    mUiPtr->horizontalSlider->setTickInterval(1);
    mUiPtr->horizontalSlider->setSliderPosition(1);
}

void MainWindow::calculationFinished()
{
    qDebug() << "MainWindow::calculationFinished";
    if(!mCalcCol.isRunning() && !mTextOut.isRunning())
    {
        qDebug() << "MainWindow::calculationFinished: both not running";
        setupButtons(true);
    }
}

void MainWindow::textEditUpdated(QTextDocument* textDoc)
{
    qDebug() << "MainWindow::textEditUpdated";

    if(textDoc)
    {
        QTextCursor cursor(mUiPtr->textEdit->textCursor());
        QTextDocumentFragment fragment(textDoc);
        cursor.insertFragment(fragment);
        cursor.movePosition(QTextCursor::End);
        emit requestNextFragment();
    }
}

void MainWindow::labelResultUpdated(QString text)
{
    mUiPtr->labelResult->setText(text);
}
