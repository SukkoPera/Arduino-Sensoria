#!/usr/bin/env python

import socket
import sys
import random
import datetime
import select
import os
import threading
import time
import copy
import binascii

import Sensoria
import Sensoria.stereotypes as stereotypes2		# FIXME!
from Sensoria.stereotypes.WeatherData import WeatherData
from Sensoria.stereotypes.RelayData import RelayData
from Sensoria.stereotypes.ControlledRelayData import ControlledRelayData
from Sensoria.stereotypes.MotionData import MotionData
from Sensoria.stereotypes.DateTimeData import DateTimeData
from Sensoria.stereotypes.TimeControlData import TimeControlData
from Sensoria.stereotypes.ValueSetData import ValueSetData
from Sensoria.stereotypes.InstantMessageData import InstantMessageData

LISTEN_PORT = 9999
NOTIFICATION_PORT = 9998
RECV_BUFSIZE = 16384
DEBUG = True

class NotificationRequest (object):
	DEFAULT_TTL = 3
	class Type:
		PERIODIC, ON_CHANGE = xrange (0, 2)

	def __init__ (self, dest, sock, sensor, typ):
		self.dest = dest
		self.sensor = sensor
		self.typ = typ
		self._sock = sock
		self.ttl = NotificationRequest.DEFAULT_TTL

	def _notify (self, reading):
		r = reading.marshal ()
		print "[NOT %s:%u TTL=%u] %s %s" % (self.dest[0], self.dest[1], self.ttl, self.sensor.name, r)
		self._sock.sendto ("NOT %u %s %s\r\n" % (self.ttl, self.sensor.name, r), self.dest)
		self.ttl -= 1
		
	def expired (self):
		return self.ttl <= 0

	def isDue (self):
		raise NotImplementedError

	def processReading (self):
		raise NotImplementedError

class PeriodicNotificationRequest (NotificationRequest):
	def __init__ (self, dest, sock, sensor, intv):
		super (PeriodicNotificationRequest, self).__init__ (dest, sock, sensor, NotificationRequest.Type.PERIODIC)
		self.lastNotified = None
		self.interval = intv

	def _isDue (self):
		return self.lastNotified is None or (datetime.datetime.now () - self.lastNotified).total_seconds () >= self.interval

	def process (self):
		if self._isDue ():
			reading = self.sensor.read ()
			self._notify (reading)
			self.lastNotified = datetime.datetime.now ()

class OnChangeNotificationRequest (NotificationRequest):
	def __init__ (self, dest, sock, sensor):
		super (OnChangeNotificationRequest, self).__init__ (dest, sock, sensor, NotificationRequest.Type.ON_CHANGE)
		self._lastReading = None

	def process (self):
		reading = self.sensor.read ()
		# Use this particular form so that the stereotype's __eq__ rich
		# comparison method gets called
		if not reading == self._lastReading:
			self._notify (reading)
			self._lastReading = reading

class Transducer (object):
	class Type:
		SENSOR, ACTUATOR = xrange (0, 2)

	def __init__ (self, typ, name, stereotype, description = "", version = ""):
		assert typ in range (Transducer.Type.SENSOR, Transducer.Type.ACTUATOR + 1)
		assert len (name) > 0
		assert len (stereotype) == 2
		assert not "|" in name and not "|" in description
		self.type = typ
		self.name = name
		self.stereotype = stereotype
		self.description = description
		self.version = version

	def get_type_string (self):
		if self.type == Sensor.Type.SENSOR:
			return "S"
		elif self.type == Sensor.Type.ACTUATOR:
			return "A"
		else:
			raise ValueError ("Unknown transducer type")

class Sensor (Transducer):
	def __init__ (self, name, stereotype, description = "", version = ""):
		super (Sensor, self).__init__ (Sensor.Type.SENSOR, name, stereotype, description, version)

	def read (self):
		raise NotImplementedError

	def configure (self):
		raise NotImplementedError

class Actuator (Transducer):
	def __init__ (self, name, stereotype, description = "", version = ""):
		super (Actuator, self).__init__ (Sensor.Type.ACTUATOR, name, stereotype, description, version)

	def write (self, value):
		raise NotImplementedError

	def read (self):
		raise NotImplementedError

