import parse


# NULL Symbol ID
SYM_ID_NULL = 0


# Encoding Dictionary.
class EncodeDict(dict):
    def __init__(self):
        # Add NULL as initialised.
        self['NULL'] = 0
        self[0] = 'NULL'
        self.count = 1
        return

    # Add a new symbol.
    def Add(self, newkey, symid=None):
        if str is not type(newkey):
            Exception("Keys must be strings.")
            pass

        # Immediately returns if newkey already in dictionary.
        if newkey in self:
            return

        # Use count as default Symbol ID.
        if None == symid:
            symid = self.count
            pass

        # Add a new entry into the dictionary.
        self[newkey] = symid
        # Reversed table for decode.
        self[symid] = newkey

        self.count += 1

        return

    # Encode header.
    def EncodeHeader(self):
        # No header attributes yet.
        return "[]"

    def DecodeHeader(self, headerstr):
        # Nothing needs to be processed yet.
        return

    # Encode an Encoding Entry.
    def EncodeEntry(self, symid, symname):
        entrystr = "{:08X}::{:s}\n".format(symid, symname)
        return entrystr

    # Decode an Encoding Entry.
    def DecodeEntry(self, entrystr):
        (symid, symname) = parse.parse("{:08x}::{}\n", entrystr)
        return (symid, symname)

    # Convert the Encoding Dictionary into a string.
    def ToStr(self):
        # Initialise with header.
        outstr = self.EncodeHeader() + "\n"

        # Encode each entry.
        for key in self:
            # NULL entry is implicit.
            if "NULL" == key:
                continue

            # Integer keys are for decoding.
            if type(key) is int:
                continue

            # Encode each entry as "{Symbol ID}::{Symbol Name}"
            symname = key
            symid = self[key]
            entrystr = self.EncodeEntry(symid, symname)
            outstr += entrystr
            pass

        return outstr

    # Export the Encoding Dictionary into a file.
    def Export(self, outpath):
        outfile = open(outpath, 'w')
        outfile.write(self.ToStr())
        return

    # Import a Encoding Dictionary.
    def Import(self, inpath):
        infile = open(inpath, 'r')

        # Read header.
        nl = infile.readline()
        self.DecodeHeader(nl)

        # Clear current Encoding Dictionary.
        self.clear()
        self.__init__()

        # Read in the dictionary file.
        while True:
            nl = infile.readline()
            if '' == nl:  # EOF
                break
            elif '\n' == nl:  # Skip blank lines.
                continue
            elif '#' == nl[0]:  # Skip comment lines.
                continue

            (symid, symname) = self.DecodeEntry(nl)
            self.Add(symname, symid=symid)
            pass
        return
