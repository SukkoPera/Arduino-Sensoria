#!/usr/bin/env python

import sys
import socket

from common import *

class ServerProxy:
	DEBUG = False
	RECV_BUFSIZE = 16384
	DEFAULT_LISTEN_PORT = 9999
	MAX_FAILURES = 3
	DEBUG_COMMS = False

	def __init__ (self, name, sock, ip, port = DEFAULT_LISTEN_PORT):
		self.name = name
		self.address = ip
		self.port = port
		#~ self._sock = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
		#~ self._sock.settimeout (5)
		self._sock = sock
		self._transducers = {}

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
			raise Error, "sendcmd() FAILED: %s" % str (ex)

	def send (self, args):
		if self.DEBUG_COMMS:
			print "<-- %s" % args
		cmd = args.split (" ", 1)[0].upper ()
		rep = self.sendcmd (args)
		if self.DEBUG_COMMS:
			print "--> %s" % rep
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

	def _checkTransducers (self):
		for name, t in self._transducers.items ():
			if t.failures >= self.MAX_FAILURES:
				print >> sys.stderr, "Removing transducer %s because of excessive failures" % name
				del self._transducers[name]

	@property
	def transducers (self):
		"""Returns a dictionary of all transducers"""
		self._checkTransducers ()
		return self._transducers

	@property
	def sensors (self):
		"""Returns a dictionary of sensors only"""
		self._checkTransducers ()
		return {name:t for (name, t) in self._transducers.iteritems () if t.genre == SENSOR}

	@property
	def actuators (self):
		"""Returns a dictionary of actuators only"""
		self._checkTransducers ()
		return {name:t for (name, t) in self._transducers.iteritems () if t.genre == ACTUATOR}
