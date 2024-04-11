import collections

from Frame import *


# Seal Trace implementation.
class Trace:
    def __init__(self, core):
        self.core = core
        self.filepath = ''
        self.tracefile = None
        self.offset = 0
        self.delonexit = False
        return

    def __del__(self):
        if self.tracefile is not None:
            self.tracefile.close()

        if self.delonexit and self.filepath != '':
            os.remove(self.filepath)

        return

    # Open a new Trace file given by ${filepath}.
    def Open(self, filepath, mode='r', flags='', args=None):
        self.filepath = filepath
        if mode == 'r':
            self.tracefile = open(filepath, 'rb')
        else:
            raise Exception("Unsupported mode for Trace.")
        return

    # Reset Trace file offset.
    def Reset(self):
        self.tracefile.seek(0)
        self.offset = 0
        return

    # Returns the next Frame, or None if the Trace reaches EOF.
    def NextFrame(self):
        if self.tracefile is None:
            raise(Exception("Trace file not opened."))

        frame = Frame(self.core)
        frame.raw = self.tracefile.read(frame.framesize)

        # EOF reached.
        if 0 == len(frame.raw):
            return None

        if not frame.Decode():
            raise Exception("Corrupted Frame.")
            return None
        else:
            self.offset += 1
            pass

        return frame

    # Invoke an emulator.
    def OpenEmulator(self, emulatorcmd, interface, stdin=None, stdout=None, stderr=None):
        self.delonexit = True
        try:
            os.mkfifo(interface)
            pass
        except FileExistsError:
            raise(Exception("Interface exists."))
            pass

        pid = os.fork()
        if pid == 0:
            # Child process.
            sys.stdin.close()
            sys.stdout.close()
            sys.stderr.close()
            ret = subprocess.Popen(
                emulatorcmd, stdin=stdin, stdout=stdout, stderr=stderr)
            exit(ret)
            pass
        else:
            # Parent process.
            self.Open(interface)
            pass

        return

    # Extract leakage with Leakgage Function ${Extractor}.
    # Returns a list of Extractor() returns.
    # ${windowsize}:
    #   This is used to control different prototypes of ${Extractor}.
    #   0 : ${Extractor} extracts leakage from the whole Trace.
    #   1 : ${Extractor} extracts leakage from a single Frame. Default.
    #   >1: ${Extractor} extracts leakage from a window of Frames of size ${windowsize}.
    def Extract(self, Extractor, windowsize=1):
        ret = list()
        count = 0  # Count of extractions.

        # Reset the Trace before extraction.
        self.Reset()

        # 0 - Extract from the whole trace.
        # Prototype:
        #   Extractor(seal.Trace)
        if 0 == windowsize:
            # Extract leakage from the whole Trace.
            ret = Extractor(self)
            pass

        # 1 - Extract from a single Frame. Default.
        # Prototype:
        #   Extractor(seal.Frame)
        elif 1 == windowsize:
            while True:
                frame = self.NextFrame()

                if frame is None:  # End of Trace
                    break

                # Extract leakage from a single Frame.
                ret.append(Extractor(frame))
                count += 1
                pass
            pass

        # >1 - Extract from a window of Frames of size ${windowsize}.
        # Prototype:
        #   Extractor(seal.Frame[windowsize])
        elif 1 < windowsize:
            # Windowed buffer of Frames
            window = collections.deque([None for i in range(windowsize)])

            # Initialisation.
            i = 0
            while i < windowsize:
                frame = self.NextFrame()

                if frame is None:  # Length of Trace less than window size.
                    break
                    pass

                window[i] = frame
                i += 1
                pass

            # Extract leakage from the first window
            if i > 0:
                ret.append(Extractor(window))
                count += 1

            # Early termination when Length of Trace is less than window size.
            if i < windowsize:
                return count
                pass

            # Regular loop.
            while True:
                # Slide the window.
                frame = self.NextFrame()
                if frame is None:  # End of Trace.
                    break
                    pass

                # Discard the earlierest Frame.
                window.popleft()

                # Put the latest Frame at the end of window.
                window.append(frame)

                # Extract leakage.
                ret.append(Extractor(window))

                count += 1
                pass
            pass
        else:
            raise Exception("Unsupported window size for Extractor.")
            pass

        # Reset the Trace after extraction.
        self.Reset()

        return ret
