#!/usr/bin/env python

import sys
import socket
import select

import stereotypes

from common import *
from ServerProxy import ServerProxy
from Proxy import TransducerProxy, SensorProxy, ActuatorProxy

class Client (object):
	RECV_BUFSIZE = 16384
	DEFAULT_LISTEN_PORT = 9999

	def __init__ (self, servers = [], autodiscover = False):
		self._load_stereotypes ()
		self.serverAddresses = {}
		self.servers = {}
		self._transducers = {}
		self._setupSocket ()
		for srv in servers:
			parts = srv.split (":")
			if len (parts) == 2:
				srv, port = parts
				self._queryServer (srv, int (port))
			else:
				self._queryServer (srv)
		if autodiscover:
			self._discoverServers ()
		self.discoverTransducers ()

	def _setupSocket (self):
		self._sock = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
		self._sock.settimeout (5)

	def _load_stereotypes (self):
		# Load stereotypes (Hmmm... A bit of a hack?)
		self.stereotypes = {}
		for clsname in stereotypes.__all__:
			exec ("import stereotypes.%s" % clsname)
			cls = eval ("stereotypes.%s.%s" % (clsname, clsname))
			id_ = cls.getIdString ()
			print "Registering stereotype: %s (%s)" % (clsname, id_)
			if id_ in self.stereotypes:
				print "ERROR: Duplicate stereotype: %s" % id_
			else:
				self.stereotypes[cls.getIdString ()] = cls

	def discoverTransducers (self):
		self.servers = {}
		self._transducers = {}
		for model, ip in self.serverAddresses.iteritems ():
			print "- Querying server '%s' at %s:%d" % (model, ip[0], ip[1])
			try:
				srv = ServerProxy (model, self._sock, *ip)
				sensorList = srv.send ("QRY")
				#~ print sensorList
				for s in sensorList.split ("|"):
					name, typ, stereotype, desc = s.split (" ", 3)
					# FIXME: Check for dup transducerss
					if typ == "S":
						print "  - Found Sensor %s (%s) using stereotype %s" % (name, desc, stereotype)
						if stereotype in self.stereotypes:
							newsens = SensorProxy (name, self.stereotypes[stereotype], srv)
							self._transducers[name] = newsens	# Add to all sensors
							srv._transducers[name] = newsens		# Also add to server
						else:
							print "ERROR: Sensor uses unknown stereotype %s" % stereotype
					elif typ == "A":
						print "  - Found Actuator %s (%s) using stereotype %s" % (name, desc, stereotype)
						if stereotype in self.stereotypes:
							newact = ActuatorProxy (name, self.stereotypes[stereotype], srv)
							self._transducers[name] = newact
							srv._transducers[name] = newact
						else:
							print "ERROR: Actuator uses unknown stereotype %s" % stereotype
					else:
						print "  - Found Unknown Transducer %s: %s (Ignored)" % (name, desc)
				# FIXME: Check for dup serverss
				self.servers[model] = srv
			except SensorError as ex:
				print "  - Cannot retrieve sensors list: %s" % str (ex)

	# FIXME: Handle connection failure
	def _queryServer (self, ip, port = ServerProxy.DEFAULT_LISTEN_PORT):
		print "Querying %s:%s" % (ip, port)
		s = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
		s.sendto ("VER", (ip, port))
		s.settimeout (10)
		reply, addr = s.recvfrom (1024)
		#~ print "Got '%s' from %s" % (reply.strip (), addr)
		parts = reply.strip ().split (None, 1)
		rep0 = parts[0]
		if len (parts) > 1:
			model = parts[1]
		else:
			model = None
		#~ print reply, addr
		#~ print "Found \"%s\" at %s" % (model, addr)
		if model in self.serverAddresses:
			print "WARNING: Duplicate server, ignoring: %s (%s)" % (model, addr)
		else:
			self.serverAddresses[model] = addr

	# New discovery method that uses broadcast, much faster!
	def _discoverServers (self):
		#~ self.serverAddresses = {}
		s = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
		s.setsockopt (socket.SOL_SOCKET,socket.SO_BROADCAST, 1)
		s.sendto ("VER", ('<broadcast>', ServerProxy.DEFAULT_LISTEN_PORT))
		s.settimeout (1)
		timeout = False
		while not timeout:
			try:
				reply, addr = s.recvfrom (1024)
				#~ print "Got '%s' from %s" % (reply.strip (), addr)
				parts = reply.strip ().split (None, 1)
				rep0 = parts[0]
				if len (parts) > 1:
					model = parts[1]
				else:
					model = None
				#~ print reply, addr
				#~ print "Found \"%s\" at %s" % (model, addr)
				if model in self.serverAddresses and not addr in self.serverAddresses.values ():
					print "WARNING: Duplicate server, ignoring: %s (%s)" % (model, addr)
				else:
					self.serverAddresses[model] = addr
			except socket.error as ex:
				timeout = True

	#~ def getTransducer (self, name):
		#~ if name in self._transducers:
			#~ return self._transducers[name]
		#~ else:
			#~ raise Error, "No such sensor: %s" % name

	def _checkTransducers (self):
		print "!"
		for name, t in self._transducers.items ():
			if t.failures >= ServerProxy.MAX_FAILURES:
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

	def waitForNotifications (self):
		while True:
			rlist = [self._sock]
			r, w, x = select.select (rlist, [], [])

			if self._sock in r:
				line, client_address = self._sock.recvfrom (Client.RECV_BUFSIZE)

				if line == "":
					break
				else:
					line = line.strip ()
					ip = client_address[0]
					port = int (client_address[1])
					print "Got message from %s:%s: '%s'" % (ip, port, line)
					parts = line.split (" ", 2)
					if len (parts) != 3:
						raise Error, "Unexpected NOT format: '%s'" % notification
					dummy, name, rest = parts
					try:
						trans = self.getTransducer (name)
						trans._processNotification (rest)
					except Error as ex:
						print "ERROR while processing notification: %s" % str (ex)


if __name__ == "__main__":
	s = Client ()
	print s.stereotypes
	s._discoverServers ()
	print s.servers
	s.discoverTransducers ()
	print s.transducers
	print s.sensors
	print s.actuators
