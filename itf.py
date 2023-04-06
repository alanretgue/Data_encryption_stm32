import serial
import struct

def init_board(tty: str):
    s = serial.Serial(tty, baudrate=115200)
    return s

def generate_key(board):
    board.write(struct.pack("BB", 1, 0))
    res = board.read(1).decode("utf-8")
    return res[0] == '0'


### update to read and write in files
def aes_encrypt(board, infile, outfile):
    msg = b'abcdefghijklmnopqrstuvwxyz'
    board.write(struct.pack("BB", 2, len(msg)))
    board.write(msg)
    print(board.read(len(msg) + (16 - (len(msg) % 16))))
    return True

def aes_decrypt(board, infile, outfile):
    f = open(infile)
    text = f.read()
    f.close()
    res = ""
    for i in range(256, len(txt), 256):
        msg = txt[i - 256 : i]
        board.write(struct.pack("BB", 4, 256))
        board.write(msg)
        res += board.read(256)
    size = len(txt) % 256
    msg[len(txt) - size-1 :]
    board.write(struct.pack("BB", 12, size))
    board.write(msg)
    res += board.read(size)
    o = open(outfile)
    o.write(res)
    return True


if __name__ == "__main__":
    s = init_board("/dev/ttyACM0")
    ## s.write(struct.pack("BB", 8, 5))
    ## s.write(b'hoooo')
    ## while True:
    ##     print(s.read(5))
    # generate_key(s)
    aes_encrypt(s, None, None)
