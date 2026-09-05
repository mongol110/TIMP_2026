#include <QtTest>
#include "../EchoServer/functionsforserver.h"
#include "../EchoServer/dataBase.h"

class FuncTest : public QObject
{
    Q_OBJECT
private slots:
    void test_parsing_invalid();
    void test_sha1();
    void test_aes_roundtrip();
    void test_newton();
};

void FuncTest::test_parsing_invalid()
{
    QString nu; bool ok = false;
    QVERIFY(parsing("", "", nu, ok) == "error");
    QVERIFY(parsing("UNKNOWN|x", "bob", nu, ok) == "error");
    // без AUTH функционал запрещён
    QVERIFY(parsing("SHA1|hi", "", nu, ok) == "error: not authenticated");
}

void FuncTest::test_sha1()
{
    // SHA1("") — известная константа
    QVERIFY(sha1Hash("") == "da39a3ee5e6b4b0d3255bfef95601890afd80709");
    QVERIFY(sha1Hash("hello") == "aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d");
}

void FuncTest::test_aes_roundtrip()
{
    QString c = aesEncrypt("secretkey", "Hello");
    QVERIFY(c != "error" && !c.isEmpty());
    QVERIFY(aesDecrypt("secretkey", c) == "Hello");
}

void FuncTest::test_newton()
{
    QString r = newtonMethod(1.5, 1e-9);
    bool ok = false;
    double v = r.toDouble(&ok);
    QVERIFY(ok);
    QVERIFY(std::fabs(v - 1.5213797) < 1e-5);
}

QTEST_APPLESS_MAIN(FuncTest)
#include "tst_tests.moc"
