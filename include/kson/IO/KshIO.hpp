#pragma once
#include "kson/Common/Common.hpp"
#include "kson/ChartData.hpp"
#include "kson/IO/KshLoadingDiag.hpp"
#include "kson/IO/KshSavingDiag.hpp"

namespace kson
{
	MetaChartData LoadKshMetaChartData(std::istream& stream);

	MetaChartData LoadKshMetaChartData(const std::string& filePath);

	ChartData LoadKshChartData(std::istream& stream, KshLoadingDiag* pKshDiag = nullptr);

	ChartData LoadKshChartData(const std::string& filePath, KshLoadingDiag* pKshDiag = nullptr);

	ErrorType SaveKshChartData(std::ostream& stream, const ChartData& chartData, KshSavingDiag* pKshSavingDiag = nullptr);

	ErrorType SaveKshChartData(const std::string& filePath, const ChartData& chartData, KshSavingDiag* pKshSavingDiag = nullptr);
}
