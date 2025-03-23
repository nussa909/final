#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "common.h"
#include <QThread>
#include "collatzcalculator.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    int getThreadNumber() const;
    tNumType getInputValue() const;

signals:

    void startCalled(tNumType value, int threadNumber);
    void stopCalled();

public slots:
    void calculationSuccess(std::shared_ptr<tResult> res);
    void calculationFailed(QString msg);

private slots:
    void startPressed();
    void stopPressed();

private:
    void setupButtons(bool isRunning);
    void setupSlider();

    QThread mCCThread;
    Ui::MainWindow* mUiPtr;
    CollatzCalculator mCalcCol;
};
#endif // MAINWINDOW_H
