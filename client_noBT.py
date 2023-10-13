import socket
#ping other pi toget IP
host = '192.168.1.211'
port = 4242
buff_size = 1024

h_p = (host,port)
fail_cmds = ['ERROR','KILL', 'IT']



def setupSocket():
		
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect((host,port))
	return s


s = setupSocket()
while True:
	cmd = input("ENTER COMMAND: ")
	if cmd in fail_cmds:
		# send server requests to other end ASAP
		s.send(str.encode(cmd))
		break

	elif cmd.startswith("TOGGLE_LED "):
		s.send(str.encode(cmd))
		# Send the complete "TOGGLE_LED" command with RGB values

	else:
		print("Invalid command. Please use the format 'TOGGLE_LED R,G,B' to control the LED.")

	reply = s.recv(buff_size)
	print(reply.decode('utf-8'))

s.close()
