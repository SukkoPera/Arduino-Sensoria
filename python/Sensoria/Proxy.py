#!/usr/bin/env python

# TODO: Handle unmarshalling failures

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

		if callback not in self.notificationClients:
			if args is not None:
				reply = self.server.send ("NRQ %s %s %s" % (self.name, typeStrings[typ], args))
			else:
				reply = self.server.send ("NRQ %s %s" % (self.name, typeStrings[typ]))
			if reply.upper () == "OK":
				self.notificationClients.append (callback)
				return True
			else:
				raise Error, "Unexpected NRQ reply: '%s'" % reply
				return False
		else:
			# Already setup
			return True

	def stopNotify (self, typ):
		typeStrings = {
			PERIODIC: "PRD",
			ON_CHANGE: "CHA"
		}

		assert self.server is not None
		assert typ in typeStrings

		reply = self.server.send ("NDL %s %s" % (self.name, typeStrings[typ]))
		if reply.upper () == "OK":
			# FIXME
			# ~ self.notificationClients.append (callback)
			return True
		else:
			raise Error, "Unexpected NDL reply: '%s'" % reply
			return False

	def cancelAllNotifications (self):
		assert self.server is not None

		reply = self.server.send ("NCL")
		if reply.upper () == "OK":
			# FIXME
			# ~ self.notificationClients.append (callback)
			return True
		else:
			raise Error, "Unexpected NCL reply: '%s'" % reply
			return False

	# This can only be called from Client
	def _processNotification (self, marshaledData):
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
			
			if self.server.protocolVersion == 0:
				name, rest = parts
				assert name == self.name, "REA sensor name mismatch: '%s' vs '%s'" % (name, self.name)
			elif self.server.protocolVersion == 1:
				status, rest = parts
				if status.upper () != "OK":
					if len (parts) > 1:
						raise Error (parts[1])
					else:
						raise Error ()
			else:
				raise Error, "Unsupported protocol version for REA"
				
			if raw:
				return rest
			else:
				return self.stereoclass.unmarshalStatic (rest)
		except Error as ex:
			print >> sys.stderr, "Sensor read failed: %s" % str (ex)
			self.server.failures += 1
			raise

class ActuatorProxy (TransducerProxy):
	# ~ def __init__ (self, name, stereoclass, srv):
		# ~ sdata = srv.send ("QRY %s" % name)
		# ~ name, typ, stereotype, description, version = sdata.split ("|")
		# ~ super (ActuatorProxy, self).__init__ (srv, ACTUATOR, name, stereotype, stereoclass, description, version)

	# This c'tor does not query sensor data, but since we'll only be missing the version, we are fine with it
	def __init__ (self, name, typ, stereotype, description, stereoclass, srv):
		super (ActuatorProxy, self).__init__ (srv, ACTUATOR, name, stereotype, stereoclass, description, "N/A")

	def write (self, what, raw = False):
		assert self.server is not None
		marshalled = what.marshal ()
		rep = self.server.send ("WRI %s %s" % (self.name, marshalled))
		parts = rep.split (" ", 1)
		rep0 = parts[0].upper ()
		if rep0 == "OK":
			if len (parts) > 1:
				# New transducer status follows
				rest = parts[1]
				if raw:
					return rest
				else:
					return self.stereoclass.unmarshalStatic (rest)
			else:
				return None
		else:
			if len (parts) > 1:
				raise Error (parts[1])
			else:
				raise Error ()

	def read (self, raw = False):
		assert self.server is not None
		reply = self.server.send ("REA %s" % self.name)
		parts = reply.split (" ", 1)
		if len (parts) != 2:
			raise Error, "Unexpected REA reply: '%s'" % reply
		
		if self.server.protocolVersion == 0:
			name, rest = parts
			assert name == self.name, "REA actuator name mismatch: '%s' vs '%s'" % (name, self.name)
		elif self.server.protocolVersion == 1:
			status, rest = parts
			if status.upper () != "OK":
				if len (parts) > 1:
					raise Error (parts[1])
				else:
					raise Error ()
		else:
			raise Error, "Unsupported protocol version for REA"
			
		if raw:
			return rest
		else:
			return self.stereoclass.unmarshalStatic (rest)
