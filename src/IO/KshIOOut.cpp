#include "kson/IO/KshIO.hpp"
#include "kson/Util/GraphUtils.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <numeric>
#include <cmath>
#include <iostream>
#include <limits>

namespace
{
	using namespace kson;

	// KSH resolution (192 pulses per 4/4 measure)
	constexpr Pulse kKSHResolution4 = 192;
	static_assert(kResolution4 % kKSHResolution4 == 0, "kResolution4 must be divisible by kKSHResolution4");

	// Convert pulse value from KSON resolution to KSH resolution
	constexpr std::int32_t ToKSHResolution(Pulse pulse)
	{
		return static_cast<std::int32_t>(pulse * kKSHResolution4 / kResolution4);
	}

	constexpr char kOptionSeparator = '=';
	constexpr char kBlockSeparator = '|';
	constexpr std::string_view kMeasureSeparator = "--";
	constexpr char kAudioEffectStrSeparator = ';';

	constexpr std::int32_t kLaserXMax = 50;

	constexpr double kManualTiltAbsMax = 1000.0;
	constexpr double kZoomAbsMaxLegacy = 300.0; // ver < 1.67
	constexpr double kZoomAbsMax = 65535.0; // ver >= 1.67
	constexpr double kCenterSplitAbsMax = 65535.0;
	constexpr double kRotationDegAbsMax = 65535.0;

	constexpr std::int32_t kRotationFlagTilt = 1 << 0;
	constexpr std::int32_t kRotationFlagSpin = 1 << 1;

	// Maximum value of BPM
	constexpr double kBPMMax = 65535.0; // ver >= 130

	constexpr std::int32_t kVerBPMLimitAdded = 130;
	constexpr std::int32_t kVerFXFormatChanged = 160; // FX format changed (alphabets to fx-l/fx-r)
	constexpr std::int32_t kVerLayerDelimiterChanged = 166; // Layer delimiter changed from "/" to ";"
	constexpr std::int32_t kVerManualTiltScaleChanged = 170; // Manual tilt scale changed from 14 degrees to 10 degrees

	// KSON to KSH parameter name mapping (reverse of s_audioEffectParamNameTable)
	const std::unordered_map<std::string_view, std::string_view> kKSONToKSHParamName
	{
		{ "attack_time", "attackTime" },
		{ "bandwidth", "bandwidth" },
		{ "chunk_size", "chunkSize" },
		{ "delay", "delay" },
		{ "depth", "depth" },
		{ "feedback", "feedback" },
		{ "feedback_level", "feedbackLevel" },
		{ "filename", "fileName" },
		{ "freq", "freq" },
		{ "freq_max", "freqMax" },
		{ "freq_2", "hiFreq" },
		{ "freq_1", "loFreq" },
		{ "gain", "gain" },
		{ "hi_cut_gain", "hiCutGain" },
		{ "hold_time", "holdTime" },
		{ "mix", "mix" },
		{ "overlap", "overWrap" },
		{ "period", "period" },
		{ "pitch", "pitch" },
		{ "q", "Q" },
		{ "rate", "rate" },
		{ "ratio", "ratio" },
		{ "reduction", "reduction" },
		{ "release_time", "releaseTime" },
		{ "speed", "speed" },
		{ "stage", "stage" },
		{ "stereo_width", "stereoWidth" },
		{ "trigger", "trigger" },
		{ "update_trigger", "updateTrigger" },
		{ "v", "v" },
		{ "vol", "volume" },
		{ "wave_length", "waveLength" },
		{ "update_period", "updatePeriod" },
	};

	// KSON to KSH AudioEffectType name mapping (for #define_fx and #define_filter type=)
	const std::unordered_map<std::string_view, std::string_view> kKSONToKSHAudioEffectTypeName
	{
		{ "retrigger", "Retrigger" },
		{ "gate", "Gate" },
		{ "flanger", "Flanger" },
		{ "pitch_shift", "PitchShift" },
		{ "bitcrusher", "BitCrusher" },
		{ "phaser", "Phaser" },
		{ "wobble", "Wobble" },
		{ "tapestop", "TapeStop" },
		{ "echo", "Echo" },
		{ "sidechain", "SideChain" },
		{ "switch_audio", "SwitchAudio" },
		{ "high_pass_filter", "HighPassFilter" },
		{ "low_pass_filter", "LowPassFilter" },
		{ "peaking_filter", "PeakingFilter" },
	};

	// KSON to KSH preset FX effect name mapping (for fx-l/fx-r)
	const std::unordered_map<std::string_view, std::string_view> kKSONToKSHPresetFXEffectName
	{
		{ "retrigger", "Retrigger" },
		{ "gate", "Gate" },
		{ "flanger", "Flanger" },
		{ "pitch_shift", "PitchShift" },
		{ "bitcrusher", "BitCrusher" },
		{ "phaser", "Phaser" },
		{ "wobble", "Wobble" },
		{ "tapestop", "TapeStop" },
		{ "echo", "Echo" },
		{ "sidechain", "SideChain" },
		{ "switch_audio", "SwitchAudio" },
	};

	// KSON to KSH preset laser filter name mapping (for filtertype= and filter:)
	const std::unordered_map<std::string_view, std::string_view> kKSONToKSHPresetFilterName
	{
		{ "peaking_filter", "peak" },
		{ "low_pass_filter", "lpf1" },
		{ "high_pass_filter", "hpf1" },
		{ "bitcrusher", "bitc" },
	};

	// Preset effect name to AudioEffectType mapping
	const std::unordered_map<std::string_view, AudioEffectType> kPresetEffectTypeMap
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

	// GraphValue (0.0-1.0) to LaserX (0-50)
	constexpr std::int32_t GraphValueToLaserX(double graphValue, bool wide)
	{
		// Handle special wide laser positions
		if (wide)
		{
			// Left laser center position (C)
			if (AlmostEquals(graphValue, 0.25))
			{
				return 12; // 'C'
			}
			// Right laser center position (b)
			if (AlmostEquals(graphValue, 0.75))
			{
				return 37; // 'b'
			}
		}

		const std::int32_t laserX = static_cast<std::int32_t>(std::round(graphValue * kLaserXMax));
		return std::clamp(laserX, 0, kLaserXMax);
	}

	// LaserX (0-50) to char
	constexpr char LaserXToChar(std::int32_t laserX)
	{
		if (laserX >= 0 && laserX <= 9)
		{
			return '0' + static_cast<char>(laserX);
		}
		else if (laserX >= 10 && laserX <= 35)
		{
			return 'A' + static_cast<char>(laserX - 10);
		}
		else if (laserX >= 36 && laserX <= 50)
		{
			return 'a' + static_cast<char>(laserX - 36);
		}
		else
		{
			return '0';
		}
	}

	// Check if the KSON effect name is a preset laser audio effect
	bool IsKSONPresetLaserFilterName(const std::string& effectName)
	{
		return kKSONToKSHPresetFilterName.contains(effectName);
	}

	// Check if the KSON effect name is a preset FX effect
	bool IsKSONPresetFXEffectName(const std::string& effectName)
	{
		return kKSONToKSHPresetFXEffectName.contains(effectName);
	}

	std::string_view KSONPresetLaserFilterNameToKSH(const std::string& effectName);

	const char* AutoTiltTypeToString(AutoTiltType type)
	{
		switch (type)
		{
		case AutoTiltType::kNormal: return "normal";
		case AutoTiltType::kBigger: return "bigger";
		case AutoTiltType::kBiggest: return "biggest";
		case AutoTiltType::kKeepNormal: return "keep_normal";
		case AutoTiltType::kKeepBigger: return "keep_bigger";
		case AutoTiltType::kKeepBiggest: return "keep_biggest";
		case AutoTiltType::kZero: return "zero";
		default: return "normal";
		}
	}

	// Write UTF-8 BOM
	void WriteBOM(std::ostream& stream)
	{
		const unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
		stream.write(reinterpret_cast<const char*>(bom), sizeof(bom));
	}

	// Round double value to 3 decimal places (0.001 precision) for KSH format
	double RoundToKSHDoubleValue(double value)
	{
		return std::round(value * 1000.0) / 1000.0;
	}

	// Format double value (remove trailing zeros)
	std::string FormatDouble(double value)
	{
		value = RoundToKSHDoubleValue(value);

		std::ostringstream oss;
		oss << std::fixed << std::setprecision(3) << value;
		std::string str = oss.str();

		// Remove trailing zeros
		str.erase(str.find_last_not_of('0') + 1, std::string::npos);

		// Remove trailing decimal point
		if (!str.empty() && str.back() == '.')
		{
			str.pop_back();
		}

		return str;
	}

	// Convert RelPulse to KSH length string
	std::string RelPulseToKSHLength(RelPulse relPulse)
	{
		const std::int32_t kshLength = ToKSHResolution(relPulse);
		return std::to_string(kshLength);
	}

	// State for measure export
	struct MeasureExportState
	{
		TimeSig currentTimeSig{ 4, 4 };
		std::string headerBPMStr; // BPM string output in header

		// Laser state
		struct LaserState
		{
			bool active = false;
			double lastValue = 0.0;
		};
		std::array<LaserState, kNumLaserLanesSZ> laserStates;

		// Laser audio effect state (output only when changed)
		std::string currentFilterType;
		std::int32_t currentChokkakuvol = 50;
		std::int32_t currentPfiltergain = 50;

		// FX audio effect state (output only when changed)
		std::array<std::string, kNumFXLanesSZ> currentFXAudioEffects;

		MeasureExportState()
		{
			currentFXAudioEffects.fill("");
		}
	};

