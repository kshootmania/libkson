#pragma once
#ifndef KSON_WITHOUT_JSON_DEPENDENCY
#include "kson/ChartData.hpp"
#include "kson/IO/KsonLoadingDiag.hpp"

namespace kson
{
	inline constexpr std::int32_t kKsonFormatVersion = 1; // kson format version number (1 for kson 0.9.0)

	ErrorType SaveKsonChartData(std::ostream& stream, const ChartData& chartData);

	ErrorType SaveKsonChartData(const std::string& filePath, const ChartData& chartData);

	ChartData LoadKsonChartData(std::istream& stream, KsonLoadingDiag* pKsonDiag = nullptr);

	ChartData LoadKsonChartData(const std::string& filePath, KsonLoadingDiag* pKsonDiag = nullptr);

	MetaChartData LoadKsonMetaChartData(std::istream& stream, KsonLoadingDiag* pKsonDiag = nullptr);

	MetaChartData LoadKsonMetaChartData(const std::string& filePath, KsonLoadingDiag* pKsonDiag = nullptr);
}
#endif
