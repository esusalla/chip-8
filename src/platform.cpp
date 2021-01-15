#include "platform.hpp"

#include <SDL2/SDL.h>

#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>

const std::unordered_map<SDL_Keycode, std::uint8_t> Platform::keymap{
    {SDLK_1, 0x01}, {SDLK_2, 0x02}, {SDLK_3, 0x03}, {SDLK_4, 0x0C}, {SDLK_q, 0x04}, {SDLK_w, 0x05},
    {SDLK_e, 0x06}, {SDLK_r, 0x0D}, {SDLK_a, 0x07}, {SDLK_s, 0x08}, {SDLK_d, 0x09}, {SDLK_f, 0x0E},
    {SDLK_z, 0x0A}, {SDLK_x, 0x00}, {SDLK_c, 0x0B}, {SDLK_v, 0x0F}};

Platform::Platform(std::pair<std::uint8_t, std::uint8_t> view_dimensions)
    : view_width{view_dimensions.first},
      view_height{view_dimensions.second},
      window{nullptr},
      renderer{nullptr},
      texture{nullptr} {
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_VIDEO)) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        std::exit(1);
    }
    std::atexit(SDL_Quit);

    window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              view_width * scale, view_height * scale, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        std::exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_DestroyWindow(window);
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        std::exit(1);
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STREAMING,
                                view_width, view_height);
    if (!texture) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
        std::exit(1);
    }

    // Attempt to load audio file, but don't exit if anything fails
    if (!SDL_LoadWAV("../assets/beep.wav", &wav_spec, &wav_buffer, &wav_length)) {
        std::cerr << "Failed to load WAV file: " << SDL_GetError() << std::endl;
    } else {
        audio_device = SDL_OpenAudioDevice(nullptr, 0, &wav_spec, nullptr, 0);
        if (!audio_device) {
            std::cerr << "Failed to open audio device: " << SDL_GetError() << std::endl;
        }
    }
}

Platform::~Platform() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_CloseAudioDevice(audio_device);
    SDL_FreeWAV(wav_buffer);
}

void Platform::handle_input(bool& running, std::array<std::uint8_t, 16>& keypad) {
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT: {
                running = false;
                return;
            }

            case SDL_KEYDOWN: {
                const auto iter = keymap.find(event.key.keysym.sym);
                if (iter != keymap.end()) { keypad[iter->second] = 1; }
                break;
            }

            case SDL_KEYUP: {
                const auto iter = keymap.find(event.key.keysym.sym);
                if (iter != keymap.end()) { keypad[iter->second] = 0; }
                break;
            }

            default: {
                break;
            }
        }
    }
}
void Platform::render(const void* pixels) {
    SDL_UpdateTexture(texture, nullptr, pixels, view_width);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

void Platform::play_audio() {
    if (!SDL_QueueAudio(audio_device, wav_buffer, wav_length)) {
        SDL_PauseAudioDevice(audio_device, 0);
    }
}
