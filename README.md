# libkson
K-Shoot MANIA chart file (.ksh/.kson) parser library written in C++20

## Status
- [x] KSH file loading (compatible with `171`)
- [ ] KSH file saving
- [ ] KSON file loading
- [x] KSON file saving (compatible with `0.4.0`)

## KSH/KSON file format specification
See this repository: https://github.com/m4saka/ksm-chart-format-spec

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
