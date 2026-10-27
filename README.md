# Edytor wielokątów (GK1, projekt 1)

## Gotowy program

W zakładce **Releases** są gotowe paczki:

- **Windows** – `p1-windows.zip`: rozpakuj i uruchom `p1.exe`.
- **Linux** – `p1-x86_64.AppImage`: `chmod +x p1-x86_64.AppImage && ./p1-x86_64.AppImage`.

## Budowanie ze źródeł

Wymagane: **Qt 6** (moduł Widgets), **CMake ≥ 3.16**, kompilator **C++17**.
CMake nie pobiera Qt – trzeba je zainstalować wcześniej.

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=<ścieżka do Qt>
cmake --build build --config Release
```

Ścieżka do Qt, np.:

- macOS (`brew install qt`): `/opt/homebrew/opt/qt`
- Windows (instalator Qt): `C:\Qt\6.8.3\msvc2022_64`
- Linux (`apt install qt6-base-dev`): zwykle niepotrzebna

Uruchomienie: `./build/p1` (Windows: `build\p1.exe`; jeśli brakuje DLL-i Qt,
uruchom raz `windeployqt build\p1.exe`).

Dokumentacja jest dostępna w programie pod **F1**.
