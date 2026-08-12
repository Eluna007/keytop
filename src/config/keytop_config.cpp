#include "keytop_config.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QDebug>
#include <QStringList>
#include <QTextStream>

#include <algorithm>

#ifndef KEYTOP_DEFAULTS_INSTALL_DIR
#define KEYTOP_DEFAULTS_INSTALL_DIR "/usr/local/share/keytop/defaults"
#endif

#ifndef KEYTOP_BUILD_DEFAULTS_DIR
#define KEYTOP_BUILD_DEFAULTS_DIR ""
#endif

namespace {

QString trim(const QString &value)
{
    return value.trimmed();
}

QString normalized(const QString &value)
{
    return trim(value).toLower();
}

bool validColor(const QString &value)
{
    if (value.size() != 7 || value.at(0) != QLatin1Char('#'))
        return false;
    for (int index = 1; index < value.size(); ++index) {
        if (!value.at(index).isDigit()
            && (value.at(index).toLower() < QLatin1Char('a')
                || value.at(index).toLower() > QLatin1Char('f')))
            return false;
    }
    return true;
}

QHash<QString, QString> readIni(const QString &path)
{
    QHash<QString, QString> values;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return values;

    QString section;
    QTextStream input(&file);
    int lineNumber = 0;
    while (!input.atEnd()) {
        ++lineNumber;
        const QString raw = input.readLine();
        const QString line = trim(raw);
        if (line.isEmpty() || line.startsWith(QLatin1Char('#'))
            || line.startsWith(QLatin1Char(';')))
            continue;
        if (line.startsWith(QLatin1Char('[')) && line.endsWith(QLatin1Char(']'))) {
            section = normalized(line.mid(1, line.size() - 2));
            continue;
        }
        const int equals = line.indexOf(QLatin1Char('='));
        if (equals <= 0 || section.isEmpty()) {
            qWarning("keytop: ignoring malformed %s:%d", qPrintable(path), lineNumber);
            continue;
        }
        const QString key = normalized(line.left(equals));
        if (!key.isEmpty())
            values.insert(section + QLatin1Char('.') + key, trim(line.mid(equals + 1)));
    }
    return values;
}

QString value(const QHash<QString, QString> &values, const QString &section, const QString &key)
{
    return values.value(section + QLatin1Char('.') + key);
}

void applyColor(const QHash<QString, QString> &values, const QString &key, QString *target)
{
    const QString candidate = value(values, QStringLiteral("colors"), key);
    if (candidate.isEmpty())
        return;
    if (validColor(candidate))
        *target = candidate;
    else
        qWarning("keytop: invalid color for %s: %s", qPrintable(key), qPrintable(candidate));
}

KeytopPalette fallbackPalette()
{
    return {
        QStringLiteral("#131318"),
        QStringLiteral("#e4e1e9"),
        QStringLiteral("#bbc3ff"),
        QStringLiteral("#c7c5d0"),
        QStringLiteral("#46464f"),
        QStringLiteral("#e6bad7"),
        QStringLiteral("#ffb4ab"),
        QStringLiteral("#3b4279"),
        QStringLiteral("#dfe0ff"),
        QStringLiteral("#c4c5dd"),
    };
}

QStringList defaultDirectories()
{
    QStringList candidates;
    const auto addCandidate = [&candidates](const QString &candidate) {
        if (!candidate.isEmpty()) {
            const QString normalizedCandidate = QDir::cleanPath(candidate);
            if (!candidates.contains(normalizedCandidate))
                candidates.push_back(normalizedCandidate);
        }
    };

    addCandidate(QStringLiteral(KEYTOP_BUILD_DEFAULTS_DIR));
    if (QCoreApplication::instance()) {
        const QDir applicationDir(QCoreApplication::applicationDirPath());
        addCandidate(applicationDir.filePath(QStringLiteral("../share/keytop/defaults")));
        addCandidate(applicationDir.filePath(QStringLiteral("../../defaults")));
    }
    addCandidate(QStringLiteral(KEYTOP_DEFAULTS_INSTALL_DIR));
    addCandidate(QStringLiteral("/usr/local/share/keytop/defaults"));
    addCandidate(QStringLiteral("/usr/share/keytop/defaults"));
    return candidates;
}

QString defaultFilePath(const QString &name)
{
    for (const QString &directory : defaultDirectories()) {
        const QString path = QDir(directory).filePath(name);
        const QFileInfo information(path);
        if (information.isFile() && information.isReadable())
            return path;
    }
    return {};
}

bool copyDefaultFile(const QString &name, QString *errorMessage)
{
    const QString target = QDir(keytopConfigDirectory()).filePath(name);
    const QFileInfo targetInformation(target);
    if (targetInformation.exists() || targetInformation.isSymLink())
        return true;

    const QString source = defaultFilePath(name);
    if (source.isEmpty()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("default template %1 is not installed").arg(name);
        return false;
    }

    QFile sourceFile(source);
    if (!sourceFile.copy(target)) {
        if (errorMessage)
            *errorMessage
                = QStringLiteral("cannot create %1: %2").arg(target, sourceFile.errorString());
        return false;
    }
    QFile::setPermissions(target,
                          QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup
                              | QFileDevice::ReadOther);
    return true;
}

} // namespace

