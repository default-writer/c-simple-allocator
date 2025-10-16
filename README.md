# C simple allocator Template 

A simple and modern C++ project template using Ninja, Clang and Visual Studio Code.

This template provides a ready-to-use development environment for C++ projects on Debian-based Linux systems (including WSL), with a focus on modern tooling.
## Credits

*   The compiler toolchain is provided by the [CLANG](https://clang.llvm.org/), [LLVM](https://llvm.org/), [LLDB](https://lldb.llvm.org), [Ninja](https://ninja-build.org/), 
*   The editor/debugger support is provided by [Visual Studio Code](https://code.visualstudio.com/) with the extensions: [CLANGD](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd), [LLDB DAP](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.lldb-dap)


## Features

*   **Build System**: A straightforward `Ninja` for compiling your project.
*   **Compiler**: Configured to use the **clang** C++ compiler.
*   **Toolchain**: Automated installation of the complete LLVM/Clang toolchain (version 21), including:
    *   `clangd` for language server support (autocompletion, diagnostics).
    *   `lldb` and `lldb-dap` for powerful debugging.
    *   `clang-format` and `clang-tidy` for code formatting and static analysis.
*   **VS Code Integration**:
    *   Recommended extensions for a seamless C++ development experience.
    *   Pre-configured `build` task.
    *   Pre-configured `test` task.
    *   Pre-configured `debug` launch configuration.

## Environment Setup

- Linux/Debian:
```bash
./bin/install.sh
```

- Windows:
```powershell
./bin/install.ps1
```

## Editors

* [Visual Studio Code](https://code.visualstudio.com/)

## Extensions

*  [CLANGD](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd)
*  [LLDB DAP](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.lldb-dap)

## History

*  25-09-2025: Initial project setup with C/C++ sources and VS Code configuration.