	// Calculate maximum pulse in chart data
	Pulse CalculateMaxPulse(const ChartData& chartData)
	{
		Pulse maxPulse = 0;

		// BT notes
		for (const auto& lane : chartData.note.bt)
		{
			if (!lane.empty())
			{
				const auto& [pulse, interval] = *lane.rbegin();
				maxPulse = std::max(maxPulse, pulse + interval.length);
			}
		}

		// FX notes
		for (const auto& lane : chartData.note.fx)
		{
			if (!lane.empty())
			{
				const auto& [pulse, interval] = *lane.rbegin();
				maxPulse = std::max(maxPulse, pulse + interval.length);
			}
		}

		// Laser notes
		for (const auto& lane : chartData.note.laser)
		{
			if (!lane.empty())
			{
				const auto& [pulse, section] = *lane.rbegin();
				if (!section.v.empty())
				{
					const RelPulse sectionLength = section.v.rbegin()->first;
					maxPulse = std::max(maxPulse, pulse + sectionLength);
				}
			}
		}

		// BPM changes
		if (!chartData.beat.bpm.empty())
		{
			maxPulse = std::max(maxPulse, chartData.beat.bpm.rbegin()->first);
		}

		// Time signature changes
		if (!chartData.beat.timeSig.empty())
		{
			const std::int64_t lastMeasureIdx = chartData.beat.timeSig.rbegin()->first;
			Pulse pulseAtLastTimeSig = 0;
			for (std::int64_t idx = 0; idx <= lastMeasureIdx; ++idx)
			{
				const TimeSig ts = ValueAtOrDefault(chartData.beat.timeSig, idx, TimeSig{ 4, 4 });
				if (idx == lastMeasureIdx)
				{
					break;
				}
				if (ts.d != 0)
				{
					pulseAtLastTimeSig += kResolution4 * ts.n / ts.d;
				}
			}
			maxPulse = std::max(maxPulse, pulseAtLastTimeSig);
		}

		// Stops
		if (!chartData.beat.stop.empty())
		{
			maxPulse = std::max(maxPulse, chartData.beat.stop.rbegin()->first);
		}

		// Scroll speed changes
		if (!chartData.beat.scrollSpeed.empty())
		{
			maxPulse = std::max(maxPulse, chartData.beat.scrollSpeed.rbegin()->first);
		}

		// Camera rotation
		if (!chartData.camera.cam.body.rotationDeg.empty())
		{
			maxPulse = std::max(maxPulse, chartData.camera.cam.body.rotationDeg.rbegin()->first);
		}

		// Camera zoom
		if (!chartData.camera.cam.body.zoomTop.empty())
		{
			maxPulse = std::max(maxPulse, chartData.camera.cam.body.zoomTop.rbegin()->first);
		}
		if (!chartData.camera.cam.body.zoomBottom.empty())
		{
			maxPulse = std::max(maxPulse, chartData.camera.cam.body.zoomBottom.rbegin()->first);
		}
		if (!chartData.camera.cam.body.zoomSide.empty())
		{
			maxPulse = std::max(maxPulse, chartData.camera.cam.body.zoomSide.rbegin()->first);
		}
		if (!chartData.camera.cam.body.centerSplit.empty())
		{
			maxPulse = std::max(maxPulse, chartData.camera.cam.body.centerSplit.rbegin()->first);
		}

		// Camera tilt
		if (!chartData.camera.tilt.empty())
		{
			maxPulse = std::max(maxPulse, chartData.camera.tilt.rbegin()->first);
		}

		// Spin events
		if (!chartData.camera.cam.pattern.laser.slamEvent.spin.empty())
		{
			maxPulse = std::max(maxPulse, chartData.camera.cam.pattern.laser.slamEvent.spin.rbegin()->first);
		}
		if (!chartData.camera.cam.pattern.laser.slamEvent.halfSpin.empty())
		{
			maxPulse = std::max(maxPulse, chartData.camera.cam.pattern.laser.slamEvent.halfSpin.rbegin()->first);
		}
		if (!chartData.camera.cam.pattern.laser.slamEvent.swing.empty())
		{
			maxPulse = std::max(maxPulse, chartData.camera.cam.pattern.laser.slamEvent.swing.rbegin()->first);
		}

		// Laser audio effects
		if (!chartData.audio.keySound.laser.vol.empty())
		{
			maxPulse = std::max(maxPulse, chartData.audio.keySound.laser.vol.rbegin()->first);
		}
		if (!chartData.audio.keySound.laser.slamEvent.empty())
		{
			for (const auto& [eventType, pulses] : chartData.audio.keySound.laser.slamEvent)
			{
				if (!pulses.empty())
				{
					maxPulse = std::max(maxPulse, *pulses.rbegin());
				}
			}
		}
		if (!chartData.audio.audioEffect.laser.pulseEvent.empty())
		{
			for (const auto& [effectName, pulses] : chartData.audio.audioEffect.laser.pulseEvent)
			{
				if (!pulses.empty())
				{
					maxPulse = std::max(maxPulse, *pulses.rbegin());
				}
			}
		}
		if (!chartData.audio.audioEffect.laser.paramChange.empty())
		{
			for (const auto& [effectName, paramMap] : chartData.audio.audioEffect.laser.paramChange)
			{
				for (const auto& [paramName, pulseValueMap] : paramMap)
				{
					if (!pulseValueMap.empty())
					{
						maxPulse = std::max(maxPulse, pulseValueMap.rbegin()->first);
					}
				}
			}
		}

		// FX audio effects
		if (!chartData.audio.audioEffect.fx.longEvent.empty())
		{
			for (const auto& [effectName, laneEvents] : chartData.audio.audioEffect.fx.longEvent)
			{
				for (std::int32_t laneIdx = 0; laneIdx < kNumFXLanes; ++laneIdx)
				{
					if (!laneEvents[laneIdx].empty())
					{
						maxPulse = std::max(maxPulse, laneEvents[laneIdx].rbegin()->first);
					}
				}
			}
		}
		if (!chartData.audio.audioEffect.fx.paramChange.empty())
		{
			for (const auto& [effectName, paramMap] : chartData.audio.audioEffect.fx.paramChange)
			{
				for (const auto& [paramName, pulseValueMap] : paramMap)
				{
					if (!pulseValueMap.empty())
					{
						maxPulse = std::max(maxPulse, pulseValueMap.rbegin()->first);
					}
				}
			}
		}

		// Editor comments
		if (!chartData.editor.comment.empty())
		{
			maxPulse = std::max(maxPulse, chartData.editor.comment.rbegin()->first);
		}

		// Unknown lines from ksh_unknown
		if (!chartData.compat.kshUnknown.line.empty())
		{
			maxPulse = std::max(maxPulse, chartData.compat.kshUnknown.line.rbegin()->first);
		}
		for (const auto& [optionKey, pulseValueMap] : chartData.compat.kshUnknown.option)
		{
			if (!pulseValueMap.empty())
			{
				maxPulse = std::max(maxPulse, pulseValueMap.rbegin()->first);
			}
		}

		return maxPulse;
	}

	// Write BPM to header
	// Returns the header BPM string that was output
	std::string WriteBPMToHeader(std::ostream& stream, const std::string& dispBPM, const ByPulse<double>& bpmMap, const CompatInfo& compat)
	{
		// If dispBPM is set, use it as-is
		if (!dispBPM.empty())
		{
			stream << "t=" << dispBPM << "\r\n";
			return dispBPM;
		}

		if (bpmMap.empty())
		{
			stream << "t=120\r\n";
			return "120";
		}

		// Apply BPM limit for ver >= 130
		const bool shouldClampBPM = !compat.isKSHVersionOlderThan(kVerBPMLimitAdded);

		// Check if single BPM
		if (bpmMap.size() == 1)
		{
			double bpm = bpmMap.begin()->second;
			if (shouldClampBPM)
			{
				bpm = std::min(bpm, kBPMMax);
			}
			std::string bpmStr = FormatDouble(bpm);
			stream << "t=" << bpmStr << "\r\n";
			return bpmStr;
		}

		// Find min and max BPM
		double minBPM = std::numeric_limits<double>::max();
		double maxBPM = std::numeric_limits<double>::lowest();

		for (const auto& [pulse, bpm] : bpmMap)
		{
			double clampedBPM = bpm;
			if (shouldClampBPM)
			{
				clampedBPM = std::min(bpm, kBPMMax);
			}
			minBPM = std::min(minBPM, clampedBPM);
			maxBPM = std::max(maxBPM, clampedBPM);
		}

		// If all BPMs are the same, output single BPM
		if (AlmostEquals(minBPM, maxBPM))
		{
			std::string bpmStr = FormatDouble(minBPM);
			stream << "t=" << bpmStr << "\r\n";
			return bpmStr;
		}
		else
		{
			std::string bpmStr = FormatDouble(minBPM) + "-" + FormatDouble(maxBPM);
			stream << "t=" << bpmStr << "\r\n";
			return bpmStr;
		}
	}

