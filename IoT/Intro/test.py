import socket

# Program to test the connection to the server and the port UDP 12345 ip: 172.16.42.9 and prints the response with a wait time of 3 seconds

data = "Hello, World!"
ip = "172.16.42.9"
port = 12345
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

response = sock.sendto(bytes(data, "utf-8"), (ip, port))
if response:
    print("Sent: %s" % data)
    print("Received: %s" % response)

print(response)
