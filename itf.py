import serial
import struct

def init_board(tty: str):
    s = serial.Serial(tty, baudrate=115200)
    return s

def generate_key(board):
    board.write(struct.pack("BB", 1, 0))
    res = board.read(1).decode("utf-8")
    return res[0] == '0'

def aes_encrypt(board, infile, outfile):
    msg = b'abcdefghijklmnopqrstuvwxyz'
    board.write(struct.pack("BB", 2, len(msg)))
    board.write(msg)
    print(board.read(len(msg) + (16 - (len(msg) % 16))))
    return True

if __name__ == "__main__":
    s = init_board("/dev/ttyACM0")
    ## s.write(struct.pack("BB", 8, 5))
    ## s.write(b'hoooo')
    ## while True:
    ##     print(s.read(5))
    # generate_key(s)
    aes_encrypt(s, None, None)
