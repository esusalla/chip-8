#ifndef PLATFORM_HPP
#define PLATFORM_HPP

#include <SDL2/SDL.h>

#include <array>
#include <cstdint>
#include <unordered_map>
#include <utility>

class Platform {
public:
    Platform(std::pair<std::uint8_t, std::uint8_t> view_dimensions);
    ~Platform();

    void handle_input(bool& running, std::array<std::uint8_t, 16>& keypad);
    void render(const void* pixels);
    void play_audio();

private:
    static const std::unordered_map<SDL_Keycode, std::uint8_t> keymap;  // Key press to keypad map
    static const std::uint8_t scale = 20;  // Screen pixels per internal pixel

    const std::uint8_t view_width;
    const std::uint8_t view_height;

    // Input
    SDL_Event event;

    // Graphics
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;

    // Audio
    SDL_AudioSpec wav_spec;
    std::uint8_t* wav_buffer;
    std::uint32_t wav_length;
    SDL_AudioDeviceID audio_device;
};

#endif  // PLATFORM_H
