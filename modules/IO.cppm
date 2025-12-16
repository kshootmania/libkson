module;

#include "kson/IO/KshIO.hpp"
#ifndef KSON_WITHOUT_JSON_DEPENDENCY
#include "kson/IO/KsonIO.hpp"
#endif

export module kson:IO;

export namespace nlohmann {
    using ::nlohmann::adl_serializer;
    using ::nlohmann::basic_json;
    using ::nlohmann::json;
    using ::nlohmann::json_pointer;
    using ::nlohmann::ordered_json;
    using ::nlohmann::ordered_map;
}

export namespace kson {
    using kson::LoadKSHMetaChartData;
    using kson::LoadKSHChartData;
    using kson::SaveKSHChartData;
    #ifndef KSON_WITHOUT_JSON_DEPENDENCY
    using kson::kKSONFormatVersion;
    using kson::SaveKSONChartData;
    using kson::LoadKSONChartData;
    #endif
}
