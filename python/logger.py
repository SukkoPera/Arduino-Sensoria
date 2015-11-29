#!/usr/bin/env python

import datetime
import time

from db import DB
from Sensoria import *

sensoria = Sensoria (autodiscover = True)

db = DB ()
dallas = sensoria.getSensor ("OT")
dht = sensoria.getSensor ("OD")
bmp = sensoria.getSensor ("PS")
ldr = sensoria.getSensor ("PR")
bh = sensoria.getSensor ("OL")

while True:
	try:
		tdht, hum = dht.read ().split ()
		tdallas = dallas.read ()
		lux = bh.read ()
		tbmp, local_px = bmp.read ().split ()
		scale, v = ldr.read ().split ()

		#~ print tdht, tdallas, tbmp, hum, local_px, 0, lux, scale
		db.insert (datetime.datetime.now (), tdht, tdallas, tbmp, hum, local_px, 0, lux, scale, commit = True)
		print "."
	except Exception as ex:
		print "ERROR: %s" % str (ex)
		#~ raise

	time.sleep (5 * 60)