	// Write header section
	void WriteHeader(std::ostream& stream, const ChartData& chartData, std::string* headerBPMStr = nullptr)
	{
		const auto& meta = chartData.meta;
		const auto& audio = chartData.audio;
		const auto& bg = chartData.bg;

		// Meta information
		stream << "title=" << meta.title << "\r\n";
		if (!meta.titleImgFilename.empty())
		{
			stream << "title_img=" << meta.titleImgFilename << "\r\n";
		}
		stream << "artist=" << meta.artist << "\r\n";
		if (!meta.artistImgFilename.empty())
		{
			stream << "artist_img=" << meta.artistImgFilename << "\r\n";
		}
		stream << "effect=" << meta.chartAuthor << "\r\n";
		stream << "jacket=" << meta.jacketFilename << "\r\n";
		stream << "illustrator=" << meta.jacketAuthor << "\r\n";

		// Difficulty
		const char* diffStr = "infinite";
		if (meta.difficulty.idx == 0)
		{
			diffStr = "light";
		}
		else if (meta.difficulty.idx == 1)
		{
			diffStr = "challenge";
		}
		else if (meta.difficulty.idx == 2)
		{
			diffStr = "extended";
		}
		stream << "difficulty=" << diffStr << "\r\n";
		stream << "level=" << meta.level << "\r\n";

		// BPM
		std::string bpmStr = WriteBPMToHeader(stream, meta.dispBPM, chartData.beat.bpm, chartData.compat);
		if (headerBPMStr != nullptr)
		{
			*headerBPMStr = bpmStr;
		}

		// Standard BPM (for hi-speed calculation)
		if (meta.stdBPM != 0.0)
		{
			stream << "to=" << FormatDouble(meta.stdBPM) << "\r\n";
		}

		// Calculate version value early (needed for layer delimiter selection)
		// FX format changed at ver=160, so output at least ver=160
		std::string verValue = "171";
		int verInt = 171;
		bool needVerCompat = false;
		if (!chartData.compat.kshVersion.empty())
		{
			verValue = chartData.compat.kshVersion;
			try
			{
				verInt = std::stoi(verValue);
				if (verInt < kVerFXFormatChanged)
				{
					needVerCompat = true;
					verValue = std::to_string(kVerFXFormatChanged);
					verInt = kVerFXFormatChanged;
				}
			}
			catch (...)
			{
				verValue = "171";
				verInt = 171;
			}
		}

		// Audio
		if (!audio.bgm.filename.empty())
		{
			stream << "m=" << audio.bgm.filename;

			// Legacy fp_filenames (e.g., m=aaa.ogg;aaa_f.ogg or m=aaa.ogg;aaa_f.ogg;aaa_fp.ogg)
			if (!audio.bgm.legacy.empty())
			{
				for (const auto& fpFilename : audio.bgm.legacy.toStrArray())
				{
					stream << ";" << fpFilename;
				}
			}

			stream << "\r\n";
		}

		// Song volume (only output if not 100)
		// For ver=100, apply inverse scaling (v100 used 0.6x multiplier on input)
		double volForOutput = audio.bgm.vol;
		if (chartData.compat.kshVersion == "100")
		{
			volForOutput /= 0.6;
		}
		const std::int32_t mvol = static_cast<std::int32_t>(std::round(volForOutput * 100));
		if (mvol != 100)
		{
			stream << "mvol=" << mvol << "\r\n";
		}

		stream << "o=" << audio.bgm.offset << "\r\n";

		// Background
		if (!bg.legacy.bg.empty() && !bg.legacy.bg[0].filename.empty())
		{
			stream << "bg=" << bg.legacy.bg[0].filename;

			// Second bg (when gauge >= 70%)
			if (bg.legacy.bg.size() > 1 && !bg.legacy.bg[1].filename.empty() &&
				bg.legacy.bg[0].filename != bg.legacy.bg[1].filename)
			{
				stream << ";" << bg.legacy.bg[1].filename;
			}

			stream << "\r\n";
		}
		if (!bg.legacy.layer.filename.empty())
		{
			stream << "layer=" << bg.legacy.layer.filename;

			// Only add duration and rotation flags if they differ from defaults
			// Default: duration=0, rotation.tilt=true, rotation.spin=true (flags=3)
			const bool isDefaultDuration = (bg.legacy.layer.duration == 0);
			const bool isDefaultRotation = (bg.legacy.layer.rotation.tilt && bg.legacy.layer.rotation.spin);

			if (!isDefaultDuration || !isDefaultRotation)
			{
				// Delimiter changed from "/" to ";" at ver=166
				const char delimiter = verInt < kVerLayerDelimiterChanged ? '/' : ';';

				// Add duration
				stream << delimiter << bg.legacy.layer.duration;

				// Calculate rotation flags (bit 0: tilt, bit 1: spin)
				std::int32_t rotationFlags = 0;
				if (bg.legacy.layer.rotation.tilt)
				{
					rotationFlags |= kRotationFlagTilt;
				}
				if (bg.legacy.layer.rotation.spin)
				{
					rotationFlags |= kRotationFlagSpin;
				}
				stream << delimiter << rotationFlags;
			}

			stream << "\r\n";
		}

		// Movie
		if (!bg.legacy.movie.filename.empty())
		{
			stream << "v=" << bg.legacy.movie.filename << "\r\n";
			stream << "vo=" << bg.legacy.movie.offset << "\r\n";
		}

		// Preview offset
		stream << "po=" << audio.bgm.preview.offset << "\r\n";

		// Preview duration
		stream << "plength=" << audio.bgm.preview.duration << "\r\n";

		// Peaking filter gain (default: 50)
		// Only use legacy.filter_gain (do not convert from param_change)
		if (!audio.audioEffect.laser.legacy.filterGain.empty())
		{
			const double filterGain = audio.audioEffect.laser.legacy.filterGain.begin()->second;
			const std::int32_t pfiltergain = static_cast<std::int32_t>(std::round(filterGain * 100.0));
			stream << "pfiltergain=" << pfiltergain << "\r\n";
		}

		// Filter type
		if (!audio.audioEffect.laser.pulseEvent.empty())
		{
			const auto& pulseEvent = audio.audioEffect.laser.pulseEvent;
			for (const auto& [effectName, pulses] : pulseEvent)
			{
				if (pulses.contains(0))
				{
					if (IsKSONPresetLaserFilterName(effectName))
					{
						stream << "filtertype=" << KSONPresetLaserFilterNameToKSH(effectName) << "\r\n";
					}
					else
					{
						stream << "filtertype=" << effectName << "\r\n";
					}
					break;
				}
			}
		}

		// Laser key sound auto volume (always output, default: 1)
		stream << "chokkakuautovol=" << (audio.keySound.laser.legacy.volAuto ? 1 : 0) << "\r\n";

		// Laser key sound volume
		if (!audio.keySound.laser.vol.empty())
		{
			const std::int32_t chokkakuvol = static_cast<std::int32_t>(std::round(audio.keySound.laser.vol.begin()->second * 100));
			stream << "chokkakuvol=" << chokkakuvol << "\r\n";
		}

		// Peaking filter delay (default: 40)
		if (audio.audioEffect.laser.peakingFilterDelay != 40)
		{
			stream << "pfilterdelay=" << audio.audioEffect.laser.peakingFilterDelay << "\r\n";
		}

		// Gauge total
		if (chartData.gauge.total != 0)
		{
			stream << "total=" << chartData.gauge.total << "\r\n";
		}

		// Information
		if (!meta.information.empty())
		{
			stream << "information=" << meta.information << "\r\n";
		}

		// Icon
		if (!meta.iconFilename.empty())
		{
			stream << "icon=" << meta.iconFilename << "\r\n";
		}

		// Version
		stream << "ver=" << verValue << "\r\n";

		// Output ver_compat if original version was <160
		if (needVerCompat)
		{
			stream << "ver_compat=" << chartData.compat.kshVersion << "\r\n";
		}

		// Output unknown meta options from ksh_unknown
		for (const auto& [key, value] : chartData.compat.kshUnknown.meta)
		{
			stream << key << "=" << value << "\r\n";
		}

		// Output unknown lines at pulse 0
		auto lineRange = chartData.compat.kshUnknown.line.equal_range(0);
		for (auto it = lineRange.first; it != lineRange.second; ++it)
		{
			stream << it->second << "\r\n";
		}

		stream << "--\r\n";
	}

	// Get BT char at pulse
	char GetBTCharAt(const ByPulse<Interval>& lane, Pulse pulse, Pulse oneLinePulse)
	{
		// Check if note starts at this pulse
		if (lane.contains(pulse))
		{
			const Interval& interval = lane.at(pulse);
			if (interval.length == 0)
			{
				return '1'; // Chip note
			}
			else
			{
				return '2'; // Long note start
			}
		}

		// Check if note continues at this pulse
		for (const auto& [startPulse, interval] : lane)
		{
			const Pulse endPulse = startPulse + interval.length;

			// Note continues at this pulse (but not at the end)
			if (pulse > startPulse && pulse < endPulse)
			{
				return '2'; // Long note continuation
			}
		}

		return '0'; // No note
	}

	// Get FX char at pulse
	char GetFXCharAt(const ByPulse<Interval>& lane, Pulse pulse, Pulse oneLinePulse)
	{
		// Check if note starts at this pulse
		if (lane.contains(pulse))
		{
			const Interval& interval = lane.at(pulse);
			if (interval.length == 0)
			{
				return '2'; // Chip note (2 in FX lane)
			}
			else
			{
				return '1'; // Long note start
			}
		}

		// Check if note continues at this pulse
		for (const auto& [startPulse, interval] : lane)
		{
			const Pulse endPulse = startPulse + interval.length;

			// Note continues at this pulse (but not at the end)
			if (pulse > startPulse && pulse < endPulse)
			{
				return '1'; // Long note continuation
			}
		}

		return '0'; // No note
	}

	// KSH laser intermediate representation (similar to v1's analog array)
	struct KSHLaserSegment
	{
		std::int32_t laneIdx; // 0=left, 1=right
		Pulse startPulse; // Absolute pulse position
		Pulse length; // Duration in pulses
		std::int32_t startValue; // 0-50 (laser position)
		std::int32_t endValue; // 0-50 (laser position)
		bool isSectionStart; // Whether this starts a new laser section
		bool wide; // Wide laser flag
	};

