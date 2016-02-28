#!/usr/bin/env python

from ServerProxy import ServerProxy
from Sensoria import *

#~ sensoria = Sensoria (servers = ["192.168.1.154", "192.168.1.155"], autodiscover = True)
<<<<<<< HEAD
#~ sensoria = Sensoria (servers = ["192.168.1.184"])
sensoria = Sensoria (servers = [], autodiscover = True)
=======
#~ sensoria = Sensoria (servers = ["192.168.1.168"])
sensoria = Sensoria (servers = ["192.168.1.162"], autodiscover = True)
>>>>>>> 5ee9d9a6c3c59933e0f8fed8ca79517959e425b9
#~ sensoria = Sensoria (servers = ["localhost"])
for sname, server in sensoria.servers.iteritems ():
	print "- Server: %s (%s:%d)" % (sname, server.address, server.port)
	for tname in sorted (server.sensors.iterkeys ()):
		t = server.sensors[tname]
		try:
			if isinstance (t, SensorProxy):
				print "  - Found Sensor %s: %s" % (t.name, t.description)
				parsed = t.read ()
				print "    - Read: %s" % parsed
			elif isinstance (t, ActuatorProxy):
				print "  - Found Actuator %s: %s" % (t.name, t.description)
		except SensorError as ex:
			pass
