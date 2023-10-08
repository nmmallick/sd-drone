import math
import numpy as np

def rad2deg(angle : float):
    return angle * (180.0 / math.pi)

def deg2rad(angle : float):
    return angle * (math.pi / 180.0)

def getRotationMat(q : list):
    c00 = q[0] ** 2 + q[1] ** 2 - q[2] ** 2 - q[3] ** 2
    c01 = 2 * (q[1] * q[2] - q[0] * q[3])
    c02 = 2 * (q[1] * q[3] + q[0] * q[2])
    c10 = 2 * (q[1] * q[2] + q[0] * q[3])
    c11 = q[0] ** 2 - q[1] ** 2 + q[2] ** 2 - q[3] ** 2
    c12 = 2 * (q[2] * q[3] - q[0] * q[1])
    c20 = 2 * (q[1] * q[3] - q[0] * q[2])
    c21 = 2 * (q[2] * q[3] + q[0] * q[1])
    c22 = q[0] ** 2 - q[1] ** 2 - q[2] ** 2 + q[3] ** 2

    rotMat = np.array([[c00, c01, c02], [c10, c11, c12], [c20, c21, c22]])
    return rotMat

## Euler Angle representation in Z-Y-X format
class EulerAngles:
    def __init__(self, roll=None, pitch=None, yaw=None):
        self.roll = roll
        self.pitch = pitch
        self.yaw = yaw

    def printAngles(self, degrees=True):
        if self.roll != None and self.pitch != None and self.yaw != None:
            print() # send stdout a new line
            print("roll : " + str(self.roll if not degrees else rad2deg(self.roll)))
            print("pitch : " + str(self.pitch if not degrees else rad2deg(self.pitch)))
            print("yaw : " + str(self.yaw if not degrees else rad2deg(self.yaw)))

    ## Converts a quaternion to euler angles in Z-Y-X convention
    def quaternion2euler(self, q : list):
        mat = getRotationMat(q)
        test = -mat[2, 0]

        if test > 0.99999:
            self.yaw = 0
            self.pitch = math.pi / 2
            self.roll = math.atan2(mat[0, 1], mat[0, 2])
        elif test < -0.99999:
            self.yaw = 0
            self.pitch = -math.pi / 2
            self.roll = math.atan2(-mat[0, 1], -mat[0, 2])
        else:
            self.yaw = math.atan2(mat[1, 0], mat[0, 0])
            self.pitch = math.asin(-mat[2, 0])
            self.roll = math.atan2(mat[2, 1], mat[2, 2])

def printQuaternion(q : list):
    print("w: " + str(q[0]))
    print("x: " + str(q[1]))
    print("y: " + str(q[2]))
    print("z: " + str(q[3]))
