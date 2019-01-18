#!/usr/bin/env python

import time
import server
import argparse

DEFAULT_NAME = "Server2"

class OutdoorSensor (server.TemperatureSensor):
	def __init__ (self):
		super (OutdoorSensor, self).__init__ ("OS", "Outdoor Sensor", "20171127")

class OutdoorLight (server.ControlledRelayActuator):
	def __init__ (self):
		super (OutdoorLight, self).__init__ ("KF", "Outdoor Light", "20171127")

parser = argparse.ArgumentParser ( description = 'Simulate some Sensoria transducers')
parser.add_argument ('-n', '--name', help = 'Server name')
parser.add_argument ('-p', '--port', type = int, default = None, help = 'UDP port to listen on')
parser.add_argument ('-a', '--advertise', type = int, default = None, help = 'Interval between server advertisement messages', metavar = "SECONDS")

args = parser.parse_args ()

os = OutdoorSensor ()
ol = OutdoorLight ()
if args.port:
	listener = server.CommandListener (args.name, args.port)
else:
	listener = server.CommandListener (args.name)
listener.register_sensor (os)
listener.register_sensor (ol)
if args.advertise:
	listener.setAdvertiseInterval (args.advertise)
listener.start ()

while True:
	time.sleep (1)
