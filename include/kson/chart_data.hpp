#pragma once
#include "kson/common/common.hpp"
#include "kson/meta/meta_info.hpp"
#include "kson/beat/beat_info.hpp"
#include "kson/gauge/gauge_info.hpp"
#include "kson/note/note_info.hpp"
#include "kson/audio/audio_info.hpp"
#include "kson/camera/camera_info.hpp"
#include "kson/bg/bg_info.hpp"
#include "kson/editor/editor_info.hpp"
#include "kson/compat/compat_info.hpp"
#include "kson/error.hpp"

#ifndef KSON_WITHOUT_JSON_DEPENDENCY
#include "third_party/nlohmann/json.hpp"
#endif

namespace kson
{
	struct MetaChartData
	{
		MetaInfo meta;
		MetaAudioInfo audio;
		MetaCompatInfo compat;

		Error error = Error::None;
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

		Error error = Error::None;
	};
}
