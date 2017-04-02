#!/usr/bin/env python

import sys
import socket
import select
import threading

import stereotypes

from common import *
from ServerProxy import ServerProxy
from Proxy import TransducerProxy, SensorProxy, ActuatorProxy


class Client (object):
	RECV_BUFSIZE = 16384
	DEFAULT_AUTODISCOVER_TIMER = 60
	MAX_SERVER_FAILURES = 3

	def __init__ (self, servers = [], autodiscover = True, autodiscTimer = DEFAULT_AUTODISCOVER_TIMER):
		self._load_stereotypes ()
		self._servers = {}
		self._setupSocket ()
		for srv in servers:
			parts = srv.split (":")
			if len (parts) == 2:
				srv, port = parts
				srvpx = self._queryServer ((srv, int (port)))
			else:
				srvpx = self._queryServer ((srv, ServerProxy.DEFAULT_PORT))
			self._addServer (srvpx)
		if autodiscover:
			for model, srvpx in self._discoverQuick ().iteritems ():
				self._servers[model] = self._realizeServer (srvpx)
		if autodiscover and autodiscTimer is not None:
			self._autodiscTimer = autodiscTimer
			self._startAutodiscoverTimer ()
		else:
			self._autodiscoverTimer = None

	def _addServer (self, srvpx):
		if srvpx.name in self._servers:
			print "WARNING: Duplicate server, ignoring: %s (%s)" % (srvpx.name, srv)
		else:
			self._servers[srvpx.name] = srvpx

	def _startAutodiscoverTimer (self):
		#~ print "AutoTimer started"
		self._autodiscoverTimer = threading.Timer (self._autodiscTimer, self._autodiscoverTimerCallback)
		self._autodiscoverTimer.daemon = True
		self._autodiscoverTimer.start ()

	def _autodiscoverTimerCallback (self):
		print "Running Autodiscover timer"
		try:
			#~ self._discoverServers ()
			#~ self.discoverTransducers ()
			discovered = self._discoverQuick ()
			for model, srvpx in discovered.iteritems ():
				if model in self._servers:
					print "Server %s is already known" % model
					knownSrv = self._servers[model]
					different = False

					# Check if address is still the same
					if knownSrv.address != srvpx.address or \
					   knownSrv.port != srvpx.port:
						print "Server has different address"
						different = True
					else:
						# Address unchanged, check transducer list
						if knownSrv.transducerList != srvpx.transducerList:
							print "Server has different transducer list"
							different = True

					if not different:
						print "Server is unchanged"
					else:
						print "Server has changed, updating"
						self._addServer (self._realizeServer (srvpx))
				else:
					print "Server %s is new" % model
					self._addServer (self._realizeServer (srvpx))

			for model, srvpx in self._servers.iteritems ():
				if model not in discovered:
					print "Server %s did not respond to autodiscovery" % model
					srvpx.failures += 1
		except Error as ex:
			print "Error in autodiscovery timer: %s" % str (ex)

		# Restart
		self._startAutodiscoverTimer ()

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

	def _realizeServer (self, srvpx):
		for sensorData in srvpx.transducerList.split ("|"):
			sensparts = sensorData.split (" ", 3)
			name, typ, stereotype = sensparts[0:3]
			if len (sensparts) > 3:
				desc = sensparts[3]
			else:
				desc = ""
			# FIXME: Check for dup transducerss
			if typ == "S":
				print "  - Found Sensor %s (%s) using stereotype %s" % (name, desc, stereotype)
				if stereotype in self.stereotypes:
					#~ newsens = SensorProxy (name, self.stereotypes[stereotype], srv)
					newsens = SensorProxy (name, typ, stereotype, desc, self.stereotypes[stereotype], srvpx)
					#~ self._transducers[name] = newsens	# Add to all sensors
					srvpx._transducers[name] = newsens		# Also add to server
				else:
					print "ERROR: Sensor uses unknown stereotype %s" % stereotype
			elif typ == "A":
				print "  - Found Actuator %s (%s) using stereotype %s" % (name, desc, stereotype)
				if stereotype in self.stereotypes:
					newact = ActuatorProxy (name, self.stereotypes[stereotype], srvpx)
					#~ self._transducers[name] = newact
					srvpx._transducers[name] = newact
				else:
					print "ERROR: Actuator uses unknown stereotype %s" % stereotype
			else:
				print "  - Found Unknown Transducer %s: %s (Ignored)" % (name, desc)
		return srvpx

	def _discoverQuick (self):
		discovered = {}
		s = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
		s.setsockopt (socket.SOL_SOCKET,socket.SO_BROADCAST, 1)
		s.sendto ("HLO", ('<broadcast>', ServerProxy.DEFAULT_PORT))
		s.settimeout (1)
		timeout = False
		while not timeout:
			try:
				reply, addr = s.recvfrom (1024)
				reply = reply.strip ()
				#~ print "Got '%s' from %s" % (reply, addr)
				parts = reply.split (" ", 2)
				if len (parts) < 2:
					print "Unexpected HLO reply: %s" % reply
				elif parts[0].upper () != "HLO":
					if parts[0].upper () == "ERR":
						print "Node at %s does not support HLO: %s" % (addr[0], reply)
					else:
						print "Unexpected HLO reply: %s" % reply
				else:
					model = parts[1]
					transducerList = parts[2]
					#~ print reply, addr
					print "Found \"%s\" at %s:%d" % (model, addr[0], addr[1])
					if model in discovered:
						print "WARNING: Duplicate server, ignoring: %s (%s)" % (model, addr)
					else:
						srv = ServerProxy (model, transducerList, self._sock, *addr)
						#~ print transducerList
						discovered[model] = srv
			except socket.error as ex:
				timeout = True
		return discovered

	def _parseHloReply (self, reply, addr):
		model = None
		transducerList = None
		reply = reply.strip ()
		parts = reply.split (" ", 2)
		if len (parts) < 2:
			print "Unexpected HLO reply: %s" % reply
		elif parts[0].upper () != "HLO":
			if parts[0].upper () == "ERR":
				print "Node at %s does not support HLO: %s" % (addr[0], reply)
			else:
				print "Unexpected HLO reply: %s" % reply
		else:
			model = parts[1]
			transducerList = parts[2]
			#~ print reply, addr
			print "Found \"%s\" at %s:%d" % (model, addr[0], addr[1])
		return model, transducerList

	def _queryServer (self, addr):
		srvpx = None
		print "Querying %s:%d" % (addr[0], addr[1])
		s = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
		s.setsockopt (socket.SOL_SOCKET,socket.SO_BROADCAST, 1)
		s.sendto ("HLO", addr)
		s.settimeout (10)
		try:
			reply, addr = s.recvfrom (1024)
			model, transducerList = self._parseHloReply (reply, addr)
			if model is not None and transducerList is not None:
				srvpx = ServerProxy (model, transducerList, self._sock, *addr)
		except socket.error as ex:
			# How can this happen?
			print "Query of server at %s:%d failed" % (addr[0], addr[1])
		return srvpx

	def _checkServers (self):
		for model in self._servers.keys ():	# Use .keys() so that we can delete while iterating
			srvpx = self._servers[model]
			if srvpx.failures >= Client.MAX_SERVER_FAILURES:
				# Remove and try to update
				addr = (srvpx.address, srvpx.port)
				del self._servers[model]
				del srvpx
				updatedSrv = self._queryServer (addr)
				if updatedSrv is None:
					print >> sys.stderr, "Removing server %s because of excessive failures" % model
				else:
					# Server is still alive, maybe with a different transducer list, so update it
					print "Updating server %s after excessive failures" % model
					self._addServer (self._realizeServer (updatedSrv))

	@property
	def servers (self):
		"""Returns a dictionary of all servers"""
		self._checkServers ()
		return self._servers

	@property
	def transducers (self):
		"""Returns a dictionary of all transducers"""
		self._checkServers ()
		transducers = {}
		for model, srvpx in self.servers.iteritems ():
			for tname, t in srvpx.transducers.iteritems ():
				transducers[tname] = t
		return transducers

	@property
	def sensors (self):
		"""Returns a dictionary of sensors only"""
		return {name:t for (name, t) in self.transducers.iteritems () if t.genre == SENSOR}

	@property
	def actuators (self):
		"""Returns a dictionary of actuators only"""
		return {name:t for (name, t) in self.transducers.iteritems () if t.genre == ACTUATOR}

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
