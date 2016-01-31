#!/usr/bin/env python

import datetime
import time

from db import DB
from Sensoria import *

#~ sensoria = Sensoria (autodiscover = True)
sensoria = Sensoria (servers = ["192.168.1.162", "192.168.1.164"])
#~ sensoria = Sensoria (servers = ["192.168.1.164"])

db = DB ()

while True:
	try:
		data = {}
		now = datetime.datetime.now ()
		for name, sensor in sensoria.sensors.iteritems ():
			data[name] = sensor.read (raw = True)
			print "%s -> %s" % (name, data[name])
		db.insert (now, data, commit = True)
		print "."
	except Exception as ex:
		print "ERROR: %s" % str (ex)
		raise

	time.sleep (5 * 60)
