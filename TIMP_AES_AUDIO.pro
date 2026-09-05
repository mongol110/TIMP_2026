TEMPLATE = subdirs

SUBDIRS = \
    EchoServer \
    Client \
    DataBase \
    Singleton \
    UnitTests

UnitTests.depends = EchoServer
