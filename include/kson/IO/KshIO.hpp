#pragma once
#include "kson/Common/Common.hpp"
#include "kson/ChartData.hpp"
#include "kson/IO/KshParserDiag.hpp"

namespace kson
{
	MetaChartData LoadKSHMetaChartData(std::istream& stream);

	MetaChartData LoadKSHMetaChartData(const std::string& filePath);

	ChartData LoadKSHChartData(std::istream& stream, KshParserDiag* pKshDiag = nullptr);

	ChartData LoadKSHChartData(const std::string& filePath, KshParserDiag* pKshDiag = nullptr);

	ErrorType SaveKSHChartData(std::ostream& stream, const ChartData& chartData);

	ErrorType SaveKSHChartData(const std::string& filePath, const ChartData& chartData);
}
