#!/usr/bin/env python

import sys
import os
import socket
import select
import threading
import logging

import stereotypes

from common import *
from ServerProxy import ServerProxy
from Proxy import TransducerProxy, SensorProxy, ActuatorProxy


class AutodiscoveryHandler (object):
	def onAutodiscoveryStarted (self):
		pass

	def onAutodiscoveryCompleted (self):
		pass

	def onTransducersAdded (self, transducers):
		pass

	def onTransducersRemoved (self, transducers):
		pass

class Client (object):
	RECV_BUFSIZE = 16384
	DEFAULT_AUTODISCOVER_INTERVAL = 60		# sec
	INITIAL_AUTODISCOVER_DELAY = 1		# sec
	DISCOVER_TIMEOUT = 3		# sec
	MAX_SERVER_FAILURES = 3
	DEFAULT_NOTIFICATION_PORT = 9998
	NOTIFICATION_BUFSIZE = 2048
	DEBUG = True

	def __init__ (self, servers = [], autodiscover = True, autodiscInterval = DEFAULT_AUTODISCOVER_INTERVAL):
		self._logger = logging.getLogger ('client')
		self._load_stereotypes ()
		self._servers = {}
		self._setupSocket ()
		self._handlers = []
		# Notification listener is disabled by default
		self._notificationListenerThread = None
		for srv in servers:
			parts = srv.split (":")
			if len (parts) == 2:
				srv, port = parts
				srvpx = self._queryServer ((srv, int (port)))
			else:
				srvpx = self._queryServer ((srv, ServerProxy.DEFAULT_PORT))
			self._addServer (self._realizeServer (srvpx))
		if autodiscover and autodiscInterval is not None:
			self._logger.debug ("Running autodiscovery every %d seconds", autodiscInterval)
			self._autodiscInterval = autodiscInterval
			self._startAutodiscoverTimer (Client.INITIAL_AUTODISCOVER_DELAY)	# First time
		else:
			self._autodiscoverTimer = None

	def enableNotifications (self):
		self.notificationRequests = []
		self._notificationSocket = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
		server_address = ('0', Client.DEFAULT_NOTIFICATION_PORT)

		print >> sys.stderr, 'Listening for notifications on %s port %s' % server_address
		self._notificationSocket.bind (server_address)

		self._shallStop = False
		self._quitPipe = os.pipe ()
		self._notificationListenerThread = threading.Thread (target = self._notificationThread, name = "Notification Listener")
		self._notificationListenerThread.daemon = True
		self._notificationListenerThread.start ()

	# TO BE TESTED!
	def stop (self):
		if self._notificationListenerThread is not None:
			self._shallStop = True
			os.write (self._quitPipe[1], "X")
			self._notificationListenerThread.join ()
			self._notificationListenerThread = None
			print "Notification listener thread stopped"

	def _notificationThread (self):
		print  >> sys.stderr, 'Waiting for notifications...'
		while not self._shallStop:
			rlist = [self._notificationSocket, self._quitPipe[0]]
			r, w, x = select.select (rlist, [], [], 5)

			if self._quitPipe[0] in r:
				# Thread shall exit
				print "Notification thread exiting!"
				os.read (self._quitPipe[0], 1)
			elif self._notificationSocket in r:
				line, client_address = self._notificationSocket.recvfrom (Client.NOTIFICATION_BUFSIZE)

				if line == "":
					break
				else:
					data = line.strip ()
					self.msg_ip = client_address[0]
					self.msg_port = int (client_address[1])

					data = data.strip ()
					if Client.DEBUG:
						print "%s:%s --> %s" % (self.msg_ip, self.msg_port, data)

					parts = data.split (" ", 2)
					if len (parts) < 3:
						print >> sys.stderr, "Malformed notification: '%s'" % data
						#~ self._reply (client_address, "ERR Malformed command: '%s'" % data)
					else:
						cmd, name, rest = parts
						if cmd != "NOT":
							print >> sys.stderr, "Received non-notification on notification channel"
						else:
							if name in self.transducers:
								try:
									trans = self.transducers[name]
									trans._processNotification (rest)
								except Error as ex:
									print >> sys.stderr, "ERROR while processing notification: %s" % str (ex)
							else:
								print >> sys.stderr, "Received notification for missing transducer: %s" % name
		print "X"

	def registerHandler (self, h):
		if h not in self._handlers:
			self._handlers.append (h)

	def _addServer (self, srvpx):
		if srvpx.name in self._servers:
			self._logger.warning ("Duplicate server, ignoring: %s", srvpx.name)
		else:
			self._servers[srvpx.name] = srvpx

	def _startAutodiscoverTimer (self, interval):
		self._autodiscoverTimer = threading.Timer (interval, self._autodiscoverTimerCallback)
		self._autodiscoverTimer.daemon = True
		self._autodiscoverTimer.start ()

	def _autodiscoverTimerCallback (self):
		logger = logging.getLogger ("client.autodiscovery")
		logger.debug ("--- Autodiscovery start ---")

		# Tell handlers we're ready to roll
		for h in self._handlers:
			h.onAutodiscoveryStarted ()

		try:
			discovered = {}
			s = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
			s.setsockopt (socket.SOL_SOCKET,socket.SO_BROADCAST, 1)
			s.sendto ("HLO", ('<broadcast>', ServerProxy.DEFAULT_PORT))
			#~ s.sendto ("HLO", ('localhost', ServerProxy.DEFAULT_PORT))
			s.settimeout (Client.DISCOVER_TIMEOUT)
			timeout = False
			while not timeout:
				try:
					reply, addr = s.recvfrom (1024)
					model, transducerList = self._parseHloReply (reply, addr)
					if model in discovered:
						# Same server discovered twice. Just some sort of useless protection, actually
						logger.warning ("Duplicate server, ignoring: %s (%s)", model, addr)
					else:
						srvpx = ServerProxy (model, transducerList, self._sock, *addr)
						discovered[model] = srvpx
						if model in self._servers:
							# This is a server we already know
							logger.debug ("Server %s is already known", model)
							knownSrv = self._servers[model]

							# Server responded, so clear failure count
							knownSrv.failures = 0

							# Check if server somehow changed
							different = False

							# Is address still the same?
							if knownSrv.address != srvpx.address or knownSrv.port != srvpx.port:
								logger.info ("Server has different address: %s -> %s", knownSrv.address, srvpx.address)
								different = True
							else:
								# Address unchanged, check transducer list
								if knownSrv.transducerList != srvpx.transducerList:
									logger.info ("Server has different transducer list")
									different = True

							if not different:
								logger.debug ("Server is unchanged")
							else:
								logger.info ("Server has changed, updating")

								# Remove old server, informing handlers
								del self._servers[model]
								for h in self._handlers:
									h.onTransducersRemoved (knownSrv.transducers.values ())
								del knownSrv

								# And add new
								self._addServer (self._realizeServer (srvpx))
								for h in self._handlers:
									h.onTransducersAdded (srvpx.transducers.values ())
						else:
							logger.info ("Server %s is new", model)
							if len (srvpx.transducers) > 0:
								self._addServer (self._realizeServer (srvpx))
							else:
								logger.warning ("Server %s has no transducers, ignoring" % model)

							# Inform handlers
							for h in self._handlers:
								h.onTransducersAdded (srvpx.transducers.values ())
				except socket.error as ex:
					timeout = True

			for model, srvpx in self._servers.iteritems ():
				if model not in discovered:
					logger.warning ("Server %s did not respond to autodiscovery", model)
					srvpx.failures += 1
		except Error as ex:
			logger.error ("Autodiscovery failed: %s", str (ex))


		# Tell handlers we're done
		for h in self._handlers:
			h.onAutodiscoveryCompleted ()

		# Restart timer
		self._startAutodiscoverTimer (self._autodiscInterval)
		logger.debug ("--- Autodiscovery end ---")

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
			self._logger.debug ("Registering stereotype: %s (%s)", clsname, id_)
			if id_ in self.stereotypes:
				self._logger.error ("Duplicate stereotype: %s", id_)
			else:
				self.stereotypes[cls.getIdString ()] = cls

	def _realizeServer (self, srvpx):
		srvpx._transducers = {}
		if srvpx.transducerList is not None:
			for sensorData in srvpx.transducerList.split ("|"):
				sensparts = sensorData.split (" ", 3)
				name, typ, stereotype = sensparts[0:3]
				if len (sensparts) > 3:
					desc = sensparts[3]
				else:
					desc = ""
				# FIXME: Check for dup transducerss
				if typ == "S":
					self._logger.info ("- Found Sensor %s (%s) using stereotype %s", name, desc, stereotype)
					if stereotype in self.stereotypes:
						#~ newsens = SensorProxy (name, self.stereotypes[stereotype], srv)
						newsens = SensorProxy (name, typ, stereotype, desc, self.stereotypes[stereotype], srvpx)
						srvpx._transducers[name] = newsens		# Also add to server
					else:
						self._logger.error ("Sensor uses unknown stereotype %s", stereotype)
				elif typ == "A":
					self._logger.info ("- Found Actuator %s (%s) using stereotype %s", name, desc, stereotype)
					if stereotype in self.stereotypes:
						newact = ActuatorProxy (name, self.stereotypes[stereotype], srvpx)
						srvpx._transducers[name] = newact
					else:
						self._logger.error ("Actuator uses unknown stereotype %s", stereotype)
				else:
					self._logger.error ("- Found Unknown Transducer %s: %s (Ignored)", name, desc)
			ret = srvpx
		else:
			# No transducers
			ret = None
		return ret

	def _parseHloReply (self, reply, addr):
		model = None
		transducerList = None
		reply = reply.strip ()
		parts = reply.split (" ", 2)
		if len (parts) < 2:
			self._logger.error ("Unexpected HLO reply: %s", reply)
		elif parts[0].upper () != "HLO":
			if parts[0].upper () == "ERR":
				self._logger.warning ("Node at %s does not support HLO: %s", addr[0], reply)
			else:
				self._logger.error ("Unexpected HLO reply: %s", reply)
		elif len (parts) > 2:
			model = parts[1]
			transducerList = parts[2]
			#~ print reply, addr
			self._logger.info ("Found \"%s\" at %s:%d", model, addr[0], addr[1])
		else:
			# No transducers
			model = parts[1]
		return model, transducerList

	def _queryServer (self, addr):
		srvpx = None
		self._logger.debug ("Querying %s:%d", addr[0], addr[1])
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
			self._logger.error ("Query of server at %s:%d failed", addr[0], addr[1])
		return srvpx

	def _checkServers (self):
		for model in self._servers.keys ():	# Use .keys() so that we can delete while iterating
			srvpx = self._servers[model]
			if srvpx.failures >= Client.MAX_SERVER_FAILURES:
				# Remove, telling handlers
				self._logger.error ("Removing server %s because of excessive failures", model)
				del self._servers[model]
				for h in self._handlers:
					h.onTransducersRemoved (srvpx.transducers.values ())
				del srvpx

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
		#~ return {name:t for (name, t) in self.transducers.iteritems () if t.genre == SENSOR}
		return dict ([(name, t) for (name, t) in self.transducers.iteritems () if t.genre == SENSOR])

	@property
	def actuators (self):
		"""Returns a dictionary of actuators only"""
		#~ return {name:t for (name, t) in self.transducers.iteritems () if t.genre == ACTUATOR}
		return dict ([(name, t) for (name, t) in self.transducers.iteritems () if t.genre == ACTUATOR])



if __name__ == "__main__":
	s = Client ()
	print s.stereotypes
	s._discoverServers ()
	print s.servers
	s.discoverTransducers ()
	print s.transducers
	print s.sensors
	print s.actuators
