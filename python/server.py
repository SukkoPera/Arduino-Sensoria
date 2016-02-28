#!/usr/bin/env python

import socket
import sys
import random
import datetime

from stereotypes.WeatherData import WeatherData

LISTEN_PORT = 9999
RECV_BUFSIZE = 16384
<<<<<<< HEAD
DEBUG = True
=======
>>>>>>> 5ee9d9a6c3c59933e0f8fed8ca79517959e425b9

class Transducer (object):
	class Type:
		SENSOR, ACTUATOR = xrange (0, 2)

	def __init__ (self, typ, name, stereotype, description = "", version = ""):
<<<<<<< HEAD
		assert typ in range (Transducer.Type.SENSOR, Transducer.Type.ACTUATOR + 1)
=======
		assert typ in range (Transducer.Type.SENSOR, Transducer.Type.ACTUATOR)
>>>>>>> 5ee9d9a6c3c59933e0f8fed8ca79517959e425b9
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

<<<<<<< HEAD
class Actuator (Transducer):
=======
class Actuator (Sensor):
>>>>>>> 5ee9d9a6c3c59933e0f8fed8ca79517959e425b9
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
		return wd.marshal ()

class KitchenTemperatureSensor (TemperatureSensor):
	def __init__ (self):
		super (KitchenTemperatureSensor, self).__init__ ("TK", "Kitchen Temperature", "20150611 By SukkoPera <software@sukkology.net>")

class BathroomTemperatureSensor (TemperatureSensor):
	def __init__ (self):
		super (BathroomTemperatureSensor, self).__init__ ("TB", "Bathroom Temperature")


<<<<<<< HEAD
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
=======
#~ class RelaySensor (ReadableWritableSensor):
	#~ class State:
		#~ OFF = 0
		#~ ON = 1

	#~ def __init__ (self, name, description = "", version = ""):
		#~ super (RelaySensor, self).__init__ (name, description, version)
		#~ self.state = RelaySensor.State.OFF
#~ ##		random.choice (["ON", "OFF"])

	#~ def read (self):
		#~ if self.state == RelaySensor.State.ON:
			#~ val = "ON"
		#~ else:
		    #~ val = "OFF"
		#~ return val, None

	#~ def write (self, value):
		#~ if value.upper () == "ON" or int (value) > 0:
			#~ self.state = RelaySensor.State.ON
		#~ else:
			#~ self.state = RelaySensor.State.OFF
		#~ return True, "Relay is now %s" % self.read ()[0]

#~ class RelayHeater (RelaySensor):
	#~ def __init__ (self):
		#~ super (RelayHeater, self).__init__ ("BH", "Bathroom Heater", "20150611 By SukkoPera <software@sukkology.net>")
>>>>>>> 5ee9d9a6c3c59933e0f8fed8ca79517959e425b9

class CommandListener (object):
	def __init__ (self, port = LISTEN_PORT):
		self.sensors = {}

		self._sock = socket.socket (socket.AF_INET, socket.SOCK_DGRAM)
<<<<<<< HEAD
		#server_address = ('localhost', port)
		server_address = ('0', port)
=======
		server_address = ('localhost', port)
>>>>>>> 5ee9d9a6c3c59933e0f8fed8ca79517959e425b9

		print >> sys.stderr, 'Starting up on %s port %s' % server_address
		self._sock.bind (server_address)

	def register_sensor (self, sensor):
		if sensor.name in self.sensors:
			print >> sys.stderr, "Duplicate sensor name: %s" % sensor.name
		else:
			self.sensors[sensor.name] = sensor
			print "Registered sensor %s" % (sensor.name)

	def _reply (self, addr, what):
<<<<<<< HEAD
		if DEBUG:
			print "%s:%s <-- %s" % (addr[0], addr[1], what)
=======
>>>>>>> 5ee9d9a6c3c59933e0f8fed8ca79517959e425b9
		self._sock.sendto (what, addr)

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
		self._reply (addr, "VER TestSensors 20160122")

	def _rea (self, addr, args):
		if args is not None and len (args) > 0:
			name = args.upper ()
			try:
				sensor = self.sensors[name]
				val = sensor.read ()
				#~ if ts is not None:
					#~ self._reply (addr, "REA %s %s %s" % (name, str (val), ts))
				#~ else:
				self._reply (addr, "REA %s %s" % (name, str (val)))
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
<<<<<<< HEAD
				if DEBUG:
					print "%s:%s --> %s" % (self.msg_ip, self.msg_port, data)
=======
				print "From '%s:%s: '%s'" % (self.msg_ip, self.msg_port, data)
>>>>>>> 5ee9d9a6c3c59933e0f8fed8ca79517959e425b9

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
<<<<<<< HEAD
	rh = RelayHeater ()
	listener = CommandListener ()
	listener.register_sensor (tk)
	listener.register_sensor (tb)
	listener.register_sensor (rh)
=======
	#~ rh = RelayHeater ()
	listener = CommandListener ()
	listener.register_sensor (tk)
	listener.register_sensor (tb)
	#~ listener.register_sensor (rh)
>>>>>>> 5ee9d9a6c3c59933e0f8fed8ca79517959e425b9
	listener.go ()