class Clock (Sensor):
	def __init__ (self, name, description = "", version = ""):
		super (Clock, self).__init__ (name, DateTimeData.getIdString (), description, version)

	def read (self):
		dt = DateTimeData.fromNow ()
		return dt

class TemperatureSensor (Sensor):
	def __init__ (self, name, description = "", version = ""):
		super (TemperatureSensor, self).__init__ (name, "WD", description, version)
		self.temperature = random.randrange (-1000, 3500) / 100.0
		self.humidity = random.randrange (4000, 9000) / 100.0

	def read (self):
		wd = WeatherData ()
		wd.temperature = self.temperature
		wd.humidity = self.humidity
		
		self.temperature += random.randrange (-150, +150) / 100.0
		self.humidity += random.randrange (-500, +500) / 100.0
		
		return wd

class TimedActuator (Actuator):
	def __init__ (self, name, description = "", version = ""):
		super (TimedActuator, self).__init__ (name, TimeControlData.getIdString (), description, version)
		# Quick hack to initialize control data
		tmp = TimeControlData ()
		self.schedule = copy.deepcopy (tmp.schedule)

	def write (self, data):
		assert isinstance (data, TimeControlData)
		self.schedule = copy.deepcopy (data.schedule)
		return True, "Schedule updated"

	def read (self):
		data = TimeControlData ()
		data.schedule = copy.deepcopy (self.schedule)
		return data

class ValueSetActuator (Actuator):
	def __init__ (self, name, description = "", version = ""):
		super (ValueSetActuator, self).__init__ (name, ValueSetData.getIdString (), description, version)
		self.values = [None] * ValueSetData.NVALUES

	def write (self, data):
		assert isinstance (data, ValueSetData)
		self.values = copy.deepcopy (data.values)
		return True, "Values updated"

	def read (self):
		data = ValueSetData ()
		data.values = copy.deepcopy (self.values)
		return data

class RelayActuator (Actuator):
	class State:
		OFF = 0
		ON = 1

	def __init__ (self, name, description = "", version = ""):
		super (RelayActuator, self).__init__ (name, "RS", description, version)
		self.state = RelayActuator.State.OFF
##		random.choice (["ON", "OFF"])

	def read (self):
		rd = RelayData ()
		if self.state == RelayActuator.State.ON:
			rd.state = RelayData.ON
		else:
		    rd.state = RelayData.OFF
		return rd

	def write (self, rd):
		assert isinstance (rd, RelayData)
		if rd.state == RelayData.ON:
			self.state = RelayActuator.State.ON
		elif rd.state == RelayData.OFF:
			self.state = RelayActuator.State.OFF
		return True, "Relay is now %s" % self.state

class ControlledRelayActuator (Actuator):
	class State:
		OFF = 0
		ON = 1

	class Controller:
		AUTO = 0
		MANUAL = 1

	def __init__ (self, name, description = "", version = ""):
		super (ControlledRelayActuator, self).__init__ (name, "CR", description, version)
		self.state = ControlledRelayActuator.State.OFF
		self.controller = ControlledRelayActuator.Controller.AUTO

	def read (self):
		crd = ControlledRelayData ()

		if self.state == ControlledRelayActuator.State.ON:
			crd.state = ControlledRelayData.ON
		else:
		    crd.state = ControlledRelayData.OFF

		if self.controller == ControlledRelayActuator.Controller.AUTO:
			crd.controller = ControlledRelayData.AUTO
		else:
			crd.controller = ControlledRelayData.MANUAL

		return crd

	def write (self, data):
		assert isinstance (data, ControlledRelayData)
		
		# NEVER set fields to UNKNOWN!
		
		if data.state == ControlledRelayData.ON:
			self.state = ControlledRelayActuator.State.ON
		elif data.state == ControlledRelayData.OFF:
			self.state = ControlledRelayActuator.State.OFF

		if data.controller == ControlledRelayData.AUTO:
			self.controller = ControlledRelayActuator.Controller.AUTO
		elif data.controller == ControlledRelayData.MANUAL:
			self.controller = ControlledRelayActuator.Controller.MANUAL

		return True, "Relay is now %s/%s" % (self.state, self.controller)


