#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <sstream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , mCCThread()
    , mUiPtr(new Ui::MainWindow)
    , mCalcCol()
{
    mUiPtr->setupUi(this);

    setWindowTitle("Collatz conjecture calculation");
    setupButtons(true);
    setupSlider();

    connect(mUiPtr->buttonStart, &QPushButton::clicked, this, &MainWindow::startPressed);
    connect(mUiPtr->buttonStop,  &QPushButton::clicked, this, &MainWindow::stopPressed);

    connect(this, &MainWindow::startCalled, &mCalcCol, &CollatzCalculator::start );

    connect(&mCalcCol, &CollatzCalculator::finished, &mCCThread, &QThread::quit);

    connect(&mCalcCol, &CollatzCalculator::calculationSuccess, this, &MainWindow::calculationSuccess );
    connect(&mCalcCol, &CollatzCalculator::calculationFailed, this, &MainWindow::calculationFailed );

    mCalcCol.moveToThread(&mCCThread);
}

MainWindow::~MainWindow()
{
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

    mCCThread.start();
    emit startCalled( getInputValue(), getThreadNumber());

    mUiPtr->textEdit->clear();
    std::stringstream ss;
    ss << "Start calculation (number:" << getInputValue() << ", threads:" << getThreadNumber()<<")...";
    mUiPtr->textEdit->append(ss.str().c_str());
}

void MainWindow::stopPressed()
{
    qDebug() << "StopPressed\n";
    setupButtons(true);
    mCalcCol.stop();
    mUiPtr->textEdit->append("Calculation stopped");
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

void MainWindow::calculationSuccess(std::shared_ptr<tResult> res)
{
    qDebug() << "MainWindow::calculationSuccess";
    if(res)
    {
        std::stringstream ss;
        for(auto& error : res->errorList)
        {
            ss<< "\t*Number " << error << "skipped \n";
        }

        ss << "Calculation finished: success \nResult: Number " << res->max->value.load() << " has the longest sequence: " << res->max->chainLen.load()<< ".\nExecution time:" << res->timeDuration<<"ms";
        mUiPtr->textEdit->append(QString(ss.str().data()));
        setupButtons(true);
    }
}

void MainWindow::calculationFailed(QString msg)
{
    qDebug() << "MainWindow::calculationFailed(" << msg << ")";
    QString msgStr("Calculation finished: failed\nError: ");
    msgStr.append(msg);
    mUiPtr->textEdit->append(msgStr);
    setupButtons(true);
}

