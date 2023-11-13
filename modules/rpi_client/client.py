import socket
import copy
import struct
import time
import board
import adafruit_bno055
import subprocess

from button import ToggleButton
from serial_read import DeviceInterface

class UDPSocket:
    def __init__(self, IP_ADDR="192.168.0.198", PORT=65432):
        self.IP_ADDR = "192.168.0.198"
        self.APP_PORT = 65432

        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    def send(self, data):
        self.socket.sendto(data, (self.IP_ADDR, self.APP_PORT))

def buttonToggleCallback():
    print("button pushed")
    
if __name__ == '__main__':

    # socket and toggle button
    s = UDPSocket()
    button = ToggleButton(17)

    # interface to stm board
    head_device = DeviceInterface(port="/dev/ttyACM0")
    head_device.start()

    # interface to handheld gyro
    i2c = board.I2C()
    hand_device = adafruit_bno055.BNO055_I2C(i2c)
    
    x,y,z,x_sign,y_sign,z_sign = hand_device.axis_remap
    hand_device.axis_remap = (z, x, y, x_sign, y_sign, z_sign)

    seq = 0

    itr = 0
    yaw_ref = 0
    while itr <= 1000:
        data = head_device.getAngles()
        yaw_ref += data[2]
        itr+=1

    yaw_ref /= itr

    print("starting")
    while True:
        start = time.time()

        angles_head = copy.deepcopy(head_device.getAngles())
        angles_head[2] = yaw_ref - angles_head[2]
        
        angles_hand = copy.deepcopy(hand_device.euler)

        # angles_head = (head_device.getAngles())
        # angles_hand = (hand_device.euler)

        if None in angles_head:
            continue

        if (angles_hand[1] == None):
            continue
        
        msg = bytearray(struct.pack('f', angles_head[0]))
        msg += bytearray(struct.pack('f', angles_head[1]))
        msg += bytearray(struct.pack('f', angles_head[2]))

        if button.getValue():
            val = angles_hand[1]/90.
            msg += bytearray(struct.pack('f', val))
        else:
            val = float(0.0)
            msg += bytearray(struct.pack('f', val))

        s.send(msg)

        end = time.time()
        if (end - start) >= 0.01:
            print("Deadline not met")

    
