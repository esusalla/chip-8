#include "chip8.hpp"

#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>

const std::array<std::uint8_t, 80> Chip8::font_data = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
    0x20, 0x60, 0x20, 0x20, 0x70,  // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
    0xF0, 0x80, 0xF0, 0x80, 0x80   // F
};
const std::uint8_t Chip8::view_width;   // Internal graphics width
const std::uint8_t Chip8::view_height;  // Internal graphics height

Chip8::Chip8(const char* game) : PC{program_start}, I{0}, SP{0}, DT{0}, ST{0}, opcode{0} {
    // Clear input and graphics
    keypad.fill(0);
    framebuffer.fill(0);

    // Clear memory and registers
    memory.fill(0);
    V.fill(0);
    stack.fill(0);

    // Load font data into start of memory
    for (int i = 0; i < font_data.size(); ++i) { memory[i] = font_data[i]; }

    // Open game file
    std::ifstream file(game, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Could not open game file: " << game << std::endl;
        std::exit(1);
    }

    // Check that file will fit in available memory
    if (file.tellg() > (memory.size() - program_start)) {
        std::cerr << "Game file is too large: " << game << std::endl;
        std::exit(1);
    }

    // Load game into start of program memory
    auto i = 0;
    std::uint8_t byte;
    file.seekg(0, std::ios::beg);
    while (file >> std::noskipws >> byte) { memory[program_start + i++] = byte; }
    file.close();

    // Pre-C++11 style randomness
    std::srand(std::time(0));
}

