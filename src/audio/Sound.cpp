#include "Sound.hpp"
#include "platform/Paths.hpp"
#include <SDL.h>

#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

namespace starfall {
namespace {

constexpr int SampleRate = 48000;
constexpr int ChannelCount = 2;
constexpr int MixerChannels = 32;

struct Effect {
    float duration;
    float frequency;
    float slide;
    float volume;
    bool noise;
};

// These are Starfall's original non-shot sounds, synthesized once during load.
// All playback subsequently uses owned mixer chunks, including the FLAC shot.
Effect effectFor(EventType type) {
    switch (type) {
    case EventType::Hit: return {.22f, 130, -350, .15f, true};
    case EventType::Explosion: return {.2f, 170, -500, .12f, true};
    case EventType::Pickup: return {.24f, 450, 2300, .1f, false};
    case EventType::Dash: return {.16f, 200, 1800, .07f, true};
    case EventType::Wave: return {.35f, 350, 700, .08f, false};
    case EventType::Win: return {.8f, 440, 650, .12f, false};
    case EventType::Lose: return {.7f, 320, -350, .13f, false};
    case EventType::Shot: break; // The shooting event must use the loaded FLAC.
    }
    return {.1f, 640, -3000, .09f, false};
}

void write16(std::vector<Uint8>& bytes, std::size_t offset, Uint16 value) {
    bytes[offset] = static_cast<Uint8>(value);
    bytes[offset + 1] = static_cast<Uint8>(value >> 8);
}

void write32(std::vector<Uint8>& bytes, std::size_t offset, Uint32 value) {
    write16(bytes, offset, static_cast<Uint16>(value));
    write16(bytes, offset + 2, static_cast<Uint16>(value >> 16));
}

} // namespace

Sound::Sound(bool requested) {
    try {
        initialize(requested);
    } catch (...) {
        release();
        throw;
    }
}

void Sound::initialize(bool requested) {
    if (!requested) {
        audible = false;
        return;
    }
    const auto shootingPath = resourceDirectory() / "shoot.flac";
    std::ifstream resource(shootingPath, std::ios::binary);
    char signature[4]{};
    if (!resource.read(signature, sizeof(signature)) || std::memcmp(signature, "fLaC", 4) != 0) {
        disable(AudioWarningKind::Resource,
                "Audio resource error: missing or corrupt " + shootingPath.u8string() +
                    ". Restore the complete game package.");
        return;
    }
    // Check a missing resource before device discovery, so a device problem does
    // not hide an incomplete package. Full FLAC decoding follows on an open mixer.
    resource.close();
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        disable(AudioWarningKind::DeviceUnavailable,
                "Audio device unavailable: " + std::string(SDL_GetError()));
        return;
    }
    audioSubsystem = true;
    mixerInitialized = true;
    if ((Mix_Init(MIX_INIT_FLAC) & MIX_INIT_FLAC) == 0) {
        disable(AudioWarningKind::Resource,
                "Audio resource error: FLAC decoder unavailable. Restore the complete game package. " +
                    std::string(Mix_GetError()));
        return;
    }
    if (Mix_OpenAudio(SampleRate, AUDIO_S16SYS, ChannelCount, 512) < 0) {
        disable(AudioWarningKind::DeviceUnavailable,
                "Audio device unavailable: " + std::string(Mix_GetError()));
        return;
    }
    deviceOpened = true;
    Mix_AllocateChannels(MixerChannels);
    clips[static_cast<std::size_t>(EventType::Shot)].reset(Mix_LoadWAV(shootingPath.u8string().c_str()));
    if (!clips[static_cast<std::size_t>(EventType::Shot)]) {
        disable(AudioWarningKind::Resource,
                "Audio resource error: cannot decode " + shootingPath.u8string() +
                    ". Restore the complete game package. " + Mix_GetError());
        return;
    }
    Mix_VolumeChunk(clips[static_cast<std::size_t>(EventType::Shot)].get(), MIX_MAX_VOLUME / 5);
    for (auto type : {EventType::Hit, EventType::Explosion, EventType::Pickup, EventType::Dash,
                      EventType::Wave, EventType::Win, EventType::Lose}) {
        clips[static_cast<std::size_t>(type)] = originalEffect(type);
        if (!clips[static_cast<std::size_t>(type)]) {
            disable(AudioWarningKind::Initialization,
                    "Audio initialization failed: " + std::string(Mix_GetError()));
            return;
        }
    }
}

