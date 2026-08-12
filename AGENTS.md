# keytop development rules

## Responsibilities

keytop is an independent repository. It owns Linux metrics sampling and parsing, the
interactive TUI, configuration defaults, and the public JSON/JSONL machine protocol:
`schemaVersion`, module names, field types and units, stream behavior, and exit semantics.
Clavis consumes this protocol; keytop tests must not depend on `../clavis` or `../key-cli`.

## Build and workflow

```bash
make format
make format-check
make test
make check
```

The CMake project uses Qt 6 Core, C++17, ncursesw and CTest. GCC and Clang builds enable
`-Wall -Wextra -Wpedantic`; warnings are not promoted to errors. Do not add `-Werror` or
clang-tidy as part of ordinary cleanup.

## Test policy

**Do not add tests automatically just because code was changed.** Fixing a bug does not
automatically require a regression test. Add a test only when it protects stable behavior
or a public contract and can explain why lint/build cannot cover it, why the chosen unit or
integration boundary is correct, and why it does not freeze implementation details.

Allowed tests include deterministic `/proc` and `/sys` parsers, samplers, counter/math
transforms, serialization, configuration, pure TUI helpers, CLI/JSON/JSONL contracts, and
packaging behavior. Existing valuable backend and protocol tests should be preserved.

Never add tests that use `grep`, `sed`, `awk`, regular expressions or source-text matching
to assert function names, class layout, file layout, or implementation shape. Do not create
`test_*_architecture.sh`, `test_*_feature.sh` or `test_*_implementation.sh` tests.

## Protocol changes

Any JSON/JSONL machine-facing change requires synchronized updates to `docs/protocol.md`
and contract tests. Preserve `schemaVersion`, module naming, JSON field types, null
semantics, units, stream flushing, and exit codes unless a deliberate protocol change is
being reviewed.

Quality checks (`clang-format`, compiler warnings, build, CTest, ShellCheck and
`git diff --check`) are not substitutes for behavior tests. Do not introduce coverage
thresholds or a large pre-commit framework merely to increase check counts.
