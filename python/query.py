#!/usr/bin/env python

import argparse

from ServerProxy import ServerProxy
from Sensoria import *

parser = argparse.ArgumentParser (description = 'Plot some data')
parser.add_argument ('addresses', metavar = "ADDRESS", nargs = '*',
                     help = "Address of node to query")
parser.add_argument ('--autodiscover', "-a", action = 'store_true', default = False,
                     help = "(Try to) Autodiscover nodes")


args = parser.parse_args ()
sensoria = Sensoria (servers = args.addresses, autodiscover = args.autodiscover)
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
