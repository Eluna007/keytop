#pragma once

#include <QString>

struct KeytopPalette {
    QString surface;
    QString onSurface;
    QString primary;
    QString muted;
    QString outline;
    QString warning;
    QString critical;
    QString selectedBackground;
    QString selectedForeground;
    QString good;
};

struct KeytopConfig {
    int updateIntervalMs = 2000;
    QString temperatureUnit = QStringLiteral("celsius");
    KeytopPalette palette;
};

QString keytopConfigDirectory();
QString keytopConfigPath();
QString keytopColorsPath();
QString keytopMatugenPath();
bool keytopInitializeConfig(QString *errorMessage = nullptr);
KeytopConfig loadKeytopConfig();
