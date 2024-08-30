import socket

ip = "10.32.69.223"
port = 57006


def send_package_udp(data):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.sendto(bytes(data, "utf-8"), (ip, port))

    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.bind((ip, port))
    while True:
        data, addr = s.recvfrom(1024)  # buffer size is 1024 bytes
        print('Data: {} \nFrom {}'.format(data, addr))


if __name__ == '__main__':
    send_package_udp('Hello')
