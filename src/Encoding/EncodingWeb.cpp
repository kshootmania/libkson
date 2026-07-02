#ifdef __EMSCRIPTEN__
#include "kson/Encoding/Encoding.hpp"
#include <cstdlib>
#include <emscripten.h>

extern "C"
{
	// Allocate memory for EM_JS without relying on exported _malloc
	EMSCRIPTEN_KEEPALIVE char* kson_encoding_alloc(std::size_t size)
	{
		return static_cast<char*>(std::malloc(size));
	}
}

namespace
{
	// Convert Shift-JIS (CP932) to UTF-8 with the browser TextDecoder
	EM_JS(char*, ShiftJISToUTF8Impl, (const char* pSrc, std::size_t srcSize), {
		try {
			const srcView = new Uint8Array(HEAPU8.buffer, pSrc, srcSize);
			const decodedStr = new TextDecoder('shift_jis', { fatal: true }).decode(srcView);
			const utf8Bytes = new TextEncoder().encode(decodedStr);
			const pDst = _kson_encoding_alloc(utf8Bytes.length + 1);
			if (pDst === 0) {
				return 0;
			}
			HEAPU8.set(utf8Bytes, pDst);
			HEAPU8[pDst + utf8Bytes.length] = 0;
			return pDst;
		} catch (e) {
			return 0;
		}
	});
}

std::string kson::Encoding::ShiftJISToUTF8(std::string_view shiftJISStr)
{
	char* pConverted = ShiftJISToUTF8Impl(shiftJISStr.data(), shiftJISStr.size());
	if (pConverted == nullptr)
	{
		// Fallback to UTF-8 on conversion failure (e.g., UTF-8 without BOM)
		return std::string{ shiftJISStr };
	}
	std::string result{ pConverted };
	std::free(pConverted);
	return result;
}
#endif
