
import socket   # for sockets
import sys      # for exit
import SentenceBases
from Output import savelog
from clear_sentences import clear 

HOST = ''   # Symbolic name, meaning all available interfaces
PORT = 6060 # Arbitrary non-privileged port



# Create a TCP/IP socket 
try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# Handling errors in python socket programs
except socket.error, msg:
	print savelog('Failed to create socket. Error code: ' + str(msg[0]) + ' , Error message : ' + msg[1])
	sys.exit();
	
print savelog('Socket Created')


 
#Bind socket to local host and port
try:
    s.bind((HOST, PORT))
    print savelog('Starting up on "%s" port "%s"' % (HOST, PORT))


except socket.error as msg:
    print savelog('Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1])
    sys.exit()
      
#Start listening on socket
s.listen(10)
print savelog('Socket now listening')

#now keep talking with the client
while 1:
    #wait to accept a connection - blocking call
    print savelog('Waiting for a connection')
    connection, address = s.accept()
    print savelog('Connected with ' + address[0] + ':' + str(address[1]))

    data = connection.recv(1024)
    print savelog('Received : "%s"' % data)
    
    if data:
        message = clear(data)
        print ('MESSAGE from Server:', message)
        final = SentenceBases.Base(message)
        print savelog('Access : "%s"' % final)
        connection.send(final)
        
        
    else:
        print savelog('No more data')
        #break KLINEI TO PROGRAMA
    
s.close()
