import socket

UDP_IP = "10.0.0.111"
UDP_PORT = 8898

MESSAGE = "cHello, world!"

print "UDP target IP:", UDP_IP
print "UDP target port", UDP_PORT
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto(MESSAGE,(UDP_IP,UDP_PORT))
