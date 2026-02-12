#pragma once
#include "kson/Common/Common.hpp"

namespace kson
{
	struct KshLayerRotationInfo
	{
		bool tilt = false;
		bool spin = false;
	};

	struct KshBGInfo
	{
		std::string filename; // UTF-8 guaranteed
	};

	struct KshLayerInfo
	{
		std::string filename; // UTF-8 guaranteed
		std::int32_t duration = 0;
		KshLayerRotationInfo rotation = { .tilt = true, .spin = true };
	};

	struct KshMovieInfo
	{
		std::string filename; // UTF-8 guaranteed
		std::int32_t offset = 0;
	};

	struct LegacyBGInfo
	{
		std::array<KshBGInfo, 2> bg; // first index: when gauge < 70%, second index: when gauge >= 70%
		KshLayerInfo layer;
		KshMovieInfo movie;
	};

	struct BGInfo
	{
		std::string filename; // UTF-8 guaranteed; reserved for future extension and not used by KSM v2
		LegacyBGInfo legacy;
	};
}
