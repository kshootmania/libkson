#ifdef _WIN32
#include "kson/Encoding/Encoding.hpp"
#include <iostream>
#include <Windows.h>

namespace
{
	// Shift-JIS (Japanese) codepage
	// Note: CP_ACP is not used for non-Japanese locales.
	//       Almost all non-UTF8 legacy charts are in Japanese, so they are always considered Shift-JIS here.
	constexpr UINT kShiftJISCodePage = 932;
}

std::string kson::Encoding::ShiftJISToUTF8(std::string_view shiftJISStr)
{
	// Convert Shift-JIS to UTF-16
	const int requiredWstrSize = MultiByteToWideChar(kShiftJISCodePage, 0, shiftJISStr.data(), static_cast<int>(shiftJISStr.size()), nullptr, 0);
	std::wstring wstr(requiredWstrSize, L'\0');
	if (MultiByteToWideChar(kShiftJISCodePage, 0, shiftJISStr.data(), static_cast<int>(shiftJISStr.size()), wstr.data(), requiredWstrSize) == 0)
	{
		const DWORD lastError = GetLastError();

		// Fallback to UTF-8 on conversion failure (e.g., UTF-8 without BOM)
		if (lastError == ERROR_NO_UNICODE_TRANSLATION)
		{
			std::cerr << "Warning: MultiByteToWideChar error (GetLastError():" << lastError << "). Input encoding may not be Shift-JIS. Assuming UTF-8.\n";
			return std::string(shiftJISStr);
		}

		std::cerr << "MultiByteToWideChar error (GetLastError():" << lastError << "). Input encoding may not be Shift-JIS.\n";
		return std::string();
	}

	// Convert UTF-16 to UTF-8
	const int requiredStrSize = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, nullptr, 0, nullptr, nullptr);
	std::string str(requiredStrSize, '\0');
	if (WideCharToMultiByte(CP_UTF8, 0, wstr.data(), -1, reinterpret_cast<char *>(str.data()), requiredStrSize, nullptr, nullptr) == 0)
	{
		std::cerr << "WideCharToMultiByte error (GetLastError():" << GetLastError() << "). Input encoding may not be Shift-JIS.\n";
		return std::string();
	}

	// Remove an extra null terminator
	if (str.back() == '\0')
	{
		str.pop_back();
	}

	return str;
}
#endif
