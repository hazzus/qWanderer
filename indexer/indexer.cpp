#include "indexer.h"

bool indexer_tools::is_fully_valid_utf8(const uint8_t* to_check, size_t len) {
    size_t n = 0;
    for (size_t i = 0; i < len; i++) {
        if (to_check[i] <= 0x7f) {
            n = 0;
        } else if ((to_check[i] & 0xE0) == 0xC0) {
            n = 1;
        } else if ((to_check[i] == 0xED) && ((to_check[i + 1] & 0xA0) == 0xA0)) {
            //invalid U+D800 - U+DFFF
            return false;
        } else if((to_check[i] & 0xF0) == 0xE0) {
            n = 2;
        } else if ((to_check[i] & 0xF8) == 0xF0) {
            n = 3;
        } else {
            return false;
        }
        for (size_t j = 0; j < n && j < len; j++) {
            if (++i == len || (to_check[i] & 0xC0) != 0x80)
                return false;
        }
    }
    return true;
}

void indexer_tools::process(Indexer& ind) {
    ind.preindexed = true;
    if (ind.fileinfo.size() > MAX_FILE_SIZE) {
        ind.ok = false;
        return;
    }
    std::string filename = ind.fileinfo.filePath().toStdString();
    std::ifstream file(filename);
    if (!file) {
        qDebug() << "Error occurred while opening file " << QString::fromStdString(filename) << "\n";
        ind.ok = false;
        return;
    }
    std::string line;
    std::unordered_set<uint32_t> tgm_set;
    while (!file.eof()) {
        std::getline(file, line);
        size_t linelen = line.length();
        if (linelen > MAX_CHARS_IN_LINE) {
            ind.ok = false;
            return;
        }
        // adding trigramms
        uint8_t symbols[linelen];
        for (size_t i = 0; i < linelen; i++) {
            symbols[i] = static_cast<uint8_t>(line[i]);
        }
        if (!indexer_tools::is_fully_valid_utf8(symbols, linelen)) {
            ind.ok = false;
            return;
        }
        uint32_t trigram = 0 | (symbols[0] << 8) | symbols[1];
        for (size_t i = 2; i < linelen; i++) {
            trigram <<= 8;
            trigram |= symbols[i];
            tgm_set.insert(trigram & 0xFFFFFF);

        }
        if (tgm_set.size() > MAX_TGM_AMOUNT) {
            ind.ok = false;
            return;
        }
    }
    ind.trigrams.clear();
    std::move(tgm_set.begin(), tgm_set.end(), std::back_inserter(ind.trigrams));
    std::sort(ind.trigrams.begin(), ind.trigrams.end());
    ind.ok = true;
}


Indexer::Indexer() : preindexed(false) {}
Indexer::Indexer(QFileInfo fileinfo) : fileinfo(fileinfo), preindexed(false), ok(false), trigrams() {}


QString Indexer::filepath() const {
    return fileinfo.filePath();
}

bool Indexer::is_text() const {
    return preindexed && ok;
}

std::vector<uint32_t> cut_string(std::string const& pattern) {
    // NOTE copypast for efficiency, in order not to do useless utf-8 validity checks and copies
    std::set<uint32_t> result;
    uint8_t symbols[pattern.length()];
    for (size_t i = 0; i < pattern.length(); i++) {
        symbols[i] = static_cast<uint8_t>(pattern[i]);
    }
    uint32_t trigram = 0 | (symbols[0] << 8) | symbols[1];
    for (size_t i = 2; i < pattern.length(); i++) {
        trigram <<= 8;
        trigram |= symbols[i];
        result.insert(trigram & 0xFFFFFF);
    }
    return std::vector<uint32_t>(result.begin(), result.end());
}

bool Indexer::may_contain(const QString & pattern) const {
    if (!preindexed) {
        qDebug() << "File " << fileinfo.filePath() << " not preindexed";
        return false;
    }
    std::vector<uint32_t> pattern_tgms = cut_string(pattern.toStdString());
    // O(NlogM) or O(N + M)

    /* O(N + M)
    std::vector<uint32_t> inter;
    std::set_intersection(trigrams.begin(), trigrams.end(),
                          pattern_tgms.begin(), pattern_tgms.end(),
                          std::back_inserter(inter));
    //return inter == pattern_tgms;
    */

    // O(NlogM)
    for (uint32_t const& i : pattern_tgms) {
        if (!std::binary_search(trigrams.begin(), trigrams.end(), i)) {
            return false;
        }
    }
    return true;
}
