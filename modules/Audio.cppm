module;

#include "kson/Audio/AudioEffect.hpp"
#include "kson/Audio/AudioInfo.hpp"
#include "kson/Audio/BGMInfo.hpp"
#include "kson/Audio/KeySound.hpp"

export module kson:Audio;

export namespace kson {
    using kson::AudioEffectType;
    using kson::StrToAudioEffectType;
    using kson::AudioEffectTypeToStr;
    using kson::AudioEffectParams;
    using kson::AudioEffectDef;
    using kson::AudioEffectDefKVP;
    using kson::AudioEffectFXInfo;
    using kson::AudioEffectLaserLegacyInfo;
    using kson::AudioEffectLaserInfo;
    using kson::AudioEffectInfo;
    using kson::AudioInfo;
    using kson::MetaAudioInfo;
    using kson::BGMPreviewInfo;
    using kson::LegacyBGMInfo;
    using kson::BGMInfo;
    using kson::MetaBGMInfo;
    using kson::KeySoundInvokeFX;
    using kson::KeySoundInvokeListFX;
    using kson::KeySoundFXInfo;
    using kson::KeySoundInvokeListLaser;
    using kson::KeySoundLaserInfo;
    using kson::KeySoundLaserInfo;
    using kson::KeySoundInfo;
}
