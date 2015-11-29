#!/usr/bin/env python

import socket

import stereotypes

from ServerProxy import *
from Proxy import *

class SensoriaError (Exception):
	pass

class Sensoria (object):
	def __init__ (self, autodiscover = False):
		self._load_stereotypes ()
		self.serverAddresses = {}
		self.servers = {}
		self.sensors = {}
		if autodiscover:
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
					name, stereotype, desc = s.split (" ", 2)
					# FIXME: Check for dup transducerss
					if stereotype == "S":
						print "  - Found Sensor %s: %s" % (name, desc)
						newsens = SensorProxy (name, srv)
						self.sensors[name] = newsens	# Add to all sensors
						srv.sensors[name] = newsens		# Also add to server
					elif stereotype == "A":
						print "  - Found Actuator %s: %s" % (name, desc)
						newact = ActuatorProxy (name, srv)
						self.sensors[name] = newact
						srv.sensors[name] = newact
					else:
						print "  - Found Unknown Transducer %s: %s (Ignored)" % (name, desc)
				# FIXME: Check for dup serverss
				self.servers[model] = srv
			except SensorError as ex:
				print "  - Cannot retrieve sensors list: %s" % str (ex)

	# New discovery method that uses broadcast, much faster!
	def _discoverServers (self):
		self.serverAddresses = {}
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
				if model in self.serverAddresses:
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
