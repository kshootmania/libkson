#ifndef KSON_WITHOUT_JSON_DEPENDENCY
#include "kson/io/kson_io.hpp"
#include <fstream>
#include <optional>
#include <limits>
#include <cmath>

namespace
{
	using namespace kson;

	// Note: Since to_json() overload makes it difficult to find minor bugs
	//       (e.g., ByPulse<T> shouldn't be converted as std::map, Pulse and RelPulse shouldn't be the same),
	//       we write our own conversion functions here.

#ifdef __cpp_concepts
	template <class T>
	concept Serializable = requires (nlohmann::json & json, const T & d)
	{
		json = d;
	};
#endif

	void Write(nlohmann::json& json, const char* key, nlohmann::json&& value)
	{
		if (!value.is_object() || !value.empty())
		{
			json.emplace(key, std::move(value)); // Note: json becomes object if it is null
		}
	}

#ifdef __cpp_concepts
	template <Serializable T>
#else
	template <typename T>
#endif
	void Write(nlohmann::json& json, const char* key, const T& value)
	{
		if constexpr (
			std::is_same_v<T, std::string> ||
			std::is_integral_v<T>)
		{
			json.emplace(key, value);
		}
		else if constexpr (std::is_floating_point_v<T>)
		{
			json.emplace(key, RemoveFloatingPointError(value));
		}
		else if constexpr (std::is_same_v<T, nlohmann::json>)
		{
			if (!value.is_object() || !value.empty())
			{
				json.emplace(key, value);
			}
		}
		else
		{
			nlohmann::json valueJSON = value;
			if (!valueJSON.is_object() || !valueJSON.empty())
			{
				json.emplace(key, std::move(valueJSON));
			}
		}
	}

#ifdef __cpp_concepts
	template <Serializable T, typename U>
#else
	template <typename T, typename U>
#endif
	void Write(nlohmann::json& json, const char* key, const T& value, const U& defaultValue)
	{
		if constexpr (std::is_floating_point_v<T>)
		{
			static_assert(std::is_same_v<T, U>);

			if (!AlmostEquals(value, defaultValue))
			{
				Write(json, key, value);
			}
		}
		else
		{
			if (value != defaultValue)
			{
				Write(json, key, value);
			}
		}
	}

#ifdef __cpp_concepts
	template <Serializable T>
#else
	template <typename T>
#endif
	void WriteByPulseElement(nlohmann::json& arrayJSON, Pulse y, const T& value) // Note: y could be ry
	{
		if constexpr (
			std::is_same_v<T, std::string> ||
			std::is_integral_v<T>)
		{
			arrayJSON.push_back(nlohmann::json::array({ y, value }));
		}
		else if constexpr (std::is_floating_point_v<T>)
		{
			arrayJSON.push_back(nlohmann::json::array({ y, RemoveFloatingPointError(value) }));
		}
		else if constexpr (std::is_same_v<T, nlohmann::json>)
		{
			if (value.is_object() && value.empty())
			{
				arrayJSON.push_back(y);
			}
			else
			{
				arrayJSON.push_back(nlohmann::json::array({ y, value }));
			}
		}
		else
		{
			nlohmann::json valueJSON = value;
			if (valueJSON.is_object() && valueJSON.empty())
			{
				arrayJSON.push_back(y);
			}
			else
			{
				arrayJSON.push_back(nlohmann::json::array({ y, std::move(valueJSON) }));
			}
		}
	}

#ifdef __cpp_concepts
	template <Serializable T, typename U>
#else
	template <typename T, typename U>
#endif
	void WriteByPulseElement(nlohmann::json& arrayJSON, Pulse y, const T& value, const U& defaultValue) // Note: y could be ry
	{
		if constexpr (
			std::is_same_v<T, std::string> ||
			std::is_integral_v<T>)
		{
			if (value == defaultValue)
			{
				arrayJSON.push_back(y);
			}
			else
			{
				arrayJSON.push_back(nlohmann::json::array({ y, value }));
			}
		}
		else if constexpr (std::is_floating_point_v<T>)
		{
			static_assert(std::is_same_v<T, U>);
			
			if (AlmostEquals(value, defaultValue))
			{
				arrayJSON.push_back(y);
			}
			else
			{
				arrayJSON.push_back(nlohmann::json::array({ y, RemoveFloatingPointError(value) }));
			}
		}
		else
		{
			if (value == defaultValue)
			{
				arrayJSON.push_back(y);
			}
			else
			{
				// Pass to a no-default-value version of WriteByPulseElement in order to skip empty objects and empty arrays in addition to the defaults
				WriteByPulseElement(arrayJSON, y, value);
			}
		}
	}

