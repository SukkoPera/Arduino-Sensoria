#!/usr/bin/env python

import socket
import sys
import random
import datetime
import select

import Sensoria
from Sensoria.stereotypes.WeatherData import WeatherData

#~ from stereotypes.WeatherData import WeatherData

LISTEN_PORT = 9999
RECV_BUFSIZE = 16384
DEBUG = True

class NotificationRequest (object):
	class Type:
		PERIODIC, ON_CHANGE = xrange (0, 2)

	def __init__ (self, dest, sock, sensor, typ):
		self.dest = dest
		self.sensor = sensor
		self.typ = typ
		self._sock = sock

	def _notify (self, reading):
		self._sock.sendto ("NOT %s %s\r\n" % (self.sensor.name, reading.marshal ()), self.dest)

	def isDue (self):
		raise NotImplementedError

	def processReading (self, reading):
		raise NotImplementedError

class PeriodicNotificationRequest (NotificationRequest):
	def __init__ (self, dest, sock, sensor, intv):
		super (PeriodicNotificationRequest, self).__init__ (dest, sock, sensor, NotificationRequest.Type.PERIODIC)
		self.lastNotified = None
		self.interval = intv

	def isDue (self):
		return self.lastNotified is None or (datetime.datetime.now () - self.lastNotified).total_seconds () >= self.interval

	def processReading (self, reading):
		self._notify (reading)
		self.lastNotified = datetime.datetime.now ()

class OnChangeNotificationRequest (NotificationRequest):
	def __init__ (self, dest, sock, sensor):
		super (OnChangeNotificationRequest, self).__init__ (dest, sock, sensor, NotificationRequest.Type.ON_CHANGE)
		self._lastReading = None

	def isDue (self):
		# Always return true, since we will have to compare the new reading
		# against the previous one
		return True

	def processReading (self, reading):
		if reading != self._lastReading:
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

class TemperatureSensor (Sensor):
	def __init__ (self, name, description = "", version = ""):
		super (TemperatureSensor, self).__init__ (name, "WD", description, version)

	def read (self):
		wd = WeatherData ()
		wd.temperature = random.randrange (-1000, 3500) / 100.0
		wd.humidity = random.randrange (-1000, 3500) / 100.0
		#~ ts = datetime.datetime.now ().strftime ("%Y-%m-%dT%H:%M:%S")    # ISO 8601
		return wd

class KitchenTemperatureSensor (TemperatureSensor):
	def __init__ (self):
		super (KitchenTemperatureSensor, self).__init__ ("TK", "Kitchen Temperature", "20150611 By SukkoPera <software@sukkology.net>")

class BathroomTemperatureSensor (TemperatureSensor):
	def __init__ (self):
		super (BathroomTemperatureSensor, self).__init__ ("TB", "Bathroom Temperature")


class RelayActuator (Actuator):
	class State:
		OFF = 0
		ON = 1

	def __init__ (self, name, description = "", version = ""):
		super (RelayActuator, self).__init__ (name, "RS", description, version)
		self.state = RelayActuator.State.OFF
##		random.choice (["ON", "OFF"])

	def read (self):
		if self.state == RelayActuator.State.ON:
			val = "ON"
		else:
		    val = "OFF"
		return val

	#~ def write (self, value):
		#~ if value.upper () == "ON" or int (value) > 0:
			#~ self.state = RelayActuator.State.ON
		#~ else:
			#~ self.state = RelayActuator.State.OFF
		#~ return True, "Relay is now %s" % self.read ()[0]

class RelayHeater (RelayActuator):
	def __init__ (self):
		super (RelayHeater, self).__init__ ("BH", "Bathroom Heater", "20160228 By SukkoPera <software@sukkology.net>")

