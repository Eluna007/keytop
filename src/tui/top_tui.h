#pragma once

#include <QString>

#include <memory>

namespace Apollo::Sysmon {
class Sampler;
}

class TopTui {
public:
    struct Options {
        int refreshIntervalMs = 2000;
        QString temperatureUnit = QStringLiteral("celsius");
        bool forceAscii = false;
    };

    explicit TopTui(Apollo::Sysmon::Sampler &sampler);
    TopTui(Apollo::Sysmon::Sampler &sampler, const Options &options);
    ~TopTui();

    TopTui(const TopTui &) = delete;
    TopTui &operator=(const TopTui &) = delete;

    int run();
    QString errorMessage() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
