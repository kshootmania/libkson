#pragma once
#ifndef KSON_WITHOUT_JSON_DEPENDENCY
#include "kson/chart_data.hpp"

namespace kson
{
	ErrorType SaveKSONChartData(std::ostream& stream, const ChartData& chartData);

	ErrorType SaveKSONChartData(const std::string& filePath, const ChartData& chartData);
}
#endif
