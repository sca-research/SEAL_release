#!/usr/bin/python3

import sys

import seal


path = sys.argv[1]

sio = seal.IO(path, verbose=True)

while True:
    try:
        print("Msg:", end='', flush=True)
        l = sys.stdin.readline()
        if len(l) == 0:
            break

        sendb = bytes(l[0], 'utf-8')
        print("Send: ({}){}".format(type(sendb), sendb))
        sio.Putchar(sendb)
        recvb = sio.Getchar()
        print("Received: {}".format(recvb))

    except Exception as err:
        print("Connection error: {}".format(err))
        break

print("Connection terminated.")
exit(0)
