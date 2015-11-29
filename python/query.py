#!/usr/bin/env python

from ServerProxy import ServerProxy
from Sensoria import *

sensoria = Sensoria (autodiscover = True)
for sname, server in sensoria.servers.iteritems ():
	print "- Server: %s (%s:%d)" % (sname, server.address, server.port)
	for tname in sorted (server.sensors.iterkeys ()):
		t = server.sensors[tname]
		if isinstance (t, SensorProxy):
			print "  - Found Sensor %s: %s" % (t.name, t.description)
			r = t.read ()
			print "    - Read: %s" % r
		elif isinstance (t, ActuatorProxy):
			print "  - Found Actuator %s: %s" % (t.name, t.description)
