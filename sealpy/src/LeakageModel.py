from Core import *
from Frame import *
from Trace import *
from EncodeDict import *

import numpy


# Root class for all LeakageModels.
class Root:
    # Init by Core
    def __init__(self, icore, encdict=None):
        assert type(icore) is Core

        # Core
        self.core = icore

        # List of Component names managed by this LeakageModel.
        self.compnames = set()

        # Dictionary for decoding Symbols.
        self.encdict = encdict

        # Set output data type.
        self._datatype = 'PROTOTYPE'

        return

    # Include a list of Components into this LeakageModel.
    def Include(self, icompnames):
        for i in icompnames:
            if i not in self.core.components:
                raise Exception("{} not in Core".format(i))
                pass

            self.compnames.add(i)
            pass

        return

    # Remove Components in the list from this LeakageModel.
    def Delete(self, icompnames):
        for i in icompnames:
            self.compnames.discard(i)
            pass
        return

    # Return a list of Components in this LeakageModel.
    def List(self):
        return list(self.compnames)

    # Set EncDict.
    def SetDictionary(self, iencdict):
        self.encdict = iencdict
        return

    # Decode a Symbol ID into a string.
    def DecSym(self, symid):
        if self.encdict is None:
            raise Exception("Dictionary not set.")
            pass
        return self.encdict[symid]

    # Return a list of numeric value leakge.
    def Num(self, comp):
        assert type(comp) is Frame.ComponentInstance
        return comp.val

    # Return a list symbolic string leakage.
    def Sym(self, comp):
        assert type(comp) is Frame.ComponentInstance

        ret = ['NULL' for i in range(comp.len)]
        for i in range(comp.len):
            ret[i] = self.DecSym(comp.symid[i])
            pass
        return ret

    # Extract a single Frame in numeric form.
    def ExtractFN(self, frame):
        assert type(frame) is Frame

        ret = dict()

        # Extract each Component included numerically.
        for i in self.compnames:
            ci = frame.components[i]
            ret[i] = self.Num(ci)
            pass

        return ret

    # Extract a single Frame in symbolic form.
    def ExtractFS(self, frame):
        assert type(frame) is Frame

        if self.encdict is None:
            print("Warning: Dictionary not set.")
            pass

        ret = dict()

        # Extract each Component included numerically.
        for i in self.compnames:
            ci = frame.components[i]
            ret[i] = self.Sym(ci)
            pass

        return ret

    # Extract a whole Trace in numeric form.
    def ExtractTN(self, trace):
        return trace.Extract(self.ExtractFN, windowsize=1)

    # Extract a whole Trace in numeric form.
    def ExtractTS(self, trace):
        return trace.Extract(self.ExtractFS, windowsize=1)

    # Get the statistical data type of leakage output.
    def GetDataType(self):
        return self._datatype

    pass


# Identity
class Identity(Root):
    def __init__(self, icore, encdict=None):
        super().__init__(icore, encdict)

        # Update leakage data type.
        self._datatype = 'NOMINAL'
        return
    pass


# Liniear
class Linear(Root):
    def __init__(self, icore, encdict=None, icoe=None):
        super().__init__(icore, encdict)

        # Set default coefficients
        self.coe = icoe

        return

    # Set coefficient.
    def SetCoe(self, icoe):
        self.coe = icoe
        return

    # Convert an integer of x bits to a binary array.
    def __Int2BinArray__(self, x, xlen):
        bitarray = numpy.zeros([1, xlen], dtype=numpy.uint8)[0]

        for i in range(xlen):
            if x & 1 != 0:
                bitarray[i] = 1
                pass

            x >>= 1
            if x == 0:
                break
            pass

        return bitarray

    def Num(self, comp):
        # Check coe
        if self.coe is None:
            raise Exception("No coefficients.")
            pass

        ret = [0 for i in range(comp.len)]

        # Get component values.
        val = super().Num(comp)

        # Coefficient for this Component not given.
        if comp.name not in self.coe:
            print("#Coefficient for {} not provided, use identity function.")
            return val
            pass

        # Compute linear leakage by coefficient.
        compsize = self.core.SUPPORT_COMPONENT_SIZE[comp.type]
        compcoe = self.coe[comp.name]

        # Apply coefficient to each element in the Component.
        for i in range(comp.len):
            # Convert the value into a binary array.
            val_bitarray = self.__Int2BinArray__(val[i], compsize * 8)

            # Coefficient for Component[i].
            coes = compcoe[i]

            # Coefficients given as in dict(): {BIT_POSITION_i : beta_i}
            for j in coes:
                # ret[i] = \sum {beta_i * b_i}
                if val_bitarray[j] == 1:
                    ret[i] += coes[j]
                pass
            pass

        return ret

    def Sym(self, comp):
        syms = super().Sym(comp)
        for i in range(len(syms)):
            syms[i] = "Linear({})".format(syms[i])
            pass
        return syms
    pass


