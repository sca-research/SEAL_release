#!/usr/bin/python3
# Example for reading a Trace file.

import sys
import seal

# Verbose flag.
verbose = False

FrameKey = 'FrameKey'


# Print FrameKey in each Frame.
def PrintFrameKey(frame):
    global FrameKey
    framekey = frame.components[FrameKey]
    print(framekey, '\n')

    return


def main(argc, argv):
    global verbose, FrameKey

    # Cmd args check.
    if argc < 3 or '-h' in argv or '--help' in argv:
        print(
            "Usage: {}  {{Core Specification}} {{Trace file or FIFO}} [-k FrameKey] [-v|--verbose] [-h|--help]".format(argv[0]))
        return 0
        pass

    # Set verbose flag.
    if '-v' in argv or '--verbose' in argv:
        verbose = True
        pass

    # Set FrameKey.
    if '-k' in argv:
        FrameKey = argv[argv.index('-k') + 1]
        pass

    # Read Core specification.
    core = seal.Core.Load(argv[1])

    if FrameKey not in core.Components():
        raise Exception("No {} in Core".format(FrameKey))
        exit()
        pass

    # Open Trace file.
    st = seal.Trace(core)
    st.Open(argv[2])

    # Parse Execution Trace by extracting FrameKey.
    st.Extract(PrintFrameKey)

    return 0


if __name__ == "__main__":
    exit(main(len(sys.argv), sys.argv))