void Chip8::emulate_cycle() {
    // Get current opcode
    opcode = memory[PC] << 8 | memory[PC + 1];

    switch (opcode & 0xF000) {
        case 0x0000: {
            switch (opcode & 0x00FF) {
                // 00E0 - CLS - Clear the display
                case 0x00E0: {
                    framebuffer.fill(0);
                    PC += 2;
                    break;
                }

                // 00EE - RET - Return from a subroutine
                case 0x00EE: {
                    PC = stack[--SP];
                    PC += 2;
                    break;
                }

                default: {
                    std::cerr << "Unsupported opcode: 0x" << std::hex << opcode << std::endl;
                    std::exit(1);
                }
            }
            break;
        }

        // 1NNN - JP addr - Jump to location NNN
        case 0x1000: {
            PC = NNN();
            break;
        }

        // 2NNN - CALL addr - Call subroutine at NNN
        case 0x2000: {
            stack[SP++] = PC;
            PC = NNN();
            break;
        }

        // 3XKK - SE VX, byte - Skip next instruction if VX = KK
        case 0x3000: {
            if (V[X()] == KK()) {
                PC += 4;
            } else {
                PC += 2;
            }
            break;
        }

        // 4XKK - SNE VX, byte - Skip next instruction if VX != KK
        case 0x4000: {
            if (V[X()] != KK()) {
                PC += 4;
            } else {
                PC += 2;
            }
            break;
        }

        // 5XY0 - SE VX, VY - Skip next instruction if VX = VY
        case 0x5000: {
            if (V[X()] == V[Y()]) {
                PC += 4;
            } else {
                PC += 2;
            }
            break;
        }

        // 6XKK - LD VX, byte - Set VX = KK
        case 0x6000: {
            V[X()] = KK();
            PC += 2;
            break;
        }

        // 7XKK = ADD VX, byte - Set VX = VX + KK
        case 0x7000: {
            V[X()] += KK();
            PC += 2;
            break;
        }

        case 0x8000: {
            switch (opcode & 0x000F) {
                // 8XY0 - LD VX, VY - Set VX = VY
                case 0x0000: {
                    V[X()] = V[Y()];
                    PC += 2;
                    break;
                }

                // 8XY1 - OR VX, VY - Set VX = VX OR VY
                case 0x0001: {
                    V[X()] |= V[Y()];
                    PC += 2;
                    break;
                }

                // 8XY2 - AND VX, VY - Set VX = VX AND VY
                case 0x0002: {
                    V[X()] &= V[Y()];
                    PC += 2;
                    break;
                }

                // 8XY3 - XOR VX, VY - Set VX = VX XOR VY
                case 0x0003: {
                    V[X()] ^= V[Y()];
                    PC += 2;
                    break;
                }

                // 8XY4 - ADD VX, VY - Set VX = VX + VY, set VF = carry
                case 0x0004: {
                    if ((0xFF - V[X()]) < V[Y()]) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[X()] += V[Y()];
                    PC += 2;
                    break;
                }

                // 8XY5 - SUB VX, VY - Set VX = VX - VY, set VF = NOT borrow
                case 0x0005: {
                    if (V[X()] > V[Y()]) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[X()] -= V[Y()];
                    PC += 2;
                    break;
                }

                // 8XY6 - SHR VX {, VY} - Set VX = VX SHR 1
                case 0x0006: {
                    V[0xF] = V[X()] & 1;
                    V[X()] >>= 1;
                    PC += 2;
                    break;
                }

                // 8XY7 - SUBN VX, VY - Set VX = VY - VX, set VF = NOT borrow
                case 0x0007: {
                    if (V[Y()] > V[X()]) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    V[X()] = V[Y()] - V[X()];
                    PC += 2;
                    break;
                }

                // 8XYE - SHL VX {, VY} - Set VX = VX SHL 1
                case 0x000E: {
                    V[0xF] = V[X()] >> 7;
                    V[X()] <<= 1;
                    PC += 2;
                    break;
                }

                default: {
                    std::cerr << "Unsupported opcode: 0x" << std::hex << opcode << std::endl;
                    std::exit(1);
                }
            }
            break;
        }

        // 9XY0 - SNE VX, VY - Skip next instruction if VX != VY
        case 0x9000: {
            if (V[X()] != V[Y()]) {
                PC += 4;
            } else {
                PC += 2;
            }
            break;
        }

        // ANNN - LD I, addr - Set I = NNN
        case 0xA000: {
            I = NNN();
            PC += 2;
            break;
        }

        // BNNN - JP V0, addr - Jump to location NNN + V0
        case 0xB000: {
            PC = NNN() + V[0];
            break;
        }

        // CXKK - RND VX, byte - Set VX = random byte AND KK
        case 0xC000: {
            V[X()] = (std::rand() % 0xFF) & KK();
            PC += 2;
            break;
        }

        // DXYN - DRW VX, VY, nibble
        // Conflicting tech specs on whether out of bounds pixels should wrap or clip
        case 0xD000: {
            auto x = V[X()];
            auto y = V[Y()];
            auto height = opcode & 0x000F;

            // Clear carry register
            V[0xF] = 0;

            // Iterate through bytes of sprite
            for (int row = 0; row < height; ++row) {
                auto byte = memory[I + row];

                // If the sprite goes below the bottom of the screen, clip it and stop drawing
                if ((y + row) >= view_height) { break; }

                // Iterate through bits in sprite byte
                for (int col = 0; col < 8; ++col) {
                    // If the sprite goes off the right side of the screen, clip it and stop drawing
                    if ((x + col) >= view_width) { break; }

                    // If the bit is set, xor the corresponding pixel in the framebuffer
                    if (byte & (1 << (7 - col))) {
                        // Get index position of this pixel in the framebuffer
                        auto pixel = (y + row) * view_width + (x + col);

                        // Check for collision and set carry register if detected
                        if (framebuffer[pixel]) { V[0xF] = 1; }
                        framebuffer[pixel] ^= 0xFF;
                    }
                }
            }
            PC += 2;
            break;
        }

        case 0xE000: {
            switch (opcode & 0x00FF) {
                // EX9E - SKP VX - Skip next instruction if key with the value of VX is pressed
                case 0x009E: {
                    if (keypad[V[X()]]) {
                        PC += 4;
                    } else {
                        PC += 2;
                    }
                    break;
                }

                // EXA1 - SKNP VX - Skip the next instruction if key with the value VX is not pressed
                case 0x00A1: {
                    if (!keypad[V[X()]]) {
                        PC += 4;
                    } else {
                        PC += 2;
                    }
                    break;
                }

                default: {
                    std::cerr << "Unsupported opcode: 0x" << std::hex << opcode << std::endl;
                    std::exit(1);
                }
            }
            break;
        }

        case 0xF000: {
            switch (opcode & 0x00FF) {
                // FX07 - LD VX, DT - Set VX = delay timer value
                case 0x0007: {
                    V[X()] = DT;
                    PC += 2;
                    break;
                }

                // FX0A - LD VX, K - Wait for a key press, store the value of the key in VX
                case 0x000A: {
                    auto pressed = false;
                    for (int i = 0; i < keypad.size(); ++i) {
                        if (keypad[i]) {
                            V[X()] = i;
                            pressed = true;
                            break;
                        }
                    }

                    // If a key was pressed, increment PC to move to next instruction
                    if (pressed) { PC += 2; }
                    break;
                }

                // FX15 - LD DT, VX - Set delay timer = VX
                case 0x0015: {
                    DT = V[X()];
                    PC += 2;
                    break;
                }

                // FX18 - LD ST, VX - Set sound timer = VX
                case 0x0018: {
                    ST = V[X()];
                    PC += 2;
                    break;
                }

                // FX1E - ADD I, VX - Set I = I + VX
                case 0x01E: {
                    I += V[X()];
                    PC += 2;
                    break;
                }

                // FX29 - LD F, VX - Set I = location of sprite for digit VX
                case 0x029: {
                    // Each font sprite is 5 bytes long
                    I = V[X()] * 5;
                    PC += 2;
                    break;
                }

                // FX33 - LD B, VX - Store BCD representation of VX in memory location I, I + 1, and I + 2
                case 0x033: {
                    const auto vx = V[X()];
                    memory[I] = vx / 100;             // Isolate hundreds
                    memory[I + 1] = (vx % 100) / 10;  // Isolate tens
                    memory[I + 2] = (vx % 10);        // Isolate ones
                    PC += 2;
                    break;
                }

                // FX55 - LD [I], VX - Store V0 to VX in memory starting at address I
                // Conflicting tech specs on whether I itself should be incremented at each step
                case 0x055: {
                    for (int i = 0; i <= X(); ++i) {
                        // memory[I++] = V[i];
                        memory[I + i] = V[i];
                    }
                    PC += 2;
                    break;
                }

                // FX65 - LD VX, [I] - Fills V0 to VX with values from memory starting at address I
                // Conflicting tech specs on whether I itself should be incremented at each step
                case 0x065: {
                    for (int i = 0; i <= X(); ++i) {
                        // V[i] = memory[I++];
                        V[i] = memory[I + i];
                    }
                    PC += 2;
                    break;
                }

                default: {
                    std::cerr << "Unsupported opcode: 0x" << std::hex << opcode << std::endl;
                    std::exit(1);
                }
            }
            break;
        }

        default: {
            std::cerr << "Unsupported opcode: 0x" << std::hex << opcode << std::endl;
            std::exit(1);
        }
    }
}

void Chip8::decrement_timers() {
    if (DT > 0) { --DT; }
    if (ST > 0) { --ST; }
}
