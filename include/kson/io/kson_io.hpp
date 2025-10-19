#pragma once
#ifndef KSON_WITHOUT_JSON_DEPENDENCY
#include "kson/chart_data.hpp"

namespace kson
{
	inline constexpr const char* kKSONFormatVersion = "0.9.0-beta1";

	ErrorType SaveKSONChartData(std::ostream& stream, const ChartData& chartData);

	ErrorType SaveKSONChartData(const std::string& filePath, const ChartData& chartData);

	ChartData LoadKSONChartData(std::istream& stream);

	ChartData LoadKSONChartData(const std::string& filePath);
}
#endif
