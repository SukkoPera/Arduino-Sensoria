# Send UDP broadcast packets

MYPORT = 9999

import socket

s = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
s.setsockopt (socket.SOL_SOCKET,socket.SO_BROADCAST, 1)

s.sendto ("VER", ('<broadcast>', MYPORT))

clients = []
s.settimeout (1)
timeout = False
while not timeout:
	try:
		reply, (addr, port) = s.recvfrom (1024)
		print "Got '%s' from %s" % (reply.strip (), addr)
		clients.append (addr)
	except socket.error as ex:
		timeout = True

print "Found %d devices:" % len (clients)
for ip in clients:
	print "- %s" % ip
