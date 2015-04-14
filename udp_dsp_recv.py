import socket
import message_pb2
UDP_IP = ""
UDP_PORT = 8898


print "UDP target IP:", UDP_IP
print "UDP target port", UDP_PORT

com =  message_pb2.DSPResponse()
com.freq1 = 1.0;
com.freq2 = 1.1;
com.freq3 = 1.2;
print len(com.SerializeToString())

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP,UDP_PORT))
s,addr = sock.recvfrom(82)

com.ParseFromString(s[0:15])
print com
