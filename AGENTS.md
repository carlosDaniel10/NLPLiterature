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
- fastText (Meta) is fetched by CMake (`FetchContent`, tag `v0.9.2`) on first configure; it needs
  network once per build tree. `NLPFront/cmake/parchear_fasttext.cmake` applies two patches
  automatically at populate time (a missing `<cstdint>` include for GCC 13, and renaming the CLI
  `main` so it does not collide with the app's). If fastText misbehaves after changing that
  script, delete the build tree's `_deps/` folder and reconfigure.

## Sentiment model (fastText)

Two analyzers coexist and are selected in the UI combo: `AnalizadorLexico` (the lexicon in
`NLPFront/sentimientos_es.txt`) and `AnalizadorFastText`, which loads a supervised fastText
model. The app looks for `sentimientos_modelo.ftz` (quantized) and then `sentimientos_modelo.bin`
next to the exe, one and two levels up (the latter covers the dev build trees).

The model is trained with weak supervision from the lexicon over the `Obras/` corpus by a
separate CMake project (it does not add targets to the app):

```powershell
$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;C:\Qt\Tools\CMake_64\bin;$env:PATH"
cmake -S NLPFront/herramientas/entrenarmodelo -B NLPFront/build/herramientas -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:/Qt/6.9.0/mingw_64
cmake --build NLPFront/build/herramientas
$env:PATH = "C:\Qt\6.9.0\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;$env:PATH"
NLPFront\build\herramientas\entrenarmodelo.exe Obras NLPFront\sentimientos_es.txt NLPFront\sentimientos_modelo.bin
```

The trainer writes `<salida>.train.txt`, trains (dim 100, 5 epochs, softmax, wordNgrams 2,
max 250 samples per work), quantizes to `<salida>.ftz` and deletes the unquantized `.bin`
(it stores the whole 2M subword bucket and is ~800 MB; the `.ftz` is ~50 MB). Model and train
file are gitignored.

The same pipeline is available from the app UI: the "Reentrenar modelo" button opens
`DialogoEntrenar`, which supports the full process from the works (the lexicon is taken from the
embedded qrc resource) or training from an existing `train.txt` adjusted by hand. Training runs
in a worker `QThread`; `EntrenadorFastText` (`NLPFront/entrenadorfasttext.*`) holds the shared
pipeline logic used by both the console tool and the dialog. On success the app reloads the
model and offers to re-analyze the open work.

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
