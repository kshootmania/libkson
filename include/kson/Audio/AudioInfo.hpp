#pragma once
#include "kson/Common/Common.hpp"
#include "BGMInfo.hpp"
#include "KeySound.hpp"
#include "AudioEffect.hpp"

namespace kson
{
	struct AudioInfo
	{
		BGMInfo bgm;
		KeySoundInfo keySound;
		AudioEffectInfo audioEffect;
	};

	struct MetaAudioInfo
	{
		MetaBGMInfo bgm;
	};
}
