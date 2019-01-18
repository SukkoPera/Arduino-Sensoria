#!/usr/bin/env python

import server
import time

from Sensoria.stereotypes.TimeControlData import TimeControlData

class KitchenTemperatureSensor (server.TemperatureSensor):
	def __init__ (self):
		super (KitchenTemperatureSensor, self).__init__ ("TK")

class BathroomTemperatureSensor (server.TemperatureSensor):
	def __init__ (self):
		super (BathroomTemperatureSensor, self).__init__ ("TB", "Bathroom Temperature")

class HeaterController (server.ControlledRelayActuator):
	def __init__ (self):
		super (HeaterController, self).__init__ ("HC", "Bathroom Heater (Controller)", "20171213 By SukkoPera <software@sukkology.net>")

#TL1:10 TL2:18 TL3:21
class HeaterTimer (server.TimedActuator):
	def __init__ (self):
		super (HeaterTimer, self).__init__ ("HT", "Bathroom Heater (Timer)", "20171213 By SukkoPera <software@sukkology.net>")
		data = TimeControlData ()
		ok = data.unmarshal ("PMO:000000001000000003222110 PTU:000000001000000003222110 PWE:000000001000000003222110 PTH:000000001000000003222110 PFR:000000001000000003222111 PSA:000000000322222222222211 PSU:000000000322222222222210")
		assert ok
		ok, msg = self.write (data)
		print msg
		assert ok

class HeaterSettings (server.ValueSetActuator):
	def __init__ (self):
		super (HeaterSettings, self).__init__ ("HS", "Bathroom Heater (Settings)", "20180112 By SukkoPera <software@sukkology.net>")
		self.levels = [10, 18, 21]

	@property
	def values (self):
		return self.levels

	@values.setter
	def values (self, v):
		self.levels = v[0:3]

class RelayFan (server.RelayActuator):
	def __init__ (self):
		super (RelayFan, self).__init__ ("KF", "Kitchen Fan", "20170126 By SukkoPera <software@sukkology.net>")

tk = KitchenTemperatureSensor ()
tb = BathroomTemperatureSensor ()
hc = HeaterController ()
ht = HeaterTimer ()
hs = HeaterSettings ()
kf = RelayFan ()
ck = server.Clock ("$T", "Clock")
listener = server.CommandListener ("Server1")
listener.register_sensor (tk)
listener.register_sensor (tb)
listener.register_sensor (hc)
listener.register_sensor (ht)
listener.register_sensor (hs)
listener.register_sensor (kf)
listener.register_sensor (ck)

listener.start ()
time.sleep (7)
listener.unregister_sensor (tk)

time.sleep (40)
listener.register_sensor (tk)

#~ del tk
#~ listener.stop ()
while True:
	time.sleep (1)
