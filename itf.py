import serial
import struct

def init_board(tty: str):
    s = serial.Serial(tty, baudrate=115200)
    return s

if __name__ == "__main__":
    s = init_board("/dev/ttyACM0")
    s.write(struct.pack("BB", 8, 5))
    s.write(b'hoooo')
    while True:
        print(s.read(5))
