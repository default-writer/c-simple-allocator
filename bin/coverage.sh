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

# prefer tools installed under .tools/llvm/bin if present
LLVM_BIN="$cwd/.tools/llvm/bin"
if [[ -x "$LLVM_BIN/llvm-cov" ]]; then
  LLVM_COV="$LLVM_BIN/llvm-cov"
else
  LLVM_COV="llvm-cov"
fi
if [[ -x "$LLVM_BIN/llvm-profdata" ]]; then
  LLVM_PROFDATA="$LLVM_BIN/llvm-profdata"
else
  LLVM_PROFDATA="llvm-profdata"
fi

ninja -f $cwd/build.linux.ninja code_coverage_examples_doubly_linked_list

LLVM_PROFILE_FILE="$PERF/examples_doubly_linked_list.profraw" ./code_coverage_examples_doubly_linked_list

# merge raw profile into .profdata then run llvm-cov
PROFDATA="$PERF/examples_doubly_linked_list.profdata"
$LLVM_PROFDATA merge -sparse "$PERF/examples_doubly_linked_list.profraw" -o "$PROFDATA"

$LLVM_COV show --format=text --instr-profile="$PROFDATA" ./code_coverage_examples_doubly_linked_list
$LLVM_COV report --instr-profile="$PROFDATA" ./code_coverage_examples_doubly_linked_list

[[ $SHLVL -eq 2 ]] && echo OK

cd "${pwd}"