class CommandListener (object):
	def __init__ (self, port = LISTEN_PORT):
		self.sensors = {}
		self.notificationRequests = []

		self._sock = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
		#server_address = ('localhost', port)
		server_address = ('0', port)

		print >> sys.stderr, 'Starting up on %s port %s' % server_address
		self._sock.bind (server_address)

	def register_sensor (self, sensor):
		if sensor.name in self.sensors:
			print >> sys.stderr, "Duplicate sensor name: %s" % sensor.name
		else:
			self.sensors[sensor.name] = sensor
			print "Registered sensor %s" % (sensor.name)

	def _reply (self, addr, what):
		if DEBUG:
			print "%s:%s <-- %s" % (addr[0], addr[1], what)
		self._sock.sendto (what + "\r\n", addr)

	def _qry (self, addr, args):
		if args is not None and len (args) > 0:
			name = args.upper ()
			try:
				sensor = self.sensors[name]
				self._reply (addr, "QRY %s|%s|%s|%s|%s" % (sensor.name, sensor.get_type_string (), sensor.stereotype, sensor.description, sensor.version))
			except KeyError:
				self._reply (addr, "ERR No such transducer: %s" % name)
		else:
			# List all sensors
			self._reply (addr, "QRY %s" % "|".join ("%s %s %s %s" % (sensor.name, sensor.get_type_string (), sensor.stereotype, sensor.description) for sensor in self.sensors.itervalues ()))

	def _ver (self, addr, args):
		self._reply (addr, "VER TestSensors 20160727")

	def _rea (self, addr, args):
		if args is not None and len (args) > 0:
			name = args.upper ()
			try:
				sensor = self.sensors[name]
				val = sensor.read ()
				#~ if ts is not None:
					#~ self._reply (addr, "REA %s %s %s" % (name, str (val), ts))
				#~ else:
				self._reply (addr, "REA %s %s" % (name, str (val.marshal ())))
				#~ else:
					#~ self._reply (addr, "ERR Sensor is not readable")
			except KeyError:
				self._reply (addr, "ERR No such sensor: %s" % name)
		else:
			self._reply (addr, "ERR Missing sensor number")

	def _wri (self, addr, args):
		if args is not None and len (args) > 0:
			parts = args.split (" ")
			name = parts[0].upper ()
			val = parts[1]
			try:
				sensor = self.sensors[name]
				if sensor.type == Sensor.Type.ACTUATOR:
					ok, msg = sensor.write (val)
					if ok:
						st = "OK"
					else:
						st = "ERR"
					if msg is not None:
						self._reply (addr, "WRI %s %s" % (st, msg))
					else:
						self._reply (addr, "WRI %s" % st)
				else:
					self._reply (addr, "ERR Sensor is not writable")
			except KeyError:
				self._reply (addr, "ERR No such sensor: %s" % name)
		else:
			self._reply (addr, "ERR Missing or malformed args")

	def _nrq (self, addr, args):
		if args is not None and len (args) > 0:
			parts = args.split (" ")
			if len (parts) > 1:
				name = parts[0].upper ()
				typ = parts[1].upper ()
				try:
					sensor = self.sensors[name]
					if typ == "CHA":
						#~ rest = parts[2:]
						print >> sys.stderr, "Notifying on change of %s" % sensor.name
						req = OnChangeNotificationRequest (addr, self._sock, sensor)
						self.notificationRequests.append (req)
						self._reply (addr, "NRQ %s CHA OK" % sensor.name)
					elif typ == "PRD":
						if len (parts) < 3:
							self._reply (addr, "NRQ %s ERR No interval specified" % sensor.name)
						else:
							intv = int (parts[2])
							print >> sys.stderr, "Notifying values of %s every %d seconds" % (sensor.name, intv)
							req = PeriodicNotificationRequest (addr, self._sock, sensor, intv)
							self.notificationRequests.append (req)
							self._reply (addr, "NRQ %s OK" % sensor.name)
				except KeyError:
					self._reply (addr, "ERR No such sensor: %s" % name)
			else:
				self._reply (addr, "ERR Missing or malformed args")
		else:
			self._reply (addr, "ERR Missing or malformed args")

	def _processNotificationRequests (self):
		for nrq in self.notificationRequests:
			if nrq.isDue ():
				#~ print "NotificationRequest is due: %s" % nrq
				r = nrq.sensor.read ()
				nrq.processReading (r)

	def go (self):
		handlers = {}
		handlers["VER"] = self._ver
		handlers["REA"] = self._rea
		handlers["WRI"] = self._wri
		handlers["QRY"] = self._qry
		handlers["NRQ"] = self._nrq

		print >> sys.stderr, 'Waiting for commands...'
		while True:
			rlist = [self._sock]
			r, w, x = select.select (rlist, [], [], 5)

			if self._sock in r:
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
			else:
				#~ print "socket timeout"
				self._processNotificationRequests ()

if __name__ == "__main__":
	tk = KitchenTemperatureSensor ()
	tb = BathroomTemperatureSensor ()
	rh = RelayHeater ()
	listener = CommandListener ()
	listener.register_sensor (tk)
	listener.register_sensor (tb)
	listener.register_sensor (rh)
	listener.go ()
