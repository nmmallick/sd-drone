
import socket
import threading
import serial

address = '192.168.1.211'
UDPport = 4242
IDinfo = (address, UDPport)
# UDP Client Connection
udpguy = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)


serPort = '/dev/ttyACM0'
BAUD = 9600

def setupUDP():
	udpguy.connect(IDinfo)


def setupSerial():
	ser = serial.Serial(serPort, BAUD, timeout=0)
	return ser

def read_fwd_data(ser):
	message = ""

	while True:
		try:
			byte = ser.read(1).decode('utf-8', errors='replace')
			if byte == '\n':
				# End of message, process it
				if message:
					print(f"Arduino cmd = {message.strip()}")
					udpguy.sendto(message.encode('utf-8'), IDinfo)
				message = ""  
				# Reset msg
			else:
				message += byte
		except UnicodeDecodeError:
			print("Unicode decode error, skipping invalid char.")

			
			
			
def closeConnection(ser):
	ser.close()
	udpguy.close()
	
	
def echo(data):
	print(f"Echoing: {data}")


if __name__ == '__main__':
		
	setupUDP()
	ser = setupSerial()

	data_thread = threading.Thread(target=read_fwd_data(ser))
	data_thread.dameon = True
	data_thread.start()

	try:
		while True:
			userInput = input("Enter msg ('exit' to quit): ")
			if userInput.lower() == 'exit':
				break
			echo(userInput)
	except KeyboardInterrup:
		pass
		
	closeConnection(ser) 


