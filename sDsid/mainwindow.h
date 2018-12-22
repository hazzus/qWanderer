#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QFileSystemWatcher>
#include <QFuture>
#include <QDirIterator>
#include <QtConcurrent/QtConcurrent>
#include <QMessageBox>
#include <QListWidgetItem>
#include <algorithm>
#include "indexer.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_selectButton_clicked();

    void on_subscribeButton_clicked();

    void on_strToSearch_textEdited(const QString &arg1);

    void on_add_file(QString const&, QString const&);

    void on_index_finished();

    void on_cancelButton_clicked();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_hideButton_clicked();

signals:
    void add_file(QString const&, QString const&);
    void add_info_line(QString const&);

private:
    const qint64 MAX_FILE_OPEN_SIZE = 307200; // 300 Kb

    Ui::MainWindow *ui;

    QFutureWatcher<void>* text_files_watcher;
    QVector<Indexer> text_files;
    QFutureWatcher<void>*  search;
    std::atomic_bool _search_abort;
    QTime timer;

    // private things runned in separate thread
    void subscribe(QString dir);
    void search_substr(QString pattern, QDir origin);
    void show_big_file(QString filename, QString pattern);
};

#endif // MAINWINDOW_H
