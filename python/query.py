#!/usr/bin/env python

import argparse
import time

import Sensoria

# ./query.py OH OT
# ./query -s
parser = argparse.ArgumentParser (description = 'Query Sensoria devices')
parser.add_argument ('--read', "-r", metavar = "TRANSDUCER_NAME", nargs = '*', default = [],
										 help = "Read a transducer")
parser.add_argument ('--address', metavar = "ADDRESS", nargs = '*', default = [], dest = "addresses",
										 help = "Address of node to query (Can be used multiple times)")
parser.add_argument ('--read-actuators', "-a", action = 'store_true', default = False,
										 help = "Read Actuators too")
parser.add_argument ('--interval', "-i", action = 'store', type = int, default = 0,
										 help = "Keep polling sensors periodically", metavar = "SECONDS")
parser.add_argument ('--autodiscover', "-A", action = 'store', type = int, default = None,
										 help = "Autodiscover interval (0 to disable)", metavar = "SECONDS")


args = parser.parse_args ()
if args.autodiscover is None:
	sensoria = Sensoria.Client (servers = args.addresses)
elif args.autodiscover > 0:
	sensoria = Sensoria.Client (servers = args.addresses, autodiscTimer = args.autodiscover)
else:
	sensoria = Sensoria.Client (servers = args.addresses, autodiscover = False)
while True:
	if len (args.read) > 0:
		for tname in args.read:
			tname = tname.upper ()
			if tname in sensoria.transducers:
				try:
					t = sensoria.transducers[tname]
					parsed = t.read ()
					print "%s: %s" % (tname, parsed)
				except Sensoria.Error as ex:
					print "%s: %s" % (tname, ex)
			else:
				print "%s: Not found" % tname
	else:
		if len (sensoria.servers) == 0:
			print "No servers available"
		else:
			for sname, server in sensoria.servers.iteritems ():
				print "- Server: %s (%s:%d)" % (sname, server.address, server.port)
				for tname, t in sorted (server.transducers.iteritems ()):
					try:
						if t.genre == Sensoria.SENSOR:
							print "  - Sensor %s: %s" % (t.name, t.description)
							parsed = t.read ()
							print "    - Read: %s" % parsed
						elif t.genre == Sensoria.ACTUATOR:
							print "  - Actuator %s: %s" % (t.name, t.description)
							if args.read_actuators:
								parsed = t.read ()
								print "    - Read: %s" % parsed
						else:
							print "  - Found unknown transducer %s: %s" % (t.name, t.description)
					except Sensoria.Error as ex:
						print ex
						pass

	if args.interval > 0:
		time.sleep (args.interval)
	else:
		break
