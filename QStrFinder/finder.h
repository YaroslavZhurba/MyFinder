#ifndef FINDER_H
#define FINDER_H

#include <QObject>
#include <QDir>
#include <QDebug>
#include <iostream>
#include <string>
#include <QFileInfo>
#include <vector>
#include <QDirIterator>
#include <unordered_set>
#include <QFileSystemWatcher>

class Finder : public QObject
{
    Q_OBJECT


    int progressBar;
    bool isCanceled;
    bool isIndexed;
    QDir dir,olddir;
    QString sample;
    int countFiles;
    std::vector<QFileInfo> files;
    std::vector<std::pair<std::unordered_set<uint32_t>,QFileInfo>> indexFiles;
    std::vector<std::pair<QFileInfo,std::vector<long long>>> result;

    void cntFiles();
    void setAll();
    void setIndex();
    void toIndexFiles();
    bool check_triples(int i);
    bool findEnd(QString& first, QString& second, long long pos, bool needIncrease, QFileInfo& fileInfo);
    bool findPiece(QString& first, QString& second, long long pos, bool needIncrease, QFileInfo& fileInfo);
    void findString();
    void safe_upd_progress(int k, int n);


public:
    explicit Finder(QObject *parent = nullptr);
    void run(QString const& sample);
    void runIndex(QDir const& dir);
    void setIndex(bool status);
    bool isIndex();

signals:
    void except(const QString &message);
    void info_message(const QString& message);
    void progress_upd(int value);
    void return_results(const std::vector<std::pair<QFileInfo,std::vector<long long>>>& result);
    void status_bar_upd(const QString &message);

public slots:

    void cancel();

};

#endif // FINDER_H
