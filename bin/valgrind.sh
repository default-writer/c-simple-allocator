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

PERF="$cwd/perf"

mkdir -p $PERF

ninja -f $cwd/build.linux.ninja examples_doubly_linked_list && valgrind --tool=callgrind --callgrind-out-file=$PERF/examples_doubly_linked_list.out $cwd/examples_doubly_linked_list callgrind_annotate --inclusive=yes --tree=callgrind --thresold=10

[[ $SHLVL -eq 2 ]] && echo OK

cd "${pwd}"