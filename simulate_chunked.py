header = "POST /cgi HTTP/1.1\r\nContent-Type: text/plain\r\nTransfer-Encoding: chunked\r\n\r\n"
with open("100kbfile", "r") as f:
    kbtext = f.read()
end_chunk =  "0\r\n\r\n"

import requests
import time
import random

HOST = "localhost"  # Standard loopback interface address (localhost)
PORT = 80  # Port to listen on (non-privileged ports are > 1023)

import socket


with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    s.send(header.encode())
    time.sleep(0.5)
    total_sent = 0
    length_to_send = 69000
    for x in range((100)):
        chunks = f"{str(length_to_send)}\r\n{''.join(random.sample(kbtext,len(kbtext)))[:length_to_send]}\r\n"
        s.send(chunks.encode())
        total_sent += length_to_send
        print(f"sent chunk, total: {total_sent}")
        time.sleep(0.01)
    s.send(end_chunk.encode())
    all_dat = ""
    data = s.recv(1000000000)
    all_dat += data.decode()
    while (len(data.decode())):
        data = s.recv(1000000000)
        all_dat += data.decode()
    with open("recieved_file.html", "w") as f:
        f.write(all_dat)

print(f"Received {all_dat}")
