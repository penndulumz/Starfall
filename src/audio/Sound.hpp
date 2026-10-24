#pragma once

#include "simulation/Run.hpp"
#include <SDL_mixer.h>
#include <array>
#include <memory>
#include <string>

namespace starfall {

// Recovery advice depends on the failure category, never diagnostic wording.
enum class AudioWarningKind { None, Resource, DeviceUnavailable, Initialization };

// Owns the single mixer/device path, decoded shooting clip, and original event
// samples. Callers send events; no custom audio callback or borrowed PCM remains.
class Sound {
  public:
    explicit Sound(bool enabled);
    ~Sound();
    Sound(const Sound&) = delete;
    Sound& operator=(const Sound&) = delete;

    void play(EventType type);
    void toggle();
    bool enabled() const { return deviceOpened && audible; }
    bool available() const { return deviceOpened; }
    const std::string& warning() const { return warningMessage; }
    AudioWarningKind warningKind() const { return warningCategory; }

  private:
    struct FreeChunk {
        void operator()(Mix_Chunk* value) const { Mix_FreeChunk(value); }
    };
    using Chunk = std::unique_ptr<Mix_Chunk, FreeChunk>;
    std::array<Chunk, 8> clips;
    bool audioSubsystem = false;
    bool mixerInitialized = false;
    bool deviceOpened = false;
    bool audible = true;
    std::string warningMessage;
    AudioWarningKind warningCategory = AudioWarningKind::None;

    void initialize(bool requested);
    static Chunk originalEffect(EventType type);
    void disable(AudioWarningKind kind, const std::string& reason);
    void release();
};

} // namespace starfall
