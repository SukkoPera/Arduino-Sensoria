#!/usr/bin/env python

import datetime
import time

from db import DB
from Sensoria import *

<<<<<<< HEAD
sensoria = Sensoria (autodiscover = True)
#sensoria = Sensoria (servers = ["192.168.1.171"], autodiscover = True)
=======
#~ sensoria = Sensoria (autodiscover = True)
sensoria = Sensoria (servers = ["192.168.1.162"], autodiscover = True)
>>>>>>> 5ee9d9a6c3c59933e0f8fed8ca79517959e425b9
#~ sensoria = Sensoria (servers = ["192.168.1.164"])

db = DB ()

while True:
	try:
		data = {}
		now = datetime.datetime.now ()
		for name, sensor in sensoria.sensors.iteritems ():
<<<<<<< HEAD
			try:
				data[name] = sensor.read (raw = True)
				print "%s -> %s" % (name, data[name])
			except Exception as ex:
				print "Cannot read sensor %s: %s" % (name, str (ex))
=======
			data[name] = sensor.read (raw = True)
			print "%s -> %s" % (name, data[name])
>>>>>>> 5ee9d9a6c3c59933e0f8fed8ca79517959e425b9
		db.insert (now, data, commit = True)
		print "."
	except Exception as ex:
		print "ERROR: %s" % str (ex)
		#~ raise

	time.sleep (5 * 60)