QString keytopConfigDirectory()
{
    const QByteArray overridePath = qgetenv("KEYTOP_CONFIG_DIR");
    if (!overridePath.isEmpty())
        return QString::fromLocal8Bit(overridePath);
    const QString home = qEnvironmentVariable("HOME", QDir::homePath());
    const QString xdg = qEnvironmentVariable("XDG_CONFIG_HOME");
    const QString base = xdg.isEmpty() ? home + QStringLiteral("/.config") : xdg;
    return QDir(base).filePath(QStringLiteral("keytop"));
}

QString keytopConfigPath()
{
    return QDir(keytopConfigDirectory()).filePath(QStringLiteral("config.conf"));
}

QString keytopColorsPath()
{
    return QDir(keytopConfigDirectory()).filePath(QStringLiteral("colors.conf"));
}

QString keytopMatugenPath()
{
    return QDir(keytopConfigDirectory()).filePath(QStringLiteral("matugen.conf"));
}

bool keytopInitializeConfig(QString *errorMessage)
{
    if (errorMessage)
        errorMessage->clear();

    const QString directory = keytopConfigDirectory();
    QDir configDirectory(directory);
    if (!configDirectory.exists() && !QDir().mkpath(directory)) {
        if (errorMessage)
            *errorMessage = QStringLiteral("cannot create config directory: %1").arg(directory);
        return false;
    }
    if (!configDirectory.exists() || !configDirectory.isReadable()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("config directory is not usable: %1").arg(directory);
        return false;
    }

    QStringList errors;
    for (const QString &name : {QStringLiteral("config.conf"), QStringLiteral("matugen.conf")}) {
        QString error;
        if (!copyDefaultFile(name, &error))
            errors.push_back(error);
    }
    if (errorMessage)
        *errorMessage = errors.join(QStringLiteral("; "));
    return errors.isEmpty();
}

