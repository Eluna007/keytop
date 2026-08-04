#include "config/keytop_config.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

class KeytopConfigTest : public QObject {
    Q_OBJECT

private slots:
    void defaultsAreStable();
    void colorsAndGeneralSettingsAreMerged();
};

void KeytopConfigTest::defaultsAreStable()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    qputenv("KEYTOP_CONFIG_DIR", directory.path().toUtf8());

    const KeytopConfig config = loadKeytopConfig();
    QCOMPARE(config.updateIntervalMs, 1000);
    QCOMPARE(config.temperatureUnit, QStringLiteral("celsius"));
    QCOMPARE(config.palette.primary, QStringLiteral("#5CD6B9"));
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
    configFile.write("[general]\nupdate_interval_ms=100\ntemperature_unit=fahrenheit\n\n[colors]\nprimary=#abcdef\n");
    configFile.close();

    const KeytopConfig config = loadKeytopConfig();
    QCOMPARE(config.updateIntervalMs, 1000);
    QCOMPARE(config.temperatureUnit, QStringLiteral("fahrenheit"));
    QCOMPARE(config.palette.primary, QStringLiteral("#abcdef"));
    QCOMPARE(config.palette.critical, QStringLiteral("#FFB4AB"));
    qunsetenv("KEYTOP_CONFIG_DIR");
}

QTEST_MAIN(KeytopConfigTest)
#include "keytop_config_test.moc"
