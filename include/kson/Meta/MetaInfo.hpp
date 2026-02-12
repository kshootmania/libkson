#pragma once
#include "kson/Common/Common.hpp"

namespace kson
{
	struct DifficultyInfo
	{
		std::int32_t idx = 0; // 0-3 in KSH
		std::string name; // empty if "meta.difficulty" is uint
	};

	struct MetaInfo
	{
		std::string title;
		std::string titleTranslit; // UTF-8 guaranteed
		std::string titleImgFilename; // UTF-8 guaranteed
		std::string artist;
		std::string artistTranslit; // UTF-8 guaranteed
		std::string artistImgFilename; // UTF-8 guaranteed
		std::string chartAuthor;
		DifficultyInfo difficulty;
		std::int32_t level = 1;
		std::string dispBPM;
		double stdBPM = 0.0;
		std::string jacketFilename; // UTF-8 guaranteed
		std::string jacketAuthor;
		std::string iconFilename; // UTF-8 guaranteed
		std::string information;
	};
}
