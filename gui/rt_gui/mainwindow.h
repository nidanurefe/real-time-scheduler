#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts/QChartView>
#include <vector>
#include <string>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onBrowseFile();
    void onRunSimulation();

private:
    Ui::MainWindow *ui;

    // Chart view pointer
    QtCharts::QChartView *chartView_ = nullptr;

    // Chart drawing
    void drawTimelineChart(const std::vector<std::string>& timeline);
};

#endif // MAINWINDOW_H
