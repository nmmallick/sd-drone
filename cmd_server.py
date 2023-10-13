import socket
import RPi.GPIO as GPIO

RED_PIN = 17
GREEN_PIN = 18
BLUE_PIN = 19

# home
# HOST = '192.168.1.211'  # Server IP or Hostname
HOST = '172.20.10.13'

PORT = 4242  # Pick an open Port (1000+ recommended), must match client
s_info = (HOST, PORT)
storedValue = "Hello World"


def setupServer():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # TCP Connection
    print('Socket created')

    # managing error exception
    try:
        s.bind((HOST, PORT))
    except socket.error as msg:
        print('Bind failed: ' + str(msg))
    print('Connected')
    return s


def setupConnection():
    s.listen(1)
    conn, addr = s.accept()
    print("Connected to " + addr[0] + ":" + str(addr[1]))
    return conn


# for testing
def GET():
    reply = storedValue
    return reply


def REPLY(msgData):
    return msgData


def toggle_led(msgData):
    try:
        # Split the RGB values
        r, g, b = map(int, msgData.split(','))
        
        # Check if the values are within valid range (0-1)
        if 0 <= r <= 1 and 0 <= g <= 1 and 0 <= b <= 1:
            # Set the LED color based on the input values
            GPIO.output(RED_PIN, r)
            GPIO.output(GREEN_PIN, g)
            GPIO.output(BLUE_PIN, b)
            return 'LED toggled'
        else:
            return 'ERROR: Invalid RGB values'
    except ValueError:
        return 'ERROR: Invalid RGB format'


def dataTransfer(conn):
    while True:
        try:
            recv_size = 1024
            data = conn.recv(recv_size)
            if not data:
                break
            
            msg = data.decode('utf-8')
            cmd, msgData = msg.split(' ', 1)
            print('cmd: ' + cmd)
            print('msgData: ' + msgData)
            
            if cmd == 'GET':
                reply = GET()
            elif cmd == 'REPEAT':
                reply = REPLY(msgData)
            elif cmd == 'TOGGLE_LED':
                r, g, b = map(int, msgData.split(','))
                # Set the LED color based on the input values
                GPIO.output(RED_PIN, r)
                GPIO.output(GREEN_PIN, g)
                GPIO.output(BLUE_PIN, b)
                reply = 'LED toggled'
            elif cmd == 'EXIT':
                print('Client exited.')
                break
            elif cmd == 'KILL':
                s.close()
                print('Server exited')
                break
            else:
                reply = 'ERROR: CMD NOT FOUND'
            
            conn.sendall(str.encode(reply))
            print("Data sent")
        
        except KeyboardInterrupt:
            GPIO.cleanup()
            break
        except Exception as e:
            print("An error occurred: " + str(e))
            GPIO.cleanup()
            break

    conn.close()


GPIO.setmode(GPIO.BCM)
GPIO.setup(RED_PIN, GPIO.OUT)
GPIO.setup(GREEN_PIN, GPIO.OUT)
GPIO.setup(BLUE_PIN, GPIO.OUT)

GPIO.output(RED_PIN,255)
GPIO.output(GREEN_PIN,255)
GPIO.output(BLUE_PIN, 255)

s = setupServer()
while True:
    try:
        conn = setupConnection()
        dataTransfer(conn)
    except KeyboardInterrupt:
        GPIO.cleanup()
        break
    except Exception as e:
        print("An error occurred: " + str(e))
        GPIO.cleanup()
        break