	void WriteGraphPoint(nlohmann::json& graphJSON, Pulse y, const GraphValue& v) // Note: y could be ry
	{
		if (AlmostEquals(v.v, v.vf))
		{
			graphJSON.push_back(nlohmann::json::array({ y, RemoveFloatingPointError(v.v) }));
		}
		else
		{
			graphJSON.push_back(nlohmann::json::array({ y, nlohmann::json::array({ RemoveFloatingPointError(v.v), RemoveFloatingPointError(v.vf) }) }));
		}
	}

#ifdef __cpp_concepts
	template <Serializable T>
#else
	template <typename T>
#endif
	void WriteByPulse(nlohmann::json& json, const char* key, const ByPulse<T>& byPulse)
	{
		if (byPulse.empty())
		{
			return;
		}

		nlohmann::json& j = json[key];
		j = nlohmann::json::array();
		for (const auto& [y, v] : byPulse)
		{
			WriteByPulseElement(j, y, v);
		}
	}

#ifdef __cpp_concepts
	template <Serializable T>
#else
	template <typename T>
#endif
	void WriteByPulse(nlohmann::json& json, const char* key, const ByPulse<T>& byPulse, const T& defaultValue)
	{
		bool allDefaultOrEmpty = true;
		for (const auto& [y, v] : byPulse)
		{
			if constexpr (std::is_floating_point_v<T>)
			{
				if (!AlmostEquals(v, defaultValue))
				{
					allDefaultOrEmpty = false;
					break;
				}
			}
			else
			{
				if (v != defaultValue)
				{
					allDefaultOrEmpty = false;
					break;
				}
			}
		}
		if (allDefaultOrEmpty)
		{
			return;
		}

		WriteByPulse(json, key, byPulse);
	}

#ifdef __cpp_concepts
	template <Serializable T>
#else
	template <typename T>
#endif
	void WriteByPulseMulti(nlohmann::json& json, const char* key, const ByPulseMulti<T>& byPulse)
	{
		if (byPulse.empty())
		{
			return;
		}

		nlohmann::json& j = json[key];
		j = nlohmann::json::array();
		for (const auto& [y, v] : byPulse)
		{
			WriteByPulseElement(j, y, v);
		}
	}

	void WriteTimeSigByMeasureIdx(nlohmann::json& json, const char* key, const ByMeasureIdx<TimeSig>& byMeasureIdx)
	{
		if (byMeasureIdx.empty())
		{
			return;
		}

		nlohmann::json& j = json[key];
		j = nlohmann::json::array();
		for (const auto& [idx, timeSig] : byMeasureIdx)
		{
			j.push_back(nlohmann::json::array({
				idx,
				nlohmann::json::array({ timeSig.n, timeSig.d }),
			}));
		}
	}

	template <std::size_t N>
	void WriteButtonLanes(nlohmann::json& json, const char* key, const std::array<ByPulse<Interval>, N>& lanes)
	{
		// Skip if empty
		bool allEmpty = true;
		for (std::size_t i = 0U; i < N; ++i)
		{
			if (!lanes[i].empty())
			{
				allEmpty = false;
				break;
			}
		}
		if (allEmpty)
		{
			return;
		}

		nlohmann::json& j = json[key];
		j = nlohmann::json::array();
		for (std::size_t i = 0U; i < N; ++i)
		{
			nlohmann::json& a = j.emplace_back(nlohmann::json::array());
			for (const auto& [y, interval] : lanes[i])
			{
				WriteByPulseElement(a, y, interval.length, 0);
			}
		}
	}

