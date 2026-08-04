#!/usr/bin/env bash
set -euo pipefail

repo_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
test_dir=$(mktemp -d /tmp/keytop-install-test.XXXXXX)
cleanup() { rm -rf -- "$test_dir"; }
trap cleanup EXIT HUP INT TERM

config_dir="$test_dir/config/keytop"
CMAKE_INSTALL_PREFIX="$test_dir/prefix" \
KEYTOP_BUILD_DIR="$test_dir/build" \
KEYTOP_CONFIG_DIR="$config_dir" \
KEYTOP_SKIP_SYSTEMD=1 \
    bash "$repo_dir/setup.sh" install

for name in config.conf matugen.conf colors.conf; do
    [[ -s "$config_dir/$name" ]]
done
[[ -x "$test_dir/prefix/bin/keytop" ]]
[[ -x "$test_dir/prefix/libexec/keytop/clavis-rapl-helper" ]]
[[ -s "$test_dir/prefix/lib/systemd/system/keytop-rapl.socket" ]]
[[ -s "$test_dir/prefix/lib/systemd/system/keytop-rapl.service" ]]
grep -Fq "$test_dir/prefix/libexec/keytop/clavis-rapl-helper --serve" \
    "$test_dir/prefix/lib/systemd/system/keytop-rapl.service"

printf '# preserved user config\n' >> "$config_dir/config.conf"
CMAKE_INSTALL_PREFIX="$test_dir/prefix" \
KEYTOP_BUILD_DIR="$test_dir/build" \
KEYTOP_CONFIG_DIR="$config_dir" \
KEYTOP_SKIP_SYSTEMD=1 \
    bash "$repo_dir/setup.sh" install
grep -Fq '# preserved user config' "$config_dir/config.conf"

printf 'Keytop setup install tests passed\n'