# Interaction. Leakage are defined over tuples of Components.
class Interaction(Root):
    def __init__(self, icore, encdict=None, flatten=False):
        super().__init__(icore, encdict)

        # List of Component tuples.
        self.comptuples = set()

        # Flatten the leakage output.
        self._flatten = flatten

        return

    # Interactions requires tuples.
    # icomptuples: list of Component tuples.

    def Include(self, icomptuples):
        for ct in icomptuples:
            if type(ct) is not tuple:
                raise Exception("Interaction requires tuples of Components.")
                pass

            for i in ct:
                if i not in self.core.components:
                    raise Exception("{} not in Core".format(i))
                    pass
                pass

            self.comptuples.add(ct)
            pass

        return

    def Delete(self, icomptuples):
        for ct in icomptuples:
            self.comptuples.discard(ct)
            pass
        return

    def List(self):
        return list(self.comptuples)

    # Numeric output for a Component tuple.
    def Num(self, comptuple):
        ret = list()

        # Decode each Symbol in each symid in each dimension.
        for ct in comptuple:
            if self._flatten:
                ret.extend(ct.val)
                pass
            else:
                ret.append(ct.val)
                pass
            pass

        return tuple(ret)

    # Symbolic output for a Component tuple.
    def Sym(self, comptuple):
        ret = list()

        # Decode each Symbol in each symid in each dimension.
        for ct in comptuple:
            syms = [self.DecSym(s) for s in ct.symid]

            if self._flatten:
                ret.extend(syms)
                pass
            else:
                ret.append(syms)
                pass
            pass

        return tuple(ret)

    # Extract a single Frame in numeric form.
    def ExtractFN(self, frame):
        assert type(frame) is Frame

        ret = dict()

        # Extract each Component included numerically.
        for ct in self.comptuples:
            ci_l = [frame.components[comp] for comp in ct]
            ci_t = tuple(ci_l)
            ret[ct] = self.Num(ci_t)
            pass

        return ret

    # Extract a single Frame in symbolic form.
    def ExtractFS(self, frame):
        assert type(frame) is Frame

        if self.encdict is None:
            print("Warning: Dictionary not set.")
            pass

        ret = dict()

        # Extract each Component included numerically.
        for ct in self.comptuples:
            ci_l = [frame.components[comp] for comp in ct]
            ci_t = tuple(ci_l)
            ret[ct] = self.Sym(ci_t)
            pass

        return ret
    pass


# Transition leakage.
# Num() and Sym takes as input a series of ${windowsize} Component Instances of the same Component.
# ExtractT IFs take a series of ${windowsize} Frames as input.
class Transition(Root):
    def __init__(self, icore, encdict=None, windowsize=2):
        if windowsize < 2:
            raise Exception("windowsize must be at least 2")
            pass

        super().__init__(icore, encdict)
        self.windowsize = windowsize

        return

    def Num(self, compseries):
        return [comp.val for comp in compseries]

    def Sym(self, compseries):
        symidseries = [comp.symid for comp in compseries]

        ret = list()
        # Decode the Symbols.
        for sid in symidseries:
            ret.append([self.DecSym(s) for s in sid])
            pass
        return ret

    # Extract a single Frame in numeric form.
    def ExtractFN(self, frameseries):
        ret = dict()

        # Extract each Component included numerically.
        for i in self.compnames:
            # Construct the Component Instances series.
            ciseries = [frame.components[i] for frame in frameseries]
            # Compute Numeric leakage for Component i.
            ret[i] = self.Num(ciseries)
            pass

        return ret

    # Extract a single Frame in symbolic form.
    def ExtractFS(self, frameseries):
        if self.encdict is None:
            print("Warning: Dictionary not set.")
            pass

        ret = dict()

        # Extract each Component included numerically.
        for i in self.compnames:
            # Construct the Component Instances series.
            ciseries = [frame.components[i] for frame in frameseries]
            # Compute Numeric leakage for Component i.
            ret[i] = self.Sym(ciseries)
            pass

        return ret

    def ExtractTN(self, trace):
        return trace.Extract(self.ExtractFN, windowsize=self.windowsize)

    def ExtractTS(self, trace):
        return trace.Extract(self.ExtractFS, windowsize=self.windowsize)
    pass


