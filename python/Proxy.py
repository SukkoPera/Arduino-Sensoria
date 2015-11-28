#!/usr/bin/env python

class TransducerProxy (object):
	def __init__ (self, server, name, stereotype, description, version):
		assert not "|" in name and not "|" in description
		self.server = server
		self.name = name
		self.description = description
		self.stereotype = stereotype
		self.version = version

	def configure (self, name, value):
		raise NotImplementedError

class SensorProxy (TransducerProxy):
	def __init__ (self, name, srv):
		sdata = srv.send ("QRY %s" % name)
		name, stereotype, description, version = sdata.split ("|", 3)
		super (SensorProxy, self).__init__ (srv, name, stereotype, description, version)

	def read (self):
		assert self.server is not None
		reply = self.server.send ("REA %s" % self.name)
		parts = reply.split (" ", 1)
		if len (parts) != 2:
			raise AAAA	# FIXME
		name, rest = parts
		assert name == self.name
		return rest

class ActuatorProxy (TransducerProxy):
	def __init__ (self, name, srv):
		sdata = srv.send ("QRY %s" % name)
		name, stereotype, description, version = sdata.split ("|", 3)
		super (ActuatorProxy, self).__init__ (srv, name, stereotype, description, version)

	def write (self, what):
		assert self.server is not None
		return self.server.send ("WRI %s %s" % (self.name, what))
