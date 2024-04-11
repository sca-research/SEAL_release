from Core import *


# Seal Frame implementation.
class Frame():
    class ComponentInstance(Core.Component):  # Inherit from Core.Component.
        def __init__(self, compdef, version=V1):
            super().__init__(compdef.name, compdef.type, compdef.len, version)
            self.raw = None
            self.val = None
            if V2 == version:
                self.symid = None
            return

        # Decode value from bytes.
        def ValFromBytes(self, valraw):
            if type(valraw) not in [bytes, bytearray]:
                raise Exception("Value raw must be bytes or bytearrary")

            if self.type == 'STRING':
                if len(valraw) > self.Size():
                    raise Exception("Oversiezed string")

            if self.type == 'BOOL':
                self.val = list()
                for i in range(self.len):
                    if valraw[i] == 0x1:
                        self.val.append(True)
                    else:
                        self.val.append(False)
                    pass

            elif self.type == 'OCTET':
                self.val = valraw
                pass

            elif self.type == 'STRING':
                self.val = valraw.decode().rstrip("\0")
                pass

            elif self.type == 'INT16':
                self.val = list()
                for i in range(self.len):
                    self.val.append(int.from_bytes(
                        valraw[i*2: i*2+2], 'big', signed=True))
                pass

            elif self.type == 'UINT16':
                self.val = list()
                for i in range(self.len):
                    self.val.append(int.from_bytes(
                        valraw[i*2: i*2+2], 'big', signed=False))
                pass

            elif self.type == 'INT32':
                self.val = list()
                for i in range(self.len):
                    self.val.append(int.from_bytes(
                        valraw[i*4: i*4+4], 'big', signed=True))
                pass

            elif self.type == 'UINT32':
                self.val = list()
                for i in range(self.len):
                    self.val.append(int.from_bytes(
                        valraw[i*4: i*4+4], 'big', signed=False))
                pass
            else:
                raise(Exception("Unknow data type"))

            return

        # Decode a Component Instance from bytes.
        def DecodeRaw(self):
            if V1 == self.version:  # |value|
                self.ValFromBytes(self.raw)
                pass
            elif V2 == self.version:  # |value|symid|
                # Decode value
                valsize = self.ValSize()
                valraw = self.raw[0:valsize]
                self.ValFromBytes(valraw)
                # Decode symid
                symsize = 4 * self.len
                symraw = self.raw[valsize:valsize + symsize]
                self.symid = numpy.empty(self.nsymid, dtype='uint32')
                for i in range(self.nsymid):
                    self.symid[i] = int.from_bytes(symraw[i*4: (i+1)*4], 'big')
                    pass
                pass
            else:
                raise Exception("Frame version not supported.")
                pass
            return

        # Default str().
        def __str__(self):
            s = "{}".format(self.val)
            return s

        # End of ComponentInstance.

    # Compute size of this Frame.
    def FrameSize(self):
        return 4 + sum([self.core.components[comp].size for comp in self.core.components])

    # Decode this Frame from bytes to Component Instances.
    def Decode(self):
        # Empty Frame.
        if 0 == len(self.raw):
            return False

        # Decode version header.
        self.header = int.from_bytes(self.raw[0:4], 'big')
        if self.header != self.core.version:
            raise Exception("Frame version does not match Core version.")
            pass

        # Set Frame body.
        self.body = self.raw[4:]

        # Decode each Component Instances.
        offset = 0
        for name in self.components:
            comp = self.components[name]
            compsize = comp.Size()
            # Raw binary of the Component Instance.
            comp.raw = self.body[offset:offset + compsize]
            if len(comp.raw) == 0:
                continue

            offset += compsize
            # comp.FromBytes(comp.raw)
            comp.DecodeRaw()

        return True

    # Indexing method. Provides quick access to components.
    def __getitem__(self, key):
        return self.components[key].val

    def __init__(self, core):
        self.core = core
        self.version = core.version
        self.components = dict()
        self.framesize = self.FrameSize()
        self.raw = None
        for compname in core.components:
            compdef = core.components[compname]
            self.components[compname] = self.ComponentInstance(
                compdef, self.core.version)
        return
