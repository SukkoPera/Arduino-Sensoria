#!/usr/bin/env python

import sys
import socket
import logging

from common import *

# Register a new logging level
# https://stackoverflow.com/questions/2183233
logging.DEBUG_COMMS = 9
logging.addLevelName (logging.DEBUG_COMMS, "DEBUG_COMMS")
def __debugcomms (self, message, *args, **kws):
	if self.isEnabledFor (logging.DEBUG_COMMS):
		self._log (logging.DEBUG_COMMS, message, args, **kws)
logging.Logger.debugComms = __debugcomms

class ServerProxy (object):
	RECV_BUFSIZE = 4096
	DEFAULT_PORT = 9999

	def __init__ (self, name, protoVer, srvId, transducerList, sock, ip, port = DEFAULT_PORT):
		self._logger = logging.getLogger ("ServerProxy")
		self.name = name
		self.protocolVersion = protoVer
		self.srvId = srvId
		self.transducerList = transducerList		# Exactly as received
		self.address = ip
		self.port = port
		self.failures = 0
		#~ self._sock = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
		#~ self._sock.settimeout (5)
		self._sock = sock
		self._transducers = {}

	def sendcmd (self, cmd):
		try:
			# Create a new socket every time, so that we always use a different
			# ephemeral port and we only get the relevant reply
			sock = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
			sock.settimeout (5)
			sock.connect ((self.address, self.port))
			localaddr, localport = sock.getsockname ()

			self._logger.debugComms ("%s:%u <-- %s:%u %s", self.address, self.port, localaddr, localport, cmd)
			sock.send (cmd)
			reply = sock.recv (ServerProxy.RECV_BUFSIZE)
			reply = reply.strip ()
			self._logger.debugComms ("%s:%u --> %s:%u %s", self.address, self.port, localaddr, localport, reply)
			return reply
		except socket.error as ex:
			raise Error, "Communication error - %s" % str (ex)
		finally:
			sock.shutdown (socket.SHUT_RDWR)
			sock.close ()

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
				raise Error, "Unexpected reply: '%s' (Command: '%s')" % (rep, args)
		else:
			# This shouldn't really happen
			raise Error, "No reply received (Command: '%s')" % args

	@property
	def transducers (self):
		"""Returns a dictionary of all transducers"""
		return self._transducers

	@property
	def sensors (self):
		"""Returns a dictionary of sensors only"""
		return {name:t for (name, t) in self._transducers.iteritems () if t.genre == SENSOR}

	@property
	def actuators (self):
		"""Returns a dictionary of actuators only"""
		return {name:t for (name, t) in self._transducers.iteritems () if t.genre == ACTUATOR}
