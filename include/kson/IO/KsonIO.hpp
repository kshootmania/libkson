#pragma once
#ifndef KSON_WITHOUT_JSON_DEPENDENCY
#include "kson/ChartData.hpp"

namespace kson
{
	inline constexpr std::int32_t kKSONFormatVersion = 1; // kson format version number (1 for kson 0.9.0)

	MetaChartData LoadKSONMetaChartData(std::istream& stream);

	MetaChartData LoadKSONMetaChartData(const std::string& filePath);

	ChartData LoadKSONChartData(std::istream& stream);

	ChartData LoadKSONChartData(const std::string& filePath);

	ErrorType SaveKSONChartData(std::ostream& stream, const ChartData& chartData);

	ErrorType SaveKSONChartData(const std::string& filePath, const ChartData& chartData);
}
#endif
