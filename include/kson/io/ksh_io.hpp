#pragma once
#include "kson/common/common.hpp"
#include "kson/chart_data.hpp"

namespace kson
{
	MetaChartData LoadKSHMetaChartData(std::istream& stream);

	MetaChartData LoadKSHMetaChartData(const std::string& filePath);

	ChartData LoadKSHChartData(std::istream& stream);

	ChartData LoadKSHChartData(const std::string& filePath);

	bool SaveKSHChartData(std::ostream& stream, const ChartData& chartData);

	bool SaveKSHChartData(const std::string& filePath, const ChartData& chartData);
}
