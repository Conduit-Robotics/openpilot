#pragma once

#include <SDL.h>

#include "messaging/messaging.h"

class ConduitJoystick {
 public:
  ~ConduitJoystick();
  ConduitJoystick(int dev_id);
  float steer() {
    return steer_;
  }
  float gas_brake() {
    return (gas_ / 2) - (brake_ / 2);
  }
  void poll();

 private:
  SDL_Joystick* joystick_{nullptr};
  SDL_Haptic* haptic_{nullptr};

  void initializeJoystick();
  void initializeHaptic();
  void onSteer(const SDL_Event& event);
  void onGas(const SDL_Event& event);
  void onBrake(const SDL_Event& event);

  float gas_{-1};
  float brake_{-1};
  float steer_{0};

  int dev_id_;
  int32_t joystick_instance_id_;
};
