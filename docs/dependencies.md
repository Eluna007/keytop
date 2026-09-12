# Dependencies

The source of truth is [`packaging/dependencies.json`](../packaging/dependencies.json).
Release tooling generates PKGBUILD dependency fields from this inventory. `defaultInstall`
selects the full installation profile; optional authorization and vendor GPU drivers are never implicit.

Runtime-only dependencies are assigned in package functions, so this repository builds and
tests independently of other Clavis repositories. CI installs only build, check and CI entries.

## Build

| Arch package | Purpose | Full install |
| --- | --- | --- |
| `cmake` | Native configuration | Yes |
| `ninja` | Native build | Yes |
| `pkgconf` | ncurses discovery | Yes |
| `qt6-base` | Qt Core and Test | Yes |
| `ncurses` | Wide character TUI | Yes |

## Tests

| Arch package | Purpose | Full install |
| --- | --- | --- |
| `python` | Release and permission contracts | Yes |
| `git` | Isolated source archive contract fixtures | Yes |

## CI quality tools

This phase is used only in CI; these tools are not installer runtime requests.

| Arch package | Purpose | Full install |
| --- | --- | --- |
| `actionlint` | GitHub Actions workflow validation | CI only |
| `clang` | C++ formatting | Yes |
| `shellcheck` | Shell quality checks | Yes |
| `python` | Release tooling | Yes |
| `git` | Source version and diff checks | Yes |

## keytop runtime

| Arch package | Purpose | Full install |
| --- | --- | --- |
| `qt6-base` | Qt Core runtime | Yes |
| `ncurses` | Wide character TUI | Yes |

## keytop-privileged-access runtime

| Arch package | Purpose | Full install |
| --- | --- | --- |
| `keytop` | Target binary | Yes |
| `libcap` | Capability tools | Yes |
| `bash` | Authorization helper | Yes |
| `coreutils` | Owner and file checks | Yes |
| `pacman` | Package ownership verification | Yes |

## keytop optional

| Arch package | Purpose | Full install |
| --- | --- | --- |
| `keytop-privileged-access` | CAP_PERFMON and CAP_DAC_READ_SEARCH access | Explicit opt-in |
| `nvidia-utils` | NVML for installed NVIDIA drivers; do not install on other hardware | Explicit opt-in |
