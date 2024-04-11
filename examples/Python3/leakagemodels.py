#!/usr/bin/python3
import sys
import seal
import copy

import json

corefile = None
tracefile = None
usedict = False
DICTFILE = None
sdict = None

# Verbose flag.
verbose = False


class MyLeakageModel(seal.LeakageModel.HammingWeight):
    # Frame Numeric Leakage.
    def ExtractFN(self, frame):
        # Get all HW as a list.
        hwlist = super().ExtractFN(frame)

        # Sum up all the HW.
        ret = 0
        for comp in hwlist:
            ret += sum(hwlist[comp])
            pass

        return ret


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

    # Read Core specification and load Trace.
    core = seal.Core.Load(corefile)
    st = seal.Trace(core)
    st.Open(tracefile)

    # Init LeakageModel
    # Root
    lmroot = seal.LeakageModel.Root(core)
    lmroot.Include(['reg', 'Execute_ALU_result', 'D2E_reg1', 'D2E_reg2'])
    print("Root: ", lmroot.GetDataType())

    # Identity
    lmidentity = seal.LeakageModel.Identity(core)
    lmidentity.Include(['reg', 'Execute_ALU_result', 'D2E_reg1', 'D2E_reg2'])
    print("Identity: ", lmidentity.GetDataType())

    # Linear
    lmlinear = seal.LeakageModel.Linear(core)
    lmlinear.Include(['reg'])
    print("Linear: ", lmlinear.GetDataType())
    coe = dict()
    coe['reg'] = [dict() for i in range(16)]
    coe['reg'][0] = {0: 1, 1: 20, 2: 300, 4: 4000}
    lmlinear.SetCoe(coe)
    print(json.dumps(coe, indent=2))

    # Interaction
    lminteract = seal.LeakageModel.Interaction(core)
    lminteract.Include([('D2E_reg1', 'D2E_reg2')])
    print("Interaction: ", lminteract.GetDataType())

    # Hamming weight
    lmhw = seal.LeakageModel.HammingWeight(core)
    lmhw.Include(['reg', 'Execute_ALU_result'])  # For uELMO
    print("HammingWeight: ", lmhw.GetDataType())

    # Hamming distance
    lmhd = seal.LeakageModel.HammingDistance(core)
    lmhd.Include(['D2E_reg1', 'D2E_reg2'])
    print("HammingDistance: ", lmhd.GetDataType())

    # Choose which leakage model to test here.
    # lm.Include(['Reg', 'ExeOpA', 'ExeOpB']) # For SealIbex
    lm = lminteract

    # Read dictionary
    if usedict:
        sdict = seal.EncodeDict()
        sdict.Import(DICTFILE)

        lm.SetDictionary(sdict)
        pass

    ntrc = lm.ExtractTN(st)
    strc = lm.ExtractTS(st)

    for i in range(len(ntrc)):
        # Print each Frame.
        print("======= FRAME {:02} ======= (Version = {})".format(
            i, core.version))
        print('Num:', ntrc[i])
        print('Sym:', strc[i])
        print("========********========\n")
        pass

    return


if __name__ == '__main__':
    exit(main(len(sys.argv), sys.argv))
