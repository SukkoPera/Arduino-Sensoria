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

	def onTransducersAdded (self, transducers):
		pass

	def onTransducersRemoved (self, transducers):
		pass

class Client (object):
	RECV_BUFSIZE = 16384
	DEFAULT_AUTODISCOVER_INTERVAL = 60		# sec
	DISCOVER_TIMEOUT = 3		# sec
	MAX_SERVER_FAILURES = 3
	DEFAULT_SERVER_PORT = ServerProxy.DEFAULT_PORT
	DEFAULT_NOTIFICATION_PORT = 9998
	NOTIFICATION_BUFSIZE = 2048
	DEBUG = True

	def __init__ (self, servers = []):
		self._logger = logging.getLogger ('client')
		# ~ self._logger.setLevel (logging.DEBUG)
		# ~ self._logger.addHandler (logging.StreamHandler ())
		self._load_stereotypes ()
		self._servers = {}
		self._setupSocket ()
		self._handlers = []
		self._autodiscoverEnabled = False
		self._autodiscoverTimer = None
		self._discoverLock = threading.RLock ()
		# Notification listener is disabled by default
		self._notificationListenerThread = None
		for srv in servers:
			parts = srv.split (":")
			if len (parts) == 2:
				srv, port = parts
				srvpx = self._queryServer ((srv, int (port)))
			else:
				srvpx = self._queryServer ((srv, Client.DEFAULT_SERVER_PORT))
			if srvpx is not None:
				self._addServer (self._realizeServer (srvpx))

	def enableNotifications (self, port = DEFAULT_NOTIFICATION_PORT):
		logger = logging.getLogger ("client.notifications")
		self.notificationRequests = []
		self._notificationSocket = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
		server_address = ('', port)

		logger.info ('Listening for notifications on %s port %s', "INADDR_ANY" if not server_address[0] else server_address[0], server_address[1])
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

	def _notificationNot (self, addr, args):
		logger = logging.getLogger ("client.notifications")
		parts = args.split (" ", 2)
		if len (parts) != 3:
			logger.error ("Malformed notification: '%s'", data)
		else:
			name, ttl, rest = parts
			if name in self.transducers:
				try:
					trans = self.transducers[name]
					trans._processNotification (rest)
				except Error as ex:
					logger.error ("ERROR while processing notification: %s", str (ex))
			else:
				logger.error ("Received notification for unknown transducer: %s", name)

	def _notificationAdv (self, addr, args):
		logger = logging.getLogger ("client.autodiscovery")
		logger.debug ("Got server advertisement: %s" % args)
		data = self._parseAdvertisement (args, addr)
		if data:
			srvModel, srvId, srvAddr = data
			logger.debug ("Parsed server advertisement to %s/%s/%s" % data)

			if srvModel in self._servers:
				# This is a server we already know
				logger.debug ("Server %s is already known", srvModel)
				knownSrv = self._servers[srvModel]

				# Server responded, so clear failure count
				knownSrv.failures = 0

				# Check if server somehow changed
				different = False

				# Is address still the same?
				if knownSrv.address != srvAddr[0] or knownSrv.port != srvAddr[1]:
					logger.info ("Server has different address: %s -> %s", knownSrv.address, (srvAddr[0], srvAddr[1]))
					different = True
				else:
					# Address unchanged, check server ID
					if knownSrv.srvId != srvId:
						logger.info ("Server has different server ID")
						different = True

				if not different:
					logger.debug ("Server is unchanged")
				else:
					logger.info ("Server has changed, updating")

					# Remove old server, informing handlers
					del self._servers[srvModel]
					for h in self._handlers:
						h.onTransducersRemoved (knownSrv.transducers.values ())
					del knownSrv

					# And add new
					srvpx = self._queryServer (srvAddr)
					if srvpx is not None and srvpx.transducerList is not None:
						self._addServer (self._realizeServer (srvpx))

						# Inform handlers
						for h in self._handlers:
							h.onTransducersAdded (srvpx.transducers.values ())
			else:
				logger.debug ("Server %s is new", srvModel)
				srvpx = self._queryServer (srvAddr)
				if srvpx is not None and srvpx.transducerList is not None:
					self._addServer (self._realizeServer (srvpx))

					# Inform handlers
					for h in self._handlers:
						h.onTransducersAdded (srvpx.transducers.values ())
				else:
					logger.warning ("Server %s has no transducers, ignoring" % srvModel)

	def _notificationThread (self):
		handlers = {
			"ADV": self._notificationAdv,
			"NOT": self._notificationNot
		}

		logger = logging.getLogger ("client.notifications")
		logger.debug ('Waiting for notifications...')
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
					if Client.DEBUG:
						logger.debug ("[NOTIFICATION] %s:%s --> %s", client_address[0], client_address[1], data)

					parts = data.split (" ", 1)
					if len (parts) < 1:
						logger.error ("Malformed notification: '%s'", data)
					else:
						cmd = parts[0].upper ()

						if len (parts) > 1:
							args = parts[1]
						else:
							args = None

						try:
							handler = handlers[cmd]
							handler (client_address, args)
						except KeyError:
							logger.error ("Unknown command: '%s'", cmd)

		logger.debug ("NotificationThread ending")

	def registerHandler (self, h):
		if h not in self._handlers:
			self._handlers.append (h)

	def _addServer (self, srvpx):
		if srvpx.name in self._servers:
			self._logger.warning ("Duplicate server, ignoring: %s", srvpx.name)
		else:
			self._servers[srvpx.name] = srvpx

	def enableAutodiscovery (self, interval = DEFAULT_AUTODISCOVER_INTERVAL):
		if self._autodiscoverEnabled:
			self.disableAutodiscovery ()
		self._logger.debug ("Autodiscovery enabled every %d seconds", interval)
		self._autodiscInterval = interval
		self._autodiscoverTimer = threading.Timer (interval, self._autodiscoverTimerCallback)
		self._autodiscoverTimer.daemon = True
		self._autodiscoverEnabled = True
		self._autodiscoverTimer.start ()

	def disableAutodiscovery (self):
		self._logger.debug ("Autodiscovery disabled")
		self._autodiscoverEnabled = False
		if self._autodiscoverTimer is not None:
			self._autodiscoverTimer.cancel ()
			self._autodiscoverTimer = None

	def discover (self):
		if self._discoverLock.acquire (blocking = False):
			logger = logging.getLogger ("client.autodiscovery")
			logger.debug ("Sending Autodiscovery packet")

			# Tell handlers we're ready to roll
			for h in self._handlers:
				h.onAutodiscoveryStarted ()

			try:
				s = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
				s.setsockopt (socket.SOL_SOCKET,socket.SO_BROADCAST, 1)
				s.sendto ("WHO", ('<broadcast>', Client.DEFAULT_SERVER_PORT))
				s.settimeout (Client.DISCOVER_TIMEOUT)
			except Error as ex:
				logger.error ("Autodiscovery failed: %s", str (ex))

			self._discoverLock.release ()
		else:
			# Autodiscovery is already in progress, ignore
			# This might happen if we are called manually while the functions is
			# already being called by the autodiscovery thread
			pass

	def _autodiscoverTimerCallback (self):
		self.discover ()

		# Restart timer
		if self._autodiscoverEnabled:
			self.enableAutodiscovery (self._autodiscInterval)

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
						# ~ newact = ActuatorProxy (name, self.stereotypes[stereotype], srvpx)
						newact = ActuatorProxy (name, typ, stereotype, desc, self.stereotypes[stereotype], srvpx)
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

	def _parseQryReply (self, reply, addr):
		model = None
		protoVer = 0
		srvId = None
		transducerList = None
		reply = reply.strip ()
		parts = reply.split (" ", 4)
		if len (parts) < 4 or len (parts) > 5:
			self._logger.error ("Unexpected QRY reply: %s", reply)
		elif parts[0].upper () != "QRY":
			if parts[0].upper () == "ERR":
				self._logger.warning ("Node at %s does not support QRY: %s", addr[0], reply)
			else:
				self._logger.error ("Unexpected QRY reply: %s", reply)
		else:
			model = parts[1]
			protoVer = int (parts[2])
			srvId = parts[3]
			if len (parts) == 5:
				transducerList = parts[4]
			self._logger.info ("Found \"%s\" speaking protocol %d at %s:%d", model, protoVer, addr[0], addr[1])
		return model, protoVer, srvId, transducerList

	# Server2 xxxxxxxx 9999
	def _parseAdvertisement (self, reply, addr):
		ok = False
		reply = reply.strip ()
		parts = reply.split (" ", 2)
		l = len (parts)
		if l == 2:
			# Use address from packet and default port
			srvModel = parts[0]
			srvId = parts[1]
			srvAddr = addr[0], Client.DEFAULT_SERVER_PORT
			ok = True
		elif l == 3:
			# Use address/port from advertisement
			srvModel = parts[0]
			srvId = parts[1]

			addrParts = parts[2].split (":")
			if len (addrParts) == 2:
				# ip:port
				srvAddr = addrParts
				ok = True
			elif len (addrParts) == 1:
				# Either only IP or only port
				if '.' in addrParts[0]:
					# IP only
					srvAddr = addrParts[0], Client.DEFAULT_SERVER_PORT
					ok = True
				else:
					try:
						srvAddr = addr[0], int (addrParts[0])
						ok = True
					except ValueError:
						self._logger.error ("Cannot parse port in server advertisement: '%s'" % reply)
			else:
				self._logger.error ("Cannot parse address in server advertisement: '%s'" % reply)
		else:
			self._logger.error ("Cannot parse server advertisement: '%s'" % reply)
		return (srvModel, srvId, srvAddr) if ok else None

	def _parseHloReply (self, reply, addr):
		model = None
		protoVer = 0
		transducerList = None
		reply = reply.strip ()
		parts = reply.split (" ", 1)
		if len (parts) > 1:
			model = parts[0]
			subparts = parts[1].split (" ", 1)
			protoVer = int (subparts[0])
			transducerList = subparts[1]
			#~ print reply, addr
			self._logger.info ("Found \"%s\" speaking protocol %d at %s:%d", model, protoVer, addr[0], addr[1])
		else:
			# No transducers
			model = parts[0]
		return model, protoVer, transducerList

	def _queryServer (self, addr):
		srvpx = None
		self._logger.debug ("Querying %s:%d", addr[0], addr[1])
		s = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
		s.setsockopt (socket.SOL_SOCKET,socket.SO_BROADCAST, 1)
		s.sendto ("QRY", addr)
		s.settimeout (10)
		try:
			reply, addr = s.recvfrom (1024)
			model, protoVer, srvId, transducerList = self._parseQryReply (reply, addr)
			if model is not None and srvId is not None and transducerList is not None:
				srvpx = ServerProxy (model, protoVer, srvId, transducerList, self._sock, *addr)
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
