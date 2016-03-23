#!/usr/bin/env python

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

	def configure (self, name, value):
		raise NotImplementedError

class SensorProxy (TransducerProxy):
	def __init__ (self, name, stereoclass, srv):
		sdata = srv.send ("QRY %s" % name)
		name, typ, stereotype, description, version = sdata.split ("|")
		super (SensorProxy, self).__init__ (srv, SENSOR, name, stereotype, stereoclass, description, version)

	def read (self, raw = False):
		assert self.server is not None
		reply = self.server.send ("REA %s" % self.name)
		parts = reply.split (" ", 1)
		if len (parts) != 2:
			raise SensorError, "AAAA"	# FIXME
		name, rest = parts
		assert name == self.name
		if raw:
			return rest
		else:
			return self.stereoclass.unmarshalStatic (rest)

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
			raise SensorError, "AAAA"	# FIXME
		name, rest = parts
		assert name == self.name
		if raw:
			return rest
		else:
			return self.stereoclass.unmarshalStatic (rest)
