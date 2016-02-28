#!/usr/bin/env python

import socket

import stereotypes

from ServerProxy import *
from Proxy import *

class SensoriaError (Exception):
	pass

class Sensoria (object):
	def __init__ (self, servers = [], autodiscover = False):
		self._load_stereotypes ()
		self.serverAddresses = {}
		self.servers = {}
		self.sensors = {}
		for srv in servers:
<<<<<<< HEAD
			parts = srv.split (":")
			if len (parts) == 2:
				srv, port = parts
				self._queryServer (srv, int (port))
			else:
				self._queryServer (srv)
=======
			self._queryServer (srv)
>>>>>>> 5ee9d9a6c3c59933e0f8fed8ca79517959e425b9
		if autodiscover:
			self._discoverServers ()
		self.discoverSensors ()

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

	def discoverSensors (self):
		if len (self.serverAddresses) == 0:
			# Try scanning for servers
			self._discoverServers ()

		self.servers = {}
		self.sensors = {}
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
							self.sensors[name] = newsens	# Add to all sensors
							srv.sensors[name] = newsens		# Also add to server
						else:
							print "ERROR: Sensor uses unknown stereotype %s" % stereotype
					elif typ == "A":
						# FIXME: Add stereotype support
						print "  - Found Actuator %s (%s) using stereotype %s" % (name, desc, stereotype)
						newact = ActuatorProxy (name, srv)
						self.sensors[name] = newact
						srv.sensors[name] = newact
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

	def getSensor (self, name):
		if name in self.sensors:
			return self.sensors[name]
		else:
			raise SensoriaError, "No such sensor: %s" % name


if __name__ == "__main__":
	s = Sensoria ()
	print s.stereotypes
	s.discoverServers ()
	print s.servers
	s.discoverSensors ()
	print s.sensors
