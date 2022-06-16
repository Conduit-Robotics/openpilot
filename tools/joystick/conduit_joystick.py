#!/usr/bin/env python
import threading
from inputs import get_gamepad

import cereal.messaging as messaging
from common.realtime import Ratekeeper
from common.numpy_fast import interp
from common.params import Params


class Joystick:
  STEER_CODE: str = 'ABS_X'
  GAS_CODE: str = 'ABS_Y'
  BRAKE_CODE: str = 'ABS_Z'

  DEADZONE: float = 0.025

  MAX_STEER_VALUE: float = 65535.
  MIN_STEER_VALUE: float = 0.
  MAX_BRAKE_VALUE: float = 255.
  MIN_BRAKE_VALUE: float = 0.
  MAX_GAS_VALUE: float = 255.
  MIN_GAS_VALUE: float = 0.

  STEER_START: float = 0.
  GAS_START: float = -1.
  BRAKE_START: float = -1.

  @property
  def gas_brake(self) -> float:
    return (self.gas/2) - (self.brake/2)

  def __init__(self):
    self.steer: float = self.STEER_START
    self.gas: float = self.GAS_START
    self.brake: float = self.BRAKE_START

  def run(self) -> None:
    while True:
      event = get_gamepad()[0]
      code = event.code
      state = event.state
      if code == self.STEER_CODE:
        self.on_steer(state)
      elif event.code == self.GAS_CODE:
        self.on_gas(state)
      elif event.code == self.BRAKE_CODE:
        self.on_brake(state)
      else:
        print(f'Code: {code} not recognized!')
    
  def on_steer(self, state: float) -> None:
    self.steer = self.normalize_controls_value(state, self.MIN_STEER_VALUE, self.MAX_STEER_VALUE)

  def on_brake(self, state: float) -> None:
    self.brake = self.normalize_controls_value(state, self.MIN_BRAKE_VALUE, self.MAX_BRAKE_VALUE)

  def on_gas(self, state: float) -> None:
    self.gas = self.normalize_controls_value(state, self.MIN_GAS_VALUE, self.MAX_GAS_VALUE)

  def normalize_controls_value(self, curr_val: float, min_val: float, max_val: float, lower_bound: float = -1., higher_bound: float = 1.) -> float:
    norm = -interp(curr_val, [min_val, max_val], [lower_bound, higher_bound])
    control_val = norm if abs(norm) > self.DEADZONE else 0.
    return float(control_val)
    

def pub_thread(joystick: Joystick) -> None:
  pub = messaging.pub_sock('testJoystick')
  rk = Ratekeeper(100, print_delay_threshold=None)
  while True:
    dat = messaging.new_message('testJoystick')
    dat.testJoystick.axes = [joystick.gas_brake, joystick.steer]
    dat.testJoystick.buttons = [False]
    pub.send(dat.to_bytes())
    print(f'STEER: {joystick.steer}\nGB: {joystick.gas_brake}')
    rk.keep_time()

def main() -> None:
  Params().put_bool('JoystickDebugMode', True)
  joystick = Joystick()
  threading.Thread(target=pub_thread, args=(joystick,), daemon=True).start()
  joystick.run()

if __name__ == '__main__':
  main()