	// Convert KSON laser sections to KSH laser segments
	// This splits sections at v!=vf points (slams) to match KSH's section structure
	std::vector<KSHLaserSegment> ConvertLaserToKSHSegments(const ByPulse<LaserSection>& lane, std::int32_t laneIdx)
	{
		std::vector<KSHLaserSegment> segments;
		constexpr Pulse kPreferredSlamLength = kResolution4 / 32;
		constexpr Pulse kPulse1_16 = kResolution4 / 16;
		constexpr Pulse kPulse1_48 = kResolution4 / 48;
		constexpr Pulse kPulse1_64 = kResolution4 / 64;
		constexpr Pulse kPulse1_96 = kResolution4 / 96;
		constexpr Pulse kPulse1_192 = kResolution4 / 192;

		for (const auto& [sectionStart, section] : lane)
		{
			// Skip empty sections
			if (section.v.empty())
			{
				continue;
			}

			// Handle single-point laser section
			if (section.v.size() == 1 && section.v.begin()->first == 0)
			{
				// Single point laser at sectionStart
				const auto& point = section.v.begin()->second;
				const bool hasSlam = !AlmostEquals(point.v.v, point.v.vf);

				if (hasSlam)
				{
					// Single point slam
					const std::int32_t startValue = GraphValueToLaserX(point.v.v, section.wide());
					const std::int32_t endValue = GraphValueToLaserX(point.v.vf, section.wide());

					segments.push_back(KSHLaserSegment{
						.laneIdx = laneIdx,
						.startPulse = sectionStart,
						.length = kPreferredSlamLength,
						.startValue = startValue,
						.endValue = endValue,
						.isSectionStart = true,
						.wide = section.wide()
					});
				}
				else
				{
					// Single point without slam
					const std::int32_t value = GraphValueToLaserX(point.v.v, section.wide());
					segments.push_back(KSHLaserSegment{
						.laneIdx = laneIdx,
						.startPulse = sectionStart,
						.length = 0,
						.startValue = value,
						.endValue = value,
						.isSectionStart = true,
						.wide = section.wide()
					});
				}
				continue;
			}

			bool isFirstSegment = true;

			// Process each point in the section
			auto it = section.v.begin();
			while (it != section.v.end())
			{
				const auto& [relPulse, point] = *it;
				const Pulse absolutePulse = sectionStart + relPulse;

				// Check if this point has a slam (v != vf)
				const bool hasSlam = !AlmostEquals(point.v.v, point.v.vf);

				auto nextIt = std::next(it);
				const bool hasNextPoint = (nextIt != section.v.end());

				if (hasSlam)
				{
					// Create a slam segment
					const std::int32_t startValue = GraphValueToLaserX(point.v.v, section.wide());
					const std::int32_t endValue = GraphValueToLaserX(point.v.vf, section.wide());

					// Determine slam length (shorten if next point is too close)
					Pulse slamLength = kPreferredSlamLength;
					if (hasNextPoint)
					{
						const auto& [nextRelPulse, nextPoint] = *nextIt;
						const Pulse distanceToNext = nextRelPulse - relPulse;
						const std::int32_t nextStartValue = GraphValueToLaserX(nextPoint.v.v, section.wide());

						if (distanceToNext < kPreferredSlamLength)
						{
							// If next point has same value as slam end, shorten slam to allow next point to be output
							// This preserves the intermediate point in lossless roundtrip
							if (nextStartValue == endValue)
							{
								slamLength = distanceToNext / 2;
								if (slamLength < 1)
								{
									slamLength = 1;
								}
							}
							else
							{
								slamLength = distanceToNext;
							}
						}
						else
						{
							// Special handling for short gaps to avoid roundtrip issues
							// If gap <= 1/16 and next point value differs from slam end value,
							// use shorter slam length to prevent the gap from being detected as a slam
							if (distanceToNext <= kPulse1_16 && nextStartValue != endValue)
							{
								if (distanceToNext > kPreferredSlamLength + kPulse1_48)
								{
									slamLength = kPulse1_48;
								}
								else if (distanceToNext > kPreferredSlamLength + kPulse1_64)
								{
									slamLength = kPulse1_64;
								}
								else if (distanceToNext > kPreferredSlamLength + kPulse1_96)
								{
									slamLength = kPulse1_96;
								}
								else
								{
									slamLength = kPulse1_192;
								}
							}
						}
					}

					segments.push_back(KSHLaserSegment{
						.laneIdx = laneIdx,
						.startPulse = absolutePulse,
						.length = slamLength,
						.startValue = startValue,
						.endValue = endValue,
						.isSectionStart = isFirstSegment,
						.wide = section.wide()
					});

					isFirstSegment = false;

					// If there's a next point, create a segment from slam end to next point
					if (hasNextPoint)
					{
						const auto& [nextRelPulse, nextPoint] = *nextIt;
						const Pulse slamEndPulse = absolutePulse + slamLength;
						const Pulse nextAbsolutePulse = sectionStart + nextRelPulse;

						// Only create segment if slam end <= next point
						if (slamEndPulse <= nextAbsolutePulse)
						{
							const std::int32_t slamEndValue = endValue;
							const std::int32_t nextStartValue = GraphValueToLaserX(nextPoint.v.v, section.wide());

							// Segment continuation after slam (within same section)
							segments.push_back(KSHLaserSegment{
								.laneIdx = laneIdx,
								.startPulse = slamEndPulse,
								.length = nextAbsolutePulse - slamEndPulse,
								.startValue = slamEndValue,
								.endValue = nextStartValue,
								.isSectionStart = false,
								.wide = section.wide()
							});
						}
					}

					++it;
				}
				else
				{
					// Normal segment: create segment to next point
					if (hasNextPoint)
					{
						const auto& [nextRelPulse, nextPoint] = *nextIt;
						const Pulse nextAbsolutePulse = sectionStart + nextRelPulse;

						const std::int32_t startValue = GraphValueToLaserX(point.v.v, section.wide());
						const std::int32_t endValue = GraphValueToLaserX(nextPoint.v.v, section.wide());

						segments.push_back(KSHLaserSegment{
							.laneIdx = laneIdx,
							.startPulse = absolutePulse,
							.length = nextAbsolutePulse - absolutePulse,
							.startValue = startValue,
							.endValue = endValue,
							.isSectionStart = isFirstSegment,
							.wide = section.wide()
						});

						isFirstSegment = false;
					}

					++it;
				}
			}
		}

		return segments;
	}

	// Get laser char at pulse using intermediate representation
	char GetLaserCharAt(const std::vector<KSHLaserSegment>& segments, Pulse pulse, std::int32_t laneIdx, MeasureExportState& state)
	{
		auto& laserState = state.laserStates[laneIdx];

		// Find the segment containing this pulse
		for (const auto& seg : segments)
		{
			const Pulse segmentEnd = seg.startPulse + seg.length;

			// Check if pulse is within this segment
			if (pulse < seg.startPulse || pulse > segmentEnd)
			{
				continue;
			}

			if (pulse == seg.startPulse)
			{
				// Segment start
				laserState.active = true;
				laserState.lastValue = static_cast<double>(seg.startValue) / kLaserXMax;
				return LaserXToChar(seg.startValue);
			}
			else if (pulse == segmentEnd)
			{
				// Segment end
				laserState.lastValue = static_cast<double>(seg.endValue) / kLaserXMax;
				return LaserXToChar(seg.endValue);
			}
			else
			{
				// Continuation within segment
				return ':';
			}
		}

		// No laser at this pulse
		laserState.active = false;
		return '-';
	}

	// Convert KSON preset FX effect name to KSH name
	// Throws std::out_of_range if effectName is not a preset
	std::string_view KSONPresetFXEffectNameToKSH(const std::string& effectName)
	{
		return kKSONToKSHPresetFXEffectName.at(effectName);
	}

	// Convert KSON preset laser filter name to KSH filter name
	// Throws std::out_of_range if effectName is not a preset
	std::string_view KSONPresetLaserFilterNameToKSH(const std::string& effectName)
	{
		return kKSONToKSHPresetFilterName.at(effectName);
	}

	// Generate KSH audio effect string from KSON long_event parameters
	std::string GenerateKSHAudioEffectString(const ChartData& chartData, const std::string& effectName, const AudioEffectParams& params, bool isFX)
	{
		std::string result = IsKSONPresetFXEffectName(effectName)
			? std::string{ KSONPresetFXEffectNameToKSH(effectName) }
			: effectName;

		constexpr std::int32_t kAudioEffectParamUnspecified = std::numeric_limits<std::int32_t>::min();
		std::int32_t param1 = kAudioEffectParamUnspecified;
		std::int32_t param2 = kAudioEffectParamUnspecified;

		// Get effect type from definitions or preset name
		AudioEffectType type = AudioEffectType::Unspecified;

		// First, try to find in custom definitions
		const auto& defs = isFX ? chartData.audio.audioEffect.fx.def : chartData.audio.audioEffect.laser.def;
		for (const auto& [name, def] : defs)
		{
			if (name == effectName)
			{
				type = def.type;
				break;
			}
		}

		// If not found in custom definitions, check if it's a preset effect
		if (type == AudioEffectType::Unspecified && kPresetEffectTypeMap.contains(effectName))
		{
			type = kPresetEffectTypeMap.at(effectName);
		}

		if (type != AudioEffectType::Unspecified)
		{
			// Convert based on effect type
			switch (type)
			{
			case AudioEffectType::Retrigger:
			case AudioEffectType::Gate:
			case AudioEffectType::Wobble:
			case AudioEffectType::Echo:
				// wave_length="1/8" -> param1=8
				if (params.contains("wave_length"))
				{
					const std::string& waveLengthStr = params.at("wave_length");
					if (waveLengthStr.starts_with("1/"))
					{
						param1 = std::atoi(waveLengthStr.c_str() + 2);
					}
				}
				if (type == AudioEffectType::Echo && params.contains("feedback_level"))
				{
					// feedback_level: rate type ("60%", "0.6", "1/2")
					const std::string& feedbackStr = params.at("feedback_level");
					if (feedbackStr.ends_with('%'))
					{
						param2 = std::atoi(feedbackStr.c_str());
					}
					else if (feedbackStr.starts_with("1/"))
					{
						const int denominator = std::atoi(feedbackStr.c_str() + 2);
						param2 = (denominator > 0) ? (100 / denominator) : 0;
					}
					else
					{
						param2 = static_cast<std::int32_t>(std::round(std::atof(feedbackStr.c_str()) * 100.0));
					}
				}
				break;
			case AudioEffectType::PitchShift:
				// pitch="12" -> param1=12
				if (params.contains("pitch"))
				{
					param1 = std::atoi(params.at("pitch").c_str());
				}
				break;
			case AudioEffectType::Bitcrusher:
				// reduction="5samples" -> param1=5
				if (params.contains("reduction"))
				{
					param1 = std::atoi(params.at("reduction").c_str());
				}
				break;
			case AudioEffectType::Tapestop:
				// speed: rate type ("50%", "0.5", "1/2")
				if (params.contains("speed"))
				{
					const std::string& speedStr = params.at("speed");
					if (speedStr.ends_with('%'))
					{
						param1 = std::atoi(speedStr.c_str());
					}
					else if (speedStr.starts_with("1/"))
					{
						const int denominator = std::atoi(speedStr.c_str() + 2);
						param1 = (denominator > 0) ? (100 / denominator) : 0;
					}
					else
					{
						param1 = static_cast<std::int32_t>(std::round(std::atof(speedStr.c_str()) * 100.0));
					}
				}
				break;
			default:
				break;
			}
		}

		if (param1 != kAudioEffectParamUnspecified)
		{
			result += ";" + std::to_string(param1);
			if (param2 != kAudioEffectParamUnspecified)
			{
				result += ";" + std::to_string(param2);
			}
		}

		return result;
	}

