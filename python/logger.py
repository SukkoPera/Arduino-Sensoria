#!/usr/bin/env python

import sys
import argparse
import time
import datetime
import logging
import signal

import Sensoria

KEEP_GOING = True

def sigHandler (signum, frame):
	global KEEP_GOING

	print 'Received signal', signum
	KEEP_GOING = False

signal.signal (signal.SIGHUP, sigHandler)
signal.signal (signal.SIGTERM, sigHandler)


parser = argparse.ArgumentParser (description = 'Log data from Sensoria transducers')
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
	sys.exit (1)
elif not args.no_discovery:
	sensoria.discover ()
	if args.autodiscovery is not None:
		sensoria.enableAutodiscovery (args.autodiscovery)

db = Sensoria.DB ()

print "Logging data every %d seconds" % args.interval

while KEEP_GOING:
	now = datetime.datetime.now ()
	print "--- %s ---" % now

	if len (sensoria.servers) == 0:
		print "No servers available"
	else:
		data = {}

		for sname, server in sensoria.servers.iteritems ():
			print "- Server: %s (%s:%d)" % (sname, server.address, server.port)
			for tname, t in sorted (server.transducers.iteritems ()):
				try:
					if t.genre == Sensoria.SENSOR:
						print "  - Sensor %s: %s" % (t.name, t.description)
						data[t] = t.read (raw = True)
						print "    - %s" % data[t]
					elif t.genre == Sensoria.ACTUATOR:
						print "  - Actuator %s: %s" % (t.name, t.description)
						if args.read_actuators:
							data[t] = t.read (raw = True)
							print "    - %s" % data[t]
					else:
						print "  - Found unknown transducer %s: %s" % (t.name, t.description)
				except Sensoria.Error as ex:
					print ex

		# All readings done, save to DB
		try:
			db.insert (now, data, commit = True)
		except Sensoria.Error as ex:
			print "Error while saving to DB: %s"  % str (ex)

	print "--- READ COMPLETE ---"

	if args.interval > 0:
		time.sleep (args.interval)
	else:
		break
