#!/usr/bin/env python

import sys
import datetime
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import argparse

import Sensoria

DEFAULT_HOURS = 48


parser = argparse.ArgumentParser (description = 'Plot sensor data')
parser.add_argument ('--output', "-o", dest = 'file', action = 'store',
                     help = "Don't show graph, save to file")
parser.add_argument ('--hours', dest = 'hours', type = int, action='store', default = DEFAULT_HOURS,
                     help = 'Show data for last N hours (default: %d)' % DEFAULT_HOURS)
parser.add_argument ('sensor', metavar = 'SENSOR', type = str, help = 'Sensor to plot data for')

args = parser.parse_args ()

xDateFmt = mdates.DateFormatter ('%d/%m/%Y %H:%M:%S')

db = Sensoria.DB ()
x = []
ys = {}
sens = args.sensor.upper ()
now = datetime.datetime.now ()
limit = now - datetime.timedelta (hours = args.hours)
prev = None
for dt, row in db.get_data_since (limit):
	if prev is not None and dt - prev > datetime.timedelta (hours = 2):
		x.append (dt - datetime.timedelta (hours = 1))
		for name in ys:
			ys[name].append (float ('nan'))
	prev = dt

	x.append (dt)
	if sens in row:
		reading = row[sens]
		for name, val in reading.getData ().iteritems ():
			if name not in ys:
				ys[name] = []
			ys[name].append (val)
	else:
		#~ print "No reading for sensor %s on %s" % (sens, dt)
		for name in ys:
			ys[name].append (float ('nan'))

# At this point, if ys[] has less elements than x, that's because those data
# started later, so prepend nan's as necessary
for name in ys:
	while len (ys[name]) < len (x):
		ys[name].insert (0, float ('nan'))

def subplot (name, i, x, y):
	plt.title (name)
	plt.plot (x, y, "b-")
	#~ plt.ylabel ('Degrees Celsius')
	plt.grid (True)
	plt.gca ().xaxis.set_major_formatter (xDateFmt)
	#~ plt.legend (loc = 'best', fancybox = True, framealpha = 0.5)

n = len (ys)
#~ print "Sensor %s has %d data items" % (sens, n)
if n > 0:
	fig = plt.figure (1, figsize = (16, 9), dpi = 120)
	fig.suptitle ("Sensor data over the last %d hours" % args.hours)

	k = ys.keys ()
	if n == 1:
		subplot (k[0], 1, x, ys[k[0]])
	elif n == 2:
		plt.subplot (1, 2, 1)
		subplot (k[0], 1, x, ys[k[0]])
		plt.subplot (1, 2, 2)
		subplot (k[1], 2, x, ys[k[1]])


	# Squeeze a bit
	#~ fig.tight_layout ()
	# rotate and align the tick labels so they look better
	fig.autofmt_xdate ()

	now = datetime.datetime.now ()
	plt.gca ().annotate('Generated on %s' % now.strftime ("%c"), xy=(1, 0), xycoords='axes fraction',
					xytext=(-150, -120), textcoords='offset points')

	if args.file is not None:
		plt.savefig (args.file)
		#~ print "Wrote graph to file %s" % args.file
	else:
		plt.show ()

	sys.exit (0)
else:
	sys.exit (1)
