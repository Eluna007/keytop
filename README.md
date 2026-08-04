# keytop

`keytop` 是独立的终端系统监测程序。它拥有采样核心、TUI、机器可读输出和可选
Intel RAPL helper，不依赖 `key-cli` 或 Clavis Shell。

职责：CPU、内存、Swap、网络、磁盘、进程、温度、GPU 和 RAPL 数据；TUI 与 JSON/JSONL
接口共用同一套 sampler。直接执行 `keytop` 默认进入 TUI；`keytop value` 保留原
`key sysmon` 的机器接口语义，`keytop stream` 输出 JSONL。

## 构建和运行

```bash
./setup.sh doctor
./setup.sh configure
./setup.sh build
./setup.sh test
./setup.sh run
```

源码构建不安装依赖、不使用 sudo。默认源码安装前缀为 `/usr/local`，可使用
`CMAKE_INSTALL_PREFIX=/usr` 和 `DESTDIR="$pkgdir"` 进行打包：

```bash
CMAKE_INSTALL_PREFIX=/usr DESTDIR="$pkgdir" ./setup.sh install
./setup.sh uninstall
```

卸载只依据安装 manifest 删除本仓库安装的文件。执行 `sudo ./setup.sh install` 会安装并
启用 `keytop-rapl.socket`，普通用户随后即可通过受限的只读 helper 获取 CPU 功耗；普通
构建、非 root 安装和 `DESTDIR` 打包不会启动系统服务。非 `DESTDIR` 安装还会为调用用户
补齐缺失的三份配置；sudo 安装通过 `SUDO_USER` 写入真实用户配置目录并设置正确所有权。

## 配置和 Matugen 配色

用户配置目录为 `${XDG_CONFIG_HOME:-$HOME/.config}/keytop/`。普通配置只有两个键：

```ini
[general]
update_interval_ms=1000
temperature_unit=celsius
```

刷新间隔限制为 250–60000 毫秒；命令行 `--interval` 优先于配置。温度还可以显示为
`fahrenheit`，但 `keytop value` 的机器输出始终保留摄氏固定单位和原有 schema。

同目录的 `matugen.conf` 是输入模板，`colors.conf` 是动态配色结果。内置颜色是最后
fallback，损坏的单个颜色不会使整个 TUI 失效。运行中的 TUI 收到 `SIGHUP` 后只重新
读取颜色，不会停止采样或清空历史曲线；一次性执行 `keytop reload` 会准确通知正在
运行的 Keytop TUI，不会启动 daemon。

RAPL helper 使用 `/run/keytop/rapl.sock`，仅返回功耗采样所需的固定 JSON。可通过
`KEYTOP_RAPL_SOCKET` 覆盖测试路径；旧 `CLAVIS_RAPL_SOCKET` 暂时兼容读取。
不再需要 `key setup cpu-power`；该命令已从 `key-cli` 删除。

## 与 Clavis Shell 的关系

Shell 的系统页直接运行稳定的 `keytop stream --format jsonl`，不经过 `key`。为了
兼容旧快捷键，`key-cli` 可以提供 `key top` 转发到 `keytop`，但 keytop 本身永远不是
daemon，也不提供常驻 socket 服务。

## 用户状态和未来 AUR

Keytop 首次显式安装时只补齐缺失的三个配置示例，不覆盖已有用户文件；运行时读取
Linux `/proc`、`/sys`、系统设备和可选 RAPL socket。CMake 标准安装变量、`DESTDIR` 和
manifest 适合未来 `package()` 阶段安装到 `/usr`，AUR 包应把 `ncurses`、Qt6 Core/Network
和可选 RAPL 集成声明为依赖。

详细协议见 [docs/protocol.md](docs/protocol.md)。
