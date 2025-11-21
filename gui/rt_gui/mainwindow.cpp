#include "mainwindow.h"
#include "ui_MainWindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <unordered_map>

#include <QtCharts/QChart>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>

#include "../src/parser.hpp"
#include "../src/factory.hpp"
#include "../src/models.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Algorithm list
    ui->comboAlg->addItems({
        "EDF", "RMS", "DMS", "LLF",
        "BACKGROUND", "POLLING", "DEFERRABLE", "SPORADIC"
    });

    // Connect UI signals
    connect(ui->btnBrowse, &QPushButton::clicked,
            this, &MainWindow::onBrowseFile);
    connect(ui->btnRun, &QPushButton::clicked,
            this, &MainWindow::onRunSimulation);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onBrowseFile()
{
    QString file = QFileDialog::getOpenFileName(this, "Select input file");
    if (!file.isEmpty()) {
        ui->linePath->setText(file);
    }
}

void MainWindow::onRunSimulation()
{
    QString qpath = ui->linePath->text();
    if (qpath.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select an input file.");
        return;
    }

    std::string path = qpath.toStdString();

    try {
        // Parse tasks
        auto [tasks, aperiodic, serverCfg] = parseInputFile(path);

        if (tasks.empty()) {
            QMessageBox::warning(this, "Error", "No periodic tasks found!");
            return;
        }

        int hp = hyperperiod(tasks);

        // Simulation time
        int sim = ui->lineSim->text().isEmpty()
                      ? hp
                      : ui->lineSim->text().toInt();

        // Algorithm selection
        std::string alg = ui->comboAlg->currentText().toStdString();

        // Scheduler
        auto scheduler = buildScheduler(alg, tasks, aperiodic, serverCfg, sim);

        // Execute
        scheduler->run();

        // Textual summary
        ui->output->setPlainText(
            QString::fromStdString(scheduler->summaryText())
            );

        // Draw chart
        drawTimelineChart(scheduler->timeline());
    }
    catch (const std::exception &e) {
        QMessageBox::critical(this, "Runtime Error", e.what());
    }
}

void MainWindow::drawTimelineChart(const std::vector<std::string>& timeline)
{
    if (timeline.empty()) return;

    // Step 1: Unique labels for rows
    std::vector<std::string> labelsOrdered;
    std::unordered_map<std::string, int> labelToRow;

    for (const auto &label : timeline) {
        if (labelToRow.find(label) == labelToRow.end()) {
            int idx = static_cast<int>(labelsOrdered.size());
            labelsOrdered.push_back(label);
            labelToRow[label] = idx;
        }
    }

    // Step 2: Create chart
    auto *chart = new QChart();
    chart->setTitle("Schedule Timeline");

    // Each task = one scatter series
    for (const auto &label : labelsOrdered) {
        int row = labelToRow[label];

        auto *series = new QScatterSeries();
        series->setName(QString::fromStdString(label));
        series->setMarkerSize(8.0);

        for (int t = 0; t < static_cast<int>(timeline.size()); t++) {
            if (timeline[t] == label) {
                series->append(t, row);
            }
        }

        chart->addSeries(series);
    }

    // Step 3: X axis = time
    auto *axisX = new QValueAxis();
    axisX->setTitleText("Time");
    axisX->setLabelFormat("%d");
    axisX->setTickInterval(1);
    axisX->setRange(0, timeline.size());
    chart->addAxis(axisX, Qt::AlignBottom);

    for (auto *s : chart->series())
        s->attachAxis(axisX);

    // Step 4: Y axis = task names
    auto *axisY = new QCategoryAxis();
    axisY->setTitleText("Task");

    for (auto &label : labelsOrdered) {
        axisY->append(QString::fromStdString(label), labelToRow[label]);
    }

    axisY->setRange(-0.5, labelsOrdered.size() - 0.5);
    chart->addAxis(axisY, Qt::AlignLeft);

    for (auto *s : chart->series())
        s->attachAxis(axisY);

    // Step 5: Clear old chartView
    if (chartView_) {
        delete chartView_;
        chartView_ = nullptr;
    }

    // Step 6: Insert chart into GUI widget
    chartView_ = new QChartView(chart, ui->chartWidget);
    chartView_->setRenderHint(QPainter::Antialiasing);

    QLayout *oldLayout = ui->chartWidget->layout();
    if (!oldLayout) {
        auto *layout = new QVBoxLayout(ui->chartWidget);
        layout->addWidget(chartView_);
    } else {
        auto *vlayout = qobject_cast<QVBoxLayout*>(oldLayout);
        if (!vlayout) {
            delete oldLayout;
            vlayout = new QVBoxLayout(ui->chartWidget);
        } else {
            QLayoutItem *item;
            while ((item = vlayout->takeAt(0)) != nullptr) {
                delete item->widget();
                delete item;
            }
        }
        vlayout->addWidget(chartView_);
    }
}
