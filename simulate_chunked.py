header = "POST /cgi HTTP/1.1\r\nContent-Type: text/plain\r\nTransfer-Encoding: chunked\r\n\r\n"
chunks = ["10\r\n1111111111\r\n", "10\r\n2222222222\r\n","10\r\n3333333333\r\n","0\r\n\r\n"]

import requests
import time


HOST = "localhost"  # Standard loopback interface address (localhost)
PORT = 80  # Port to listen on (non-privileged ports are > 1023)

import socket


with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    s.send(header.encode())
    time.sleep(0.5)
    for x in range(len(chunks)):
        print(f"sending: {chunks[x]}")
        s.send(chunks[x].encode())
        print("sent chunk")
        time.sleep(0.5)
    data = s.recv(100000)

print(f"Received {data}")
