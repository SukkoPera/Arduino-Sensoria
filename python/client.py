import SocketServer
import socket
import threading
import time
import random

CLIENT_ID = "PyTester"
LISTEN_IP = ""  # INADDR_ANY
LISTEN_PORT = 8888

from uuid import getnode as get_mac

class Broadcaster (object):
	PORT = 9999

	def __init__ (self):
		self._sock = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
		self._sock.bind (('', 0))
		self._sock.setsockopt (socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		self._sock.setsockopt (socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
		if CLIENT_ID is not None:
			self._id = CLIENT_ID
		else:
			# Use MAC address (or whatever we'll get from get_mac())
			self._id = ':'.join (("%012x" % get_mac ())[i:i + 2] for i in xrange (0, 12, 2))

	def broadcast (self):
		self._sock.sendto ('ADV %d %s' % (LISTEN_PORT, self._id), ('<broadcast>', self.PORT))

class CommandListener (SocketServer.DatagramRequestHandler):
	def setup (self):
		SocketServer.DatagramRequestHandler.setup (self)
		self._sock = None

	def sendsrv (self, what):
		self._sock = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
		self._sock.connect ((self.msg_ip, 9999))
		self._sock.send (what)

	def reg (self, args):
		print "Registering with server %s:%d" % (self.msg_ip, self.msg_port)

	def qry (self, args):
		line = "%s %d %c %s" % ("KT", 0, 'S', "Kitchen Temperature")
		self.sendsrv (line)

	def rea (self, args):
		parts = data.split (" ", 1)
		sid = int (parts[0])
		if sid == 0:
			sendsrv ("%d" % random.randrange (16, 30))

 	def handle (self):
		data = self.rfile.readline ()
		while data != "":
			data = data.strip ()
			self.msg_ip = self.client_address[0]
			self.msg_port = int (self.client_address[1])
			print "From '%s:%s: '%s'" % (self.msg_ip, self.msg_port, data)
			parts = data.split (" ", 1)
			cmd = parts[0].upper ()
			if len (parts) > 1:
				args = parts[1]
			else:
				args = None

			# Process command
			if cmd == "REG":
				self.reg (args)
			elif cmd == "QRY" :
				self.qry (args)
			else:
				print "Unknown command: '%s'" % cmd

			# Read new line
			data = self.rfile.readline ()


# Start CommandListener thread
server = SocketServer.UDPServer ((LISTEN_IP, LISTEN_PORT), CommandListener)
ip, port = server.server_address
print "Listening for commands on %s:%s..." % (ip, port)

server_thread = threading.Thread (target = server.serve_forever)
server_thread.daemon = True
server_thread.start ()
print "Server loop running in thread:", server_thread.name

bc = Broadcaster ()
while True:
	bc.broadcast ()
	time.sleep (5)


##time.sleep (600)
##print "Shutting down server..."
##server.shutdown ()
##server_thread.join ()
##del server
##print "Server shutdown complete"
