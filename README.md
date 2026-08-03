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

卸载只依据安装 manifest 删除本仓库安装的文件。RAPL helper 是可选的独立系统集成，
不会由普通构建或 Shell 安装隐式启用。

## 与 Clavis Shell 的关系

Shell 的系统页直接运行稳定的 `keytop stream --format jsonl`，不经过 `key`。为了
兼容旧快捷键，`key-cli` 可以提供 `key top` 转发到 `keytop`，但 keytop 本身永远不是
daemon，也不提供常驻 socket 服务。

## 用户状态和未来 AUR

keytop 本身不写用户配置；运行时只读取 Linux `/proc`、`/sys`、系统设备和可选 RAPL
socket。CMake 标准安装变量、`DESTDIR` 和 manifest 适合未来 `package()` 阶段安装到
`/usr`，AUR 包应把 `ncurses`、Qt6 Core/Network 和可选 RAPL 集成声明为依赖。

详细协议见 [docs/protocol.md](docs/protocol.md)。

