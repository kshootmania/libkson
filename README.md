# libkson
K-Shoot MANIA chart file (.kson/.ksh) parser library written in C++20

## Compilation
### With Visual Studio 2022
Open kson.vcxproj and click the build button.

### With CMake
```bash
$ cmake -B build
$ cmake --build build
```
- In MSVC, the build type can be specified in the second command with `--config Debug` for debug builds and `--config Release` for release builds.

## Dependency
- [nlohmann/json](https://github.com/nlohmann/json) (included in `include/kson/third_party/nlohmann/json.hpp`)

## License
MIT License
