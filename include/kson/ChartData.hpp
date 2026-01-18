#pragma once
#include "kson/Common/Common.hpp"
#include "kson/Meta/MetaInfo.hpp"
#include "kson/Beat/BeatInfo.hpp"
#include "kson/Gauge/GaugeInfo.hpp"
#include "kson/Note/NoteInfo.hpp"
#include "kson/Audio/AudioInfo.hpp"
#include "kson/Camera/CameraInfo.hpp"
#include "kson/BG/BGInfo.hpp"
#include "kson/Editor/EditorInfo.hpp"
#include "kson/Compat/CompatInfo.hpp"
#include "kson/Error.hpp"

#ifndef KSON_WITHOUT_JSON_DEPENDENCY
#include "third_party/nlohmann/json.hpp"
#endif

namespace kson
{
	struct MetaChartData
	{
		MetaInfo meta;
		MetaAudioInfo audio;

		ErrorType error = ErrorType::None;

		std::vector<std::string> warnings;
	};

	struct ChartData
	{
		MetaInfo meta;
		BeatInfo beat;
		GaugeInfo gauge;
		NoteInfo note;
		AudioInfo audio;
		CameraInfo camera;
		BGInfo bg;
		EditorInfo editor;
		CompatInfo compat;

#ifndef KSON_WITHOUT_JSON_DEPENDENCY
		nlohmann::json impl = nlohmann::json::object();
#endif

		ErrorType error = ErrorType::None;

		std::vector<std::string> warnings;
	};

	[[nodiscard]]
	ChartData CreateEditorDefaultChartData();
}
