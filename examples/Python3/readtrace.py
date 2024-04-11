#!/usr/bin/python3
# Example for reading a Trace file.

import sys
import seal

# Verbose flag.
verbose = False


def PrintComponent(comp):
    global verbose

    print("|{:<12s}|{:<8s}|{:02d}| =".format(
        comp.name, comp.type, comp.len, comp.val), end='')

    if verbose:
        for i in range(comp.Size()):
            print(" {:02X}".format(comp.raw[i]), end='')
        pass

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
    elif comp.type in {'OCTET', 'INT16', 'UINT16', 'INT32', 'UINT32'}:
        for i in range(comp.len):
            print("{:02X} ".format(comp.val[i]), end='')
            if i != len(comp.val) - 1:
                print(',', end='')
    else:
        raise ("Unknown Component type")
    print(')', end='')

    print('')
    return


def main(argc, argv):
    global verbose

    # Cmd args check.
    if argc < 3 or '-h' in argv or '--help' in argv:
        print(
            "Usage: {}  {{Core Specification}} {{Trace file or FIFO}} [-v|--verbose] [-h|--help]".format(argv[0]))
        return 0

    # Set verbose flag.
    if '-v' in argv or '--verbose' in argv:
        verbose = True
        pass

    # Read Core specification.
    core = seal.Core.Load(argv[1])
    st = seal.Trace(core)

    # Open Trace file.
    st.Open(argv[2])

    count = 0
    while True:
        # Print each Frame.
        frame = st.NextFrame()
        if not frame:
            break
        print("======= FRAME {:02} ======= (Version = {})".format(
            count, frame.version))
        for i in frame.components:
            PrintComponent(frame.components[i])
            pass
        count += 1
        print("========********========\n")

    return 0


if __name__ == "__main__":
    exit(main(len(sys.argv), sys.argv))
