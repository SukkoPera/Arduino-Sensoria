#!/usr/bin/env python

import socket

import stereotypes

from common import *
from ServerProxy import ServerProxy
from Proxy import TransducerProxy, SensorProxy, ActuatorProxy

class Client (object):
	def __init__ (self, servers = [], autodiscover = False):
		self._load_stereotypes ()
		self.serverAddresses = {}
		self.servers = {}
		self.transducers = {}
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
		self.transducers = {}
		for model, ip in self.serverAddresses.iteritems ():
			print "- Querying server '%s' at %s:%d" % (model, ip[0], ip[1])
			try:
				srv = ServerProxy (model, *ip)
				sensorList = srv.send ("QRY")
				#~ print sensorList
				for s in sensorList.split ("|"):
					name, typ, stereotype, desc = s.split (" ", 3)
					# FIXME: Check for dup transducerss
					if typ == "S":
						print "  - Found Sensor %s (%s) using stereotype %s" % (name, desc, stereotype)
						if stereotype in self.stereotypes:
							newsens = SensorProxy (name, self.stereotypes[stereotype], srv)
							self.transducers[name] = newsens	# Add to all sensors
							srv.transducers[name] = newsens		# Also add to server
						else:
							print "ERROR: Sensor uses unknown stereotype %s" % stereotype
					elif typ == "A":
						print "  - Found Actuator %s (%s) using stereotype %s" % (name, desc, stereotype)
						if stereotype in self.stereotypes:
							newact = ActuatorProxy (name, self.stereotypes[stereotype], srv)
							self.transducers[name] = newact
							srv.transducers[name] = newact
						else:
							print "ERROR: Actuator uses unknown stereotype %s" % stereotype
					else:
						print "  - Found Unknown Transducer %s: %s (Ignored)" % (name, desc)
				# FIXME: Check for dup serverss
				self.servers[model] = srv
			except SensorError as ex:
				print "  - Cannot retrieve sensors list: %s" % str (ex)

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

	def getTransducer (self, name):
		if name in self.transducers:
			return self.transducers[name]
		else:
			raise SensoriaError, "No such sensor: %s" % name

	@property
	def sensors (self):
		"""Returns a list of sensors only"""
		return {name:t for (name, t) in self.transducers.iteritems () if t.genre == SENSOR}

	@property
	def actuators (self):
		"""Returns a list of actuators only"""
		return {name:t for (name, t) in self.transducers.iteritems () if t.genre == ACTUATOR}

if __name__ == "__main__":
	s = Client ()
	print s.stereotypes
	s._discoverServers ()
	print s.servers
	s.discoverTransducers ()
	print s.transducers
	print s.sensors
	print s.actuators