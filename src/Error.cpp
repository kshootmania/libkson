#include "kson/Error.hpp"

const char *kson::GetErrorString(kson::ErrorType errorType)
{
	switch (errorType)
	{
	case kson::ErrorType::None:
		return "";
	case kson::ErrorType::GeneralIOError:
		return "IO error";
	case kson::ErrorType::FileNotFound:
		return "File not found";
	case kson::ErrorType::CouldNotOpenInputFileStream:
		return "Could not open input file stream";
	case kson::ErrorType::CouldNotOpenOutputFileStream:
		return "Could not open output file stream";
	case kson::ErrorType::GeneralChartFormatError:
		return "Chart format error";
	case kson::ErrorType::KsonParseError:
		return "KSON parse error";
	case kson::ErrorType::EncodingError:
		return "Encoding error";
	default:
		return "Unknown error";
	}
}
