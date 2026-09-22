# keytop packaging

Source installation is exposed through a stable interface in the root Makefile, still
backed by a CMake build underneath. `DESTDIR` affects only the staging install path — it
never writes user configuration, starts services, or sets capabilities.

Arch metadata is generated from `dependencies.json` and `arch/PKGBUILD.in`. See
[release setup](../docs/releasing.md) and [optional access](../docs/installation.md).
