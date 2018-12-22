#include <QObject>
#include <QTest>
#include <QFileInfo>
#include "indexer.h"
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
    void imageIndexationTest();
    void deletedFileTest();
    void permissionsFileTest();
    void containStringTest();
    //some gui tests
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
    QFile big("../../bigtest.tst");
    if (!big.exists())
        QSKIP("This test requires big file \"bigtest.tst\" with many trigrams");
    Indexer big_ind(big);
    indexer_tools::process(big_ind);
    QVERIFY(!big_ind.is_text());
}

void sDsidTests::imageIndexationTest() {
    QFile image("../../test.jpg");
    if (!image.exists())
        QSKIP("This test requires image file \"test.jpg\"");
    Indexer image_ind(image);
    indexer_tools::process(image_ind);
    QVERIFY2(!image_ind.is_text(), "This may fail, pictures are bad for trigraming");
}

void sDsidTests::permissionsFileTest() {
    QFile unread("unreadable.tst");
    if (!unread.exists())
        QSKIP("This test requires unreadable for programm file");
    Indexer ind(unread);
    indexer_tools::process(ind);
    QVERIFY(!ind.is_text());
}

void sDsidTests::deletedFileTest() {
    QFile no("i_dont_exist.tst");
    no.remove(); // for somedy who would create it ))
    Indexer no_ind(no);
    indexer_tools::process(no_ind);
    QVERIFY(!no_ind.is_text());
}


void sDsidTests::containStringTest() {
    QFile sample("sample.tst");
    sample.open(QFile::WriteOnly);
    sample.write("Maма мыла раму, папа мыл капот\n");
    sample.write("How many wood would a woodchuck chuck?\n");
    sample.write("اهلا صديقي\n");
    sample.write("דאָס איז אויף שיסל\n");
    sample.write("非常に興味深い言語\n");
    sample.close();
    Indexer index(sample);
    indexer_tools::process(index);
    QVERIFY2(index.is_text(), "File is text");
    QVERIFY2(index.may_contain("мама"), "Russian full substr");
    QVERIFY2(index.may_contain("рама"), "Russian not substring but matches trigrams");
    QVERIFY2(index.may_contain("many wou"), "English full substring");
    QVERIFY2(index.may_contain("אויף"), "Idish full substring");
    QVERIFY2(index.may_contain("هلا"), "Arabic full substring");
    QVERIFY2(index.may_contain("興味深"), "Japan full substring");
}


QTEST_MAIN(sDsidTests)
#include "tests.moc"
