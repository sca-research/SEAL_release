import json
import numpy

# Versions.
V1 = 1
V2 = 2

# List of supported versions.
SUPPORTED_VERSION = {V1, V2}


# Seal Core implementation.
class Core:
    class Component:
        def __init__(self, compname, comptype, complen, version=1):
            try:
                if comptype not in Core.SUPPORT_COMPONENT_TYPE:
                    raise Exception("Unsupported type")
                self.version = version  # Version
                self.name = compname    # Component Name
                self.type = comptype    # Component Type
                self.len = complen      # Length of Component.
                self.size = self.Size()  # Size of Component Instance in bytes.

                # Number of Symbols. Supported only in V2.
                if V2 == self.version:
                    if 'STRING' == self.type:
                        self.nsymid = 1
                        pass
                    else:
                        self.nsymid = self.len
                        pass
                    pass

                pass

            except Exception as err:
                raise err

            return

        # Return size of the value field in bytes.
        def ValSize(self):
            if self.type == 'BOOL':
                return self.len
            if self.type == 'OCTET':
                return self.len
            elif self.type == 'STRING':
                return self.len
            elif self.type == 'INT16':
                return self.len * 2
            elif self.type == 'UINT16':
                return self.len * 2
            elif self.type == 'INT32':
                return self.len * 4
            elif self.type == 'UINT32':
                return self.len * 4
            else:
                raise Exception("Invalid Core Component")
                return 0

        # Return size of the Symbol field in bytes.
        def SymSize(self):
            # Symbol field requires V2.
            if V2 != self.version:
                return 0
                pass
            if 'STRING' == self.type:
                return 4
                pass
            else:
                return 4 * self.len
                pass

        # Compute Component Instance size.
        def Size(self):
            if V1 == self.version:
                return self.ValSize()

            elif V2 == self.version:
                valsize = self.ValSize()
                symsize = self.SymSize()
                return valsize + symsize
            else:
                raise Exception(
                    "Not supported version: {}".format(self.version))
                return 0

    # Supported Component types.
    SUPPORT_COMPONENT_TYPE = set(
        {'BOOL', 'OCTET', 'STRING', 'INT16', 'UINT16', 'INT32', 'UINT32'})

    # Supported Component type size in bytes.
    SUPPORT_COMPONENT_SIZE = {
        'BOOL': 1,
        'OCTET': 1,
        'STRING': 1,
        'INT16': 2,
        'UINT16': 2,
        'INT32': 4,
        'UINT32': 4
    }

    # Initialiser for Core
    def __init__(self, ver=1):
        global SUPPORTED_VERSION

        if ver not in SUPPORTED_VERSION:
            raise Exception("Invald Core version: {}".format(ver))
            pass

        self.version = ver
        self.components = dict()
        return

    # Returns a set of supoorted core component types.
    def SupportedComponentType(self):
        return Core.SUPPORT_COMPONENT_TYPE

    # Returns a dictionary of core components.
    def Components(self):
        return self.components

    # Add a new core component to the core specification.
    def NewComponent(self, compname, comptype='OCTET', complen=1):
        try:
            if compname in self.components:
                raise Exception("Component already exists.")
            self.components[compname] = Core.Component(
                compname, comptype, complen, self.version)

        except Exception as err:
            raise err

        return

    # Remove a component from the core specification.
    def DelComponent(self, compname):
        del self.components[compname]
        return

    # Save a Core specification into a json file given by ${filepath} .
    def Save(self, filepath=None):
        # Convert a Core into a dictionary tree.
        cs = dict()
        cs['version'] = str(self.version)
        cs['components'] = dict()
        for s in self.components:
            cs['components'][s] = dict()
            cs['components'][s]['type'] = self.components[s].type
            cs['components'][s]['len'] = self.components[s].len

        if filepath == None:    # Return a JSON string of filepath is not given.
            return json.dumps(cs, indent=4)
        else:                   # Otherwise write into the file specified by filepath.
            with open(filepath, 'w', encoding='utf-8') as f:
                json.dump(cs, f, indent=4)

        return

    # Load a Core object from a file specified by ${filepath}.
    def Load(filepath):
        with open(filepath, 'r', encoding='utf-8') as f:
            # Read a Core as a JSON string from the file.
            corespec = json.load(f)

            # Convert the JSON string into a Core object.
            cs = Core(ver=int(corespec['version']))
            corecomponents = corespec['components']
            for s in corecomponents:
                cs.NewComponent(
                    s, corecomponents[s]['type'], corecomponents[s]['len'])

        return cs
