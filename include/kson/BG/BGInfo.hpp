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

	enum class KshMoviePlacement
	{
		kPanel,
		kBehindLayer,
		kInFrontOfLayer,
	};

	enum class KshMovieBlendMode
	{
		kNormal,
		kAdditive,
	};

	struct KshMovieRotationInfo
	{
		bool tilt = false;
		bool spin = false;
	};

	struct KshMovieInfo
	{
		std::string filename; // UTF-8 guaranteed
		std::int32_t offset = 0;
		bool loop = false;
		KshMoviePlacement placement = KshMoviePlacement::kPanel;
		KshMovieBlendMode blendMode = KshMovieBlendMode::kNormal;
		double opacity = 1.0;
		double scale = 1.0;
		double x = 0.0; // in units of the screen height
		double y = 0.0; // in units of the screen height
		KshMovieRotationInfo rotation;
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
