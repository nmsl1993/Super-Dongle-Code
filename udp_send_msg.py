import socket
import message_pb2
UDP_IP = "10.0.0.111"
UDP_PORT = 8898


print "UDP target IP:", UDP_IP
print "UDP target port", UDP_PORT

com =  message_pb2.Command()
com.LEDCommand.on = False

print com

msg = com.SerializeToString()
print msg
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto(msg,(UDP_IP,UDP_PORT))
