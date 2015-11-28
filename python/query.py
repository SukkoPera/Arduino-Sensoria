#!/usr/bin/env python

from ServerProxy import ServerProxy


#UDP_IP = "127.0.0.1"
UDP_IP = "192.168.1.183"
LISTEN_PORT = 9999

print "UDP target IP:", UDP_IP
print "UDP target port:", LISTEN_PORT

px = ServerProxy (UDP_IP, LISTEN_PORT)

px.send ("ver")

try:
	sensors = px.send ("qry")
	for s in sensors.split ("|"):
		name, typ, desc = s.split (" ", 2)
		print "Found sensor: %s - %s (%s)" % (name, desc, typ)
		sdata = px.send ("qry %s" % name)
		if typ == "S":
			sdata = px.send ("rea %s" % name)
			print "Read: %s" % sdata
except ServerProxy.SensorError as ex:
	print "Cannot retrieve sensors list: %s" % str (ex)

#~ try:
	#~ px.send ("qry bh")
#~ except ServerProxy.SensorError as ex:
	#~ print "Failed: %s" % str (ex)
#~ sendcmd ("rea", srv)
#~ sendcmd ("rea 0", srv)

#~ sendcmd ("wri tb 42", srv)
#~
#~ sendcmd ("rea bh", srv)
#~ sendcmd ("wri bh on", srv)
#~ sendcmd ("rea bh", srv)
