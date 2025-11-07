#pragma once
#include "kson/Common/Common.hpp"
#include "kson/ChartData.hpp"

namespace kson
{
	MetaChartData LoadKSHMetaChartData(std::istream& stream);

	MetaChartData LoadKSHMetaChartData(const std::string& filePath);

	ChartData LoadKSHChartData(std::istream& stream);

	ChartData LoadKSHChartData(const std::string& filePath);

	ErrorType SaveKSHChartData(std::ostream& stream, const ChartData& chartData);

	ErrorType SaveKSHChartData(const std::string& filePath, const ChartData& chartData);
}
