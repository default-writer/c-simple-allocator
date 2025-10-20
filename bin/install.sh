#!/usr/bin/env bash

set -e
if [[ "${BASHOPTS}" != *extdebug* ]]; then
    set -e
fi

err_report() {
    rm ./llvm.sh
    cd ${source}
    echo "ERROR: $0:$*"
    exit 8
}

if [[ "${BASHOPTS}" != *extdebug* ]]; then
    trap 'err_report $LINENO' ERR
fi

cwd=$(cd "$(dirname $(dirname "${BASH_SOURCE[0]}"))" &> /dev/null && pwd)
TOOLS_DIR="${cwd}/.tools"

if [[ -f "$TOOLS_DIR/llvm/clang" ]]; then
  sudo rm $TOOLS_DIR/llvm/clang
fi

if [[ -f "$TOOLS_DIR/llvm/lldb-dap" ]]; then
  sudo rm $TOOLS_DIR/llvm/lldb-dap
fi

if [[ -f "$TOOLS_DIR/llvm/llvm-cov" ]]; then
  sudo rm $TOOLS_DIR/llvm/llvm-cov
fi

if [[ -f "$TOOLS_DIR/llvm/llvm-profdata" ]]; then
  sudo rm $TOOLS_DIR/llvm/llvm-profdata
fi

rm -rf "$TOOLS_DIR"
mkdir -p "$TOOLS_DIR/llvm"

sudo apt-get install -y lsb-release wget gpg gcc make unzip

LLVM_LANG_VERSION=20

wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh $LLVM_LANG_VERSION all

rm ./llvm.sh

if [[ ! -f "$TOOLS_DIR/llvm/clang" ]]; then
  sudo ln -s /usr/bin/clang-$LLVM_LANG_VERSION $TOOLS_DIR/llvm/clang
fi

if [[ ! -f "$TOOLS_DIR/llvm/lldb-dap" ]]; then
  sudo ln -s /usr/bin/lldb-dap-$LLVM_LANG_VERSION $TOOLS_DIR/llvm/lldb-dap
fi

if [[ ! -f "$TOOLS_DIR/llvm/llvm-cov" ]]; then
  sudo ln -s /usr/bin/llvm-cov-$LLVM_LANG_VERSION $TOOLS_DIR/llvm/llvm-cov
fi

if [[ ! -f "$TOOLS_DIR/llvm/llvm-profdata" ]]; then
  sudo ln -s /usr/bin/llvm-profdata-$LLVM_LANG_VERSION $TOOLS_DIR/llvm/llvm-profdata
fi

NINJA_VERSION="1.13.1"

ARCH=$(uname -m)

if [[ "$ARCH" == "aarch64" ]]; then
    NINJA_ZIP="ninja-linux-aarch64.zip"
elif [[ "$ARCH" == "x86_64" ]]; then
    NINJA_ZIP="ninja-linux.zip"
else 
    echo "Unsupported architecture: $ARCH. Exiting."
    exit 1
fi

mkdir -p "$TOOLS_DIR/ninja"
wget -O "$TOOLS_DIR/ninja/$NINJA_ZIP" "https://github.com/ninja-build/ninja/releases/download/v$NINJA_VERSION/$NINJA_ZIP"
unzip -o "$TOOLS_DIR/ninja/$NINJA_ZIP" -d "$TOOLS_DIR/ninja"
rm "$TOOLS_DIR/ninja/$NINJA_ZIP"
chmod +x "$TOOLS_DIR/ninja/ninja"

export PATH=$TOOLS_DIR:$PATH
export CLANG="$(dirname $(whereis clang | sed 's/.* //g'))"

grep -qxF '# ninja' $HOME/.bashrc || (tail -1 $HOME/.bashrc | grep -qxF '' || echo '' >> $HOME/.bashrc && echo '# ninja' >> $HOME/.bashrc)
grep -qxF 'export PATH='"$TOOLS_DIR/ninja"':$PATH' $HOME/.bashrc || echo 'export PATH='"$TOOLS_DIR/ninja"':$PATH' >> $HOME/.bashrc
grep -qxF '# llvm' $HOME/.bashrc || (tail -1 $HOME/.bashrc | grep -qxF '' || echo '' >> $HOME/.bashrc && echo '# llvm' >> $HOME/.bashrc)
grep -qxF 'export PATH='"$TOOLS_DIR/llvm"':$PATH' $HOME/.bashrc || echo 'export PATH='"$TOOLS_DIR/llvm"':$PATH' >> $HOME/.bashrc

. $HOME/.bashrc

[[ $SHLVL -eq 2 ]] && echo OK

cd "${pwd}"