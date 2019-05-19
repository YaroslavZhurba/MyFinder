#include "finder.h"

void Finder::cntFiles()
{
    QDirIterator it(dir.path(), QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext() && !isCanceled) {
        it.next();
        QString rel_path = dir.relativeFilePath(it.fileInfo().path()) + "/";
        if (rel_path == "./") {
            rel_path = "";
        }
        files.push_back(it.fileInfo());
        countFiles++;
    }
}

void Finder::setAll()
{
    isCanceled = false;
    files.clear();
    result.clear();
    progressBar = 0;
}

void Finder::setIndex()
{
    isCanceled = false;
    countFiles = 0;
    files.clear();
    indexFiles.clear();
    progressBar = 0;
}



void Finder::toIndexFiles()
{
    for (int j = 0; j< files.size() && !isCanceled; j++) {
        std::unordered_set<uint32_t> uset;
        safe_upd_progress(j,files.size()-1);
        QString abs_path = files[j].absoluteFilePath();
        QFile file(abs_path);
        if(file.open(QFile::ReadOnly)) {
            QByteArray blob;
            char last;
            char prelast;
            uint32_t int_triple;
            bool start = false;
            long long  count = 0;

            while (!file.atEnd() && !isCanceled) {
                blob = file.read(32 * 1024);
                for (int i = 0; i < blob.size() - 2 && !isCanceled; i++) {
                    int_triple = blob[i+2] * 256 * 256 + blob[i+1] * 256 + blob[i];
                    if (uset.find(int_triple) == uset.end()) {
                        count++;
                        uset.insert(int_triple);
                    }
                }
                if (start && !isCanceled) {
                    if (blob.size() >= 1) {
                        int_triple = blob[0] * 256 * 256 + last * 256 + prelast;
                        if (uset.find(int_triple) == uset.end()) {
                            count++;
                            uset.insert(int_triple);
                        }
                    }
                    if (blob.size() >= 2) {
                        int_triple = blob[1] * 256 * 256 + blob[0] * 256 + last;
                        if (uset.find(int_triple) == uset.end()) {
                            count++;
                            uset.insert(int_triple);
                        }
                    }
                }
                if (blob.size() >= 2) {
                    last = blob[blob.size() - 1];
                    prelast = blob[blob.size() - 2];
                }
                start = true;
                if (count > 20000) {
                    count = -1;
                    break;
                }
            }




            if (count >= 0 && !isCanceled) {
                QString strFile;
                uset.clear();
                bool start = false;
                file.close();
                file.open(QFile::ReadOnly);
                while(!file.atEnd() && !isCanceled)  {
                    strFile = file.read(32*1024);
                    for (int i = 0; i < strFile.size() - 2 && !isCanceled; i++) {
                        int_triple = strFile[i+2].toLatin1() * 256 * 256 + strFile[i+1].toLatin1() * 256 + strFile[i].toLatin1();
                        if (uset.find(int_triple) == uset.end()) {
                            uset.insert(int_triple);
                        }
                    }
                    if (start && !isCanceled) {
                        if (strFile.size() >= 1) {
                            int_triple = strFile[0].toLatin1() * 256 * 256 + last * 256 + prelast;
                            if (uset.find(int_triple) == uset.end()) {
                                uset.insert(int_triple);
                            }
                        }
                        if (strFile.size() >= 2) {
                            int_triple = strFile[1].toLatin1() * 256 * 256 + strFile[0].toLatin1() * 256 + last;
                            if (uset.find(int_triple) == uset.end()) {
                                uset.insert(int_triple);
                            }
                        }
                    }
                    if (strFile.size() >= 2) {
                        last = strFile[strFile.size() - 1].toLatin1();
                        prelast = strFile[strFile.size() - 2].toLatin1();
                    }
                    start = true;
                }
                std::pair<std::unordered_set<uint32_t>,QFileInfo> pair;
                pair.first = uset;
                pair.second = files[j];
                indexFiles.push_back(pair);
            }
        } else {
            //cant open file
            emit except("Can't open file " + files[j].absoluteFilePath());
        }
        file.close();
    }

}

