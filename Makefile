PREFIX ?= /usr/local
DESTDIR ?=
BUILD_DIR ?= build
CMAKE ?= cmake
CMAKE_BUILD_TYPE ?= Release
CMAKE_ARGS ?=

KEYTOP_BINARY := $(DESTDIR)$(PREFIX)/bin/keytop

.DEFAULT_GOAL := all
CPP_FILES := $(shell find src tests -type f \( -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) -print)
SHELL_FILES := $(shell find tests -type f -name '*.sh' -print)

.PHONY: all build configure test format format-check shellcheck diff-check check install setcap uninstall clean help

all: build

build: configure
	$(CMAKE) --build "$(BUILD_DIR)" --parallel

configure:
	$(CMAKE) -S . -B "$(BUILD_DIR)" \
		-DCMAKE_BUILD_TYPE="$(CMAKE_BUILD_TYPE)" \
		-DBUILD_TESTING=OFF \
		-DCMAKE_INSTALL_PREFIX="$(PREFIX)" \
		$(CMAKE_ARGS)

test:
	$(CMAKE) -S . -B "$(BUILD_DIR)" \
		-DCMAKE_BUILD_TYPE=RelWithDebInfo \
		-DBUILD_TESTING=ON \
		-DCMAKE_INSTALL_PREFIX="$(PREFIX)" \
		$(CMAKE_ARGS)
	$(CMAKE) --build "$(BUILD_DIR)" --parallel
	ctest --test-dir "$(BUILD_DIR)" --output-on-failure

format:
	@command -v clang-format >/dev/null 2>&1 || { \
		printf 'keytop: clang-format is required for format\n' >&2; \
		exit 127; \
	}
	@if test -n "$(CPP_FILES)"; then \
		clang-format -i $(CPP_FILES); \
	fi

format-check:
	@command -v clang-format >/dev/null 2>&1 || { \
		printf 'keytop: clang-format is required for format-check\n' >&2; \
		exit 127; \
	}
	@if test -n "$(CPP_FILES)"; then \
		clang-format --dry-run --Werror $(CPP_FILES); \
	fi

shellcheck:
	@if test -n "$(SHELL_FILES)"; then \
		command -v shellcheck >/dev/null 2>&1 || { \
			printf 'keytop: shellcheck is required for shellcheck\n' >&2; \
			exit 127; \
		}; \
		shellcheck $(SHELL_FILES); \
	fi

diff-check:
	git diff --check

check: format-check test shellcheck diff-check

install: build
	DESTDIR="$(DESTDIR)" $(CMAKE) --install "$(BUILD_DIR)"

setcap:
	@test -f "$(KEYTOP_BINARY)" || { \
		printf 'keytop: installed binary not found: %s\n' "$(KEYTOP_BINARY)" >&2; \
		exit 1; \
	}
	@test -x "$(KEYTOP_BINARY)" || { \
		printf 'keytop: installed binary is not executable: %s\n' "$(KEYTOP_BINARY)" >&2; \
		exit 1; \
	}
	@if test -n "$(DESTDIR)"; then \
		printf 'keytop: refusing setcap while DESTDIR is set; install the package first, then run make setcap on the real prefix\n' >&2; \
		exit 1; \
	fi
	@command -v setcap >/dev/null 2>&1 || { \
		printf 'keytop: setcap is required (install the libcap utility package)\n' >&2; \
		exit 1; \
	}
	@setcap "cap_perfmon=+ep cap_dac_read_search=+ep" "$(KEYTOP_BINARY)"
	@if command -v getcap >/dev/null 2>&1; then \
		getcap "$(KEYTOP_BINARY)"; \
	else \
		printf 'keytop: capabilities set; getcap is unavailable, so the result could not be displayed\n'; \
	fi

uninstall:
	@printf 'Removing: %s\n' "$(KEYTOP_BINARY)"
	@rm -f -- "$(KEYTOP_BINARY)"
	@printf 'Removing: %s\n' "$(DESTDIR)$(PREFIX)/share/keytop"
	@rm -rf -- "$(DESTDIR)$(PREFIX)/share/keytop"

clean:
	@if test -d "$(BUILD_DIR)"; then \
		$(CMAKE) --build "$(BUILD_DIR)" --target clean; \
	fi

help:
	@printf 'keytop makefile\n\n'
	@printf 'Usage: make [target] [PREFIX=/path] [DESTDIR=/stage]\n\n'
	@printf 'Targets:\n'
	@printf '  all        Build keytop (default)\n'
	@printf '  build      Configure and compile keytop\n'
	@printf '  install    Install the binary and shared resources\n'
	@printf '  setcap     Grant cap_perfmon and cap_dac_read_search to keytop\n'
	@printf '  uninstall  Remove known keytop files; keep user configuration\n'
	@printf '  clean      Remove generated build objects\n'
	@printf '  test       Build and run tests\n'
	@printf '  format     Format first-party C++\n'
	@printf '  format-check  Check first-party C++ formatting\n'
	@printf '  check      Run format, build, tests, shellcheck and diff checks\n'
	@printf '  help       Show this help\n'
