#include <QObject>
#include <QTest>
#include <QFileInfo>
#include "indexer.h"
// WARNING WHYYYY ????
#include "indexer.cpp"

class sDsidTests : public QObject
{
    Q_OBJECT
private:

private slots:
    // some indexer tests
    void similarIndexerTest();
    void utf8ValidationTest();
    void utf8InvalidationTest();
    void biggerTgmAmountTest();
};

void sDsidTests::similarIndexerTest() {
    QFile file("little_test.tst");
    file.open(QFile::WriteOnly);
    file.write("Hello, my little file for a little test");
    file.close();
    Indexer little_indexer(file);
    indexer_tools::process(little_indexer);
    file.remove();
    QVERIFY(little_indexer.is_text());
}

void sDsidTests::utf8ValidationTest() {
    QFile good("good_utf8.tst");
    good.open(QFile::WriteOnly);
    good.write("\xe4\xbd\xa0\xe5\xa5\xbd\xe4\xb8\x96\xe7\x95\x8c");
    good.close();
    Indexer little_indexer(good);
    indexer_tools::process(little_indexer);
    good.remove();
    QVERIFY(little_indexer.is_text());
}

void sDsidTests::utf8InvalidationTest() {
    QFile bad("bad_utf8.tst");
    bad.open(QFile::WriteOnly);
    bad.write("\xa0\xa1\xa0\xa1\xa0\xa1");
    bad.close();
    Indexer little_indexer(bad);
    indexer_tools::process(little_indexer);
    bad.remove();
    QVERIFY(!little_indexer.is_text());
}

void sDsidTests::biggerTgmAmountTest() {
    QFile big("bigtest.tst");
    if (!big.exists())
        QSKIP("This test requires big file \"bigtest.tst\" with many trigrams");
    Indexer big_ind(big);
    indexer_tools::process(big_ind);
    QVERIFY(!big_ind.is_text());
}





QTEST_MAIN(sDsidTests)
#include "tests.moc"
