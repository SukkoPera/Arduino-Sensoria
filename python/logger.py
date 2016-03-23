#!/usr/bin/env python

import sys
import datetime
import time
import argparse

import Sensoria

parser = argparse.ArgumentParser (description = 'Plot some data')
parser.add_argument ('addresses', metavar = "ADDRESS", nargs = '*',
                     help = "Address of node to query")
parser.add_argument ('--no-autodiscover', "-n", action = 'store_true', default = False,
                     help = "Do not autodiscover nodes")
parser.add_argument ('--read-actuators', "-a", action = 'store_true', default = False,
                     help = "Read Actuators too")

args = parser.parse_args ()
sensoria = Sensoria.Client (servers = args.addresses, autodiscover = not args.no_autodiscover)
if len (sensoria.transducers) == 0:
	print "No sensors to log"
	sys.exit (1)

db = Sensoria.DB ()

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
