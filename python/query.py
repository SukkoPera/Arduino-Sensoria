#!/usr/bin/env python

import argparse
import time
import logging

import Sensoria

# ./query.py OH OT
# ./query -s
parser = argparse.ArgumentParser (description = 'Query Sensoria transducers')
parser.add_argument ('--address', metavar = "ADDRESS", nargs = '*', default = [], dest = "addresses",
										 help = "Address of node to query (Can be used multiple times)")
parser.add_argument ('--read-actuators', "-a", action = 'store_true', default = False,
										 help = "Read Actuators too")
parser.add_argument ('--interval', "-i", action = 'store', type = int, default = 0,
										 help = "Keep reading transducers periodically", metavar = "SECONDS")
parser.add_argument ('--no-discovery', "-n", action = 'store_true', default = False,
										 help = "Do not discover transducers at startup")
parser.add_argument ('--autodiscovery', "-A", action = 'store', type = int, default = None,
										 help = "Autodiscovery interval", metavar = "SECONDS")
parser.add_argument ('--verbose', "-v", action = 'count',
										 help = "Enable debugging messages")

args = parser.parse_args ()
if args.verbose == 2:
	logging.basicConfig (level = logging.DEBUG_COMMS, format = '[%(asctime)s - %(levelname)s:%(filename)s:%(lineno)d] %(message)s')
elif args.verbose == 1:
	logging.basicConfig (level = logging.DEBUG, format = '[%(asctime)s - %(levelname)s:%(filename)s:%(lineno)d] %(message)s')
else:
	logging.basicConfig (level = logging.INFO, format = '[%(asctime)s] %(message)s')

sensoria = Sensoria.Client (servers = args.addresses)
if args.no_discovery and args.autodiscovery:
	# Someone must be losing his/her mind...
	parser.print_help ()
elif not args.no_discovery:
	sensoria.discover ()
	if args.autodiscovery is not None:
		self._sensoria.enableAutodiscovery (args.autodiscovery)

while True:
	if len (args.addresses) > 0:
		for tname in args.addresses:
			tname = tname.upper ()
			if tname in sensoria.transducers:
				try:
					t = sensoria.transducers[tname]
					parsed = t.read ()
					print "%s: %s" % (tname, parsed)
				except Sensoria.Error as ex:
					print "%s: %s" % (tname, ex)
			else:
				print "%s: No such transducer" % tname
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
