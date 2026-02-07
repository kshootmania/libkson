#pragma once
#ifndef KSON_WITHOUT_JSON_DEPENDENCY
#include "kson/ChartData.hpp"
#include "kson/IO/KsonLoadingDiag.hpp"

namespace kson
{
	inline constexpr std::int32_t kKSONFormatVersion = 1; // kson format version number (1 for kson 0.9.0)

	ErrorType SaveKSONChartData(std::ostream& stream, const ChartData& chartData);

	ErrorType SaveKSONChartData(const std::string& filePath, const ChartData& chartData);

	ChartData LoadKSONChartData(std::istream& stream, KsonLoadingDiag* pKsonDiag = nullptr);

	ChartData LoadKSONChartData(const std::string& filePath, KsonLoadingDiag* pKsonDiag = nullptr);
}
#endif
