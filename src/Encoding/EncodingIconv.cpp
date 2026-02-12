#ifndef _WIN32
#include "kson/Encoding/Encoding.hpp"
#include <iostream>
#include <cerrno>
#include <iconv.h>

std::string kson::Encoding::ShiftJISToUTF8(std::string_view shiftJISStr)
{
	// Convert Shift-JIS (CP932) to UTF-8
	const iconv_t cd = iconv_open("UTF-8", "CP932");
	if (cd == (iconv_t)(-1))
	{
		std::cerr << "iconv_open error (errno:" << errno << "). The system may not support Shift-JIS to UTF-8 conversion.\n";
		return std::string();
	}

	std::string src(shiftJISStr);
	char *pSrc = src.data();
	std::size_t srcSize = src.size();
	std::string dst(shiftJISStr.size() * 4U, '\0'); // enough number
	char *pDst = dst.data();
	std::size_t dstSize = dst.size();
	if (iconv(cd, &pSrc, &srcSize, &pDst, &dstSize) == (size_t)-1)
	{
		const int errnoCopy = errno;
		iconv_close(cd);

		// Fallback to UTF-8 on conversion failure (e.g., UTF-8 without BOM)
		if (errnoCopy == EILSEQ || errnoCopy == EINVAL)
		{
			std::cerr << "Warning: iconv error (errno:" << errnoCopy << "). Input encoding may not be Shift-JIS. Assuming UTF-8.\n";
			return std::string(shiftJISStr);
		}

		std::cerr << "iconv error (errno:" << errnoCopy << "). Input encoding may not be Shift-JIS.\n";
		return std::string();
	}
	iconv_close(cd);

	return std::string(dst.data());
}
#endif
