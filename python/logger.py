#!/usr/bin/env python

import sys
import datetime
import time

from db import DB
from Sensoria import *

sensoria = Sensoria (autodiscover = True)
#sensoria = Sensoria (servers = ["192.168.1.171"], autodiscover = True)
#~ sensoria = Sensoria (servers = ["192.168.1.164"])

if len (sensoria.sensors) == 0:
	print "No sensors to log"
	sys.exit (1)

db = DB ()

while True:
	try:
		data = {}
		now = datetime.datetime.now ()
		for name, sensor in sensoria.sensors.iteritems ():
			try:
				data[name] = sensor.read (raw = True)
				print "%s -> %s" % (name, data[name])
			except Exception as ex:
				print "Cannot read sensor %s: %s" % (name, str (ex))
		db.insert (now, data, commit = True)
		print "."
	except Exception as ex:
		print "ERROR: %s" % str (ex)
		raise

	time.sleep (5 * 60)
