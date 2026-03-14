#include "kson/Audio/AudioEffect.hpp"

namespace
{
	using namespace kson;

	const std::unordered_map<std::string_view, AudioEffectType> s_strToAudioEffectType
	{
		{ "retrigger", AudioEffectType::Retrigger },
		{ "gate", AudioEffectType::Gate },
		{ "flanger", AudioEffectType::Flanger },
		{ "pitch_shift", AudioEffectType::PitchShift },
		{ "bitcrusher", AudioEffectType::Bitcrusher },
		{ "phaser", AudioEffectType::Phaser },
		{ "wobble", AudioEffectType::Wobble },
		{ "tapestop", AudioEffectType::Tapestop },
		{ "echo", AudioEffectType::Echo },
		{ "sidechain", AudioEffectType::Sidechain },
		{ "switch_audio", AudioEffectType::SwitchAudio },
		{ "high_pass_filter", AudioEffectType::HighPassFilter },
		{ "low_pass_filter", AudioEffectType::LowPassFilter },
		{ "peaking_filter", AudioEffectType::PeakingFilter },
	};

	// Parameter order for each audio effect type according to the format specification
	const std::unordered_map<AudioEffectType, std::vector<std::string_view>> s_audioEffectParamOrder
	{
		{ AudioEffectType::Retrigger, { "update_period", "wave_length", "rate", "update_trigger", "mix" } },
		{ AudioEffectType::Gate, { "wave_length", "rate", "mix" } },
		{ AudioEffectType::Flanger, { "period", "delay", "depth", "feedback", "stereo_width", "vol", "mix" } },
		{ AudioEffectType::PitchShift, { "pitch", "chunk_size", "overlap", "mix" } },
		{ AudioEffectType::Bitcrusher, { "reduction", "mix" } },
		{ AudioEffectType::Phaser, { "period", "stage", "freq_1", "freq_2", "q", "feedback", "stereo_width", "hi_cut_gain", "mix" } },
		{ AudioEffectType::Wobble, { "wave_length", "freq_1", "freq_2", "q", "mix" } },
		{ AudioEffectType::Tapestop, { "speed", "trigger", "mix" } },
		{ AudioEffectType::Echo, { "update_period", "wave_length", "update_trigger", "feedback_level", "mix" } },
		{ AudioEffectType::Sidechain, { "period", "hold_time", "attack_time", "release_time", "ratio" } },
		{ AudioEffectType::SwitchAudio, { "filename" } },
		{ AudioEffectType::HighPassFilter, { "v", "freq", "freq_max", "q", "mix" } },
		{ AudioEffectType::LowPassFilter, { "v", "freq", "freq_max", "q", "mix" } },
		{ AudioEffectType::PeakingFilter, { "v", "freq", "freq_max", "bandwidth", "gain", "mix" } },
	};

	const std::unordered_map<AudioEffectType, std::string_view> s_audioEffectTypeToStr
	{
		{ AudioEffectType::Retrigger, "retrigger" },
		{ AudioEffectType::Gate, "gate" },
		{ AudioEffectType::Flanger, "flanger" },
		{ AudioEffectType::PitchShift, "pitch_shift" },
		{ AudioEffectType::Bitcrusher, "bitcrusher" },
		{ AudioEffectType::Phaser, "phaser" },
		{ AudioEffectType::Wobble, "wobble" },
		{ AudioEffectType::Tapestop, "tapestop" },
		{ AudioEffectType::Echo, "echo" },
		{ AudioEffectType::Sidechain, "sidechain" },
		{ AudioEffectType::SwitchAudio, "switch_audio" },
		{ AudioEffectType::HighPassFilter, "high_pass_filter" },
		{ AudioEffectType::LowPassFilter, "low_pass_filter" },
		{ AudioEffectType::PeakingFilter, "peaking_filter" },
	};
}

kson::AudioEffectType kson::StrToAudioEffectType(std::string_view str)
{
	if (s_strToAudioEffectType.contains(str))
	{
		return s_strToAudioEffectType.at(str);
	}
	else
	{
		return AudioEffectType::Unspecified;
	}
}

std::string_view kson::AudioEffectTypeToStr(AudioEffectType type)
{
	if (s_audioEffectTypeToStr.contains(type))
	{
		return s_audioEffectTypeToStr.at(type);
	}
	else
	{
		return "";
	}
}

bool kson::AudioEffectFXInfo::defContains(std::string_view name) const
{
	// Note: This is inefficient, so we recommend caching the result of defAsDict if this is used often
	return std::any_of(def.begin(), def.end(), [name](const auto& kvp) { return kvp.name == name; });
}

const AudioEffectDef& kson::AudioEffectFXInfo::defByName(std::string_view name) const
{
	// Note: This is inefficient, so we recommend caching the result of defAsDict if this is used often
	const auto it = std::find_if(def.begin(), def.end(), [name](const auto& kvp) { return kvp.name == name; });
	if (it == def.end())
	{
		throw std::runtime_error("audio.audio_effect.fx.def does not contain name '" + std::string(name) + "'");
	}
	return it->v;
}

Dict<AudioEffectDef> kson::AudioEffectFXInfo::defAsDict() const
{
	Dict<AudioEffectDef> ret;
	for (const auto& kvp : def)
	{
		ret.emplace(kvp.name, kvp.v);
	}
	return ret;
}

bool kson::AudioEffectLaserInfo::defContains(std::string_view name) const
{
	// Note: If you call this function frequently, it's recommended to first call defAsDict to get the dictionary and use it,
	//       as this function uses linear search.
	return std::any_of(def.begin(), def.end(), [name](const auto& kvp) { return kvp.name == name; });
}

const AudioEffectDef& kson::AudioEffectLaserInfo::defByName(std::string_view name) const
{
	// Note: If you call this function frequently, it's recommended to first call defAsDict to get the dictionary and use it,
	//       as this function uses linear search.
	const auto it = std::find_if(def.begin(), def.end(), [name](const auto& kvp) { return kvp.name == name; });
	if (it == def.end())
	{
		throw std::runtime_error("audio.audio_effect.laser.def does not contain name '" + std::string(name) + "'");
	}
	return it->v;
}

Dict<AudioEffectDef> kson::AudioEffectLaserInfo::defAsDict() const
{
	Dict<AudioEffectDef> ret;
	for (const auto& kvp : def)
	{
		ret.emplace(kvp.name, kvp.v);
	}
	return ret;
}

std::vector<std::string> kson::SortAudioEffectParamNames(AudioEffectType type, const AudioEffectParams& params)
{
	std::vector<std::string> result;
	result.reserve(params.size());

	std::set<std::string> added;

	// Spec order
	if (const auto it = s_audioEffectParamOrder.find(type); it != s_audioEffectParamOrder.end())
	{
		for (const auto& name : it->second)
		{
			const std::string nameStr(name);
			if (params.contains(nameStr))
			{
				result.push_back(nameStr);
				added.insert(nameStr);
			}
		}
	}

	// Remaining in alphabetical order
	for (const auto& [name, _] : params)
	{
		if (!added.contains(name))
		{
			result.push_back(name);
		}
	}

	return result;
}
