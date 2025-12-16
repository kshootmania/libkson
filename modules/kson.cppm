module;

#include "kson/ChartData.hpp"
#include "kson/Error.hpp"

export module kson;

export import :Audio;
export import :Beat;
export import :BG;
export import :Camera;
export import :Common;
export import :Compat;
export import :Editor;
export import :Encoding;
export import :Gauge;
export import :IO;
export import :Meta;
export import :Note;
export import :Util;

export namespace kson {
    using kson::MetaChartData;
    using kson::ChartData;
    using kson::ErrorType;
    using kson::GetErrorString;
}
