#!/usr/bin/env bash
set -euo pipefail

repo_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
build_dir=${KEYTOP_BUILD_DIR:-"$repo_dir/.build"}

usage() {
    cat <<'EOF'
keytop source build helper

Usage:
  ./setup.sh doctor
  ./setup.sh configure [--build-dir PATH]
  ./setup.sh build [--build-dir PATH]
  ./setup.sh test [--build-dir PATH]
  ./setup.sh run [--build-dir PATH] [-- <keytop arguments>]
  ./setup.sh install [--build-dir PATH]
  ./setup.sh uninstall [--build-dir PATH]

Environment:
  CMAKE_INSTALL_PREFIX (default /usr/local)
  DESTDIR             (honoured by cmake --install)
  KEYTOP_SYSTEMCTL    (systemctl command override for tests)
  KEYTOP_SKIP_SYSTEMD (set to 1 to skip activation)
EOF
}

command_name=${1:-help}
shift || true
while [[ $# -gt 0 ]]; do
    case "$1" in
        --build-dir)
            [[ $# -ge 2 ]] || { printf 'missing --build-dir value\n' >&2; exit 2; }
            build_dir=$2
            shift 2
            ;;
        --help|-h)
            usage
            exit 0
            ;;
        --)
            shift
            break
            ;;
        *)
            printf 'unknown option: %s\n' "$1" >&2
            usage >&2
            exit 2
            ;;
    esac
done

if [[ "$build_dir" != /* ]]; then
    build_dir="$repo_dir/$build_dir"
fi

doctor() {
    local failed=0
    for command in cmake c++ pkg-config; do
        if command -v "$command" >/dev/null 2>&1; then
            printf '[OK]   %s\n' "$command"
        else
            printf '[FAIL] %s\n' "$command"
            failed=1
        fi
    done
    for module in Qt6Core Qt6Network ncursesw; do
        if pkg-config --exists "$module" 2>/dev/null; then
            printf '[OK]   pkg-config %s\n' "$module"
        else
            printf '[FAIL] pkg-config %s\n' "$module"
            failed=1
        fi
    done
    return "$failed"
}

configure() {
    local prefix_arg=()
    if [[ -n "${CMAKE_INSTALL_PREFIX:-}" ]]; then
        prefix_arg=(-DCMAKE_INSTALL_PREFIX="$CMAKE_INSTALL_PREFIX")
    fi
    cmake -S "$repo_dir" -B "$build_dir" \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TESTING=OFF \
        "${prefix_arg[@]}"
}

build() {
    configure
    cmake --build "$build_dir" --parallel
}

test_cmd() {
    local prefix_arg=()
    if [[ -n "${CMAKE_INSTALL_PREFIX:-}" ]]; then
        prefix_arg=(-DCMAKE_INSTALL_PREFIX="$CMAKE_INSTALL_PREFIX")
    fi
    cmake -S "$repo_dir" -B "$build_dir" \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DBUILD_TESTING=ON \
        "${prefix_arg[@]}"
    cmake --build "$build_dir" --parallel
    ctest --test-dir "$build_dir" --output-on-failure
}

install_user_defaults() {
    [[ -z "${DESTDIR:-}" ]] || return 0

    local config_dir="${KEYTOP_CONFIG_DIR:-}"
    local user_home="${HOME:-}"
    local xdg_config_home="${XDG_CONFIG_HOME:-}"
    local -a owner_args=()
    if (( EUID == 0 )); then
        if [[ -z "${SUDO_USER:-}" || "${SUDO_USER}" == root ]]; then
            if [[ -z "$config_dir" ]]; then
                printf 'Skipping user Keytop config initialization (no SUDO_USER).\n'
                return 0
            fi
        else
            local passwd_entry
            passwd_entry=$(getent passwd "$SUDO_USER") || {
                printf 'Unable to resolve sudo user: %s\n' "$SUDO_USER" >&2
                return 1
            }
            IFS=: read -r _ _ sudo_uid sudo_gid _ user_home _ <<< "$passwd_entry"
            owner_args=(-o "$sudo_uid" -g "$sudo_gid")
            xdg_config_home=""
        fi
    fi
    [[ -n "$config_dir" ]] \
        || config_dir="${xdg_config_home:-${user_home:?}/.config}/keytop"
    install -d -m 0755 "${owner_args[@]}" "$config_dir"
    local name
    for name in config.conf matugen.conf colors.conf; do
        [[ -e "$config_dir/$name" ]] \
            || install -m 0644 "${owner_args[@]}" \
                "$repo_dir/defaults/$name" "$config_dir/$name"
    done
    printf 'Keytop config: %s\n' "$config_dir"
}

systemctl_command() {
    printf '%s\n' "${KEYTOP_SYSTEMCTL:-systemctl}"
}

activate_rapl_socket() {
    [[ -z "${DESTDIR:-}" && "${KEYTOP_SKIP_SYSTEMD:-0}" != 1 ]] || return 0
    if (( EUID != 0 )); then
        printf 'Keytop installed without privileged RAPL access; rerun install with sudo to enable CPU power readings.\n' >&2
        return 0
    fi
    local command
    command=$(systemctl_command)
    command -v "$command" >/dev/null 2>&1 || {
        printf 'systemctl is required to activate keytop-rapl.socket\n' >&2
        return 1
    }
    "$command" daemon-reload
    "$command" enable --now keytop-rapl.socket
}

deactivate_rapl_socket() {
    [[ -z "${DESTDIR:-}" && "${KEYTOP_SKIP_SYSTEMD:-0}" != 1 ]] || return 0
    (( EUID == 0 )) || return 0
    local command
    command=$(systemctl_command)
    command -v "$command" >/dev/null 2>&1 || return 0
    "$command" disable --now keytop-rapl.socket >/dev/null 2>&1 || true
}

install_cmd() {
    build
    cmake --install "$build_dir"
    install_user_defaults
    activate_rapl_socket
}

uninstall_cmd() {
    local manifest="$build_dir/install_manifest.txt"
    local destdir=${DESTDIR:-}
    [[ -f "$manifest" ]] || { printf 'no install manifest: %s\n' "$manifest" >&2; exit 1; }
    deactivate_rapl_socket
    while IFS= read -r path; do
        [[ -n "$path" ]] || continue
        local target="$path"
        if [[ -n "$destdir" ]]; then
            target="$destdir$path"
        fi
        [[ -e "$target" ]] || continue
        rm -f -- "$target"
    done < "$manifest"
    if [[ -z "$destdir" && "${KEYTOP_SKIP_SYSTEMD:-0}" != 1 && EUID -eq 0 ]]; then
        local command
        command=$(systemctl_command)
        command -v "$command" >/dev/null 2>&1 && "$command" daemon-reload
    fi
}

case "$command_name" in
    help|-h|--help) usage ;;
    doctor) doctor ;;
    configure) configure ;;
    build) build ;;
    test) test_cmd ;;
    run) build; exec "$build_dir/bin/keytop" "$@" ;;
    install) install_cmd ;;
    uninstall) uninstall_cmd ;;
    *) printf 'unknown command: %s\n' "$command_name" >&2; usage >&2; exit 2 ;;
esac