	// Write zoom parameter (zoom_top, zoom_bottom, zoom_side)
	void WriteZoomParameter(std::ostream& stream, const std::string& paramName, const GraphPoint& graphPoint)
	{
		const double clampedV = std::clamp(graphPoint.v.v, -kZoomAbsMax, kZoomAbsMax);
		const std::int32_t zoomValue = static_cast<std::int32_t>(std::round(clampedV));
		stream << paramName << "=" << zoomValue << "\r\n";

		// Output vf on next line if v != vf (immediate change)
		if (!AlmostEquals(graphPoint.v.v, graphPoint.v.vf))
		{
			const double clampedVf = std::clamp(graphPoint.v.vf, -kZoomAbsMax, kZoomAbsMax);
			const std::int32_t zoomValueFinal = static_cast<std::int32_t>(std::round(clampedVf));
			if (zoomValue != zoomValueFinal)
			{
				stream << paramName << "=" << zoomValueFinal << "\r\n";
			}
		}

		if (graphPoint.curve.a != 0.0 || graphPoint.curve.b != 0.0)
		{
			stream << paramName << "_curve=" << graphPoint.curve.a << ";" << graphPoint.curve.b << "\r\n";
		}
	}

	// Write note line
	void WriteNoteLine(std::ostream& stream, const ChartData& chartData, const std::array<std::vector<KSHLaserSegment>, kNumLaserLanes>& laserSegments, Pulse pulse, Pulse oneLinePulse, MeasureExportState& state, bool useLegacyScaleForManualTilt)
	{
		// Note: The output order below should be the same as v1's order (*command_save in kshooteditor.hsp) for better compatibility of internet ranking hashing

		// Output FX chip events for this pulse
		if (!chartData.audio.keySound.fx.chipEvent.empty())
		{
			for (std::int32_t laneIdx = 0; laneIdx < kNumFXLanes; ++laneIdx)
			{
				for (const auto& [chipName, lanes] : chartData.audio.keySound.fx.chipEvent)
				{
					if (lanes[laneIdx].contains(pulse))
					{
						const auto& chipData = lanes[laneIdx].at(pulse);
						const std::int32_t vol = static_cast<std::int32_t>(std::round(chipData.vol * 100));
						stream << "fx-" << (laneIdx == 0 ? 'l' : 'r') << "_se=" << chipName;
						if (vol != 100)
						{
							stream << ";" << vol;
						}
						stream << "\r\n";
					}
				}
			}
		}

		if (chartData.beat.bpm.contains(pulse))
		{
			double bpm = chartData.beat.bpm.at(pulse);
			// Apply BPM limit for ver >= 130
			if (!chartData.compat.isKSHVersionOlderThan(kVerBPMLimitAdded))
			{
				bpm = std::min(bpm, kBPMMax);
			}

			std::string bpmStr = FormatDouble(bpm);

			// Skip output at pulse 0 if it matches the header BPM
			bool shouldSkipDuplicate = pulse == 0 && !state.headerBPMStr.empty() && bpmStr == state.headerBPMStr;

			if (!shouldSkipDuplicate)
			{
				stream << "t=" << bpmStr << "\r\n";
			}
		}

		// Output comments for this pulse
		auto commentRange = chartData.editor.comment.equal_range(pulse);
		for (auto it = commentRange.first; it != commentRange.second; ++it)
		{
			stream << "//" << it->second << "\r\n";
		}

		// Output unknown lines for this pulse
		// Skip output at pulse 0 (already output in header)
		if (pulse != 0)
		{
			auto lineRange = chartData.compat.kshUnknown.line.equal_range(pulse);
			for (auto it = lineRange.first; it != lineRange.second; ++it)
			{
				stream << it->second << "\r\n";
			}
		}

		// Output unknown options for this pulse
		for (const auto& [optionKey, pulseValueMap] : chartData.compat.kshUnknown.option)
		{
			auto optionRange = pulseValueMap.equal_range(pulse);
			for (auto it = optionRange.first; it != optionRange.second; ++it)
			{
				stream << optionKey << "=" << it->second << "\r\n";
			}
		}

		// Output center_split
		if (chartData.camera.cam.body.centerSplit.contains(pulse))
		{
			const auto& graphPoint = chartData.camera.cam.body.centerSplit.at(pulse);

			const double clampedV = std::clamp(graphPoint.v.v, -kCenterSplitAbsMax, kCenterSplitAbsMax);
			stream << "center_split=" << clampedV << "\r\n";

			// Output vf on next line if v != vf (immediate change)
			if (!AlmostEquals(graphPoint.v.v, graphPoint.v.vf))
			{
				const double clampedVf = std::clamp(graphPoint.v.vf, -kCenterSplitAbsMax, kCenterSplitAbsMax);
				stream << "center_split=" << clampedVf << "\r\n";
			}

			if (graphPoint.curve.a != 0.0 || graphPoint.curve.b != 0.0)
			{
				stream << "center_split_curve=" << graphPoint.curve.a << ";" << graphPoint.curve.b << "\r\n";
			}
		}

		// Check for FX param_change (fx:effect_name:param_name=value)
		if (!chartData.audio.audioEffect.fx.paramChange.empty())
		{
			for (const auto& [effectName, paramMap] : chartData.audio.audioEffect.fx.paramChange)
			{
				for (const auto& [paramName, pulseValueMap] : paramMap)
				{
					if (pulseValueMap.contains(pulse))
					{
						const std::string& value = pulseValueMap.at(pulse);
						const std::string_view kshEffectName = IsKSONPresetFXEffectName(effectName)
							? KSONPresetFXEffectNameToKSH(effectName)
							: std::string_view{ effectName };
						const std::string kshParamName = kKSONToKSHParamName.contains(paramName)
							? std::string{ kKSONToKSHParamName.at(paramName) }
							: paramName;
						stream << "fx:" << kshEffectName << ":" << kshParamName << "=" << value << "\r\n";
					}
				}
			}
		}

		// Check for laser param_change (filter:effect_name:param_name=value)
		if (!chartData.audio.audioEffect.laser.paramChange.empty())
		{
			for (const auto& [effectName, paramMap] : chartData.audio.audioEffect.laser.paramChange)
			{
				for (const auto& [paramName, pulseValueMap] : paramMap)
				{
					if (pulseValueMap.contains(pulse))
					{
						const std::string& value = pulseValueMap.at(pulse);
						const std::string_view kshEffectName = IsKSONPresetLaserFilterName(effectName)
							? KSONPresetLaserFilterNameToKSH(effectName)
							: std::string_view{ effectName };
						const std::string kshParamName = kKSONToKSHParamName.contains(paramName)
							? std::string{ kKSONToKSHParamName.at(paramName) }
							: paramName;
						stream << "filter:" << kshEffectName << ":" << kshParamName << "=" << value << "\r\n";
					}
				}
			}
		}

		// Check for peaking filter gain changes
		// Only use legacy.filter_gain (do not convert from param_change)
		if (!chartData.audio.audioEffect.laser.legacy.filterGain.empty())
		{
			const auto it = std::find_if(
				chartData.audio.audioEffect.laser.legacy.filterGain.begin(),
				chartData.audio.audioEffect.laser.legacy.filterGain.end(),
				[pulse](const auto& pair) { return pair.first == pulse; }
			);
			if (it != chartData.audio.audioEffect.laser.legacy.filterGain.end())
			{
				const double filterGain = it->second;
				const std::int32_t pfiltergain = static_cast<std::int32_t>(std::round(filterGain * 100.0));
				if (pfiltergain != state.currentPfiltergain)
				{
					// Skip output at pulse 0 (already output in header)
					if (pulse != 0)
					{
						stream << "pfiltergain=" << pfiltergain << "\r\n";
					}
					state.currentPfiltergain = pfiltergain;
				}
			}
		}

		// Check for filter type changes
		if (!chartData.audio.audioEffect.laser.pulseEvent.empty())
		{
			std::string newFilterType;
			const auto& pulseEvent = chartData.audio.audioEffect.laser.pulseEvent;
			if (pulseEvent.contains("peaking_filter") &&
				pulseEvent.at("peaking_filter").contains(pulse))
			{
				newFilterType = "peak";
			}
			else if (pulseEvent.contains("low_pass_filter") &&
				pulseEvent.at("low_pass_filter").contains(pulse))
			{
				newFilterType = "lpf1";
			}
			else if (pulseEvent.contains("high_pass_filter") &&
				pulseEvent.at("high_pass_filter").contains(pulse))
			{
				newFilterType = "hpf1";
			}
			else if (pulseEvent.contains("bitcrusher") &&
				pulseEvent.at("bitcrusher").contains(pulse))
			{
				newFilterType = "bitc";
			}

			if (!newFilterType.empty() && pulse != 0)
			{
				stream << "filtertype=" << newFilterType << "\r\n";
				state.currentFilterType = newFilterType;
			}

			// Check for user-defined filter changes (from pulse_event)
			for (const auto& [effectName, pulses] : pulseEvent)
			{
				// Skip preset filters (already handled above)
				if (!IsKSONPresetLaserFilterName(effectName))
				{
					if (pulses.contains(pulse) && pulse != 0)
					{
						stream << "filtertype=" << effectName << "\r\n";
						state.currentFilterType = effectName;
					}
				}
			}
		}

		if (chartData.audio.keySound.laser.vol.contains(pulse))
		{
			const std::int32_t chokkakuvol = static_cast<std::int32_t>(std::round(chartData.audio.keySound.laser.vol.at(pulse) * 100));
			if (chokkakuvol != state.currentChokkakuvol)
			{
				// Skip output at pulse 0 (already output in header)
				if (pulse != 0)
				{
					stream << "chokkakuvol=" << chokkakuvol << "\r\n";
				}
				state.currentChokkakuvol = chokkakuvol;
			}
		}

		// Check for laser slam key sounds (chokkakuse)
		if (!chartData.audio.keySound.laser.slamEvent.empty())
		{
			const auto& slamEvent = chartData.audio.keySound.laser.slamEvent;

			// Check for down slam
			if (slamEvent.contains("down") &&
				slamEvent.at("down").contains(pulse))
			{
				stream << "chokkakuse=down\r\n";
			}
			// Check for up slam
			else if (slamEvent.contains("up") &&
				slamEvent.at("up").contains(pulse))
			{
				stream << "chokkakuse=up\r\n";
			}
			// Check for swing slam
			else if (slamEvent.contains("swing") &&
				slamEvent.at("swing").contains(pulse))
			{
				stream << "chokkakuse=swing\r\n";
			}
			// Check for mute slam
			else if (slamEvent.contains("mute") &&
				slamEvent.at("mute").contains(pulse))
			{
				stream << "chokkakuse=mute\r\n";
			}
		}

		if (chartData.camera.tilt.contains(pulse))
		{
			const auto& tiltValue = chartData.camera.tilt.at(pulse);

			if (std::holds_alternative<TiltGraphPoint>(tiltValue))
			{
				const TiltGraphPoint& graphPoint = std::get<TiltGraphPoint>(tiltValue);

				// Apply inverse legacy scale for ver < 170 if any large tilt value exists in the chart
				// Legacy charts with large manual tilt values depend on the tilt scale (14 degrees) used before v1.70
				constexpr double kFromLegacyScale = 10.0 / 14.0;
				const double scale = useLegacyScaleForManualTilt ? kFromLegacyScale : 1.0;

				// Output curve if present
				if (!graphPoint.curve.isLinear())
				{
					stream << "tilt_curve=" << FormatDouble(graphPoint.curve.a) << ";" << FormatDouble(graphPoint.curve.b) << "\r\n";
				}

				const double clampedV = std::clamp(graphPoint.v.v * scale, -kManualTiltAbsMax, kManualTiltAbsMax);
				stream << "tilt=" << FormatDouble(clampedV) << "\r\n";

				// Output vf on next line if v != vf (immediate change)
				if (std::holds_alternative<double>(graphPoint.v.vf))
				{
					const double vfValue = std::get<double>(graphPoint.v.vf);
					if (!AlmostEquals(graphPoint.v.v, vfValue))
					{
						const double clampedVf = std::clamp(vfValue * scale, -kManualTiltAbsMax, kManualTiltAbsMax);
						stream << "tilt=" << FormatDouble(clampedVf) << "\r\n";
					}
				}
				else if (std::holds_alternative<AutoTiltType>(graphPoint.v.vf))
				{
					// vf is AutoTiltType: output it as string
					const AutoTiltType vfAutoTilt = std::get<AutoTiltType>(graphPoint.v.vf);
					stream << "tilt=" << AutoTiltTypeToString(vfAutoTilt) << "\r\n";
				}
			}
			else if (std::holds_alternative<AutoTiltType>(tiltValue))
			{
				const AutoTiltType autoTiltType = std::get<AutoTiltType>(tiltValue);
				stream << "tilt=" << AutoTiltTypeToString(autoTiltType) << "\r\n";
			}
		}

		if (chartData.camera.cam.body.zoomTop.contains(pulse))
		{
			WriteZoomParameter(stream, "zoom_top", chartData.camera.cam.body.zoomTop.at(pulse));
		}

		if (chartData.camera.cam.body.zoomBottom.contains(pulse))
		{
			WriteZoomParameter(stream, "zoom_bottom", chartData.camera.cam.body.zoomBottom.at(pulse));
		}

		if (chartData.camera.cam.body.zoomSide.contains(pulse))
		{
			WriteZoomParameter(stream, "zoom_side", chartData.camera.cam.body.zoomSide.at(pulse));
		}

		// Check for laser wide annotation changes before processing notes
		for (std::int32_t i = 0; i < kNumLaserLanes; ++i)
		{
			// Find segment starting at this pulse
			for (const auto& seg : laserSegments[i])
			{
				if (seg.startPulse == pulse && seg.isSectionStart)
				{
					auto& laserState = state.laserStates[i];
					// Output wide annotation if changed
					if (seg.wide)
					{
						stream << "laserrange_" << (i == 0 ? 'l' : 'r') << "=2x\r\n";
					}
					break;
				}
			}
		}

		// Output laser curve for points at this pulse
		for (std::int32_t i = 0; i < kNumLaserLanes; ++i)
		{
			for (const auto& [sectionPulse, section] : chartData.note.laser[i])
			{
				const RelPulse relPulse = pulse - sectionPulse;
				if (relPulse >= 0 && section.v.contains(relPulse))
				{
					const auto& point = section.v.at(relPulse);
					if (point.curve.a != 0.0 || point.curve.b != 0.0)
					{
						stream << "laser_" << (i == 0 ? 'l' : 'r') << "_curve="
							<< FormatDouble(point.curve.a) << ";" << FormatDouble(point.curve.b) << "\r\n";
					}
					break;
				}
			}
		}

		if (chartData.beat.stop.contains(pulse))
		{
			const RelPulse stopLength = chartData.beat.stop.at(pulse);
			stream << "stop=" << RelPulseToKSHLength(stopLength) << "\r\n";
		}

		if (chartData.beat.scrollSpeed.contains(pulse))
		{
			const auto& graphPoint = chartData.beat.scrollSpeed.at(pulse);
			const double scrollSpeed = graphPoint.v.v;

			// Skip output if scroll_speed has only one element with the default value of 1.0
			const bool isDefaultOnly = chartData.beat.scrollSpeed.size() == 1 &&
				AlmostEquals(chartData.beat.scrollSpeed.begin()->second.v.v, 1.0);
			if (!isDefaultOnly)
			{
				stream << "scroll_speed=" << FormatDouble(scrollSpeed) << "\r\n";
			}

			// Output vf on next line if v != vf (immediate change)
			if (!AlmostEquals(graphPoint.v.v, graphPoint.v.vf))
			{
				stream << "scroll_speed=" << FormatDouble(graphPoint.v.vf) << "\r\n";
			}

			if (graphPoint.curve.a != 0.0 || graphPoint.curve.b != 0.0)
			{
				stream << "scroll_speed_curve=" << FormatDouble(graphPoint.curve.a) << ";" << FormatDouble(graphPoint.curve.b) << "\r\n";
			}
		}

		if (chartData.camera.cam.body.rotationDeg.contains(pulse))
		{
			const auto& graphPoint = chartData.camera.cam.body.rotationDeg.at(pulse);

			const double clampedV = std::clamp(graphPoint.v.v, -kRotationDegAbsMax, kRotationDegAbsMax);
			stream << "rotation_deg=" << static_cast<std::int32_t>(std::round(clampedV)) << "\r\n";

			// Output vf on next line if v != vf (immediate change)
			if (!AlmostEquals(graphPoint.v.v, graphPoint.v.vf))
			{
				const double clampedVf = std::clamp(graphPoint.v.vf, -kRotationDegAbsMax, kRotationDegAbsMax);
				stream << "rotation_deg=" << static_cast<std::int32_t>(std::round(clampedVf)) << "\r\n";
			}

			if (graphPoint.curve.a != 0.0 || graphPoint.curve.b != 0.0)
			{
				stream << "rotation_deg_curve=" << FormatDouble(graphPoint.curve.a) << ";" << FormatDouble(graphPoint.curve.b) << "\r\n";
			}
		}

		// Check for FX audio effect annotations (fx-l, fx-r)
		// Output in lane order (fx-l before fx-r) to match v1 behavior
		if (!chartData.audio.audioEffect.fx.longEvent.empty())
		{
			for (std::int32_t laneIdx = 0; laneIdx < kNumFXLanes; ++laneIdx)
			{
				for (const auto& [effectName, laneEvents] : chartData.audio.audioEffect.fx.longEvent)
				{
					if (!laneEvents[laneIdx].contains(pulse))
					{
						continue;
					}

					// Empty effect name represents "effect off"
					if (effectName.empty())
					{
						stream << "fx-" << (laneIdx == 0 ? 'l' : 'r') << "=\r\n";
						state.currentFXAudioEffects[laneIdx].clear();
						break;
					}

					const auto& params = laneEvents[laneIdx].at(pulse);
					const std::string audioEffectStr = GenerateKSHAudioEffectString(chartData, effectName, params, true);

					// Check if this pulse is the start of an FX long note
					const bool isNoteStart = chartData.note.fx[laneIdx].contains(pulse) &&
						chartData.note.fx[laneIdx].at(pulse).length > 0;

					// Output fx-l/fx-r
					stream << "fx-" << (laneIdx == 0 ? 'l' : 'r') << "=" << audioEffectStr << "\r\n";
					state.currentFXAudioEffects[laneIdx] = audioEffectStr;
					break;
				}
			}
		}

		// BT notes (4 chars)
		std::array<char, kNumBTLanes> btChars;
		for (std::int32_t i = 0; i < kNumBTLanes; ++i)
		{
			btChars[i] = GetBTCharAt(chartData.note.bt[i], pulse, oneLinePulse);
		}

		// FX notes (2 chars)
		std::array<char, kNumFXLanes> fxChars;
		for (std::int32_t i = 0; i < kNumFXLanes; ++i)
		{
			fxChars[i] = GetFXCharAt(chartData.note.fx[i], pulse, oneLinePulse);
		}

		// Laser notes (2 chars)
		std::array<char, kNumLaserLanes> laserChars;
		for (std::int32_t i = 0; i < kNumLaserLanes; ++i)
		{
			laserChars[i] = GetLaserCharAt(laserSegments[i], pulse, i, state);
		}

		// Output: "0011|22|AB" or "0011|22|AB@)240"
		stream << btChars[0] << btChars[1] << btChars[2] << btChars[3] << kBlockSeparator
			<< fxChars[0] << fxChars[1] << kBlockSeparator
			<< laserChars[0] << laserChars[1];

		// Check for spin/half-spin/swing events (append to laser line)
		if (chartData.camera.cam.pattern.laser.slamEvent.spin.contains(pulse))
		{
			const auto& spinEvent = chartData.camera.cam.pattern.laser.slamEvent.spin.at(pulse);
			const char dirChar = spinEvent.d < 0 ? '(' : ')';
			const std::int32_t kshLength = ToKSHResolution(spinEvent.length);
			stream << "@" << dirChar << kshLength;
		}
		else if (chartData.camera.cam.pattern.laser.slamEvent.halfSpin.contains(pulse))
		{
			const auto& spinEvent = chartData.camera.cam.pattern.laser.slamEvent.halfSpin.at(pulse);
			const char dirChar = spinEvent.d < 0 ? '<' : '>';
			const std::int32_t kshLength = ToKSHResolution(spinEvent.length);
			stream << "@" << dirChar << kshLength;
		}
		else if (chartData.camera.cam.pattern.laser.slamEvent.swing.contains(pulse))
		{
			const auto& swingEvent = chartData.camera.cam.pattern.laser.slamEvent.swing.at(pulse);
			const char dirChar = swingEvent.d < 0 ? '<' : '>';
			const std::int32_t kshLength = ToKSHResolution(swingEvent.length);
			const std::int32_t scale = static_cast<std::int32_t>(std::round(swingEvent.v.scale));
			const std::int32_t repeat = swingEvent.v.repeat;
			const std::int32_t decayOrder = swingEvent.v.decayOrder;

			stream << "S" << dirChar << kshLength;

			// Output non-default parameters
			bool needParams = (scale != 250 || repeat != 3 || decayOrder != 2);
			if (needParams)
			{
				stream << ";" << scale << ";" << repeat << ";" << decayOrder;
			}
		}

		stream << "\r\n";
	}

