#!/usr/bin/python3

import seal

core = seal.Core()
print("Supported Seal Core Components Types: {}".format(core.SupportedComponentType()))

core.NewComponent("Flags", 'BOOL', 3)
core.NewComponent("RegA")
core.NewComponent("RegB", 'OCTET', 4)
core.NewComponent("I16Regs", 'INT16', 2)
core.NewComponent("Ui16Regs", 'UINT16', 2)
core.NewComponent("ExeIns", 'STRING', 10)
core.NewComponent("Mode", 'INT32', 1)
core.NewComponent("CycleCount", 'UINT32', 1)

testfile="/tmp/testcorespec.scs"
s = core.Save(testfile)
print("Seal Core saved at: {}".format(testfile))

cs = seal.Core.Load(testfile)
print("Seal Core loaded.")
comps = cs.Components()
for s in comps:
    print("{}: type={}, len={}".format(s, comps[s].type, comps[s].len))
