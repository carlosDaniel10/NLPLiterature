# AGENTS.md

Early-stage Qt Widgets desktop app (`NLPFront`): C++17 sandbox for implementing NLP algorithms
over Spanish literature. Only the Qt Creator boilerplate MainWindow exists today; docs, UI and the
translation catalog target Spanish (`es_CO`).

## Layout

- `NLPFront/` is the app, and its `CMakeLists.txt` is the CMake root (there is none at repo root).
  One target: `NLPFront`.
- New `.cpp`/`.h`/`.ui` files must be added to `PROJECT_SOURCES` in `NLPFront/CMakeLists.txt`;
  there is no source globbing.
- `NLPFront/build/` holds Qt Creator build trees and is gitignored (`/build*/` in
  `NLPFront/.gitignore`). Never edit generated files (`ui_mainwindow.h`, `moc_*`, `*_autogen/`).
- `NLPFront/CMakeLists.txt.user` is Qt Creator's machine-local config (gitignored) — don't edit;
  read it only to learn the active kit.

## Build (verified on this machine)

Qt Creator kit "Desktop Qt 6.9.0 MinGW 64-bit": Qt `C:\Qt\6.9.0\mingw_64`,
GCC 13.1 `C:\Qt\Tools\mingw1310_64`, Ninja `C:\Qt\Tools\Ninja`. System `cmake` is not on PATH;
use Qt's bundled `C:\Qt\Tools\CMake_64\bin\cmake.exe`.

**Gotcha:** if MinGW's `bin` is not on PATH, `cc1plus` fails silently — g++ exits 1 with no
output and the build stops with a bare `AutoMoc subprocess error ... failed with return value 1`.
Always prepend the toolchain to PATH before a CLI build:

```powershell
$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;C:\Qt\Tools\CMake_64\bin;$env:PATH"
cmake -S NLPFront -B NLPFront/build/cli -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=C:/Qt/6.9.0/mingw_64
cmake --build NLPFront/build/cli
```

- Incremental rebuild of the Qt Creator tree:
  `cmake --build NLPFront/build/Desktop_Qt_6_9_0_MinGW_64_bit-Debug`.
- Running the exe outside Qt Creator needs `C:\Qt\6.9.0\mingw_64\bin` on PATH (or `windeployqt`).

## Conventions

- No tests, CI, linter, or formatter exist — compiling the app is the only verification step;
  don't invent `ctest`/lint commands.
- C++17, Qt Widgets, AUTOMOC/AUTOUIC/AUTORCC enabled. Edit `mainwindow.ui`, not the generated
  `ui_mainwindow.h`.
- Wrap user-visible strings in `tr()`. `NLPFront_es_CO.ts` is the (currently empty) translation
  catalog; `qt_create_translation` is called, but `.qm` files are not produced by the default
  build, so changes to the catalog don't affect the running app yet.

## Dev rules 

- Control structures always braced.
- `class` only, never `struct` .
- Class attributes must be private.
- Source must be plain ASCII: no `ñ`, no accents. Folder names contain them because they come from the LMS; code files currently satisfy the rule.
- No C++ templates for data structures
