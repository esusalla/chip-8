#ifndef CHIP_8_HPP
#define CHIP_8_HPP

#include <array>
#include <cstdint>
#include <utility>

class Chip8 {
public:
    Chip8(const char* game);

    // Return internal view dimensions for platform window
    static inline auto get_view_dimensions() { return std::make_pair(view_width, view_height); }
    // Return writable keypad for platform input
    inline auto& get_keypad() { return keypad; }
    // Return raw pixels for platform graphics
    inline auto get_pixels() const { return framebuffer.data(); }
    // Return sound timer for platform audio
    inline auto get_sound_timer() const { return ST; }

    void emulate_cycle();
    void decrement_timers();

private:
    static const std::array<std::uint8_t, 80> font_data;  // Hexadecimal font sprite data
    static const std::uint16_t program_start = 512;       // Memory address where program is loaded
    static const std::uint8_t view_width = 64;            // Internal graphics width
    static const std::uint8_t view_height = 32;           // Internal graphics height

    std::array<std::uint8_t, 16> keypad;                             // Internal input
    std::array<std::uint8_t, view_width * view_height> framebuffer;  // Internal graphics

    std::array<std::uint8_t, 4096> memory;  // Address space for both code and data
    std::array<std::uint8_t, 16> V;         // General purpose registers
    std::array<std::uint16_t, 16> stack;    // Subroutine address stack

    std::uint16_t PC;  // Program counter
    std::uint16_t I;   // Address register
    std::uint8_t SP;   // Stack pointer
    std::uint8_t DT;   // Delay timer
    std::uint8_t ST;   // Sound timer

    // Current opcode and helpers
    std::uint16_t opcode;
    inline std::uint8_t X() const { return (opcode & 0x0F00) >> 8; }
    inline std::uint8_t Y() const { return (opcode & 0x00F0) >> 4; }
    inline std::uint8_t KK() const { return opcode & 0x00FF; }
    inline std::uint16_t NNN() const { return opcode & 0x0FFF; }
};

#endif  // CHIP_8
