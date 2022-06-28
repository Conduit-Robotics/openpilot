#include <thread>
#include <iostream>

#include "tools/joystick/conduit_joystick.h"

#include "common/params.h"

void joystick_thread_callback(ConduitJoystick* joystick) {
  while (true) {
    joystick->poll();
  }
}

std::thread joystick_thread(ConduitJoystick* joystick) {
  return std::thread(&joystick_thread_callback, joystick);
}

void pub_thread_callback(PubSocket* pub_sock, ConduitJoystick* joystick) {
  while (true) {
    MessageBuilder msg;
    auto dat = msg.initEvent().initTestJoystick();
    dat.setAxes(
        kj::ArrayPtr<const float>{joystick->gas_brake(), joystick->steer()});
    dat.setButtons(kj::ArrayPtr<const bool>{false});
    std::cout << "STEER: " << joystick->steer()
              << " GB: " << joystick->gas_brake() << "\n";
    auto msg_bytes = msg.toBytes();
    pub_sock->send((char*)msg_bytes.begin(), msg_bytes.size());
  }
}

std::thread pub_thread(PubSocket* pub_sock, ConduitJoystick* joystick) {
  return std::thread(&pub_thread_callback, pub_sock, joystick);
}

int main() {
  ConduitJoystick* joystick = new ConduitJoystick(0);
  Context* c = Context::create();
  PubSocket* pub_sock = PubSocket::create(c, "testJoystick");

  std::thread jt = joystick_thread(joystick);
  std::thread pt = pub_thread(pub_sock, joystick);

  Params().putBool("testJoystick", true);

  jt.join();
  pt.join();

  return 0;
}