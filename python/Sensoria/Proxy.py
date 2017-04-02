#!/usr/bin/env python

import sys

from common import *
from ServerProxy import ServerProxy

class TransducerProxy (object):
	def __init__ (self, server, genre, name, stereotype, stereoclass, description, version):
		assert not "|" in name and not "|" in description
		self.genre = genre
		self.server = server
		self.name = name
		self.stereotype = stereotype
		self.stereoclass = stereoclass
		self.description = description
		self.version = version
		self.notificationClients = []

	def configure (self, name, value):
		raise NotImplementedError

	def notify (self, callback, typ, args = None):
		typeStrings = {
			PERIODIC: "PRD",
			ON_CHANGE: "CHA"
		}

		assert callable (callback)
		assert self.server is not None
		assert typ in typeStrings

		if callable not in self.notificationClients:
			if args is not None:
				reply = self.server.send ("NRQ %s %s %s" % (self.name, typeStrings[typ], args))
			else:
				reply = self.server.send ("NRQ %s %s" % (self.name, typeStrings[typ]))
			parts = reply.split (" ", 4)
			if len (parts) != 3:
				raise Error, "Unexpected NRQ reply: '%s'" % reply
			name, t, rep = parts
			assert name == self.name
			assert t == typeStrings[typ]
			if rep.upper () == "OK":
				self.notificationClients.append (callback)
				return True
			else:
				return False
		else:
			# Already setup
			return True

	# This can only be called from Client
	def _processNotification (self, marshaledData, raw = False):
		if raw:
			#~ return marshaledData
			assert False
		else:
			data = self.stereoclass.unmarshalStatic (marshaledData)
			for callback in self.notificationClients:
				callback (data)


class SensorProxy (TransducerProxy):
	#~ def __init__ (self, name, stereoclass, srv):
		#~ sdata = srv.send ("QRY %s" % name)
		#~ name, typ, stereotype, description, version = sdata.split ("|")
		#~ super (SensorProxy, self).__init__ (srv, SENSOR, name, stereotype, stereoclass, description, version)

	# This c'tor does not query sensor data, but since we'll only be missing the version, we are fine with it
	def __init__ (self, name, typ, stereotype, description, stereoclass, srv):
		super (SensorProxy, self).__init__ (srv, SENSOR, name, stereotype, stereoclass, description, "N/A")

	def read (self, raw = False):
		assert self.server is not None
		try:
			reply = self.server.send ("REA %s" % self.name)
			parts = reply.split (" ", 1)
			if len (parts) != 2:
				raise Error, "Unexpected REA reply: '%s'" % reply
			name, rest = parts
			assert name == self.name
			if raw:
				return rest
			else:
				return self.stereoclass.unmarshalStatic (rest)
		except Error as ex:
			print >> sys.stderr, "Sensor read failed: %s" % str (ex)
			self.server.failures += 1
			raise

class ActuatorProxy (TransducerProxy):
	def __init__ (self, name, stereoclass, srv):
		sdata = srv.send ("QRY %s" % name)
		name, typ, stereotype, description, version = sdata.split ("|")
		super (ActuatorProxy, self).__init__ (srv, ACTUATOR, name, stereotype, stereoclass, description, version)

	def write (self, what):
		assert self.server is not None
		return self.server.send ("WRI %s %s" % (self.name, what))

	def read (self, raw = False):
		assert self.server is not None
		reply = self.server.send ("REA %s" % self.name)
		parts = reply.split (" ", 1)
		if len (parts) != 2:
			raise Error, "Unexpected REA reply: '%s'" % reply
		name, rest = parts
		assert name == self.name
		if raw:
			return rest
		else:
			return self.stereoclass.unmarshalStatic (rest)
