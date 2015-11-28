#!/usr/bin/env python

import datetime
import time

from Sensoria import *


sensoria = Sensoria ()
sensoria.discoverServers ()
sensoria.discoverSensors ()

try:
	intThermo = sensoria.getSensor ("IT")
	print "Found internal thermometer on %s at %s:%s" % (intThermo.server.name, intThermo.server.address, intThermo.server.port)
except SensoriaError as ex:
	print "Cannot get internal thermometer: %s" % str (ex)
	intThermo = None

try:
	extThermo = sensoria.getSensor ("OT")
	print "Found external thermometer on %s at %s:%s" % (extThermo.server.name, extThermo.server.address, extThermo.server.port)
except SensoriaError as ex:
	print "Cannot get external thermometer: %s" % str (ex)
	extThermo = None

try:
	hygrometer = sensoria.getSensor ("OD")
	print "Found hygrometer on %s at %s:%s" % (hygrometer.server.name, hygrometer.server.address, hygrometer.server.port)
except SensoriaError as ex:
	print "Cannot get hygrometer: %s" % str (ex)
	hygrometer = None

try:
	barometer = sensoria.getSensor ("PS")
	print "Found barometer on %s at %s:%s" % (barometer.server.name, barometer.server.address, barometer.server.port)
except SensoriaError as ex:
	print "Cannot get barometer: %s" % str (ex)

try:
	display = sensoria.getSensor ("LD")
	print "Found display on %s at %s:%s" % (display.server.name, display.server.address, display.server.port)
except SensoriaError as ex:
	print "Cannot get display: %s" % str (ex)



# Go!
last = None
while True:
	outTemp = 0
	inTemp = 0
	humid = 0
	px = 0

	now = datetime.datetime.now ()
	if last is None or now - last > datetime.timedelta (seconds = 5):
		try:
			if extThermo is not None:
				outTemp = float (extThermo.read ())
		except SensorError as ex:
			print "Cannot read outside thermometer: %s" % str (ex)

		try:
			if intThermo is not None:
				inTemp = float (intThermo.read ())
		except SensorError as ex:
			print "Cannot read inside thermometer: %s" % str (ex)

		try:
			if hygrometer is not None:
				humid = float (hygrometer.read ().split ()[1])
		except SensorError as ex:
			print "Cannot read hygrometer: %s" % str (ex)

		try:
			if barometer is not None:
				px = float (barometer.read ().split ()[1])
		except SensorError as ex:
			print "Cannot read barometer: %s" % str (ex)

	try:
		display.write ("CLR")
		display.write ("prp 0 0 OUT:%.1f IN:%.1f" % (outTemp, inTemp))
		display.write ("prp 1 0 HUM:%.2f%%" % humid)
		display.write ("prp 2 0 PX:%.2f mbar" % px)
		now = datetime.datetime.now ()
		display.write ("prp 3 0 %s" % now.strftime ("%d/%m %H:%M"))
	except SensorError as ex:
			print "Cannot update display: %s" % str (ex)
	time.sleep (10)
