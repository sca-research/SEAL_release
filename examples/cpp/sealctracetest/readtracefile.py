#!/usr/bin/python3
import sys
import seal

TESTCORE = "testcore.scs"
TESTTRACE = "testtrace.stc"

if len(sys.argv) >= 1:
    TESTTRACE = sys.argv[1]


def PrintComponent(comp):
    print("|{:<12s}|{:<8s}|{:02d}| =".format(
        comp.name, comp.type, comp.len, comp.val), end='')
    for i in range(comp.Size()):
        print(" {:02X}".format(comp.raw[i]), end='')

    if comp.type != 'OCTET':
        print(' (', end='')
        if comp.type == 'BOOL':
            for i in range(comp.len):
                if comp.val[i]:
                    print('T ', end='')
                    pass
                else:
                    print('F ', end='')
                    pass
                pass
            pass
        elif comp.type == 'STRING':
            print("{:s}".format(comp.val), end='')
            pass
        elif comp.type in {'INT16', 'UINT16', 'INT32', 'UINT32'}:
            for i in range(comp.len):
                print("{:02X} ".format(comp.val[i]), end='')
                if i != len(comp.val) - 1:
                    print(',', end='')
        else:
            raise("Unknown Component type")
        print(')', end='')

    print('')
    return


core = seal.Core.Load(TESTCORE)
st = seal.Trace(core)
st.Open(TESTTRACE)
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
