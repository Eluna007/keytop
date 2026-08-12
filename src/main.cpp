#include "commands/command_result.h"
#include "commands/reload_command.h"
#include "commands/sysmon_command.h"
#include "commands/top_command.h"

#include <QCoreApplication>
#include <QJsonDocument>
#include <QTextStream>

namespace {

CommandResult versionResult()
{
    return {
        0,
        false,
        {},
        QStringLiteral("keytop 0.1.0"),
        false,
    };
}

CommandResult helpResult()
{
    return {0,
            false,
            {},
            QStringLiteral(
                "keytop - standalone system monitor\n\n"
                "Usage:\n"
                "  keytop\n"
                "  keytop value snapshot [--format json|text] [--modules LIST]\n"
                "  keytop value stream [--format jsonl|text] [--interval MS] [--modules LIST]\n"
                "  keytop value system|cpu|memory|gpu|disk|network|battery [options]\n"
                "  keytop value processes [options]\n"
                "  keytop stream [options]\n"
                "  keytop reload\n"
                "  keytop --version\n\n"
                "The default command enters the interactive TUI. Machine output uses\n"
                "the schema and units formerly exposed by `key sysmon`.")};
}

CommandResult route(const QStringList &arguments)
{
    if (arguments.isEmpty())
        return TopCommand().run({});

    const QString command = arguments.first().toLower();
    if (command == QStringLiteral("--help") || command == QStringLiteral("-h"))
        return helpResult();
    if (command == QStringLiteral("--version") || command == QStringLiteral("-v")
        || command == QStringLiteral("version"))
        return versionResult();
    if (command == QStringLiteral("top"))
        return TopCommand().run(arguments.mid(1));
    if (command == QStringLiteral("reload"))
        return ReloadCommand().run(arguments.mid(1));
    if (command == QStringLiteral("value") || command == QStringLiteral("sysmon"))
        return SysmonCommand().run(arguments.mid(1));
    if (command == QStringLiteral("stream") || command == QStringLiteral("snapshot")
        || command == QStringLiteral("modules") || command == QStringLiteral("system")
        || command == QStringLiteral("cpu") || command == QStringLiteral("memory")
        || command == QStringLiteral("gpu") || command == QStringLiteral("disk")
        || command == QStringLiteral("network") || command == QStringLiteral("battery")
        || command == QStringLiteral("processes"))
        return SysmonCommand().run(arguments);

    return {
        2,
        false,
        {},
        QStringLiteral("Unknown keytop command: %1\n\n").arg(command) + helpResult().text,
        true,
    };
}

int emitResult(const CommandResult &result)
{
    if (result.outputHandled)
        return result.exitCode;

    QTextStream output(result.textIsError ? stderr : stdout);
    if (result.jsonRequested)
        output << QJsonDocument(result.json).toJson(QJsonDocument::Compact);
    else
        output << result.text;
    if (!result.text.isEmpty() || result.jsonRequested)
        output << Qt::endl;
    if (output.status() != QTextStream::Ok) {
        QTextStream(stderr) << "keytop: unable to write command output" << Qt::endl;
        return 3;
    }
    return result.exitCode;
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);
    application.setApplicationName(QStringLiteral("keytop"));
    return emitResult(route(application.arguments().mid(1)));
}
