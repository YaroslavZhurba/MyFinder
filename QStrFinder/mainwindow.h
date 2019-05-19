#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include "finder.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
signals:
    void except(const QString &message);
    void cancel_thread();
private slots:
    void select_directory();
    void log_error(const QString &message);
    void upd_progress_bar(int value);
    void upd_status_bar(const QString &message);
    void view_results(const std::vector<std::pair<QFileInfo,std::vector<long long>>>& result);
    void log_info(const QString &message);
//    void on_start();
//    void on_cancel();
    void clear_log();
    void clearGUI();
    void on_start_clicked();
    void on_cancel_clicked();
    void on_index_clicked();
    void changedDir(const QString& path);

private:
    Ui::MainWindow *ui;
    Finder finder;
    QDir dir;
    QString sample;
    QVBoxLayout *lay;
    QFileSystemWatcher *watcher;
};

#endif // MAINWINDOW_H
