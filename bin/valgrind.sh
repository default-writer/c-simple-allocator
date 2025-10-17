#!/usr/bin/env bash

set -e
if [[ "${BASHOPTS}" != *extdebug* ]]; then
    set -e
fi

err_report() {
    cd ${source}
    echo "ERROR: $0:$*"
    exit 8
}

if [[ "${BASHOPTS}" != *extdebug* ]]; then
    trap 'err_report $LINENO' ERR
fi

cwd=$(cd "$(dirname $(dirname "${BASH_SOURCE[0]}"))" &> /dev/null && pwd)

ninja -f build.linux.ninja examples_doubly_linked_list && valgrind --tool=callgrind --callgrind-out-file=examples_doubly_linked_list.call callgrind_annotate --inclusive=yes --tree=callgrind --thresold=10 ./examples_doubly_linked_list

[[ $SHLVL -eq 2 ]] && echo OK

cd "${pwd}"