Toy example of Fully Annotated Execution Trace (FAET)

# FILES:
createcore.py   : Core definition generating script.
testcore.scs    : Toy core definition.
gentrace.cpp    : Trace generator.
readtrace.cpp   : Trace reader in C++.
print.cpp       : Printer library for C++.
readtracefile.py: Trace reader in Python3.


# HOW TO USE
1. Compile
```shell
make 
```

2. Generate a toy trace.
```shell
./gentrace /tmp/test.trace
```

3. Read the toy trace.
```shell
./readtrace /tmp/test.trace # C++ reader.
python3 readtracefile.py /tmp/test.trace # Python3 reader
```

# Q&A
## Symbol fields are all integers?
Symbols are internally represented as integers in FAETs. To decode integers into strings an additaional dictionary file but this is not implemented in this example. 

An example of using Seal dictionary is given at:
https://github.com/sca-research/Seal/blob/main/pyexamples/sealdict.py

## Most Symbol fields are 0?
Yes, except RegB which is annotated in each simulated clock cycle at gentrace.cpp:L91.