bool Finder::check_triples(int i)
{
    if (sample.size() <= 2) {
        return true;
    }
    std::unordered_set<uint32_t> uset = indexFiles[i].first;
    for (int j = 0; j < sample.size() - 2; j++) {
        char a =sample[j+2].toLatin1();
        char b =sample[j+1].toLatin1();
        char c =sample[j].toLatin1();
        uint32_t ind = a*256*256 + b*256 + c;
        if (uset.find(ind) == uset.end()) {
            return false;
        }
    }
    return true;
}

bool Finder::findEnd(QString& first, QString& second, long long pos, bool needIncrease, QFileInfo& fileInfo)
{
    QString str = first + second;
    long long limit = str.size() - sample.size();
    for (int i = 0; i < limit; i++) {
        bool flag = true;
        for (int j = 0; j < sample.size(); j++) {
            if (sample[j] != str[j+i]) {
                flag = false;
                break;
            }
        }
        if (flag) {
            if (!needIncrease) {
                needIncrease = true;
                std::pair<QFileInfo,std::vector<long long>> elem;
                elem.first = fileInfo;
                result.push_back(elem);
            }
            result[result.size()-1].second.push_back(pos+i);
        }
    }

    return needIncrease;
}

bool Finder::findPiece(QString& first, QString& second, long long pos, bool needIncrease, QFileInfo& fileInfo)
{
    QString str = first + second;
    long long limit = 32*1024;
    for (int i = 0; i < limit; i++) {
        bool flag = true;
        for (int j = 0; j < sample.size(); j++) {
            if (sample[j] != str[j+i]) {
                flag = false;
                break;
            }
        }
        if (flag) {
            if (!needIncrease) {
                needIncrease = true;
                std::pair<QFileInfo,std::vector<long long>> elem;
                elem.first = fileInfo;
                result.push_back(elem);
            }
            result[result.size()-1].second.push_back(pos+i);
        }
    }
    return needIncrease;
}


void Finder::findString()
{
    bool needIncrease;
    int j = 0;
    for (int i = 0; i < indexFiles.size() && !isCanceled; i++) {
        safe_upd_progress(i,indexFiles.size()-1);
//        std::cout << i << std::endl;
        if(check_triples(i)) {
            needIncrease = false;
            QFile file(indexFiles[i].second.absoluteFilePath());
            file.open(QFile::ReadOnly);
            QString first;
            QString second = "";
            long long pos = 0;
            first = file.read(32*1024);
            if (file.atEnd()) {
                if(findEnd(second,first,pos,needIncrease,indexFiles[i].second)) {
                    needIncrease = true;
                }
            } else {
                while(!file.atEnd() && !isCanceled)  {
                    second = file.read(32*1024);
                    if (file.atEnd()) {
                        if (findEnd(first,second,pos,needIncrease,indexFiles[i].second)) {
                            needIncrease = true;
                        }
                    } else {
                        if (findPiece(first,second,pos,needIncrease,indexFiles[i].second)) {
                            needIncrease = true;
                        }
                        pos+=32*1024;
                        first = second;
                    }

                }
            }
            file.close();
            if (needIncrease) {
                j++;
            }
        }
    }
}

void Finder::safe_upd_progress(int k, int n)
{
    int progress = int((k / (double)n) * 100);
    if (progress > progressBar) {
        progressBar = progress;
        emit progress_upd(progress);
    }
}





Finder::Finder(QObject *parent) : QObject(parent), isIndexed(false)
{

}

void Finder::runIndex(const QDir &dir)
{
    this->dir = dir;
    // REMOVE
    setIndex();
    emit status_bar_upd("Counting Files");
    cntFiles();
    //std::cout << countFiles << std::endl;

    emit info_message("There're " + QString::number(countFiles) + " files");
    if (!isCanceled) {
        emit status_bar_upd("Indexing Files");
    }
    toIndexFiles();
    emit info_message("Index finished");
    if (!isCanceled) {
        isIndexed = true;
    }
}

void Finder::setIndex(bool status)
{
    isIndexed = status;
}

bool Finder::isIndex()
{
    return isIndexed;
}

void Finder::run(const QString &sample)
{
    if (!isIndexed) {
        emit except("Need indexing");
        return;
    }
    this->sample = sample;
    setAll();

    if (!isCanceled) {
        emit status_bar_upd("Searching...");
    }
    progressBar = -1;
    safe_upd_progress(0,100);
    findString();
    if (!isCanceled) {
        emit status_bar_upd("Finished");
        emit return_results(result);
    }

}

void Finder::cancel()
{
    isCanceled = true;
}
