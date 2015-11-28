#!/usr/bin/env python

import socket
import sys
import random
import datetime

LISTEN_PORT = 9999
REPLY_PORT = 8888
RECV_BUFSIZE = 16384

class Sensor (object):
	class Type:
		READ, WRITE, READ_WRITE = xrange (0, 3)

	def __init__ (self, name, typ, description = "", version = ""):
		assert not "|" in name and not "|" in description
		self.name = name
		self.description = description
		self.type = typ
		self.version = version

	def get_type_string (self):
		if self.type == Sensor.Type.READ:
			return "R"
		elif self.type == Sensor.Type.WRITE:
			return "W"
		elif self.type == Sensor.Type.READ_WRITE:
			return "RW"
		else:
			raise ValueError ("Unknown sensor type")

class ReadableSensor (Sensor):
	def __init__ (self, name, description = "", version = ""):
		super (ReadableSensor, self).__init__ (name, Sensor.Type.READ, description, version)

	def read (self):
		raise NotImplementedError

class WritableSensor (Sensor):
	def __init__ (self, name, description = "", version = ""):
		super (WritableSensor, self).__init__ (name, Sensor.Type.WRITE, description, version)

	def write (self, value):
		raise NotImplementedError

class ReadableWritableSensor (Sensor):
	def __init__ (self, name, description = "", version = ""):
		super (ReadableWritableSensor, self).__init__ (name, Sensor.Type.READ_WRITE, description, version)

	def read (self):
		raise NotImplementedError

	def write (self, value):
		raise NotImplementedError


class TemperatureSensor (ReadableSensor):
	def read (self):
		val = random.randrange (-10, 35)
		ts = datetime.datetime.now ().strftime ("%Y-%m-%dT%H:%M:%S")    # ISO 8601
		return val, ts

class KitchenTemperatureSensor (TemperatureSensor):
	def __init__ (self):
		super (KitchenTemperatureSensor, self).__init__ ("TK", "Kitchen Temperature", "20150611 By SukkoPera <software@sukkology.net>")

class BathroomTemperatureSensor (TemperatureSensor):
	def __init__ (self):
		super (BathroomTemperatureSensor, self).__init__ ("TB", "Bathroom Temperature")


class RelaySensor (ReadableWritableSensor):
	class State:
		OFF = 0
		ON = 1

	def __init__ (self, name, description = "", version = ""):
		super (RelaySensor, self).__init__ (name, description, version)
		self.state = RelaySensor.State.OFF
##		random.choice (["ON", "OFF"])

	def read (self):
		if self.state == RelaySensor.State.ON:
			val = "ON"
		else:
		    val = "OFF"
		return val, None

	def write (self, value):
		if value.upper () == "ON" or int (value) > 0:
			self.state = RelaySensor.State.ON
		else:
			self.state = RelaySensor.State.OFF
		return True, "Relay is now %s" % self.read ()[0]

class RelayHeater (RelaySensor):
	def __init__ (self):
		super (RelayHeater, self).__init__ ("BH", "Bathroom Heater", "20150611 By SukkoPera <software@sukkology.net>")

class CommandListener (object):
	def __init__ (self, port = LISTEN_PORT):
		self.sensors = {}

		self._sock = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
		server_address = ('localhost', port)

		print >> sys.stderr, 'starting up on %s port %s' % server_address
		self._sock.bind (server_address)

	def register_sensor (self, sensor):
		if sensor.name in self.sensors:
			print >> sys.stderr, "Duplicate sensor name: %s" % sensor.name
		else:
			self.sensors[sensor.name] = sensor
			print "Registered sensor %s" % (sensor.name)

	def _reply (self, addr, what):
		self._sock.sendto (what, addr)

	def _qry (self, addr, args):
		if args is not None and len (args) > 0:
			name = args.upper ()
			try:
				sensor = self.sensors[name]
				self._reply (addr, "QRY %s|%s|%s|%s" % (sensor.name, sensor.get_type_string (), sensor.description, sensor.version))
			except KeyError:
				self._reply (addr, "ERR No such sensor: %s" % name)
		else:
			# List all sensors
			self._reply (addr, "|".join ("%s %s %s" % (sensor.name, sensor.get_type_string (), sensor.description) for sensor in self.sensors.itervalues ()))

	def _ver (self, addr, args):
		self._reply (addr, "CLIENT_TEST 0.1 11/06/2015")

	def _rea (self, addr, args):
		if args is not None and len (args) > 0:
			name = args.upper ()
			try:
				sensor = self.sensors[name]
				if sensor.type == Sensor.Type.READ or sensor.type == Sensor.Type.READ_WRITE:
					val, ts = sensor.read ()
					if ts is not None:
						self._reply (addr, "REA %s %s %s" % (name, str (val), ts))
					else:
						self._reply (addr, "REA %s %s" % (name, str (val)))
				else:
					self._reply (addr, "ERR Sensor is not readable")
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
				if sensor.type == Sensor.Type.WRITE or sensor.type == Sensor.Type.READ_WRITE:
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

	def go (self):
		handlers = {}
		handlers["VER"] = self._ver
		handlers["REA"] = self._rea
		handlers["WRI"] = self._wri
		handlers["QRY"] = self._qry

		print >> sys.stderr, 'Waiting for commands...'
		while True:
			data, client_address = self._sock.recvfrom (RECV_BUFSIZE)

			if data == "":
				break
			else:
				data = data.strip ()
				self.msg_ip = client_address[0]
				self.msg_port = int (client_address[1])
				print "From '%s:%s: '%s'" % (self.msg_ip, self.msg_port, data)

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

if __name__ == "__main__":
	tk = KitchenTemperatureSensor ()
	tb = BathroomTemperatureSensor ()
	rh = RelayHeater ()
	listener = CommandListener ()
	listener.register_sensor (tk)
	listener.register_sensor (tb)
	listener.register_sensor (rh)
	listener.go ()
