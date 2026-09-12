# Arch installation and permissions

Install `keytop` from AUR for the normal binary, TUI and JSON/JSONL interface. The build uses
only this repository, Qt Core/Test and ncurses. `VERSION` is the single version source for
CMake and `keytop --version`; machine schemaVersion remains unchanged.

The base package never sets capabilities or enables services. Ordinary CPU utilization,
memory and other readable kernel metrics work as a regular user. RAPL package power needs
supported hardware and permission to read energy counters.

`keytop-privileged-access` is a separate, explicit opt-in package. It grants **CAP_PERFMON and
CAP_DAC_READ_SEARCH**, including privileged performance access and broad file-read permission
bypass; these permissions are not confined to RAPL. A pacman transaction hook applies them
on access-package installation and after keytop upgrades. Uninstalling the access package
removes exactly the matching capabilities before removing the helper. Unexpected capability
sets, foreign-owned files, symlinks and writable paths are rejected for manual review.
Already running processes can retain access. Unsupported sensors remain unavailable even
when authorization succeeds.

Existing source installation and `make setcap` remain separate administrator workflows.
Do not point package hooks at source binaries. The optional NVIDIA provider loads the installed
NVML library; the installer must not change GPU drivers to make that feature available.

See [dependencies](dependencies.md) and [release/AUR setup](releasing.md).
