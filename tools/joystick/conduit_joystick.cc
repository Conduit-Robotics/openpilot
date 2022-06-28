#include "tools/joystick/conduit_joystick.h"

#include <algorithm>
#include <iostream>

namespace {

float constexpr kDeadZone = 0.05;
float constexpr kJoyMaxVal = 32767.0;
float constexpr kJoyMinVal = -32768.0;
int constexpr kAutoCenter = 50;

float interp(float xv, std::array<float, 2> xp, std::array<float, 2> fp) {
  int N = 2;
  int hi = 0;
  while ((hi < N) && (xv > xp[hi])) {
    hi += 1;
  }
  int low = hi - 1;
  return ((hi == N) && (xv > xp[low]))
             ? fp[1]
             : ((hi == 0)
                    ? fp[0]
                    : (xv - xp[low]) * (fp[hi] - fp[low]) / (xp[hi] - xp[low]) +
                          fp[low]);
}

float normalize_sdl_values(float x) {
  float norm = -1 * interp(x, std::array<float, 2>{kJoyMinVal, kJoyMaxVal},
                           std::array<float, 2>{-1, 1});
  return ((std::abs(norm) > kDeadZone)) ? norm : 0;
}

}  // namespace

ConduitJoystick::ConduitJoystick(int dev_id) {
  dev_id_ = dev_id;
  initializeJoystick();
  initializeHaptic();
}

ConduitJoystick::~ConduitJoystick() {
  if (joystick_ != nullptr) {
    printf("Closing Joystick!");
    SDL_JoystickClose(joystick_);
  }
  if (haptic_ != nullptr) {
    printf("Closing Haptic!");
    SDL_HapticClose(haptic_);
  }
  SDL_Quit();
}

void ConduitJoystick::initializeHaptic() {
  haptic_ = SDL_HapticOpenFromJoystick(joystick_);
  if (haptic_ == nullptr) {
    printf("Unable to create force feedback: %s", SDL_GetError());
  }
  SDL_HapticSetAutocenter(haptic_, kAutoCenter);
}

void ConduitJoystick::initializeJoystick() {
  if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC) < 0) {
    throw std::runtime_error("SDL could not be initialized: " +
                             std::string(SDL_GetError()));
  }
  joystick_ = SDL_JoystickOpen(dev_id_);
  if (joystick_ == nullptr) {
    printf("Unable to open joystick %d: %s", dev_id_, SDL_GetError());
    return;
  }

  joystick_instance_id_ = SDL_JoystickGetDeviceInstanceID(dev_id_);
  if (joystick_instance_id_ < 0) {
    printf("Failed to get instance ID for joystick: %s", SDL_GetError());
    SDL_JoystickClose(joystick_);
    joystick_ = nullptr;
    return;
  }

  int num_buttons = SDL_JoystickNumButtons(joystick_);
  if (num_buttons < 0) {
    printf("Failed to get number of buttons: %s", SDL_GetError());
    SDL_JoystickClose(joystick_);
    joystick_ = nullptr;
    return;
  }

  int num_axes = SDL_JoystickNumAxes(joystick_);
  if (num_axes < 0) {
    printf("Failed to get number of axes: %s", SDL_GetError());
    SDL_JoystickClose(joystick_);
    joystick_ = nullptr;
    return;
  }
  int num_hats = SDL_JoystickNumHats(joystick_);
  if (num_hats < 0) {
    printf("Failed to get number of hats: %s", SDL_GetError());
    SDL_JoystickClose(joystick_);
    joystick_ = nullptr;
    return;
  }
}

void ConduitJoystick::poll() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_JOYAXISMOTION) {
      std::cout << event.jaxis.axis << std::endl;
      switch (event.jaxis.axis) {
        case 0:
          onSteer(event);
          break;
        case 1:
          onGas(event);
          break;
        case 2:
          onBrake(event);
          break;
      }
    }
  }
}

void ConduitJoystick::onSteer(const SDL_Event& event) {
  steer_ = normalize_sdl_values(event.jaxis.value);
}

void ConduitJoystick::onGas(const SDL_Event& event) {
  gas_ = normalize_sdl_values(event.jaxis.value);
}

void ConduitJoystick::onBrake(const SDL_Event& event) {
  brake_ = normalize_sdl_values(event.jaxis.value);
}
