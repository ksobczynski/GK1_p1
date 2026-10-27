# Edytor wielokątów (GK1, projekt 1)

## Wymagania

- **Qt 6** (moduł Widgets) – testowane na 6.8
- **CMake ≥ 3.16**
- kompilator **C++17**: MSVC 2022 (Windows), Clang (macOS) lub GCC ≥ 9 (Linux)

CMake nie pobiera Qt – trzeba je zainstalować wcześniej.

## Instalacja Qt

**Windows**
1. Zainstaluj **Visual Studio 2022** z obciążeniem *Programowanie aplikacji klasycznych w C++*.
2. Pobierz **Qt Online Installer** z https://www.qt.io/download-qt-installer (wymaga darmowego konta Qt).
3. W instalatorze zaznacz *Qt 6.8 → MSVC 2022 64-bit* (opcjonalnie *Qt Creator*, CMake i Ninja z sekcji *Developer and Designer Tools*).

**macOS**
```
xcode-select --install
brew install qt cmake
```

**Linux (Ubuntu 22.04+ / Debian)**
```
sudo apt install build-essential cmake qt6-base-dev
```

## Budowanie i uruchomienie

**Najprościej:** otwórz folder projektu w **Qt Creatorze** (*File → Open File or Project → CMakeLists.txt*), wybierz zestaw z Qt 6 i uruchom (Ctrl+R).

**Z wiersza poleceń:**
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=<ścieżka do Qt>
cmake --build build --config Release
```

Ścieżka do Qt:
- Windows: np. `C:\Qt\6.8.3\msvc2022_64`
- macOS: `/opt/homebrew/opt/qt`
- Linux: zwykle niepotrzebna

Uruchomienie: `./build/p1` (Windows: `build\Release\p1.exe` lub `build\p1.exe`).
Na Windowsie, jeśli program zgłasza brak bibliotek Qt, uruchom raz
`C:\Qt\6.8.3\msvc2022_64\bin\windeployqt.exe <ścieżka do p1.exe>`.

Dokumentacja jest dostępna w programie pod **F1**.