	void WriteLaserLanes(nlohmann::json& json, const char* key, const LaserLane<LaserSection>& lanes)
	{
		// Skip if empty
		bool allEmpty = true;
		for (std::size_t i = 0U; i < kNumLaserLanesSZ; ++i)
		{
			if (!lanes[i].empty())
			{
				allEmpty = false;
				break;
			}
		}
		if (allEmpty)
		{
			return;
		}

		nlohmann::json& j = json[key];
		for (std::size_t i = 0U; i < kNumLaserLanesSZ; ++i)
		{
			nlohmann::json& laneJSON = j.emplace_back(nlohmann::json::array());
			for (const auto& [y, laserSection] : lanes[i])
			{
				if (laserSection.v.empty())
				{
					continue;
				}

				nlohmann::json a = nlohmann::json::array();
				for (const auto& [ry, v] : laserSection.v)
				{
					WriteGraphPoint(a, ry, v);
				}
				if (laserSection.w == kLaserXScale1x)
				{
					laneJSON.push_back(nlohmann::json::array({ y, std::move(a) }));
				}
				else
				{
					laneJSON.push_back(nlohmann::json::array({ y, std::move(a), laserSection.w }));
				}
			}
		}
	}

	void WriteGraph(nlohmann::json& json, const char* key, const Graph& graph)
	{
		if (graph.empty())
		{
			return;
		}

		nlohmann::json& j = json[key];
		j = nlohmann::json::array();
		for (const auto& [y, v] : graph)
		{
			WriteGraphPoint(j, y, v);
		}
	}

	void WriteGraph(nlohmann::json& json, const char* key, const Graph& graph, double defaultValue)
	{
		bool allDefaultOrEmpty = true;
		for (const auto& [y, v] : graph)
		{
			if (!AlmostEquals(v.v, defaultValue) || !AlmostEquals(v.vf, defaultValue))
			{
				allDefaultOrEmpty = false;
				break;
			}
		}
		if (allDefaultOrEmpty)
		{
			return;
		}

		WriteGraph(json, key, graph);
	}

	void WriteGraphSections(nlohmann::json& json, const char* key, const ByPulse<GraphSection>& graphSections)
	{
		if (graphSections.empty())
		{
			return;
		}

		nlohmann::json& j = json[key];
		j = nlohmann::json::array();
		for (const auto& [y, graphSection] : graphSections)
		{
			if (graphSection.v.empty())
			{
				continue;
			}

			nlohmann::json& graphJSON = j.emplace_back(nlohmann::json::array({ y, nlohmann::json::array() }))[1];
			for (const auto& [ry, v] : graphSection.v)
			{
				WriteGraphPoint(graphJSON, ry, v);
			}
		}
	}

	void WriteAudioEffectDef(nlohmann::json& json, const char* key, const std::vector<AudioEffectDefKVP>& def)
	{
		if (def.empty())
		{
			return;
		}

		nlohmann::json& j = json[key];
		for (const auto& kvp : def)
		{
			nlohmann::json defJSON = {
				{ "type", AudioEffectTypeToStr(kvp.v.type) },
			};
			Write(defJSON, "v", kvp.v.v);
			j.push_back(nlohmann::json::array({ kvp.name, std::move(defJSON) }));
		}
	}

	void WriteAudioEffectParamChange(nlohmann::json& json, const char* key, const Dict<Dict<ByPulse<std::string>>>& paramChange)
	{
		if (paramChange.empty())
		{
			return;
		}

		nlohmann::json& j1 = json[key];
		for (const auto& [audioEffectName, params] : paramChange)
		{
			if (params.empty())
			{
				continue;
			}

			nlohmann::json& j2 = j1[audioEffectName];
			for (const auto& [paramName, byPulse] : params)
			{
				WriteByPulse(j2, paramName.c_str(), byPulse);
			}
		}
	}

