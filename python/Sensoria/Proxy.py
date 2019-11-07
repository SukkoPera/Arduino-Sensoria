#!/usr/bin/env python

# TODO: Handle unmarshalling failures

import sys

from common import *
from ServerProxy import ServerProxy

class TransducerProxy (object):
	TYPE_STRINGS = {
		PERIODIC: "PRD",
		ON_CHANGE: "CHA"
	}
	
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
		active = self._requestNotification (callback, typ, args)
		self.notificationClients.append ((typ, args, callback, active))
		return active
		
	def _requestNotification (self, callback, typ, args = None):
		assert callable (callback)
		assert self.server is not None
		assert typ in TransducerProxy.TYPE_STRINGS

		if callback not in self.notificationClients:
			if args is not None:
				reply = self.server.send ("NRQ %s %s %s" % (self.name, TransducerProxy.TYPE_STRINGS[typ], args))
			else:
				reply = self.server.send ("NRQ %s %s" % (self.name, TransducerProxy.TYPE_STRINGS[typ]))
			if reply.upper () == "OK":
				return True
			else:
				raise Error, "Unexpected NRQ reply: '%s'" % reply
				return False
		else:
			# Already setup
			return True

	def stopNotify (self, typ):
		assert self.server is not None
		assert typ in TransducerProxy.TYPE_STRINGS

		reqs = filter (lambda (ntyp, args, callback, active): ntyp == typ, self.notificationClients)
		if len (reqs) != 1:
			print "No such NRQ (or multiple ones!!!)"
			return False
		else:
			reply = self.server.send ("NDL %s %s" % (self.name, TransducerProxy.TYPE_STRINGS[typ]))
			if reply.upper () == "OK":
				self.notificationClients.remove (reqs[0])
				return True
			else:
				raise Error, "Unexpected NDL reply: '%s'" % reply
				return False

	# FIXME: Isn't this out of place?
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
	def _processNotification (self, ttl, marshaledData):
			data = self.stereoclass.unmarshalStatic (marshaledData)
			for typ, args, callback, active in self.notificationClients:
				renew = callback (self, data)
				if ttl <= 2:		# Renew before the last one
					if renew:
						print "Renewing NRQ"
						if not self._requestNotification (callback, typ, args):
							print "Renewal failed"
							active = False		# FIXME: I don't think this is this a reference...							
					else:
						print "NRQ expired and was not renewed"


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