Sound::~Sound() {
    release();
}

void Sound::release() {
    // A channel must stop borrowing a chunk before that chunk is freed. Closing
    // the mixer also stops its callback before SDL audio/library shutdown.
    if (deviceOpened) {
        Mix_HaltChannel(-1);
    }
    for (auto& clip : clips) {
        clip.reset();
    }
    if (deviceOpened) {
        Mix_CloseAudio();
        deviceOpened = false;
    }
    if (mixerInitialized) {
        Mix_Quit();
        mixerInitialized = false;
    }
    if (audioSubsystem) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        audioSubsystem = false;
    }
}

void Sound::disable(AudioWarningKind kind, const std::string& reason) {
    warningCategory = kind;
    warningMessage = reason;
    std::cerr << warningMessage << '\n';
    release();
}

void Sound::play(EventType type) {
    const auto index = static_cast<std::size_t>(type);
    if (enabled() && index < clips.size() && clips[index]) {
        // A full channel pool drops new feedback instead of interrupting an
        // already-playing cue or growing an unbounded queue during rapid fire.
        Mix_PlayChannel(-1, clips[index].get(), 0);
    }
}

void Sound::toggle() {
    if (!available()) {
        return;
    }
    audible = !audible;
    if (!audible) {
        Mix_HaltChannel(-1);
    }
}

Sound::Chunk Sound::originalEffect(EventType type) {
    const auto effect = effectFor(type);
    const auto frames = static_cast<std::size_t>(effect.duration * SampleRate);
    const auto dataBytes = static_cast<Uint32>(frames * ChannelCount * sizeof(Sint16));
    // A small in-memory PCM WAV gives Mix_LoadWAV ownership of converted samples,
    // so no mixer channel borrows a temporary vector after this method returns.
    std::vector<Uint8> wav(44 + dataBytes);
    std::memcpy(wav.data(), "RIFF", 4);
    write32(wav, 4, 36 + dataBytes);
    std::memcpy(wav.data() + 8, "WAVEfmt ", 8);
    write32(wav, 16, 16);
    write16(wav, 20, 1);
    write16(wav, 22, ChannelCount);
    write32(wav, 24, SampleRate);
    write32(wav, 28, SampleRate * ChannelCount * sizeof(Sint16));
    write16(wav, 32, ChannelCount * sizeof(Sint16));
    write16(wav, 34, 16);
    std::memcpy(wav.data() + 36, "data", 4);
    write32(wav, 40, dataBytes);
    float phase = 0;
    std::uint32_t noiseState = 421;
    for (std::size_t frame = 0; frame < frames; ++frame) {
        const float age = static_cast<float>(frame) / SampleRate;
        const float decay = 1.f - age / effect.duration;
        const float envelope = std::min(1.f, age * 200) * decay * decay;
        const float frequency = std::max(30.f, effect.frequency + effect.slide * age);
        phase += frequency / SampleRate;
        phase -= std::floor(phase);
        float wave = std::sin(phase * Pi * 2);
        if (effect.noise) {
            noiseState = noiseState * 1664525u + 1013904223u;
            wave = wave * .3f + (static_cast<float>((noiseState >> 8) & 65535) / 32767.5f - 1) * .7f;
        }
        const auto sample = static_cast<Sint16>(std::clamp(wave * envelope * effect.volume, -1.f, 1.f) * 32767);
        const auto offset = 44 + frame * ChannelCount * sizeof(Sint16);
        write16(wav, offset, static_cast<Uint16>(sample));
        write16(wav, offset + sizeof(Sint16), static_cast<Uint16>(sample));
    }
    SDL_RWops* memory = SDL_RWFromConstMem(wav.data(), static_cast<int>(wav.size()));
    return Chunk(memory ? Mix_LoadWAV_RW(memory, 1) : nullptr);
}

} // namespace starfall
