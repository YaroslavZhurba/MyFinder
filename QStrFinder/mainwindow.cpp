#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QLabel>
#include <QTreeWidget>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    dir(""),
    sample("")
{

    ui->setupUi(this);
    ui->progressBar->setRange(0, 100);
    lay = new QVBoxLayout(this);
    watcher = new QFileSystemWatcher(this);
    clearGUI();
    ui->scrollAreaWidgetContents->setLayout(lay);
    setWindowTitle(QString("Directory Content - %1").arg(""));


    connect(ui->actionScan_Directory, &QAction::triggered, this, &MainWindow::select_directory);
    connect(ui->startButton, SIGNAL(clicked()), this, SLOT(on_start_clicked()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(on_cancel_clicked()));
    connect(ui->indexButton,SIGNAL(clicked(bool)),this,SLOT(on_index_clicked()));

    connect(watcher, SIGNAL(directoryChanged(const QString&)), this, SLOT(changedDir(const QString&)));

    connect(this, SIGNAL(cancel_thread()), &finder, SLOT(cancel()));

    connect(this, SIGNAL(except(const QString&)),this, SLOT(log_error(const QString&)));
    connect(&finder, SIGNAL(except(const QString&)),this, SLOT(log_error(const QString&)));
    connect(&finder, SIGNAL(info_message(const QString&)),this, SLOT(log_info(const QString&)));

    connect(&finder, SIGNAL(progress_upd(int)),this, SLOT(upd_progress_bar(int)));

    qRegisterMetaType<std::vector<std::pair<QFileInfo,std::vector<long long>>>>("std::vector<std::pair<QFileInfo,std::vector<long long>>>");
    connect(&finder, SIGNAL(return_results(const std::vector<std::pair<QFileInfo,std::vector<long long>>>&)),
            this, SLOT(view_results(const std::vector<std::pair<QFileInfo,std::vector<long long>>>&)));
    connect(&finder,SIGNAL(status_bar_upd(QString)),this,SLOT(upd_status_bar(QString)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::select_directory() {
    QString name = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    this->dir = name;
    setWindowTitle(QString("Directory Content - %1").arg(name));
}

void MainWindow::clear_log()
{
    QLayoutItem* item;
    while( (item = lay->itemAt(0)) )
        {
                lay->removeItem( item );
                lay->removeWidget(item->widget());
                delete item->widget();
                delete item;
                lay->update();
    }
}

void MainWindow::clearGUI()
{
    clear_log();
    ui->progressBar->setValue(0);
    ui->treeWidget->clear();
    upd_status_bar("Ready");
}

void MainWindow::on_start_clicked()
{
    emit cancel_thread();
    clearGUI();
    if (!finder.isIndex()) {
        emit except("Need indexing");
        return;
    }
    this->sample = ui->input->text();

//    QDir empty("");
//    if (dir == empty) {
//        log_error("Chose directory");
//    } else
        if (sample == "") {
        log_error("Input is empty");
    } else {
        QFuture<void> future = QtConcurrent::run(&finder, &Finder::run, sample);
    }
}

void MainWindow::log_error(const QString &message) {
    auto label = new QLabel(message);
    label->setStyleSheet("QLabel { color : red; }");
    lay->addWidget(label);
}

void MainWindow::upd_progress_bar(int value)
{
    ui->progressBar->setValue(value);
}

void MainWindow::upd_status_bar(const QString &message)
{
    ui->statusBar->showMessage(message);
}

void MainWindow::view_results(const std::vector<std::pair<QFileInfo, std::vector<long long> > > &result)
{
    ui->treeWidget->clear();

    for (size_t i = 0; i < result.size(); i++) {

        auto *file = new QTreeWidgetItem(ui->treeWidget);
        QString path = result[i].first.absoluteFilePath();
        file->setText(0, path);
        ui->treeWidget->addTopLevelItem(file);
        for (int j = 0; j < result[i].second.size(); j++) {
            auto *item = new QTreeWidgetItem(file);
            item->setText(0, QString::number(result[i].second[j]));

        }
    }
}

void MainWindow::log_info(const QString &message)
{
    auto label = new QLabel(message);
    lay->addWidget(label);
}

void MainWindow::on_cancel_clicked()
{
    emit cancel_thread();
    upd_status_bar("Canceled");
}

void MainWindow::on_index_clicked()
{
    emit cancel_thread();
    watcher->removePaths(watcher->files());
    watcher->addPath(dir.absolutePath());
    QFuture<void> future = QtConcurrent::run(&finder, &Finder::runIndex, dir);
}

void MainWindow::changedDir(const QString& path)
{
    finder.setIndex(false);
}



