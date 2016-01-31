#!/usr/bin/env python

import datetime
import time

from db import DB
from Sensoria import *

sensoria = Sensoria (autodiscover = True)
#sensoria = Sensoria (servers = ["192.168.1.155", "192.168.1.154"])
#sensoria = Sensoria (servers = ["192.168.1.155"])

db = DB ()
dallas = sensoria.getSensor ("OT")
dht = sensoria.getSensor ("OH")
bmp = sensoria.getSensor ("OP")
ldr = sensoria.getSensor ("PR")
bh = sensoria.getSensor ("OL")
#dallas2 = sensoria.getSensor ("T2")
#bmp2 = sensoria.getSensor ("P2")
#irlight = sensoria.getSensor ("OR")
#ldr2 = sensoria.getSensor ("L2")

while True:
	try:
		rdht = dht.read ()
		rdallas = dallas.read ()
		rlux = bh.read ()
		rbmp = bmp.read ()
		rldr = ldr.read ()

		#tdallas2 = dallas2.read ()
		#tbmp2, local_px2 = bmp2.read ().split ()
		#lux_ir = irlight.read ().split ()[0].split (":")[1]
		#ldrval2, v2 = ldr2.read ().split ()

		#~ print tdallas2, tbmp2, local_px2, lux_ir
		#~ print rdht.temperature, rdht.humidity, rdallas.temperature, rlux.lightLux, rbmp.temperature, rbmp.localPressure, rldr.light10bit
		#~ print tdht, tdallas, tbmp, hum, local_px, 0, lux, scale

		#db.insert (datetime.datetime.now (), tdht, tdallas, tbmp, hum, local_px, 0, lux, scale,    local_px2, lux_ir, tdallas2, ldrval2, commit = True)
		#~ db.insert (datetime.datetime.now (), tdht, tdallas, tbmp, hum, local_px, 0, lux, scale, commit = True)
		db.insert (datetime.datetime.now (), rdht.temperature, rdallas.temperature, rbmp.temperature, rdht.humidity, rbmp.localPressure, 0, rlux.lightLux, rldr.light10bit, commit = True)
		print "."
	except Exception as ex:
		print "ERROR: %s" % str (ex)
		#raise

	time.sleep (5 * 60)
