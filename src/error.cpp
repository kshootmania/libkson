#include "kson/error.hpp"

const char *kson::GetErrorString(kson::Error error)
{
    switch (error)
    {
    case kson::Error::None:
        return "";
    case kson::Error::GeneralIOError:
        return "IO error";
    case kson::Error::FileNotFound:
        return "File not found";
    case kson::Error::CouldNotOpenInputFileStream:
        return "Could not open input file stream";
    case kson::Error::CouldNotOpenOutputFileStream:
        return "Could not open input file stream";
    case kson::Error::GeneralChartFormatError:
        return "Chart format error";
    case kson::Error::EncodingError:
        return "Encoding error";
    default:
        return "Unknown error";
    }
}
