import socket
import bluetooth
import threading

# Bluetooth server information
server_name = "?"  
service_uuid = "3A8688BC330C12D2D120DE040EF1B553"  

# Bluetooth settings
bluetooth_port = 1  # Bluetooth port number
bluetooth_buff_size = 1024

# TCP settings
# home
# tcp_host = '192.168.1.211'
# hotspot 
tcp_host = '172.20.10.13'
tcp_port = 4242
tcp_buff_size = 1024

fail_cmds = ['ERROR', 'KILL', 'IT']

def setupBluetoothSocket():
    nearby_devices = bluetooth.discover_devices(lookup_names=True)
    
    server_address = None
    for addr, name in nearby_devices:
        if name == server_name:
            server_address = addr
            break
    
    if server_address is None:
        print(f"Bluetooth server '{server_name}' not found.")
        return None

    s = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
    s.connect((server_address, bluetooth_port))
    return s

def setupTcpSocket():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((tcp_host, tcp_port))
    return s

def bluetooth_thread():
    bt_socket = setupBluetoothSocket()
    if bt_socket is not None:
        while True:
            bt_data = bt_socket.recv(bluetooth_buff_size)
            if not bt_data:
                break
                
            print("receved from Arduino: ", bt_data.decode('utf-8'))
            tcp_socket.send(bt_data)

def tcp_thread():
    while True:
        cmd = input("ENTER COMMAND: ")
        if cmd in fail_cmds:
            tcp_socket.send(cmd.encode())
            break
        elif cmd.startswith("TOGGLE_LED "):
            tcp_socket.send(cmd.encode())
        else:
            print("Invalid command. Please use the format 'TOGGLE_LED R,G,B' to control the LED.")

        reply = tcp_socket.recv(tcp_buff_size)
        print(reply.decode('utf-8'))

# Create a TCP socket for communication with the server
tcp_socket = setupTcpSocket()

if tcp_socket is not None:
    # Start the Bluetooth thread to handle Bluetooth communication
    bt_thread = threading.Thread(target=bluetooth_thread)
    bt_thread.start()

    # Start the TCP thread to handle user input and TCP communication
    tcp_thread()

    # Close the Bluetooth socket when the TCP thread exits
    bt_thread.join()
    tcp_socket.close()
