#pragma once

namespace kson
{
	enum class ErrorType : int
	{
		None = 0,

		GeneralIOError = 10000,
		FileNotFound = 10001,
		CouldNotOpenInputFileStream = 10002,
		CouldNotOpenOutputFileStream = 10003,

		GeneralChartFormatError = 20000,
		KSONParseError = 20001,

		EncodingError = 30000,

		UnknownError = 90000,
	};

	[[nodiscard]]
	const char *GetErrorString(ErrorType errorType);
}
