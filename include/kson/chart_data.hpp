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

#ifndef KSON_WITHOUT_JSON_DEPENDENCY
#include "third_party/nlohmann/json.hpp"
#endif

namespace kson
{
	enum class Error : int
	{
		None = 0,

		GeneralIOError = 10000,
		FileNotFound = 10001,
		CouldNotOpenInputFileStream = 10002,
		CouldNotOpenOutputFileStream = 10003,

		GeneralChartFormatError = 20000,

		UnknownError = 90000,
	};

	struct MetaChartData
	{
		MetaInfo meta;
		MetaAudioInfo audio;
		MetaCompatInfo compat;

		std::string filePath; // Note: OS native encoding (Not UTF-8 in Windows)
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

		std::string filePath; // Note: OS native encoding (Not UTF-8 in Windows)
		Error error = Error::None;
	};
}