	// Calculate optimal division for a measure
	std::int32_t CalculateOptimalDivision(const ChartData& chartData, const std::array<std::vector<KSHLaserSegment>, kNumLaserLanes>& laserSegments, Pulse measureStart, Pulse measureLength, std::vector<std::string>& warnings)
	{
		const Pulse measureEnd = measureStart + measureLength;
		Pulse gcd = measureLength;
		bool shouldDoubleResolution = false;

		// 1/32 measure length for slam representation (30 pulses at kResolution=240)
		constexpr Pulse kSlamLength = kResolution4 / 32; // 1/32 measure = 30 pulses

		// Helper function to update GCD with a pulse if it's within the measure
		auto updateGCD = [&](Pulse pulse) {
			if (pulse >= measureStart && pulse < measureEnd)
			{
				const Pulse relPulse = pulse - measureStart;
				if (relPulse > 0 && relPulse < measureLength)
				{
					gcd = std::gcd(gcd, relPulse);
				}
			}
		};

		// BT notes
		for (const auto& lane : chartData.note.bt)
		{
			for (const auto& [pulse, interval] : lane)
			{
				updateGCD(pulse);
				updateGCD(pulse + interval.length);

				// Long BT notes starting in this measure require doubled resolution
				if (interval.length > 0 && pulse >= measureStart && pulse < measureEnd)
				{
					shouldDoubleResolution = true;
				}

				// Long BT notes ending in this measure require doubled resolution
				const Pulse endPulse = pulse + interval.length;
				if (interval.length > 0 && endPulse >= measureStart && endPulse < measureEnd)
				{
					shouldDoubleResolution = true;
				}
			}
		}

		// FX notes
		for (const auto& lane : chartData.note.fx)
		{
			for (const auto& [pulse, interval] : lane)
			{
				updateGCD(pulse);
				updateGCD(pulse + interval.length);

				// Long FX notes starting in this measure require doubled resolution
				if (interval.length > 0 && pulse >= measureStart && pulse < measureEnd)
				{
					shouldDoubleResolution = true;
				}

				// Long FX notes ending in this measure require doubled resolution
				const Pulse endPulse = pulse + interval.length;
				if (interval.length > 0 && endPulse >= measureStart && endPulse < measureEnd)
				{
					shouldDoubleResolution = true;
				}
			}
		}

		// Laser notes
		for (std::int32_t laneIdx = 0; laneIdx < kNumLaserLanes; ++laneIdx)
		{
			const auto& segments = laserSegments[laneIdx];
			for (std::size_t segIdx = 0; segIdx < segments.size(); ++segIdx)
			{
				const auto& seg = segments[segIdx];
				updateGCD(seg.startPulse);
				updateGCD(seg.startPulse + seg.length);

				// Laser segments starting or ending in this measure require doubled resolution
				if (seg.startPulse >= measureStart && seg.startPulse < measureEnd)
				{
					shouldDoubleResolution = true;
				}

				const Pulse endPulse = seg.startPulse + seg.length;
				if (endPulse >= measureStart && endPulse < measureEnd)
				{
					shouldDoubleResolution = true;
				}
			}
		}

		// BPM changes
		for (const auto& [pulse, bpm] : chartData.beat.bpm)
		{
			updateGCD(pulse);
		}

		// Stops
		for (const auto& [pulse, length] : chartData.beat.stop)
		{
			updateGCD(pulse);
		}

		// Scroll speed
		for (const auto& [pulse, graphPoint] : chartData.beat.scrollSpeed)
		{
			updateGCD(pulse);
		}

		// Camera rotation
		for (const auto& [pulse, graphPoint] : chartData.camera.cam.body.rotationDeg)
		{
			updateGCD(pulse);
		}

		// Camera zoom
		for (const auto& [pulse, graphPoint] : chartData.camera.cam.body.zoomTop)
		{
			updateGCD(pulse);
		}
		for (const auto& [pulse, graphPoint] : chartData.camera.cam.body.zoomBottom)
		{
			updateGCD(pulse);
		}
		for (const auto& [pulse, graphPoint] : chartData.camera.cam.body.zoomSide)
		{
			updateGCD(pulse);
		}
		for (const auto& [pulse, graphPoint] : chartData.camera.cam.body.centerSplit)
		{
			updateGCD(pulse);
		}

		// Camera tilt
		for (const auto& [pulse, tiltValue] : chartData.camera.tilt)
		{
			updateGCD(pulse);
		}

		// Spin events
		for (const auto& [pulse, spinEvent] : chartData.camera.cam.pattern.laser.slamEvent.spin)
		{
			updateGCD(pulse);
		}
		for (const auto& [pulse, spinEvent] : chartData.camera.cam.pattern.laser.slamEvent.halfSpin)
		{
			updateGCD(pulse);
		}
		for (const auto& [pulse, swingEvent] : chartData.camera.cam.pattern.laser.slamEvent.swing)
		{
			updateGCD(pulse);
		}

		if (!chartData.audio.audioEffect.fx.longEvent.empty())
		{
			for (const auto& [effectName, laneEvents] : chartData.audio.audioEffect.fx.longEvent)
			{
				for (std::int32_t laneIdx = 0; laneIdx < kNumFXLanes; ++laneIdx)
				{
					for (const auto& [pulse, params] : laneEvents[laneIdx])
					{
						updateGCD(pulse);
					}
				}
			}
		}

		if (!chartData.audio.audioEffect.fx.paramChange.empty())
		{
			for (const auto& [effectName, paramMap] : chartData.audio.audioEffect.fx.paramChange)
			{
				for (const auto& [paramName, pulseValueMap] : paramMap)
				{
					for (const auto& [pulse, value] : pulseValueMap)
					{
						updateGCD(pulse);
					}
				}
			}
		}

		if (!chartData.audio.audioEffect.laser.paramChange.empty())
		{
			for (const auto& [effectName, paramMap] : chartData.audio.audioEffect.laser.paramChange)
			{
				for (const auto& [paramName, pulseValueMap] : paramMap)
				{
					for (const auto& [pulse, value] : pulseValueMap)
					{
						updateGCD(pulse);
					}
				}
			}
		}

		if (!chartData.audio.audioEffect.laser.pulseEvent.empty())
		{
			for (const auto& [effectName, pulses] : chartData.audio.audioEffect.laser.pulseEvent)
			{
				for (const auto& pulse : pulses)
				{
					updateGCD(pulse);
				}
			}
		}

		for (const auto& [pulse, vol] : chartData.audio.keySound.laser.vol)
		{
			updateGCD(pulse);
		}

		for (const auto& [pulse, gain] : chartData.audio.audioEffect.laser.legacy.filterGain)
		{
			updateGCD(pulse);
		}

		if (!chartData.audio.keySound.laser.slamEvent.empty())
		{
			for (const auto& [slamType, pulses] : chartData.audio.keySound.laser.slamEvent)
			{
				for (const auto& pulse : pulses)
				{
					updateGCD(pulse);
				}
			}
		}

		// FX chip events
		if (!chartData.audio.keySound.fx.chipEvent.empty())
		{
			for (const auto& [chipName, lanes] : chartData.audio.keySound.fx.chipEvent)
			{
				for (std::int32_t laneIdx = 0; laneIdx < kNumFXLanes; ++laneIdx)
				{
					for (const auto& [pulse, chipData] : lanes[laneIdx])
					{
						updateGCD(pulse);
					}
				}
			}
		}

		for (const auto& [pulse, comment] : chartData.editor.comment)
		{
			updateGCD(pulse);
		}

		for (const auto& [optionKey, pulseValueMap] : chartData.compat.kshUnknown.option)
		{
			for (const auto& [pulse, values] : pulseValueMap)
			{
				updateGCD(pulse);
			}
		}

		for (const auto& [pulse, line] : chartData.compat.kshUnknown.line)
		{
			updateGCD(pulse);
		}

		// Calculate division in KSON resolution (960) to preserve all note timings
		std::int32_t division = gcd > 0 ? static_cast<std::int32_t>(measureLength / gcd) : static_cast<std::int32_t>(measureLength);

		// Apply doubling for long notes/lasers
		if (division < measureLength && shouldDoubleResolution)
		{
			const int doubled = division * 2;
			// Only apply doubling if it divides measureLength evenly
			if (measureLength % doubled == 0)
			{
				division = doubled;
			}
			// If 1 line = 15 pulses (1/64th), try tripling when doubling doesn't work
			// Without this, Heliodor[EX] (SFES2022) kson->ksh->kson roundtrip causes unexpected laser section merging
			else if (measureLength / division == 15)
			{
				const int tripled = division * 3;
				if (measureLength % tripled == 0)
				{
					division = tripled;
				}
			}
		}

		// Ensure division divides measureLength evenly
		if (measureLength % division != 0)
		{
			division = static_cast<std::int32_t>(measureLength);
		}

		// Ensure division is at least 1
		if (division < 1)
		{
			division = 1;
		}

		// Limit division to reasonable range (use measureLength as max)
		if (division > measureLength)
		{
			return static_cast<std::int32_t>(measureLength);
		}

		return division;
	}

