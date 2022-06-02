# Copyright (c) Kuba Szczodrzyński 2022-06-02.


def crc16(data):
    # https://gist.github.com/pintoXD/a90e398bba5a1b6c121de4e1265d9a29
    crc = 0x0000
    for b in data:
        crc ^= b
        for j in range(0, 8):
            if (crc & 0x0001) > 0:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc = crc >> 1
    return crc