	void WriteCamPatternInvokeSpinByPulse(nlohmann::json& json, const char* key, const ByPulse<CamPatternInvokeSpin>& byPulse)
	{
		if (byPulse.empty())
		{
			return;
		}

		nlohmann::json& j = json[key];
		j = nlohmann::json::array();
		for (const auto& [y, invoke] : byPulse)
		{
			j.push_back(nlohmann::json::array({ y, invoke.d, invoke.length }));
		}
	}

	void WriteCamPatternInvokeSwingByPulse(nlohmann::json& json, const char* key, const ByPulse<CamPatternInvokeSwing>& byPulse)
	{
		if (byPulse.empty())
		{
			return;
		}

		nlohmann::json& j = json[key];
		j = nlohmann::json::array();
		for (const auto& [y, invoke] : byPulse)
		{
			nlohmann::json vJSON = nlohmann::json::object();
			{
				Write(vJSON, "scale", invoke.v.scale, 1.0);
				Write(vJSON, "repeat", invoke.v.repeat, 1);
				Write(vJSON, "decay_order", invoke.v.decayOrder, 0);
			}
			if (vJSON.empty())
			{
				j.push_back(nlohmann::json::array({ y, invoke.d, invoke.length }));
			}
			else
			{
				j.push_back(nlohmann::json::array({ y, invoke.d, invoke.length, std::move(vJSON) }));
			}
		}
	}

	nlohmann::json ToJSON(const MetaInfo& d)
	{
		nlohmann::json j = nlohmann::json::object();
		Write(j, "title", d.title);
		Write(j, "title_translit", d.titleTranslit, "");
		Write(j, "title_img_filename", d.titleImgFilename, "");
		Write(j, "artist", d.artist);
		Write(j, "artist_translit", d.artistTranslit, "");
		Write(j, "artist_img_filename", d.artistImgFilename, "");
		Write(j, "chart_author", d.chartAuthor);
		if (d.difficulty.name.empty())
		{
			Write(j, "difficulty", d.difficulty.idx);
		}
		else
		{
			Write(j, "difficulty", d.difficulty.name);
		}
		Write(j, "level", d.level);
		Write(j, "disp_bpm", d.dispBPM);
		Write(j, "std_bpm", d.stdBPM, 0.0);
		Write(j, "jacket_filename", d.jacketFilename, "");
		Write(j, "jacket_author", d.jacketAuthor, "");
		Write(j, "icon_filename", d.iconFilename, "");
		Write(j, "information", d.information, "");
		return j;
	}

	nlohmann::json ToJSON(const BeatInfo& d)
	{
		nlohmann::json j = nlohmann::json::object();
		WriteByPulse(j, "bpm", d.bpm);
		WriteTimeSigByMeasureIdx(j, "time_sig", d.timeSig);
		WriteGraph(j, "scroll_speed", d.scrollSpeed);
		return j;
	}

	nlohmann::json ToJSON(const GaugeInfo& d)
	{
		nlohmann::json j = nlohmann::json::object();
		Write(j, "total", d.total, 0);
		return j;
	}

	nlohmann::json ToJSON(const NoteInfo& d)
	{
		nlohmann::json j = nlohmann::json::object();
		WriteButtonLanes(j, "bt", d.bt);
		WriteButtonLanes(j, "fx", d.fx);
		WriteLaserLanes(j, "laser", d.laser);
		return j;
	}

