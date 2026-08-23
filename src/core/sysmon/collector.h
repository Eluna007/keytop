#pragma once

#include "parsers.h"
#include "types.h"

#include <QHash>
#include <QString>
#include <QStringList>
#include <QVector>

#include <optional>
#include <memory>

namespace Clavis::Sysmon {

class GpuManager;

struct RawCpuInfo {
    CpuCounters counters;
    OptionalNumber frequencyCurrentMHz;
    OptionalNumber frequencyAverageMHz;
    OptionalNumber frequencyMinMHz;
    OptionalNumber frequencyMaxMHz;
    OptionalNumber temperatureCelsius;
    OptionalNumber packageTemperatureCelsius;
    OptionalInteger packageEnergyMicroJoules;
    OptionalInteger packageEnergyRangeMicroJoules;
    OptionalNumber fanRpm;
};

struct RawDiskInfo {
    DiskInfo info;
    QString counterKey;
    std::optional<DiskCounter> counters;
};

struct RawNetworkInterfaceInfo {
    NetworkInterfaceInfo info;
    NetworkCounter counters;
};

struct RawProcessInfo {
    ProcessInfo info;
    quint64 cpuTicks = 0;
    quint64 startTicks = 0;
};

struct RawSnapshot {
    qint64 cpuTimestampNs = 0;
    qint64 diskTimestampNs = 0;
    qint64 networkTimestampNs = 0;
    qint64 processTimestampNs = 0;
    SystemInfo system;
    RawCpuInfo cpu;
    MemoryCounters memory;
    QVector<GpuInfo> gpus;
    QVector<RawDiskInfo> disks;
    QVector<RawNetworkInterfaceInfo> networkInterfaces;
    QString defaultNetworkInterface;
    BatteryInfo battery;
    QVector<RawProcessInfo> processes;
    QVector<Error> errors;
};

class LinuxCollector {
public:
    LinuxCollector();
    ~LinuxCollector();

    RawSnapshot collect(const ModuleSet &modules);

private:
    SystemInfo collectSystem(QVector<Error> *errors) const;
    RawCpuInfo collectCpu(QVector<Error> *errors) const;
    MemoryCounters collectMemory(QVector<Error> *errors) const;
    QVector<RawDiskInfo> collectDisks(QVector<Error> *errors) const;
    QVector<RawNetworkInterfaceInfo> collectNetwork(QString *defaultInterface,
                                                    QVector<Error> *errors) const;
    BatteryInfo collectBattery(QVector<Error> *errors) const;
    QVector<RawProcessInfo>
    collectProcesses(quint64 totalMemoryBytes, qint64 bootTimeMs, QVector<Error> *errors) const;

    void loadStaticSystemInfo();
    SystemInfo m_staticSystem;
    mutable QStringList m_cpuTemperaturePaths;
    mutable QString m_packageTemperaturePath;
    mutable QString m_packageEnergyPath;
    mutable QString m_packageEnergyRangePath;
    mutable QString m_fanPath;
    std::unique_ptr<GpuManager> m_gpuManager;
};

} // namespace Clavis::Sysmon
