# Starfall audio

The shooting effect is the unchanged reference `shoot.flac`, decoded by SDL2_mixer. Damage, destruction, pickups, dash, wave announcements, victory, and defeat use Starfall's original sounds, generated once into owned PCM chunks during initialization. SDL2_mixer plays both kinds through the same device and channel pool.

## Player controls and recovery

Use the sound button on every screen, or **M** outside pilot-name entry. The button remains usable while typing a name. **Sound Off** means the device is available and playback is muted. Muting stops currently playing cues; unmuting enables subsequent cues.

**No Audio** means the game could not prepare audio. The footer explains the cause:

| Footer message | Recovery |
| --- | --- |
| Audio resource error | Restore the complete game package, including `resources/reference/shoot.flac`. Keep resources beside the executable in their supplied folders. |
| Audio device unavailable | Check the Windows output device and restart the game to open audio again. Gameplay and score saving remain available. |
| Audio initialization failed | Restart the game to retry preparing sound. Gameplay and score saving remain available. |

Starting from another working directory does not change which clip is loaded: resources are resolved relative to the executable. A missing/corrupt clip is identified separately from an unavailable device. Full diagnostic details include the resource path and decoder/device error in standard error, while the on-screen footer gives a short recovery action.

## Ownership and implementation

`src/audio/Sound.*` owns the mixer initialization, open device, decoded shooting clip, generated event chunks, and playback channels. `src/platform/Paths.*` supplies the same executable-relative resource directory used by visual resources. `Sound(true)` prepares audio; `Sound(false)` is used by controlled application scenarios that intentionally do not open a device. Normal mute/unmute retains the already loaded clips. `warningKind()` exposes a small category (`None`, `Resource`, `DeviceUnavailable`, or `Initialization`) alongside `warning()`'s detailed diagnostic. Screen recovery advice uses that category, so editing a diagnostic cannot change the advice.

The audio device uses 48 kHz, signed 16-bit stereo output with 512-frame buffers. The shooting FLAC is converted by SDL2_mixer to the device format. Other effects are synthesized once using the original frequency, envelope, and deterministic noise parameters, encoded into small in-memory PCM WAV buffers, and loaded as mixer-owned chunks. Their temporary generation buffers do not outlive loading or become borrowed channel data.

The channel pool is bounded at 32. When it is full, an incoming cue is dropped. Existing cues are not interrupted and a firing burst cannot create an unbounded queue. Sound state is changed on the application thread; SDL2_mixer coordinates playback with its audio thread.

Shutdown stops playback channels, frees chunks, closes the mixer device, shuts down its decoder state, and releases the SDL audio-subsystem reference acquired by the owner. Initialization failures use the same cleanup, and unexpected C++ exceptions also release partially initialized audio.

The [resource manifest](../third_party/RESOURCE-MANIFEST.md) records the selected library versions, source pins, dependency notices, and original asset hashes. The user attested permission covering the reference assets; exact terms/source are forthcoming and remain labeled as such. Original generated non-shot samples are Starfall contributions. Companion builds use bundled dr_flac and static FreeType; the game needs SDL2 plus the image/font/mixer DLLs, with no separate FLAC DLL.

## Verification

The existing application-flow suite exercises actual FLAC loading/playback, every gameplay cue, mute/unmute, the sound button during name entry, missing/corrupt clip diagnostics and resource categories, clean/restored warning state, and gameplay with an unavailable SDL audio driver. These scenarios use isolated staged package files and restore them on success or failure. They do not alter the reference source bytes or player scores.

Run the registered application test through CTest. Both GCC 15.2.0 and MSVC 19.50.35725.0 Windows x64 builds passed, together with the focused gameplay/persistence suite. The test output includes resource-warning, device-warning, and restored-audio captures. Audio checks use SDL's dummy driver and verify decoding/mixing. Audible output from a real speaker has not been verified in this implementation pass.

One shared CMake staging target prepares resources and DLLs before the game and application tests run. This prevents concurrent executable links from trying to overwrite the same staged resource directory.
