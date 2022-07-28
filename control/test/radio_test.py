import socket
import time # noqa: F401

# field computer
local_ip = "172.16.1.76"
local_port = 25565
buf_size = 1024

server_msg = "hellorobot!"
bytes_to_send = str.encode(server_msg)

udp_socket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
udp_socket.bind((local_ip, local_port))

print(f"UDP server up and listening at {local_ip}@{local_port}")

# change to robot ip
send_to_addr = "172.16.1.10"
send_to_port = 25566

while True:
    udp_socket.sendto(bytes_to_send, (send_to_addr, send_to_port))

    msg, addr = udp_socket.recvfrom(buf_size)
    print(f"received {msg} from {addr}")