class CommandListener (object):
	PROTOCOL_VERSION = 1
	DEFAULT_ADVERTISE_INTERVAL = 60		# Seconds

	def __init__ (self, name, port = LISTEN_PORT):
		self._load_stereotypes ()
		self.serverName = name
		self.sensors = {}
		self.notificationRequests = []
		self._nrLock = threading.RLock ()
		self._thread = None
		self.advertiseInterval = CommandListener.DEFAULT_ADVERTISE_INTERVAL

		self._sock = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
		self._sock.setsockopt (socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		server_address = ('', port)

		print >> sys.stderr, 'Starting up on %s port %u' % ("INADDR_ANY" if not server_address[0] else server_address[0], server_address[1])
		self._sock.bind (server_address)

	def _load_stereotypes (self):
		# Load stereotypes (Hmmm... A bit of a hack?)
		self.stereotypes = {}
		for clsname in stereotypes2.__all__:
			# ~ exec ("import stereotypes.%s" % clsname)
			cls = eval ("stereotypes2.%s.%s" % (clsname, clsname))
			id_ = cls.getIdString ()
			print "Registering stereotype: %s (%s)" % (clsname, id_)
			if id_ in self.stereotypes:
				print "Duplicate stereotype: %s" % id_
			else:
				self.stereotypes[cls.getIdString ()] = cls

	def register_sensor (self, sensor):
		if sensor.name in self.sensors:
			print >> sys.stderr, "Duplicate sensor name: %s" % sensor.name
		else:
			self.sensors[sensor.name] = sensor
			print "Registered sensor %s" % (sensor.name)

	def unregister_sensor (self, sensor):
		if not sensor.name in self.sensors:
			print >> sys.stderr, "Cannot unregister non-existing sensor: %s" % sensor.name
		else:
			del self.sensors[sensor.name]
			print "Unregistered sensor %s" % (sensor.name)

	def setAdvertiseInterval (self, sec):
		self.advertiseInterval = sec

	def _reply (self, addr, what):
		if DEBUG:
			print "%s:%s <-- %s" % (addr[0], addr[1], what)
		self._sock.sendto (what + "\r\n", addr)

	def _getServerId (self):
		idStr = "%s %s" % (CommandListener.PROTOCOL_VERSION, "|".join (("%s %s %s %s" % (sensor.name, sensor.get_type_string (), sensor.stereotype, sensor.description)).rstrip () for sensor in self.sensors.itervalues ()))
		return "%08x" % (binascii.crc32 (idStr) & 0xffffffff)

	def _advertise (self):
		try:
			advMsg = "ADV %s %s %u" % (self.serverName, self._getServerId (), LISTEN_PORT)
			s = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
			s.setsockopt (socket.SOL_SOCKET,socket.SO_BROADCAST, 1)
			s.sendto (advMsg, ('<broadcast>', NOTIFICATION_PORT))
			# ~ s.sendto (advMsg, ('127.0.0.1', NOTIFICATION_PORT))
		except Exception as ex:
			print >> sys.stderr, "Periodic advertisement failed: %s" % str (ex)

	def _qry (self, addr, args):
		transducerList = "|".join (("%s %s %s %s" % (sensor.name, sensor.get_type_string (), sensor.stereotype, sensor.description)).rstrip () for sensor in self.sensors.itervalues ())
		self._reply (addr, "QRY %s %d %s %s" % (self.serverName, CommandListener.PROTOCOL_VERSION, self._getServerId (), transducerList))

	def _ver (self, addr, args):
		self._reply (addr, "VER %s" % self.serverName)

	def _hlo (self, addr, args):
		self._reply (addr, "HLO %s %d %s" % (self.serverName, CommandListener.PROTOCOL_VERSION, "|".join (("%s %s %s %s" % (sensor.name, sensor.get_type_string (), sensor.stereotype, sensor.description)).rstrip () for sensor in self.sensors.itervalues ())))

	def _who (self, addr, args):
		self._advertise ()

	def _rea (self, addr, args):
		if args is not None and len (args) > 0:
			name = args.upper ()
			try:
				sensor = self.sensors[name]
				val = sensor.read ()
				if val is not None:
					self._reply (addr, "REA OK %s" % str (val.marshal ()))
				else:
					self._reply (addr, "REA ERR Read failed")
			except KeyError:
				self._reply (addr, "REA ERR No such sensor: %s" % name)
			except Exception as ex:
				reason = str (ex)
				if len (reason) > 0:
					self._reply (addr, "REA ERR Read failed: %s" % reason)
				else:
					self._reply (addr, "REA ERR Read failed")
		else:
			self._reply (addr, "REA ERR Missing transducer name")

	def _wri (self, addr, args):
		if args is not None and len (args) > 0:
			parts = args.split (" ", 1)
			name = parts[0].upper ()
			rawdata = parts[1]
			try:
				sensor = self.sensors[name]
				if sensor.type == Sensor.Type.ACTUATOR:
					stereoclass = self.stereotypes[sensor.stereotype]
					data = stereoclass ()
					if data.unmarshal (rawdata):
						ok, msg = sensor.write (data)		# FIXME: Discard msg
						if ok:
							# Write succeeded, try to read back new status
							try:
								val = sensor.read ()
								if val is not None:
									# Read ok, append result to reply
									self._reply (addr, "WRI OK %s" % str (val.marshal ()))
								else:
									# Write succeded but subsequent Read failed,
									# WTF??? Report success and leave it to the
									# client to sort out the situation.
									self._reply (addr, "WRI OK")
							except Exception as ex:
								# Write succeded but marshaling the new
								# status failed, report success anyway, the
								# client can always try a new Read.
								print >> sys.stderr, "Read after Write failed: %s" % str (ex)
								self._reply (addr, "WRI OK")
						else:
							# Write failed
							self._reply (addr, "WRI ERR Write failed")
					else:
						self._reply (addr, "WRI ERR Unmarshal failed")
				else:
					self._reply (addr, "WRI ERR Sensor is not writable")
			except KeyError:
				self._reply (addr, "WRI ERR No such sensor: %s" % name)
		else:
			self._reply (addr, "WRI ERR Missing or malformed args")

	def _nrq (self, addr, args):
		if args is not None and len (args) > 0:
			parts = args.split (" ")
			if len (parts) > 1:
				name = parts[0].upper ()
				typ = parts[1].upper ()
				try:
					sensor = self.sensors[name]
					# FIXME: Check type too
					self._nrLock.acquire ()
					reqs = filter (lambda n: n.dest == (addr[0], NOTIFICATION_PORT) and n.sensor == sensor, self.notificationRequests)
					self._nrLock.release ()
					if len (reqs) == 1:
						# Notification already present, renew TTL
						req = reqs[0]
						print "Renewing TTL for NRQ %s" % str (req)
						req.ttl = NotificationRequest.DEFAULT_TTL
						self._reply (addr, "NRQ OK")
					else:
						print "New NRQ"
						if typ == "CHA":
							#~ rest = parts[2:]
							print >> sys.stderr, "Notifying on change of %s" % sensor.name
							req = OnChangeNotificationRequest ((addr[0], NOTIFICATION_PORT), self._sock, sensor)
							self._nrLock.acquire ()
							self.notificationRequests.append (req)
							self._nrLock.release ()
							self._reply (addr, "NRQ OK")
						elif typ == "PRD":
							if len (parts) < 3:
								self._reply (addr, "NRQ ERR No interval specified")
							else:
								intv = int (parts[2])
								print >> sys.stderr, "Notifying values of %s every %d seconds" % (sensor.name, intv)
								req = PeriodicNotificationRequest ((addr[0], NOTIFICATION_PORT), self._sock, sensor, intv)
								self._nrLock.acquire ()
								self.notificationRequests.append (req)
								self._nrLock.release ()
								self._reply (addr, "NRQ OK")
						else:
							self._reply (addr, "NRQ ERR Missing or malformed args")
				except KeyError:
					self._reply (addr, "NRQ ERR No such sensor: %s" % name)
			else:
				self._reply (addr, "NRQ ERR Missing or malformed args")
		else:
			self._reply (addr, "NRQ ERR Missing or malformed args")

	def _ndl (self, addr, args):
		if args is not None and len (args) > 0:
			parts = args.split (" ")
			if len (parts) > 1:
				name = parts[0].upper ()
				typ = parts[1].upper ()
				self._nrLock.acquire ()
				# FIXME: Check type as well
				reqs = filter (lambda nrq: nrq.sensor.name == name, self.notificationRequests)
				if len (reqs) == 1:
					print "Found notification to delete: %s" % str (reqs[0])
					try:
						self.notificationRequests.remove (reqs[0])
						self._reply (addr, "NDL OK")
					except ValueError as ex:
						print "Cannot delete notification: %s" % str (ex)
						self._reply (addr, "NDL ERR")
				elif len (reqs) == 0:
					print "No such notification"
					self._reply (addr, "NDL ERR")
				else:
					print "WTF!?!?"
					self._reply (addr, "NDL ERR")
				self._nrLock.release ()
			else:
				self._reply (addr, "NDL ERR Missing or malformed args")
		else:
			self._reply (addr, "NDL ERR Missing or malformed args")
				
	def _ncl (self, addr, args):
		self._nrLock.acquire ()
		self.notificationRequests = []
		self._nrLock.release ()
		self._reply (addr, "NCL OK")

	def start (self):
		self._shallStop = False
		self._quitPipe = os.pipe ()
		self._thread = threading.Thread (target = self._serverThread, name = "Server Thread")
		self._thread.daemon = True
		self._thread.start ()

	def _serverThread (self):
		handlers = {
			"VER": self._ver,
			"WHO": self._who,
			"HLO": self._hlo,
			"REA": self._rea,
			"WRI": self._wri,
			"QRY": self._qry,
			"NRQ": self._nrq,
			"NDL": self._ndl,
			"NCL": self._ncl
		}

		print >> sys.stderr, 'Waiting for commands...'
		nTimeouts = 0
		while not self._shallStop:
			# ~ rlist = [self._sock, self._quitPipe[0]]
			rlist = [self._sock]
			r, w, x = select.select (rlist, [], [], 1)

			if self._quitPipe[0] in r:
				# Thread shall exit
				print "Server thread exiting!"
				os.read (self._quitPipe[0], 1)
			elif self._sock in r:
				try:
					line, client_address = self._sock.recvfrom (RECV_BUFSIZE)

					if line == "":
						break
					else:
						line = line.strip ()
						self.msg_ip = client_address[0]
						self.msg_port = int (client_address[1])

						for data in line.split ("\n"):
							data = data.strip ()
							if DEBUG:
								print "%s:%s --> %s" % (self.msg_ip, self.msg_port, data)

							parts = data.split (" ", 1)
							if len (parts) < 1:
								print >> sys.stderr, "Malformed command: '%s'" % data
								self._reply (client_address, "ERR Malformed command: '%s'" % data)
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
									print >> sys.stderr, "Unknown command: '%s'" % cmd
									self._reply (client_address, "ERR Unknown command: '%s'" % cmd)
				except Exception as ex:
					print >> sys.stderr, "Socket error: %s" % str (ex)
			else:
				# Periodically process notifications
				self._nrLock.acquire ()
				delenda = []
				for nrq in self.notificationRequests:
					nrq.process ()
					if nrq.expired ():
						delenda.append (nrq)
				for nrq in delenda:
					print "Deleting expired notification request: %s" % str (nrq)
					self.notificationRequests.remove (nrq)
				self._nrLock.release ()

				# Also send periodic server advertisement
				nTimeouts += 1
				if self.advertiseInterval != 0 and nTimeouts >= self.advertiseInterval:
					print "Sending periodic advertisement"
					self._advertise ()
					nTimeouts = 0
		print "X"

	def stop (self):
		if self._thread is not None:
			self._shallStop = True
			os.write (self._quitPipe[1], "X")
			self._thread.join ()
			self._thread = None
