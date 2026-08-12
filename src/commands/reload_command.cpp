#include "reload_command.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <csignal>
#include <unistd.h>

namespace {

bool isTuiCommand(const QStringList &arguments)
{
    for (const QString &argument : arguments) {
        if (argument == QStringLiteral("value") || argument == QStringLiteral("stream")
            || argument == QStringLiteral("snapshot") || argument == QStringLiteral("modules")
            || argument == QStringLiteral("reload") || argument == QStringLiteral("--help")
            || argument == QStringLiteral("--version") || argument == QStringLiteral("version"))
            return false;
    }
    return true;
}

} // namespace

CommandResult ReloadCommand::run(const QStringList &arguments) const
{
    if (!arguments.isEmpty()) {
        return {
            2,
            false,
            {},
            QStringLiteral("keytop reload does not accept arguments."),
            true,
        };
    }

    int notified = 0;
    const QDir proc(QStringLiteral("/proc"));
    const QStringList entries = proc.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &entry : entries) {
        bool ok = false;
        const qint64 pid = entry.toLongLong(&ok);
        if (!ok || pid <= 1 || pid == static_cast<qint64>(::getpid()))
            continue;

        const QString executable
            = QFileInfo(QStringLiteral("/proc/%1/exe").arg(pid)).symLinkTarget();
        if (executable.isEmpty() || QFileInfo(executable).fileName() != QStringLiteral("keytop"))
            continue;

        QFile commandLine(QStringLiteral("/proc/%1/cmdline").arg(pid));
        if (!commandLine.open(QIODevice::ReadOnly))
            continue;
        const QStringList commandArguments
            = QString::fromLocal8Bit(commandLine.readAll()).split(QChar('\0'), Qt::SkipEmptyParts);
        if (commandArguments.isEmpty() || !isTuiCommand(commandArguments.mid(1)))
            continue;

        if (::kill(static_cast<pid_t>(pid), SIGHUP) == 0)
            ++notified;
    }

    return {
        0,
        false,
        {},
        QStringLiteral("Notified %1 Keytop TUI process(es) to reload colors.").arg(notified),
        false,
    };
}
