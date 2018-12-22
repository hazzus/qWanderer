#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    text_files_watcher(new QFutureWatcher<void>(this)),
    text_files(),
    search(new QFutureWatcher<void>(this))
{
    ui->setupUi(this);
    ui->groupBox->hide();
    ui->progressBar->reset();
    connect(this, SIGNAL(add_file(QString const&, QString const&)), this, SLOT(on_add_file(QString const&, QString const&)));
    connect(text_files_watcher, SIGNAL(finished()), this, SLOT(on_index_finished()));
    connect(text_files_watcher, SIGNAL(canceled()), this, SLOT(on_index_finished()));
    connect(text_files_watcher, SIGNAL(progressRangeChanged(int, int)), this->ui->progressBar, SLOT(setRange(int, int)));
    connect(text_files_watcher, SIGNAL(progressValueChanged(int)), this->ui->progressBar, SLOT(setValue(int)));
    connect(this, SIGNAL(add_info_line(QString const&)), this->ui->textEdit, SLOT(insertHtml(QString const&)));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete text_files_watcher;
    delete search;
}

void MainWindow::on_selectButton_clicked()
{
    QString directory = QFileDialog::getExistingDirectory(this);
    ui->pathToDirectory->setText(directory);
}

void MainWindow::on_index_finished() {
    QString detailed = "Time elapsed: %1 seconds, Files in folder: %2, Files cached: %3, Index clipping: %4 %";
    int first_amount = text_files.size();
    QtConcurrent::blockingFilter(text_files, [](Indexer const& ind){return ind.is_text(); });
    ui->subscribeButton->setEnabled(true);
    ui->pathToDirectory->setEnabled(true);
    ui->selectButton->setEnabled(true);
    ui->cancelButton->setDisabled(true);
    ui->details->setText(detailed.arg(timer.elapsed() / 1000.0)
                                 .arg(first_amount)
                                 .arg(text_files.size())
                                 .arg(static_cast<int>((1 - text_files.size() / static_cast<double>(first_amount)) * 100) / 100.0));
}

void MainWindow::on_add_file(QString const& path, QString const& pattern) {
    if (pattern == ui->strToSearch->text())
        ui->listWidget->addItem(path);
}

// this method runs in other thread
void MainWindow::subscribe(QString dir) {
    timer.start();
    QDirIterator it(dir, QDir::Files | QDir::Hidden, QDirIterator::Subdirectories);
    text_files.clear();
    while (it.hasNext()) {
        it.next();
        text_files.push_back(Indexer(it.fileInfo()));
    }
    qDebug() << "Vector size " << text_files.size();
    text_files_watcher->setFuture(QtConcurrent::map(text_files, indexer_tools::process));
}

void MainWindow::on_subscribeButton_clicked()
{
    ui->groupBox->hide();
    if (QDir(ui->pathToDirectory->text()).exists()) {
        // ui preparations
        ui->subscribeButton->setDisabled(true);
        ui->pathToDirectory->setDisabled(true);
        ui->selectButton->setDisabled(true);
        ui->cancelButton->setEnabled(true);
        ui->strToSearch->clear();
        ui->listWidget->clear();
        ui->details->setText("Indexing...");
        QtConcurrent::run(this, &MainWindow::subscribe, ui->pathToDirectory->text());
    } else {
        QMessageBox box;
        box.setText(QString("No such directory %1").arg(ui->pathToDirectory->text()));
        box.exec();
    }
}

void MainWindow::search_substr(QString pattern, QDir origin) {
    // TODO parallel_search?
    timer.start();
    for (Indexer const& ind : text_files) {
        if (_search_abort) break;
        if (ind.may_contain(pattern)) {
            QFile in(ind.filepath());
            if (in.open(QFile::ReadOnly)) {
                while (!in.atEnd()) {
                    if (_search_abort) break;
                    QString line = in.read(1048576); //is buffer size 1Mb ok?
                    if (line.indexOf(pattern) >= 0) {
                        emit add_file(origin.relativeFilePath(ind.filepath()), pattern);
                        break;
                    }
                }
            } else {
                qDebug() << "Something happened with " << ind.filepath() << " - " << in.errorString();
            }
        }
    }
    if (!_search_abort)
        qDebug() << "Search finished on " << timer.elapsed() / 1000.0 << " seconds";
}

void MainWindow::on_strToSearch_textEdited(const QString &arg1)
{
    // check if subscription is running or search is going on
    if (text_files_watcher->isRunning()) return;
    ui->listWidget->clear();
    if (arg1.length() < 3) return;

    if (search->isRunning()) {
        _search_abort = true;
        search->waitForFinished();
    }
    ui->groupBox->hide();

    _search_abort = false;
    search->setFuture(QtConcurrent::run(this, &MainWindow::search_substr,
                                        arg1, QDir(ui->pathToDirectory->text())));
}


void MainWindow::on_cancelButton_clicked()
{
    if (!text_files_watcher->isCanceled())
        text_files_watcher->cancel();
    //text_files_watcher->waitForFinished();
}

void MainWindow::show_big_file(QString filename, QString pattern) {
    int linecount = 0;
    QFile file(filename);
    if (file.open(QFile::ReadOnly)) {
        while (!file.atEnd()) {
            QString text = file.readLine();
            linecount++;
            int count = 0;
            int from = -1;
            while (true) {
                from = text.indexOf(pattern, from + 1);
                if (from < 0)
                    break;
                count++;
            }
            if (count > 0)
                emit add_info_line(QString("<div style=\"font-size: 14px;\"> • %1 match(es) on line %2 </div><br>").arg(count).arg(linecount));
        }
        emit add_info_line("</ul>");
    } else {
        qDebug() << "Bad file " << filename << " - " << file.errorString();
    }
}

void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    if (search->isRunning()) return;
    ui->textEdit->clear();
    ui->groupBox->setTitle("..." + item->text().right(item->text().lastIndexOf('/')).right(30));
    ui->groupBox->show();
    QDir origin(ui->pathToDirectory->text());

    QString filename = origin.absoluteFilePath(item->text());
    QFile file(filename);

    if (file.size() > MAX_FILE_OPEN_SIZE) {
        ui->textEdit->insertHtml("<h2>File is too big to be displayed here</h2>"
                                 "<h3>But here is statistic:<h3>");
        QtConcurrent::run(this, &MainWindow::show_big_file, filename, ui->strToSearch->text());
        return;
    }

    if (file.open(QFile::ReadOnly)) {
       ui->textEdit->setPlainText(file.readAll());
    } else {
        qDebug() << file.errorString();
        ui->textEdit->append(file.errorString());
        return;
    }

    QTextCursor cursor(ui->textEdit->document());
    QTextCharFormat fmt;
    fmt.setBackground(Qt::green);

    // BUG colors all text
    QString pat = ui->strToSearch->text();
    QString text = ui->textEdit->toPlainText();
    int from = -1;
    while (true) {
        from = text.indexOf(pat, from + 1);
        if (from >= 0) {
            cursor.setPosition(from, QTextCursor::MoveAnchor);
            cursor.setPosition(from + pat.length(), QTextCursor::KeepAnchor);
            cursor.setCharFormat(fmt);
        } else {
            break;
        }
    }
}

void MainWindow::on_hideButton_clicked() {
    ui->groupBox->hide();
}
