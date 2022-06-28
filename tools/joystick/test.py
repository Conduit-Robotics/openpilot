# pylint: disable-all
from cereal import messaging

def main():
    sub_sock = messaging.sub_sock('testJoystick')
    while True:
        dat = messaging.recv_one(sub_sock)
        if dat is None:
            continue
        print(dat)
    
if __name__ == "__main__":
    main()