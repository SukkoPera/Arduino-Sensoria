#!/usr/bin/env python

import time
import server

class OutdoorSensor (server.TemperatureSensor):
	def __init__ (self):
		super (OutdoorSensor, self).__init__ ("OS", "Outdoor Sensor", "20171127")

class OutdoorLight (server.ControlledRelayActuator):
	def __init__ (self):
		super (OutdoorLight, self).__init__ ("KF", "Outdoor Light", "20171127")

os = OutdoorSensor ()
ol = OutdoorLight ()
listener = server.CommandListener ("Server2")
listener.register_sensor (os)
listener.register_sensor (ol)
listener.start ()

while True:
	time.sleep (1)
