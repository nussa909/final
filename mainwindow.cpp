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

    setWindowTitle("Collatz Conjecture calculation");
    setupButtons(true);
    setupSlider();

    connect(mUiPtr->buttonStart, &QPushButton::clicked, this, &MainWindow::startPressed);
    connect(mUiPtr->buttonStop,  &QPushButton::clicked, this, &MainWindow::stopPressed);

    connect(this, &MainWindow::startCalled, &mCalcCol, &CollatzCalculator::start );

    connect(&mCalcCol, &CollatzCalculator::finished, &mCCThread, &QThread::quit);

    connect(&mCalcCol, &CollatzCalculator::calculationFinished, this, &MainWindow::calculationFinished );

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

    mUiPtr->textEdit->clear();

    mCCThread.start();
    emit startCalled( getInputValue(), getThreadNumber());
}

void MainWindow::stopPressed()
{
    qDebug() << "StopPressed\n";
    setupButtons(true);
    mCalcCol.stop();
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

void MainWindow::calculationFinished(std::shared_ptr<tResult> res)
{
    qDebug() << "MainWindow::calculationFinished";
    if(res)
    {
        std::stringstream ss;
        ss << "Value " << res->max->value.load() << " has the longest chain: " << res->max->chainLen.load()<< ".\nExecution time:" << res->timeDuration<<"ms";
        mUiPtr->textEdit->setText(QString(ss.str().data()));
        setupButtons(true);
    }
}