	// Write measures
	void WriteMeasures(std::ostream& stream, const ChartData& chartData, MeasureExportState& state, std::vector<std::string>& warnings)
	{
		// Check if legacy manual tilt scale should be used
		// This matches the logic in ksh_io_in.cpp: ver < 170 && any abs(tilt) >= 10.0
		bool useLegacyScaleForManualTilt = false;
		if (chartData.compat.isKSHVersionOlderThan(kVerManualTiltScaleChanged))
		{
			for (const auto& [pulse, tiltValue] : chartData.camera.tilt)
			{
				if (std::holds_alternative<TiltGraphPoint>(tiltValue))
				{
					const TiltGraphPoint& point = std::get<TiltGraphPoint>(tiltValue);
					bool largeVf = std::holds_alternative<double>(point.v.vf) && std::abs(std::get<double>(point.v.vf)) >= 10.0;
					if (std::abs(point.v.v) >= 10.0 || largeVf)
					{
						useLegacyScaleForManualTilt = true;
						break;
					}
				}
			}
		}

		// Initialize laser audio effect state from header values
		if (!chartData.audio.keySound.laser.vol.empty())
		{
			state.currentChokkakuvol = static_cast<std::int32_t>(std::round(chartData.audio.keySound.laser.vol.begin()->second * 100));
		}
		if (!chartData.audio.audioEffect.laser.legacy.filterGain.empty())
		{
			const double filterGain = chartData.audio.audioEffect.laser.legacy.filterGain.begin()->second;
			state.currentPfiltergain = static_cast<std::int32_t>(std::round(filterGain * 100.0));
		}
		if (!chartData.audio.audioEffect.laser.pulseEvent.empty())
		{
			const auto& pulseEvent = chartData.audio.audioEffect.laser.pulseEvent;
			if (pulseEvent.contains("peaking_filter") &&
				pulseEvent.at("peaking_filter").contains(0))
			{
				state.currentFilterType = "peak";
			}
			else if (pulseEvent.contains("low_pass_filter") &&
				pulseEvent.at("low_pass_filter").contains(0))
			{
				state.currentFilterType = "lpf1";
			}
			else if (pulseEvent.contains("high_pass_filter") &&
				pulseEvent.at("high_pass_filter").contains(0))
			{
				state.currentFilterType = "hpf1";
			}
		}

		// Convert KSON laser sections to KSH laser segments (intermediate representation)
		std::array<std::vector<KSHLaserSegment>, kNumLaserLanes> laserSegments;
		for (std::int32_t laneIdx = 0; laneIdx < kNumLaserLanes; ++laneIdx)
		{
			laserSegments[laneIdx] = ConvertLaserToKSHSegments(chartData.note.laser[laneIdx], laneIdx);
		}

		const Pulse maxPulse = CalculateMaxPulse(chartData);
		Pulse currentPulse = 0;
		std::int64_t measureIdx = 0;

		while (currentPulse <= maxPulse)
		{
			// Get current time signature
			TimeSig timeSig = ValueAtOrDefault(chartData.beat.timeSig, measureIdx, TimeSig{ 4, 4 });
			const Pulse measureLength = kResolution4 * timeSig.n / timeSig.d;

			// Check for time signature change
			if (chartData.beat.timeSig.contains(measureIdx) ||
				(timeSig.n != state.currentTimeSig.n || timeSig.d != state.currentTimeSig.d))
			{
				stream << "beat=" << timeSig.n << "/" << timeSig.d << "\r\n";
				state.currentTimeSig = timeSig;
			}

			// Calculate optimal division for this measure
			const std::int32_t division = CalculateOptimalDivision(chartData, laserSegments, currentPulse, measureLength, warnings);
			const Pulse oneLinePulse = measureLength / division;

			// Write each line
			for (std::int32_t lineIdx = 0; lineIdx < division; ++lineIdx)
			{
				const Pulse pulse = currentPulse + lineIdx * oneLinePulse;

				WriteNoteLine(stream, chartData, laserSegments, pulse, oneLinePulse, state, useLegacyScaleForManualTilt);
			}

			stream << kMeasureSeparator << "\r\n";
			currentPulse += measureLength;
			++measureIdx;
		}
	}