	nlohmann::json ToJSON(const AudioInfo& d)
	{
		nlohmann::json j = nlohmann::json::object();
		{
			nlohmann::json bgmJSON = nlohmann::json::object();
			Write(bgmJSON, "filename", d.bgm.filename, "");
			Write(bgmJSON, "vol", d.bgm.vol, 1.0);
			Write(bgmJSON, "offset", d.bgm.offset, 0);
			{
				nlohmann::json previewJSON = nlohmann::json::object();
				Write(previewJSON, "offset", d.bgm.preview.offset);
				Write(previewJSON, "duration", d.bgm.preview.duration);
				Write(bgmJSON, "preview", std::move(previewJSON));
			}
			{
				nlohmann::json legacyJSON = nlohmann::json::object();
				if (!d.bgm.legacy.empty())
				{
					Write(legacyJSON, "fp_filenames", d.bgm.legacy.toStrArray());
				}
				Write(bgmJSON, "legacy", std::move(legacyJSON));
			}
			Write(j, "bgm", std::move(bgmJSON));
		}
		{
			nlohmann::json keySoundJSON = nlohmann::json::object();
			{
				nlohmann::json fxJSON = nlohmann::json::object();
				{
					nlohmann::json chipEventJSON = nlohmann::json::object();
					for (const auto& [filename, lanes] : d.keySound.fx.chipEvent)
					{
						bool isEmpty = true;
						for (const auto& lane : lanes)
						{
							if (!lane.empty())
							{
								isEmpty = false;
								break;
							}
						}
						if (isEmpty)
						{
							continue;
						}

						nlohmann::json lanesJSON = nlohmann::json::array();
						for (const auto& lane : lanes)
						{
							nlohmann::json laneJSON = nlohmann::json::array();
							for (const auto& [y, v] : lane)
							{
								nlohmann::json vJSON = nlohmann::json::object();
								{
									Write(vJSON, "vol", v.vol, 1.0);
								}
								WriteByPulseElement(laneJSON, y, vJSON);
							}
							lanesJSON.push_back(std::move(laneJSON));
						}
						chipEventJSON.emplace(filename, std::move(lanesJSON));
					}
					Write(keySoundJSON, "chip_event", std::move(chipEventJSON));
				}
				Write(keySoundJSON, "fx", std::move(fxJSON));
			}
			{
				nlohmann::json laserJSON = nlohmann::json::object();
				WriteByPulse(laserJSON, "vol", d.keySound.laser.vol);
				{
					nlohmann::json slamEventJSON = nlohmann::json::object();
					for (const auto& [filename, pulseSet] : d.keySound.laser.slamEvent)
					{
						if (pulseSet.empty())
						{
							continue;
						}

						nlohmann::json& pulseSetJSON = slamEventJSON[filename];
						pulseSetJSON = nlohmann::json::array();
						for (const auto& y : pulseSet)
						{
							pulseSetJSON.push_back(y);
						}
					}
					Write(laserJSON, "slam_event", std::move(slamEventJSON));
				}
				{
					nlohmann::json legacy = nlohmann::json::object();
					Write(legacy, "vol_auto", d.keySound.laser.legacy.volAuto, false);
					Write(laserJSON, "legacy", std::move(legacy));
				}
				Write(keySoundJSON, "laser", std::move(laserJSON));
			}
			Write(j, "key_sound", std::move(keySoundJSON));
		}
		{
			nlohmann::json audioEffectJSON = nlohmann::json::object();
			{
				nlohmann::json fxJSON = nlohmann::json::object();
				WriteAudioEffectDef(fxJSON, "def", d.audioEffect.fx.def);
				WriteAudioEffectParamChange(fxJSON, "param_change", d.audioEffect.fx.paramChange);
				{
					nlohmann::json longEventJSON = nlohmann::json::object();
					for (const auto& [audioEffectName, lanes] : d.audioEffect.fx.longEvent)
					{
						bool isEmpty = true;
						for (const auto& lane : lanes)
						{
							if (!lane.empty())
							{
								isEmpty = false;
								break;
							}
						}
						if (isEmpty)
						{
							continue;
						}

						nlohmann::json& lanesJSON = longEventJSON[audioEffectName];
						for (const auto& lane : lanes)
						{
							nlohmann::json& laneJSON = lanesJSON.emplace_back(nlohmann::json::array());
							for (const auto& [y, v] : lane)
							{
								WriteByPulseElement(laneJSON, y, v);
							}
						}
					}
					Write(fxJSON, "long_event", std::move(longEventJSON));
				}
				Write(audioEffectJSON, "fx", std::move(fxJSON));
			}
			{
				nlohmann::json laserJSON = nlohmann::json::object();
				WriteAudioEffectDef(laserJSON, "def", d.audioEffect.laser.def);
				WriteAudioEffectParamChange(laserJSON, "param_change", d.audioEffect.laser.paramChange);
				{
					nlohmann::json pulseEvent = nlohmann::json::object();
					for (const auto& [audioEffectName, pulseSet] : d.audioEffect.laser.pulseEvent)
					{
						if (pulseSet.empty())
						{
							continue;
						}

						nlohmann::json pulseSetJSON = nlohmann::json::array();
						for (const Pulse& pulse : pulseSet)
						{
							pulseSetJSON.push_back(pulse);
						}
						pulseEvent.emplace(audioEffectName, std::move(pulseSetJSON));
					}
					Write(laserJSON, "pulse_event", std::move(pulseEvent));
				}
				Write(laserJSON, "peaking_filter_delay", d.audioEffect.laser.peakingFilterDelay, 0);
				Write(audioEffectJSON, "laser", std::move(laserJSON));
			}
			Write(j, "audio_effect", std::move(audioEffectJSON));
		}

		return j;
	}

