# libkson
K-Shoot MANIA chart file (.ksh/.kson) parser library written in C++20

## Status
- [x] KSH file loading (compatible with `171`)
- [x] KSH file saving
- [x] KSON file loading
- [x] KSON file saving (compatible with `0.9.0`)

## KSH/KSON file format specification
See this repository: https://github.com/kshootmania/ksm-chart-format

## Usage
### Library
```cpp
// Load KSH chart file
kson::ChartData chartData = kson::LoadKshChartData("chart.ksh");

// Save KSON chart file
kson::SaveKshChartData("chart.kson", chartData);

// Access to chart meta data
const std::string& title = chartData.meta.title;
const std::string& artist = chartData.meta.artist;

// Access to note data, convert pulses to seconds
const kson::TimingCache timingCache = kson::CreateTimingCache(chartData.beat);
for (int i = 0; i < kson::kNumBTLanes; ++i)
{
    for (const auto& [y, note] : chart.note.bt[i])
    {
        const double noteStartSec = kson::PulseToSec(y, chartData.beat, timingCache);
        const double noteEndSec = kson::PulseToSec(y + note.length, chartData.beat, timingCache);

        // ...
    }
}
```

Almost all variables are the same as in the KSON specification, but the following are different:
- Array types in KSON specification are stored as struct or map/set.
    - `ByPulse<T>[]` in KSON specification is stored as `kson::ByPulse<T>` (alias of `std::map<kson::Pulse, T>`).
- All unsigned numbers in KSON specification are stored as signed numbers.

### ksh2kson tool
ksh2kson is a command line tool that converts KSH file to KSON. Converted KSON is output to stdout.

```bash
$ ./ksh2kson [KSH file] > [KSON file]

    or

$ cat [KSH file] | ./ksh2kson > [KSON file]
```

## Compilation
### With Visual Studio 2022
Open kson.sln and click the build button.

### With CMake
```bash
$ cmake -B build -D KSON_BUILD_TOOL_KSH2KSON=ON
$ cmake --build build
```
- For MSVC, the build type can be specified in the second command with `--config Debug` for debug builds and `--config Release` for release builds.

## Dependency
- [nlohmann/json](https://github.com/nlohmann/json) (included in `include/kson/third_party/nlohmann/json.hpp`)
- iconv (Linux/macOS)

## Contributors

@m4saka, @123jimin, @mikomikotaishi

## License
MIT License
