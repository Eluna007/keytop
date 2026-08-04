# keytop

`keytop` 是独立的终端系统监测程序，提供交互式 TUI、机器可读输出和 CPU、内存、网络、
磁盘、进程、温度、GPU 以及可选 RAPL 功耗数据。主程序直接读取 Linux 的监测接口；不依赖
systemd 服务、Unix socket 或特权 helper。

## 源码安装

```bash
git clone <repository-url>
cd keytop
make
sudo make install
sudo make setcap
```

默认前缀是 `/usr/local`，因此程序和共享资源分别安装到：

```text
/usr/local/bin/keytop
/usr/local/share/keytop/
```

`make setcap` 会对已安装的主程序设置：

```text
cap_perfmon=+ep cap_dac_read_search=+ep
```

这些 capability 让普通用户在系统允许的情况下读取受限的性能和系统监测接口。没有执行
`setcap` 时 keytop 仍然可以正常启动；不可读取的指标显示为不可用，不需要以 root 身份运行
整个 TUI。`setcap` 是独立步骤，不会由 `make install` 自动执行，也不支持在设置了 `DESTDIR`
时操作暂存文件。

构建需要 CMake、C++17 编译器、Qt6 Core、`pkg-config` 和 `ncursesw`；`setcap/getcap`
来自系统的 libcap 工具包。

## 用户级安装

```bash
make
make PREFIX="$HOME/.local" install
```

用户目录中的文件 capability 通常无法可靠保留或使用，因此受限指标可能不可用。不要假定
用户级安装后一定可以执行 `make PREFIX="$HOME/.local" setcap`。

## 自定义前缀

```bash
sudo make PREFIX=/opt/keytop install
sudo make PREFIX=/opt/keytop setcap
```

`PREFIX` 和 `DESTDIR` 始终组合为 `DESTDIR + PREFIX + 相对安装路径`。例如 Arch/AUR 的
`package()` 阶段使用：

```bash
make PREFIX=/usr DESTDIR="$pkgdir" install
```

结果应为 `$pkgdir/usr/bin/keytop` 和 `$pkgdir/usr/share/keytop/`。安装阶段只部署程序、
README 和 `defaults/` 共享模板，不会：

- 写入用户配置目录；
- 启动或启用 systemd；
- 执行 `setcap`；
- 运行 keytop 或修改当前用户的运行环境。

包管理器安装的版本应由包管理器卸载，例如：

```bash
sudo pacman -Rns keytop
```

不要用源码 Makefile 卸载 pacman/AUR 安装的版本。

源码安装会直接写入 `PREFIX`，并可由管理员在安装后单独授予 capability。AUR/pacman 的
`package()` 只使用 `PREFIX=/usr DESTDIR="$pkgdir" install` 收集文件，不执行 capability
设置；文件删除、权限和升级由包管理器负责。

## 卸载

源码安装使用：

```bash
sudo make uninstall
```

自定义前缀使用同一个前缀：

```bash
sudo make PREFIX=/opt/keytop uninstall
```

卸载按当前 `PREFIX` 和 `DESTDIR` 直接删除已知的 keytop 二进制和专属共享目录，不依赖构建
目录中的 CMake 安装记录或服务状态。它会保留所有用户配置。

## 配置初始化

用户配置目录按以下优先级确定：

```text
KEYTOP_CONFIG_DIR
→ ${XDG_CONFIG_HOME}/keytop
→ ${HOME}/.config/keytop
```

第一次进入 keytop TUI 时，程序以当前用户身份创建目录，并只补齐缺失的：

```text
config.conf
matugen.conf
```

已有文件永远不会被覆盖。`config.conf` 是用户主配置；`matugen.conf` 是需要直接编辑时
使用的 Matugen 输入模板。`colors.conf` 是 Matugen 生成的配色结果，不在首次运行时强制
创建；缺失时使用已安装的默认配色和程序内置的紧急 fallback，Matugen 真正运行后再写入。

普通配置示例：

```ini
[general]
update_interval_ms=1000
temperature_unit=celsius
```

刷新间隔限制为 250–60000 毫秒，命令行 `--interval` 优先于配置。运行中的 TUI 收到
`SIGHUP` 后会重新读取 `colors.conf`，不清空采样历史。

## RAPL 和权限

Linux collector 直接扫描并读取：

```text
/sys/class/powercap/intel-rapl:*/energy_uj
/sys/class/powercap/intel-rapl:*/max_energy_range_uj
```

它保留 package RAPL 选择、单调时钟功率计算和计数器回绕处理；接口不存在、不可读或运行在
虚拟机/非 Intel 环境时，功耗指标会优雅降级。keytop 不安装、启动或连接任何 helper、
daemon、socket、Polkit 或 D-Bus 服务。

详细机器输出协议见 [docs/protocol.md](docs/protocol.md)。