	// Write audio effect definitions (#define_fx and #define_filter)
	void WriteAudioEffectDefinitions(std::ostream& stream, const ChartData& chartData)
	{
		// Write #define_fx
		if (!chartData.audio.audioEffect.fx.def.empty())
		{
			for (const auto& [name, def] : chartData.audio.audioEffect.fx.def)
			{
				stream << "#define_fx " << name << " type=";
				const std::string_view typeStr = AudioEffectTypeToStr(def.type);
				if (kKSONToKSHAudioEffectTypeName.contains(typeStr))
				{
					stream << kKSONToKSHAudioEffectTypeName.at(typeStr);
				}
				else
				{
					stream << typeStr;
				}

				for (const auto& [paramName, value] : def.v)
				{
					stream << ";";
					if (kKSONToKSHParamName.contains(paramName))
					{
						stream << kKSONToKSHParamName.at(paramName);
					}
					else
					{
						stream << paramName;
					}
					stream << "=" << value;
				}
				stream << "\r\n";
			}
		}

		// Write #define_filter
		if (!chartData.audio.audioEffect.laser.def.empty())
		{
			for (const auto& [name, def] : chartData.audio.audioEffect.laser.def)
			{
				stream << "#define_filter " << name << " type=";
				const std::string_view typeStr = AudioEffectTypeToStr(def.type);
				if (kKSONToKSHAudioEffectTypeName.contains(typeStr))
				{
					stream << kKSONToKSHAudioEffectTypeName.at(typeStr);
				}
				else
				{
					stream << typeStr;
				}

				for (const auto& [paramName, value] : def.v)
				{
					stream << ";";
					if (kKSONToKSHParamName.contains(paramName))
					{
						stream << kKSONToKSHParamName.at(paramName);
					}
					else
					{
						stream << paramName;
					}
					stream << "=" << value;
				}
				stream << "\r\n";
			}
		}
	}
}

kson::ErrorType kson::SaveKSHChartData(std::ostream& stream, const ChartData& chartData)
{
	if (!stream.good())
	{
		return ErrorType::GeneralIOError;
	}

	WriteBOM(stream);

	MeasureExportState state;
	std::vector<std::string> warnings;

	// Write header and store the header BPM string in state
	WriteHeader(stream, chartData, &state.headerBPMStr);
	WriteMeasures(stream, chartData, state, warnings);
	WriteAudioEffectDefinitions(stream, chartData);

	// Output warnings to stderr
	for (const auto& warning : warnings)
	{
		std::cerr << "Warning: " << warning << std::endl;
	}

	return stream.good() ? ErrorType::None : ErrorType::GeneralIOError;
}

kson::ErrorType kson::SaveKSHChartData(const std::string& filePath, const ChartData& chartData)
{
	std::ofstream ofs(filePath, std::ios_base::binary);
	if (!ofs.good())
	{
		return ErrorType::GeneralIOError;
	}

	const ErrorType result = SaveKSHChartData(ofs, chartData);
	ofs.close();

	return result;
}
