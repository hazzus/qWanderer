#ifndef INDEXER_H
#define INDEXER_H

#include <QFileInfo>
#include <QDebug>
#include <fstream>
#include <set>
#include <unordered_set>

static const size_t MAX_TGM_AMOUNT = 20000;
static const size_t MAX_CHARS_IN_LINE = 2000;
static const qint64 MAX_FILE_SIZE = 1073741824; // 1 << 30 == 1Gb

struct Indexer;

namespace indexer_tools {
    void process(Indexer&);
    bool is_fully_valid_utf8(const uint8_t* to_check, size_t len);
}

struct Indexer {

    Indexer();
    Indexer(QFileInfo fileinfo);

    bool may_contain(QString const&) const;

    QString filepath() const;

    bool is_text() const;

    friend void indexer_tools::process(Indexer &);

private:
    QFileInfo fileinfo;
    bool preindexed;
    bool ok;
    std::vector<uint32_t> trigrams;
};


#endif // INDEXER_H
