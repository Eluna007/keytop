#pragma once

#include "command_result.h"

#include <QStringList>

class ReloadCommand {
public:
    CommandResult run(const QStringList &arguments) const;
};