	nlohmann::json ToJSON(const CameraInfo& d)
	{
		nlohmann::json j = nlohmann::json::object();
		{
			nlohmann::json tiltJSON = nlohmann::json::object();
			WriteByPulse(tiltJSON, "scale", d.tilt.scale, 1.0);
			WriteGraphSections(tiltJSON, "manual", d.tilt.manual);
			WriteByPulse(tiltJSON, "keep", d.tilt.keep, false);
			Write(j, "tilt", std::move(tiltJSON));
		}
		{
			nlohmann::json camJSON = nlohmann::json::object();
			{
				nlohmann::json bodyJSON = nlohmann::json::object();
				WriteGraph(bodyJSON, "zoom", d.cam.body.zoom, 0.0);
				WriteGraph(bodyJSON, "shift_x", d.cam.body.shiftX, 0.0);
				WriteGraph(bodyJSON, "rotation_x", d.cam.body.rotationX, 0.0);
				WriteGraph(bodyJSON, "rotation_z", d.cam.body.rotationZ, 0.0);
				WriteGraph(bodyJSON, "center_split", d.cam.body.centerSplit, 0.0);
				Write(camJSON, "body", bodyJSON);
			}
			{
				nlohmann::json patternJSON = nlohmann::json::object();
				{
					nlohmann::json laserJSON = nlohmann::json::object();
					{
						nlohmann::json slamEventJSON = nlohmann::json::object();
						WriteCamPatternInvokeSpinByPulse(slamEventJSON, "spin", d.cam.pattern.laser.slamEvent.spin);
						WriteCamPatternInvokeSpinByPulse(slamEventJSON, "half_spin", d.cam.pattern.laser.slamEvent.halfSpin);
						WriteCamPatternInvokeSwingByPulse(slamEventJSON, "swing", d.cam.pattern.laser.slamEvent.swing);
						Write(laserJSON, "slam_event", std::move(slamEventJSON));
					}
					Write(camJSON, "laser", std::move(laserJSON));
				}
				Write(camJSON, "pattern", std::move(patternJSON));
			}
			Write(j, "cam", std::move(camJSON));
		}
		return j;
	}

