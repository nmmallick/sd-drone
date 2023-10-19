import struct
import serial
import threading
import subprocess
import copy

class DeviceInterface:
    def __init__(self, port="/dev/ttyUSB0", baud=115200):
        __port__ = port
        self.__dev__ = serial.Serial(__port__, baud, timeout=0.5)
        self.__dev__.close()
        self.__dev__.open()

        self.__NUM_FLOATS__ = 4
        self.__FLOAT_SIZE__ = 4
        self.__TOTAL_BYTES__ = self.__NUM_FLOATS__*self.__FLOAT_SIZE__

        self.__thread__ = None

        self.buf = None
        self.trigger = bytearray([1])
        self.quaternion = []
        self.done = False

    def readData(self):
        self.__dev__.reset_input_buffer()
        while not self.done:
            self.__dev__.write(self.trigger)
            self.buf = self.__dev__.read(self.__TOTAL_BYTES__)

            if len(self.buf) != self.__TOTAL_BYTES__:
                print("timedout")
                continue

            self.quaternion.clear()
            for i in range(self.__NUM_FLOATS__):
                start = self.__NUM_FLOATS__*i
                end = start + self.__FLOAT_SIZE__
                self.quaternion.append(struct.unpack('f', self.buf[start:end])[0])

    def getQuaternion(self):
        return copy.deepcopy(self.quaternion)

    def start(self):
        self.__thread__ = threading.Thread(target=self.readData)
        self.__thread__.start()

    def stop(self):
        self.done = True
        self.__thread__.join()