# Hamming Weight
class HammingWeight(Linear):
    def __init__(self, icore, encdict=None):
        super().__init__(icore, encdict)

        # Update leakage data type.
        self._datatype = 'PROPORTIONAL'
        return

    def Num(self, comp):
        assert type(comp) is Frame.ComponentInstance

        # Init
        ret = [0 for i in range(comp.len)]

        # Component type check.
        if comp.type not in ['OCTET', 'INT16', 'UINT16', 'INT32', 'UINT32']:
            raise Exception(
                "HammingWeight requires Component type of octet or integers.")
            pass

        for i in range(comp.len):
            if comp.type == 'OCTET':  # Convert bytes to integer
                v = int.from_bytes(comp.val[i], 'little')
                pass
            else:
                v = comp.val[i]
                pass

            # Compute Hamming weight.
            ret[i] = v.bit_count()

            pass

        return ret

    def Sym(self, comp):
        assert type(comp) is Frame.ComponentInstance

        # Init
        ret = [None for i in range(comp.len)]

        # Component type check.
        if comp.type not in ['OCTET', 'INT16', 'UINT16', 'INT32', 'UINT32']:
            raise Exception(
                "HammingWeight requires Component type of octet or integers.")
            pass

        # Extract symbolic string.
        for i in range(comp.len):
            if comp.symid[i] == SYM_ID_NULL:
                ret[i] = 'NULL'
                pass
            else:
                ret[i] = "HW({})".format(self.DecSym(comp.symid[i]))
                pass
            pass

        return ret
    pass


# Hamming Distance
class HammingDistance(Transition):
    def __init__(self, icore, encdict=None, windowsize=2):
        super().__init__(icore, encdict, windowsize)
        self._datatype = 'PROPORTIONAL'
        return

    def Num(self, currcomp, prevcomp):
        hd = [0 for i in range(currcomp.len)]

        # Compute Hamming distance.
        for i in range(currcomp.len):
            hd[i] = (currcomp.val[i] ^ prevcomp.val[i]).bit_count()
            pass

        return hd

    def Sym(self, currcomp, prevcomp):
        hd = ['NULL' for i in range(currcomp.len)]

        # Hamming distance of curr and prev symbol.
        for i in range(currcomp.len):
            currsym = currcomp.symid[i]
            prevsym = prevcomp.symid[i]

            if SYM_ID_NULL in {currsym, prevsym}:
                hd[i] = 'NULL'
                continue
                pass

            hd[i] = "HD({},{})".format(self.DecSym(
                currsym), self.DecSym(prevsym))
            pass
        return hd

    def ExtractFN(self, frames):
        prevframe = frames[0]
        currframe = frames[1]
        ret = dict()

        # Compute HD for each Component included.
        for c in self.compnames:
            ret[c] = self.Num(currframe.components[c], prevframe.components[c])
            pass

        return ret

    def ExtractFS(self, frames):
        prevframe = frames[0]
        currframe = frames[1]
        ret = dict()

        # Compute HD for each Component included.
        for c in self.compnames:
            ret[c] = self.Sym(currframe.components[c], prevframe.components[c])
            pass

        return ret

    def ExtractTN(self, trace):
        return trace.Extract(self.ExtractFN, windowsize=2)

    def ExtractTS(self, trace):
        return trace.Extract(self.ExtractFS, windowsize=2)
    pass