KeytopConfig loadKeytopConfig()
{
    KeytopConfig result;
    result.palette = fallbackPalette();

    const QHash<QString, QString> defaultColors
        = readIni(defaultFilePath(QStringLiteral("colors.conf")));
    applyColor(defaultColors, QStringLiteral("surface"), &result.palette.surface);
    applyColor(defaultColors, QStringLiteral("on_surface"), &result.palette.onSurface);
    applyColor(defaultColors, QStringLiteral("primary"), &result.palette.primary);
    applyColor(defaultColors, QStringLiteral("on_surface_variant"), &result.palette.muted);
    applyColor(defaultColors, QStringLiteral("outline_variant"), &result.palette.outline);
    applyColor(defaultColors, QStringLiteral("tertiary"), &result.palette.warning);
    applyColor(defaultColors, QStringLiteral("error"), &result.palette.critical);
    applyColor(
        defaultColors, QStringLiteral("primary_container"), &result.palette.selectedBackground);
    applyColor(
        defaultColors, QStringLiteral("on_primary_container"), &result.palette.selectedForeground);
    applyColor(defaultColors, QStringLiteral("secondary"), &result.palette.good);

    const QHash<QString, QString> colors = readIni(keytopColorsPath());
    applyColor(colors, QStringLiteral("surface"), &result.palette.surface);
    applyColor(colors, QStringLiteral("on_surface"), &result.palette.onSurface);
    applyColor(colors, QStringLiteral("primary"), &result.palette.primary);
    applyColor(colors, QStringLiteral("on_surface_variant"), &result.palette.muted);
    applyColor(colors, QStringLiteral("outline_variant"), &result.palette.outline);
    applyColor(colors, QStringLiteral("tertiary"), &result.palette.warning);
    applyColor(colors, QStringLiteral("error"), &result.palette.critical);
    applyColor(colors, QStringLiteral("primary_container"), &result.palette.selectedBackground);
    applyColor(colors, QStringLiteral("on_primary_container"), &result.palette.selectedForeground);
    applyColor(colors, QStringLiteral("secondary"), &result.palette.good);

    const QHash<QString, QString> defaults
        = readIni(defaultFilePath(QStringLiteral("config.conf")));
    const QString defaultInterval
        = value(defaults, QStringLiteral("general"), QStringLiteral("update_interval_ms"));
    if (!defaultInterval.isEmpty()) {
        bool ok = false;
        const int parsed = defaultInterval.toInt(&ok);
        if (ok && parsed >= 250 && parsed <= 60000)
            result.updateIntervalMs = parsed;
    }
    const QString defaultUnit = normalized(
        value(defaults, QStringLiteral("general"), QStringLiteral("temperature_unit")));
    if (defaultUnit == QStringLiteral("celsius") || defaultUnit == QStringLiteral("fahrenheit")) {
        result.temperatureUnit = defaultUnit;
    }

    const QHash<QString, QString> config = readIni(keytopConfigPath());
    const QString interval
        = value(config, QStringLiteral("general"), QStringLiteral("update_interval_ms"));
    if (!interval.isEmpty()) {
        bool ok = false;
        const int parsed = interval.toInt(&ok);
        if (ok && parsed >= 250 && parsed <= 60000)
            result.updateIntervalMs = parsed;
        else
            qWarning("keytop: update_interval_ms must be an integer from 250 to 60000: %s",
                     qPrintable(interval));
    }

    const QString unit
        = normalized(value(config, QStringLiteral("general"), QStringLiteral("temperature_unit")));
    if (unit == QStringLiteral("celsius") || unit == QStringLiteral("fahrenheit"))
        result.temperatureUnit = unit;
    else if (!unit.isEmpty())
        qWarning("keytop: temperature_unit must be celsius or fahrenheit: %s", qPrintable(unit));

    // config.conf is the explicit user override layer. The documented general
    // keys remain the only ordinary settings, but accepting a colors section
    // keeps the common precedence model useful without adding TUI switches.
    applyColor(config, QStringLiteral("surface"), &result.palette.surface);
    applyColor(config, QStringLiteral("on_surface"), &result.palette.onSurface);
    applyColor(config, QStringLiteral("primary"), &result.palette.primary);
    applyColor(config, QStringLiteral("on_surface_variant"), &result.palette.muted);
    applyColor(config, QStringLiteral("outline_variant"), &result.palette.outline);
    applyColor(config, QStringLiteral("tertiary"), &result.palette.warning);
    applyColor(config, QStringLiteral("error"), &result.palette.critical);
    applyColor(config, QStringLiteral("primary_container"), &result.palette.selectedBackground);
    applyColor(config, QStringLiteral("on_primary_container"), &result.palette.selectedForeground);
    applyColor(config, QStringLiteral("secondary"), &result.palette.good);
    return result;
}
