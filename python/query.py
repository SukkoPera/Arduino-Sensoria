#!/usr/bin/env python

import argparse

import Sensoria

# ./query.py OH OT
# ./query -s
parser = argparse.ArgumentParser (description = 'Query Sensoria devices')
parser.add_argument ('addresses', metavar = "ADDRESS", nargs = '*',
                     help = "Address of node to query")
parser.add_argument ('--no-autodiscover', "-n", action = 'store_true', default = False,
                     help = "Do not autodiscover nodes")
parser.add_argument ('--read-actuators', "-a", action = 'store_true', default = False,
                     help = "Read Actuators too")


args = parser.parse_args ()
sensoria = Sensoria.Client (servers = args.addresses, autodiscover = not args.no_autodiscover)
for sname, server in sensoria.servers.iteritems ():
	print "- Server: %s (%s:%d)" % (sname, server.address, server.port)
	for tname in sorted (server.transducers.iterkeys ()):
		t = server.transducers[tname]
		try:
			if t.genre == Sensoria.SENSOR:
				print "  - Found Sensor %s: %s" % (t.name, t.description)
				parsed = t.read ()
				print "    - Read: %s" % parsed
			elif t.genre == Sensoria.ACTUATOR:
				print "  - Found Actuator %s: %s" % (t.name, t.description)
				if args.read_actuators:
					parsed = t.read ()
					print "    - Read: %s" % parsed
			else:
				print "  - Found unknown transducer %s: %s" % (t.name, t.description)
		except Sensoria.Error as ex:
			print ex
			pass
