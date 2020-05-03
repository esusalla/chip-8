#include "chip8.h"

#include <chrono>
#include <iostream>
#include <thread>

#include "chip8.h"
#include "platform.h"

int main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cout << "Usage: chip8 GAME " << std::endl;
    return 1;
  }

  Chip8 chip8(argv[1]);
  Platform platform(chip8.get_view_dimensions());

  // Emulation speed configuration
  const auto cycle_rate = 540;  // CPU clock rate
  const auto refresh_rate = 60; // Input handling and display render rate
  const auto cycles_per_refresh = cycle_rate / refresh_rate;
  const auto nanos_per_refresh = std::chrono::nanoseconds(1000000000 / refresh_rate);

  auto running = true;
  while (running) {
    const auto start = std::chrono::high_resolution_clock::now();

    platform.handle_input(running, chip8.get_keypad());

    for (int i = 0; i < cycles_per_refresh; ++i)
      chip8.emulate_cycle();

    platform.render(chip8.get_pixels());

    chip8.decrement_timers();
    if (chip8.get_sound_timer())
      platform.play_audio();

    const auto end = std::chrono::high_resolution_clock::now();

    std::this_thread::sleep_for(nanos_per_refresh - (end - start));
  }

  return 0;
}