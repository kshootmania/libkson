#ifndef _WIN32
#include "kson/encoding/encoding.hpp"
#include <cerrno>
#include <iconv.h>

std::string kson::Encoding::ShiftJISToUTF8(std::string_view shiftJISStr)
{
	// Convert Shift-JIS (CP932) to UTF-8
	const iconv_t cd = iconv_open("UTF-8", "CP932");
	if (cd == (iconv_t)(-1))
	{
		return "!!! iconv_open error (errno:" + std::to_string(errno) + ") !!!"; // TODO: error handling
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
		return "!!! iconv error (errno:" + std::to_string(errnoCopy) + ") !!!"; // TODO: error handling
	}
	iconv_close(cd);

	return std::string(dst.data());
}

#endif
