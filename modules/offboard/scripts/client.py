import socket
import struct
import serial
import time

from sread import DeviceInterface

LOOP_PERIOD = 1/200.0

if __name__ == '__main__':

    dev = DeviceInterface(port="/dev/ttyACM0")
    dev.start()

    # Localhost
    HOST = "127.0.0.1"
    PORT = 65432

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    while True:
        start = time.time()
        quaternion = dev.getQuaternion()
        print(quaternion)

        ## Create packet
        packet = bytearray()
        for q in quaternion:
            packet.extend(struct.pack('f', q))
        sock.sendto(packet, (HOST, PORT))

        while time.time() - start < LOOP_PERIOD:
            continue
