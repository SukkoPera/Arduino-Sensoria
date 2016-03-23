#!/usr/bin/env python

import sys
import datetime
import time
import argparse

import Sensoria

DEFAULT_INTERVAL = 5		# minutes

parser = argparse.ArgumentParser (description = 'Plot some data')
parser.add_argument ('addresses', metavar = "ADDRESS", nargs = '*',
                     help = "Address of node to query")
parser.add_argument ('--no-autodiscover', "-n", action = 'store_true', default = False,
                     help = "Do not autodiscover nodes")
parser.add_argument ('--read-actuators', "-a", action = 'store_true', default = False,
                     help = "Read Actuators too")
parser.add_argument ('--interval', "-i", default = DEFAULT_INTERVAL, type = int,
                     help = "Interval between reads (minutes)")

args = parser.parse_args ()
sensoria = Sensoria.Client (servers = args.addresses, autodiscover = not args.no_autodiscover)
if len (sensoria.transducers) == 0:
	print "No sensors to log"
	sys.exit (1)

print "Logging data every %d minutes" % args.interval

db = Sensoria.DB ()

# FIXME: Stop with SIGHUP
while True:
	try:
		data = {}
		now = datetime.datetime.now ()
		for tname in sorted (sensoria.transducers.iterkeys ()):
			t = sensoria.transducers[tname]
			if t.genre == Sensoria.SENSOR or (t.genre == Sensoria.ACTUATOR and args.read_actuators):
				try:
					data[t] = t.read (raw = True)
					print "%s -> %s" % (tname, data[t])
				except Exception as ex:
					print "Cannot read sensor %s: %s" % (tname, str (ex))

		# All readings done, save to DB
		db.insert (now, data, commit = True)
		print "."
	except Exception as ex:
		print "ERROR: %s" % str (ex)
		raise

	# Sleep until next update
	time.sleep (args.interval * 60)
