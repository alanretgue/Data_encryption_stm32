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

    # Case where the file is empty
    if len(txt) == 0:
        o = open(outfile, "wb")
        o.close()
        board.write(struct.pack("BB", flag_end, 0))
        board.write(b'0')
        # Check if communication is enable
        error = board.read(1).decode("utf-8")
        if error == '1':
            return False
        return True

    x = [ txt[i:i+256] for i in range(0, len(txt), 256) ]
    print(x)

    res = b''

    for s in x[:-1]:
        print("in for")
        board.write(struct.pack("BB", flag, len(s) - 1))
        board.write(s)

        # Check if communication is enable
        error = board.read(1).decode("utf-8")
        if error == '1':
            return False

        # Look for padding
        padding = board.read(1) ## should be 0
        # concatenate result
        res += board.read(256 + int.from_bytes(padding, "big"))

    print("out for")
    board.write(struct.pack("BB", flag + flag_end, len(x[-1]) - 1))
    board.write(x[-1])

    # Check if communication is enable
    error = board.read(1).decode("utf-8")
    if error == '1':
        return False
    # Look for padding
    padding = board.read(1)
    # concatenate result
    res += board.read(len(x[-1]) + int.from_bytes(padding, "big"))

    # write result in ouput file
    o = open(outfile, "wb")
    o.write(res)
    o.close()
    return True

def aes_decrypt(board, infile, outfile):
    f = open(infile, "rb")
    txt = f.read()
    f.close()

    ## FLAGS used in header
    flag = 4
    flag_end = 8

    if len(txt) == 0:
        o = open(outfile, "w")
        o.close()
        board.write(struct.pack("BB", flag_end, 0))
        board.write(b'0')
        # Check if communication is enable
        error = board.read(1).decode("utf-8")
        if error == '1':
            return False
        return True

    x = [ txt[i:i+256] for i in range(0, len(txt), 256) ]

    res = ""

    for s in x[:-1]:
        board.write(struct.pack("BB", flag, len(s) - 1))
        board.write(s)

        # Check if communication is enable
        error = board.read(1).decode("utf-8")
        if error == '1':
            return False

        # Look for padding
        padding = board.read(1) ## should be 0

        # concatenate result
        res += board.read(256 - int.from_bytes(padding, "big")).decode("utf-8")

    board.write(struct.pack("BB", flag + flag_end, len(x[-1]) - 1))
    board.write(x[-1])

    # Check if communication is enable
    error = board.read(1).decode("utf-8")
    if error == '1':
        return False

    # Look for padding
    padding = board.read(1)
    # concatenate result
    res += board.read(len(x[-1]) - int.from_bytes(padding, "big")).decode("utf-8")

    # write result in ouput file
    o = open(outfile, "w")
    o.write(res)
    o.close()
    return True

