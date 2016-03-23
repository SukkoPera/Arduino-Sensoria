#!/usr/bin/env python

import socket

class ServerProxy:
	DEBUG = False
	RECV_BUFSIZE = 16384
	DEFAULT_LISTEN_PORT = 9999

	def __init__ (self, name, ip, port = DEFAULT_LISTEN_PORT):
		self.name = name
		self.address = ip
		self.port = port
		self._sock = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
		self._sock.settimeout (5)
		self.transducers = {}

	def sendcmd (self, cmd):
		self._sock.sendto (cmd, (self.address, self.port))
		try:
			if ServerProxy.DEBUG:
				print "<-- %s" % cmd
			reply, addr = self._sock.recvfrom (ServerProxy.RECV_BUFSIZE)
			if ServerProxy.DEBUG:
				print "--> %s" % reply.strip ()
			return reply.strip ()
		except socket.error as ex:
			print "FAILED: %s" % str (ex)

	def send (self, args):
		cmd = args.split (" ", 1)[0].upper ()
		rep = self.sendcmd (args)
		if rep is not None:
			parts = rep.split (" ", 1)
			rep0 = parts[0]
			if len (parts) > 1:
				rest = parts[1]
			else:
				rest = None
			if rep0 == "ERR":
				raise SensorError (rest)
			elif rep0 == cmd:
				return rest
			else:
				raise SensorError, "Unexpected reply: '%s' (Command: '%s')" % (rep, args)
		else:
			raise SensorError, "No reply received (Command: '%s')" % args
