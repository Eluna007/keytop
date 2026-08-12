#!/usr/bin/env bash
set -euo pipefail

repo_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
test_dir=$(mktemp -d /tmp/keytop-make-install-test.XXXXXX)
cleanup() { rm -rf -- "$test_dir"; }
trap cleanup EXIT HUP INT TERM

build_dir="$test_dir/build"
stage_dir="$test_dir/stage"
prefix_dir="$test_dir/prefix"
config_home="$test_dir/config-home"

XDG_CONFIG_HOME="$config_home" HOME="$test_dir/home" \
make -C "$repo_dir" BUILD_DIR="$build_dir" PREFIX=/usr \
    DESTDIR="$stage_dir" install

[[ -x "$stage_dir/usr/bin/keytop" ]]
[[ -s "$stage_dir/usr/share/keytop/README.md" ]]
for name in config.conf matugen.conf colors.conf; do
    [[ -s "$stage_dir/usr/share/keytop/defaults/$name" ]]
done
[[ ! -e "$config_home" ]]
expected_files=$(printf '%s\n' \
    'usr/bin/keytop' \
    'usr/share/keytop/README.md' \
    'usr/share/keytop/defaults/colors.conf' \
    'usr/share/keytop/defaults/config.conf' \
    'usr/share/keytop/defaults/matugen.conf' | sort)
actual_files=$(find "$stage_dir" -type f -printf '%P\n' | sort)
[[ "$actual_files" == "$expected_files" ]]

XDG_CONFIG_HOME="$config_home" HOME="$test_dir/home" \
make -C "$repo_dir" BUILD_DIR="$build_dir" PREFIX=/usr \
    DESTDIR="$stage_dir" uninstall
[[ ! -e "$stage_dir/usr/bin/keytop" ]]
[[ ! -e "$stage_dir/usr/share/keytop" ]]
[[ -d "$stage_dir/usr" ]]

XDG_CONFIG_HOME="$config_home" HOME="$test_dir/home" \
make -C "$repo_dir" BUILD_DIR="$test_dir/prefix-build" \
    PREFIX="$prefix_dir" install
[[ -x "$prefix_dir/bin/keytop" ]]
XDG_CONFIG_HOME="$config_home" HOME="$test_dir/home" \
make -C "$repo_dir" BUILD_DIR="$test_dir/prefix-build" \
    PREFIX="$prefix_dir" uninstall
[[ ! -e "$prefix_dir/bin/keytop" ]]
[[ ! -e "$prefix_dir/share/keytop" ]]

printf 'Keytop make install tests passed\n'