	nlohmann::json ToJSON(const BGInfo& d)
	{
		nlohmann::json j = nlohmann::json::object();
		{
			Write(j, "filename", d.filename, "");
			Write(j, "offset", d.offset, 0);
			nlohmann::json legacyJSON = nlohmann::json::object();
			{
				nlohmann::json bgJSON = nlohmann::json::array();
				if (!d.legacy.bg[0].filename.empty())
				{
					bgJSON.push_back({
						{ "filename", d.legacy.bg[0].filename },
					});
				}
				if (!d.legacy.bg[1].filename.empty() && d.legacy.bg[0].filename != d.legacy.bg[1].filename)
				{
					bgJSON.push_back({
						{ "filename", d.legacy.bg[1].filename },
					});
				}
				Write(legacyJSON, "bg", std::move(bgJSON));
			}
			{
				nlohmann::json layerJSON = nlohmann::json::object();
				Write(layerJSON, "filename", d.legacy.layer.filename, "");
				Write(layerJSON, "duration", d.legacy.layer.duration, 0);
				{
					nlohmann::json rotationJSON = nlohmann::json::object();
					Write(rotationJSON, "tilt", d.legacy.layer.rotation.tilt, true);
					Write(rotationJSON, "spin", d.legacy.layer.rotation.spin, true);
					Write(layerJSON, "rotation", std::move(rotationJSON));
				}
				Write(legacyJSON, "layer", std::move(layerJSON));
			}
			{
				nlohmann::json movieJSON = nlohmann::json::object();
				Write(movieJSON, "filename", d.legacy.movie.filename, "");
				Write(movieJSON, "offset", d.legacy.movie.offset, 0);
				Write(legacyJSON, "movie", std::move(movieJSON));
			}
			Write(j, "legacy", std::move(legacyJSON));
		}
		return j;
	}

	nlohmann::json ToJSON(const EditorInfo& d)
	{

		nlohmann::json j = nlohmann::json::object();
		Write(j, "app_name", d.appName, "");
		Write(j, "app_version", d.appVersion, "");
		WriteByPulse(j, "comment", d.comment);
		return j;
	}

	nlohmann::json ToJSON(const CompatInfo& d)
	{
		nlohmann::json j = nlohmann::json::object();
		Write(j, "ksh_version", d.kshVersion, "");
		{
			nlohmann::json kshUnknownJSON = nlohmann::json::object();
			Write(kshUnknownJSON, "meta", d.kshUnknown.meta);
			{
				nlohmann::json optionJSON = nlohmann::json::object();
				for (const auto& [key, value] : d.kshUnknown.option)
				{
					WriteByPulseMulti(optionJSON, key.c_str(), value);
				}
				Write(kshUnknownJSON, "option", std::move(optionJSON));
			}
			WriteByPulseMulti(kshUnknownJSON, "line", d.kshUnknown.line);
			Write(j, "ksh_unknown", std::move(kshUnknownJSON));
		}
		return j;
	}
}

kson::Error kson::SaveKSONChartData(std::ostream& stream, const ChartData& chartData)
{
	if (!stream.good())
	{
		return Error::GeneralIOError;
	}

	nlohmann::json json = nlohmann::json::object();
	Write(json, "version", "0.7.0");
	Write(json, "meta", ToJSON(chartData.meta));
	Write(json, "beat", ToJSON(chartData.beat));
	Write(json, "gauge", ToJSON(chartData.gauge));
	Write(json, "note", ToJSON(chartData.note));
	Write(json, "audio", ToJSON(chartData.audio));
	Write(json, "camera", ToJSON(chartData.camera));
	Write(json, "bg", ToJSON(chartData.bg));
	Write(json, "editor", ToJSON(chartData.editor));
	Write(json, "compat", ToJSON(chartData.compat));
	Write(json, "impl", chartData.impl);
	
	stream << json.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace);

	return Error::None;
}

kson::Error kson::SaveKSONChartData(const std::string& filePath, const ChartData& chartData)
{
	std::ofstream ofs(filePath);
	if (!ofs.good())
	{
		return Error::CouldNotOpenOutputFileStream;
	}
	return kson::SaveKSONChartData(ofs, chartData);
}
#endif
