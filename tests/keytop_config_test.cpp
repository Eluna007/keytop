#include "config/keytop_config.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTest>

class KeytopConfigTest : public QObject {
    Q_OBJECT

private slots:
    void defaultsAreStable();
    void firstRunInitializationIsIdempotent();
    void colorsAndGeneralSettingsAreMerged();
};

void KeytopConfigTest::defaultsAreStable()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    qputenv("KEYTOP_CONFIG_DIR", directory.path().toUtf8());

    const KeytopConfig config = loadKeytopConfig();
    QCOMPARE(config.updateIntervalMs, 2000);
    QCOMPARE(config.temperatureUnit, QStringLiteral("celsius"));
    QCOMPARE(config.palette.primary, QStringLiteral("#bbc3ff"));
    qunsetenv("KEYTOP_CONFIG_DIR");
}

void KeytopConfigTest::firstRunInitializationIsIdempotent()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    qputenv("KEYTOP_CONFIG_DIR", directory.path().toUtf8());

    QString error;
    QVERIFY2(keytopInitializeConfig(&error), qPrintable(error));
    QVERIFY(QFileInfo::exists(directory.filePath(QStringLiteral("config.conf"))));
    QVERIFY(QFileInfo::exists(directory.filePath(QStringLiteral("matugen.conf"))));
    QVERIFY(!QFileInfo::exists(directory.filePath(QStringLiteral("colors.conf"))));

    const QString configPath = directory.filePath(QStringLiteral("config.conf"));
    QFile configFile(configPath);
    QVERIFY(configFile.open(QIODevice::Append | QIODevice::Text));
    configFile.write("# user change\n");
    configFile.close();

    QFile beforeFile(configPath);
    QVERIFY(beforeFile.open(QIODevice::ReadOnly));
    const QByteArray before = beforeFile.readAll();
    beforeFile.close();

    QVERIFY(keytopInitializeConfig(&error));
    QFile afterFile(configPath);
    QVERIFY(afterFile.open(QIODevice::ReadOnly));
    QCOMPARE(afterFile.readAll(), before);
    afterFile.close();

    QVERIFY(QFile::remove(configPath));
    QVERIFY(keytopInitializeConfig(&error));
    QVERIFY(QFileInfo::exists(configPath));
    QVERIFY(QFileInfo::exists(directory.filePath(QStringLiteral("matugen.conf"))));
    qunsetenv("KEYTOP_CONFIG_DIR");
}

void KeytopConfigTest::colorsAndGeneralSettingsAreMerged()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    qputenv("KEYTOP_CONFIG_DIR", directory.path().toUtf8());
    QVERIFY(QDir().mkpath(directory.path()));

    QFile colors(directory.path() + QStringLiteral("/colors.conf"));
    QVERIFY(colors.open(QIODevice::WriteOnly | QIODevice::Text));
    colors.write("[colors]\nprimary=#123456\nerror=broken\nunknown=#ffffff\n");
    colors.close();

    QFile configFile(directory.path() + QStringLiteral("/config.conf"));
    QVERIFY(configFile.open(QIODevice::WriteOnly | QIODevice::Text));
    configFile.write("[general]\nupdate_interval_ms=100\ntemperature_unit=fahrenheit\n\n[colors]"
                     "\nprimary=#abcdef\n");
    configFile.close();

    const KeytopConfig config = loadKeytopConfig();
    QCOMPARE(config.updateIntervalMs, 2000);
    QCOMPARE(config.temperatureUnit, QStringLiteral("fahrenheit"));
    QCOMPARE(config.palette.primary, QStringLiteral("#abcdef"));
    QCOMPARE(config.palette.critical, QStringLiteral("#ffb4ab"));
    qunsetenv("KEYTOP_CONFIG_DIR");
}

QTEST_MAIN(KeytopConfigTest)
#include "keytop_config_test.moc"
