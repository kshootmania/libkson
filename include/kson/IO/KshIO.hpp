#pragma once
#include "kson/Common/Common.hpp"
#include "kson/ChartData.hpp"
#include "kson/IO/KshLoadingDiag.hpp"
#include "kson/IO/KshSavingDiag.hpp"

namespace kson
{
	MetaChartData LoadKSHMetaChartData(std::istream& stream);

	MetaChartData LoadKSHMetaChartData(const std::string& filePath);

	ChartData LoadKSHChartData(std::istream& stream, KshLoadingDiag* pKshDiag = nullptr);

	ChartData LoadKSHChartData(const std::string& filePath, KshLoadingDiag* pKshDiag = nullptr);

	ErrorType SaveKSHChartData(std::ostream& stream, const ChartData& chartData, KshSavingDiag* pKshSavingDiag = nullptr);

	ErrorType SaveKSHChartData(const std::string& filePath, const ChartData& chartData, KshSavingDiag* pKshSavingDiag = nullptr);
}
