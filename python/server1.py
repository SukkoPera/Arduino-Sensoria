#!/usr/bin/env python

import server
import time

class KitchenTemperatureSensor (server.TemperatureSensor):
	def __init__ (self):
		super (KitchenTemperatureSensor, self).__init__ ("TK", "Kitchen Temperature", "20150611 By SukkoPera <software@sukkology.net>")

class BathroomTemperatureSensor (server.TemperatureSensor):
	def __init__ (self):
		super (BathroomTemperatureSensor, self).__init__ ("TB", "Bathroom Temperature")

class RelayHeater (server.RelayActuator):
	def __init__ (self):
		super (RelayHeater, self).__init__ ("BH", "Bathroom Heater", "20160228 By SukkoPera <software@sukkology.net>")

class RelayFan (server.ControlledRelayActuator):
	def __init__ (self):
		super (RelayFan, self).__init__ ("KF", "Kitchen Fan", "20170126 By SukkoPera <software@sukkology.net>")

tk = KitchenTemperatureSensor ()
tb = BathroomTemperatureSensor ()
rh = RelayHeater ()
kf = RelayFan ()
listener = server.CommandListener ("Server1")
listener.register_sensor (tk)
listener.register_sensor (tb)
listener.register_sensor (rh)
listener.register_sensor (kf)

listener.start ()
time.sleep (7)
listener.unregister_sensor (tk)
del tk
#~ listener.stop ()
while True:
	time.sleep (1)
