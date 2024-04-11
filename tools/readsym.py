#!/usr/bin/python3
import sys
import seal

corefile = None
tracefile = None
usedict = False
DICTFILE = None
sdict = None

# Verbose flag.
verbose = False


# Format control for Component printing.
def PrintComponent(comp):
    global sdict, usedict
    # Component info.
    print("{:s} : [ ".format(comp.name), end='')

    # Symbolic info.
    for i in range(len(comp.symid)):
        if usedict:
            # Options only available with a EncDict.
            # Print decoded Symbol.
            symstr = sdict[comp.symid[i]]

            # Simplify NULL by '--'
            if 'NULL' == symstr:
                symstr = '-'

            # Print Symbols with their IDs.
            # print("{:d}:{:s}({:d}) ".format(
            #    i, symstr, comp.symid[i]), end='')
            print("{:s} ".format(symstr), end='')

            pass

        else:
            # Print the encoded Symbol ID.
            print("{:02d} ".format(comp.symid[i]), end='')
            pass

    # Remove last space, then newline.
    print(']')

    return


def main(argc, argv):
    global usedict, sdict, verbose

    # Cmd args check.
    if argc < 3 or '-h' in argv or '--help' in argv:
        print(
            "Usage: {}  {{Core Specification}} {{Trace file or FIFO}} [--dict DICTIONARY] [-v|--verbose] [-h|--help]".format(argv[0]))
        exit(0)
        pass

    # Set verbose flag.
    if '-v' in argv or '--verbose' in argv:
        verbose = True
        pass

    # Check if a dictionary file is provided.
    if '--dict' in sys.argv:
        usedict = True
        DICTFILE = sys.argv[sys.argv.index('--dict') + 1]
        pass

    # Target trace.
    corefile = sys.argv[1]
    tracefile = sys.argv[2]

    # Read dictionary
    if usedict:
        sdict = seal.EncodeDict()
        sdict.Import(DICTFILE)
        pass

    # Read Core specification.
    core = seal.Core.Load(corefile)
    st = seal.Trace(core)

    st.Open(tracefile)
    count = 0
    while True:
        frame = st.NextFrame()
        if frame is None:
            break
        print("======= FRAME {:02} =======".format(count))

        # Print current Execution cycle.
        print("Core status : {:s}".format(
            frame['Execute_instr_disp'].strip('\0').replace('Execute: ', '')))

        nosym = True

        # Print all Components with Symbols.
        for i in frame.components:
            if frame.components[i].symid.any():
                PrintComponent(frame.components[i])
                nosym = False
                pass

        if nosym:
            print("Symbolic information not available.")
            pass
        pass

        count += 1
        print("========================\n")

    return


if __name__ == '__main__':
    exit(main(len(sys.argv), sys.argv))
