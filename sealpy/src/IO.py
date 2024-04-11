import sys
import os
import socket
import subprocess


# SealIO class.
class IO:
    def __init__(self, path, verbose=False):
        self.verbose = verbose
        try:
            self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            self.sock.connect(path)
        except BaseException as err:
            raise err

        self.INFO("#Connection established.")
        return

    # Print info message if verbose is set.
    def INFO(self, msg):
        if self.verbose:
            print(msg)
        return

    # Get a byte from SealIO.
    def Getchar(self):
        return self.sock.recv(1)

    # Write a byte from SealIO.
    def Putchar(self, data):
        if type(data) not in {bytes, bytearray}:
            raise Exception("data must be of type bytes or bytearray.")

        if len(data) != 1:
            raise Exception("Only 1 byte is allowed.")

        return self.sock.send(data)
