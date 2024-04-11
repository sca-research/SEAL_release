#!/usr/bin/python3
import sys
import time
import subprocess
import os

import seal


TESTCORE = "testcore.scs"
TESTTRACE = "/tmp/testtrace.if"


def PrintComponent(comp):
    print("|{:<12s}|{:<8s}|{:02d}| =".format(
        comp.name, comp.type, comp.len, comp.val), end='')
    for i in range(comp.Size()):
        print(" {:02X}".format(comp.raw[i]), end='')

    if comp.type != 'OCTET':
        print(' (', end='')
        if comp.type == "STRING":
            print("{:s}".format(comp.val), end='')
            pass
        elif comp.type in {'INT32', 'UINT32'}:
            for i in range(len(comp.val)):
                print("{:02X}".format(comp.val[i]), end='')
                if i != len(comp.val) - 1:
                    print(',', end='')
        else:
            raise("Unknown Component type")
        print(')', end='')

    print('')
    return


tracefile = TESTTRACE
if len(sys.argv) > 1:
    tracefile = sys.argv[1]

core = seal.Core.Load(TESTCORE)
st = seal.Trace(core)

# Open an emulator.
st.OpenEmulator(["./gentrace", tracefile], tracefile, stdout=open('/dev/null'))
print("#Trace opened.")
count = 0
while True:
    frame = st.NextFrame()
    if not frame:
        break
    print("======= FRAME {:02} =======".format(count))
    for i in frame.components:
        PrintComponent(frame.components[i])
        pass
    count += 1
    print("========********========\n")
    time.sleep(1)
