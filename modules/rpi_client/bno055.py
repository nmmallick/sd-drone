import time
import board
import adafruit_bno055
import copy

i2c = board.I2C()

sensor = adafruit_bno055.BNO055_I2C(i2c)

x,y,z,x_sign,y_sign,z_sign = sensor.axis_remap
sensor.axis_remap = (z, x, y, x_sign, y_sign, z_sign)

while True:
    #print("Quaternion: {}".format(sensor.quaternion))
    euler = copy.deepcopy(sensor.euler)
    if (euler[0] == None) or (euler[1] == None) or (euler[2] == None):
        continue

    #print("Euler: {:.2f} {:.2f} {:.2f}".format(euler[0], euler[1], euler[2]))
    print("Value : {:.2f}".format(euler[1]/90.))
    #print("Euler: {}".format(sensor.euler))
