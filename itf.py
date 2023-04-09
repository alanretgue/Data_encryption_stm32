import serial
import struct

def init_board(tty: str):
    s = serial.Serial(tty, baudrate=115200)
    return s

def generate_key(board):
    board.write(struct.pack("BB", 9, 0))
    res = board.read(1).decode("utf-8")
    return res[0] == '0'


### update to read and write in files
def aes_encrypt(board, infile, outfile):
    f = open(infile)
    txt = f.read().encode()
    f.close()

    ## FLAGS used in header
    flag = 2
    flag_end = 8

    res = b''
    for i in range(256, len(txt), 256):
        msg = txt[i - 256 : i]
        board.write(struct.pack("BB", flag, len(msg)))
        board.write(msg)
        error = board.read(1).decode("utf-8")
        if error == '1':
            return False
        padding = board.read(1) ## should be 0
        res += board.read(256 + int.from_bytes(padding, "big"))

    size = len(txt) % 256
    msg = txt[len(txt) - size :]
    board.write(struct.pack("BB", flag + flag_end, len(msg)))
    board.write(msg)

    error = board.read(1).decode("utf-8")
    if error == '1':
        return False
    padding = board.read(1)

    res += board.read(len(msg) + int.from_bytes(padding, "big"))

    o = open(outfile, "wb")
    o.write(res)
    o.close()
    return True

def aes_decrypt(board, infile, outfile):
    f = open(infile, "rb")
    txt = f.read()
    f.close()
    print(txt)

    ## FLAGS used in header
    flag = 4
    flag_end = 8

    res = ""
    for i in range(256, len(txt), 256):
        msg = txt[i - 256 : i]
        board.write(struct.pack("BB", flag, 256))
        board.write(msg)

        error = board.read(1).decode("utf-8")
        if error == '1':
            return False
        padding = board.read(1) ## should be 0

        res += board.read(256 - int.from_bytes(padding, "big")).decode("utf-8")

    size = len(txt) % 256
    print(size)
    msg = txt[len(txt) - size :]
    board.write(struct.pack("BB", flag + flag_end, size))
    board.write(msg)

    error = board.read(1).decode("utf-8")
    print(error)
    if error == '1':
        return False
    padding = board.read(1)
    print(int.from_bytes(padding, "big"))

    res += board.read(size - int.from_bytes(padding, "big")).decode("utf-8")
    print(res)

    o = open(outfile, "w")
    o.write(res)
    o.close()
    return True


if __name__ == "__main__":
    s = init_board("/dev/ttyACM0")
    ## s.write(struct.pack("BB", 8, 5))
    ## s.write(b'hoooo')
    ## while True:
    ##     print(s.read(5))
    # generate_key(s)
    aes_encrypt(s, None, None)
