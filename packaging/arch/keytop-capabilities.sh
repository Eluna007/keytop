#!/usr/bin/env bash
# Owned only by keytop-privileged-access. Never called by the base package.
set -euo pipefail

main() {
    local operation=${1:-} root=/ binary current owner
    if [[ ${2:-} == --root && $# == 3 && $3 == /* && $3 != */../* ]]; then
        root=$3
    elif [[ $# != 1 ]]; then
        printf 'usage: privileged-access enable|disable [--root /test-root]\n' >&2
        return 2
    fi
    [[ $operation == enable || $operation == disable ]] || return 2
    [[ $(id -u) == 0 ]] || { printf 'keytop: administrator privileges required\n' >&2; return 1; }
    binary=${root%/}/usr/bin/keytop
    if [[ ! -e $binary && ! -L $binary && $operation == disable ]]; then return 0; fi
    [[ -f $binary && -x $binary && ! -L $binary && $(readlink -f -- "$binary") == "$binary" ]] || {
        printf 'keytop: refusing missing, indirect or non-executable target: %s\n' "$binary" >&2
        return 1
    }
    # Reject writable files and every writable or non-root-owned parent.
    local path=$binary mode
    while [[ $path != / ]]; do
        owner=$(stat -c '%u' -- "$path")
        mode=$(stat -c '%a' -- "$path")
        if [[ $owner != 0 ]] || (( (8#$mode & 8#022) != 0 )); then
            printf 'keytop: untrusted owner or permissions: %s\n' "$path" >&2
            return 1
        fi
        path=$(dirname -- "$path")
    done
    [[ $(pacman --root "$root" -Qqo -- "$binary") == keytop ]] || {
        printf 'keytop: target is not owned by the keytop package\n' >&2
        return 1
    }
    current=$(getcap -n -- "$binary")
    current=${current#"$binary "}
    if [[ -n $current && $current != cap_dac_read_search,cap_perfmon=ep ]]; then
        printf 'keytop: existing capabilities preserved for manual review: %s\n' "$current" >&2
        return 1
    fi
    if [[ $operation == enable ]]; then
        [[ -n $current ]] || setcap 'cap_dac_read_search,cap_perfmon=ep' "$binary"
        current=$(getcap -n -- "$binary")
        [[ $current == "$binary cap_dac_read_search,cap_perfmon=ep" ]] || {
            printf 'keytop: capability verification failed\n' >&2; return 1;
        }
    elif [[ -n $current ]]; then
        setcap -r "$binary"
    fi
}

main "$@"
