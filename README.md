# keytop


<p align="center">
  <img src="https://raw.githubusercontent.com/StatIndet/picture/main/keytop1.png" alt="keytop1" width="49%" />
  <img src="https://raw.githubusercontent.com/StatIndet/picture/main/keytop2.png" alt="keytop2" width="49%" />
</p>

A standalone system monitor for Linux with an interactive terminal UI and machine-readable JSON/JSONL output.

`keytop` provides system metrics for both terminal users and [Clavis Shell](https://github.com/StatIndet/quickshell).

## Features

- CPU usage, frequency and load
- Memory and swap
- Network throughput
- Disk usage and I/O
- Process monitoring
- Temperatures
- GPU metrics
- Battery information
- Optional Intel RAPL power metrics
- Interactive ncurses TUI
- JSON and JSONL output for integrations
- Matugen-compatible colors

## Project family

| Project | Role |
| --- | --- |
| **[Clavis Shell](https://github.com/StatIndet/quickshell)** | Quickshell UI and desktop shell |
| **[key-cli](https://github.com/StatIndet/key-cli)** | `key` command and discrete system tasks |
| **[keytop](https://github.com/StatIndet/keytop)** | System monitoring, TUI and machine-readable metrics |

`keytop` can be used completely on its own.

## Usage

Launch the interactive monitor:

```bash
keytop
```

Machine-readable output:

```bash
keytop value snapshot --format json
keytop value stream --format jsonl --interval 1000
keytop value cpu --format json
keytop value memory --format json
keytop value processes --format json
```

The shorter aliases are also available:

```bash
keytop snapshot
keytop stream --format jsonl
keytop cpu
keytop processes
```

Reload colors in a running TUI:

```bash
keytop reload
```

See the machine-output schema in [`docs/protocol.md`](docs/protocol.md).

## Build from source

Requirements:

- CMake
- C++17 compiler
- Qt 6 Core and Network
- pkg-config
- ncursesw

```bash
cmake -S . -B build \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build
ctest --test-dir build --output-on-failure
```

Install system-wide:

```bash
sudo cmake --install build
```

## Configuration

Configuration is stored in:

```text
${XDG_CONFIG_HOME:-$HOME/.config}/keytop/
```

Main files:

```text
config.conf    General settings
colors.conf    Active color scheme
matugen.conf   Matugen template
```

Example:

```ini
[general]
update_interval_ms=1000
temperature_unit=celsius
```

Command-line options such as `--interval` override the configuration file.

## Intel RAPL

When the optional RAPL helper is installed, `keytop` can expose CPU package power data to regular users through the packaged systemd socket.

Enable it with:

```bash
sudo systemctl enable --now keytop-rapl.socket
```

Systems without RAPL support continue to expose the remaining metrics normally.

## Matugen

`matugen.conf` can be used to generate `colors.conf` from a Material color scheme. Clavis can manage this integration automatically from its settings center.

## License

See [LICENSE](LICENSE).
