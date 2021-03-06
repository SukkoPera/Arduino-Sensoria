#!/usr/bin/env python

# WU Protocol Description:
# http://wiki.wunderground.com/index.php/PWS_-_Upload_Protocol

import sys
import datetime
import time
import math
import urllib
import urllib2
import logging
import signal
import argparse

import Sensoria


ELEVATION = 230.0			# meters
DEFAULT_INTERVAL = 10		# minutes
WU_STATION_ID = ""
WU_STATION_KEY = ""
WU_UPLOAD_URL = "https://weatherstation.wunderground.com/weatherstation/updateweatherstation.php"

KEEP_GOING = True

def sigHandler (signum, frame):
	global KEEP_GOING

	print 'Received signal', signum
	KEEP_GOING = False

signal.signal (signal.SIGHUP, sigHandler)
signal.signal (signal.SIGTERM, sigHandler)


parser = argparse.ArgumentParser (description = 'Send data to WeatherUnderground')
parser.add_argument ('--interval', "-i", action = 'store', type = int, default = DEFAULT_INTERVAL,
					 help = "Data log interval", metavar = "MINUTES")
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

# ~ sensoria = Sensoria.Client (servers = args.addresses)
sensoria = Sensoria.Client ()
if args.no_discovery and args.autodiscovery:
	# Someone must be losing his/her mind...
	parser.print_help ()
	sys.exit (1)
elif not args.no_discovery:
	sensoria.discover ()
	if args.autodiscovery is not None:
		sensoria.enableAutodiscovery (args.autodiscovery)

print "Logging data every %d minutes" % args.interval

def celsius2fahrenheit (t):
	return float (t) *  9 / 5 + 32

def mbar2inches (p):
	return float (p) * 0.0295301

def local2sea (p):
	"http://www.sandhurstweather.org.uk/barometric.pdf"
	return p / math.exp (-ELEVATION / ((t + 273.15) * 29.263))


# t is in celsius, h in percentage
def dewPoint (t, h):
	"""
	NOAA
	reference (1) : http://wahiduddin.net/calc/density_algorithms.htm
	reference (2) : http://www.colorado.edu/geography/weather_station/Geog_site/about.htm
	(1) Saturation Vapor Pressure = ESGG(T)
	"""

	if not math.isnan (t) and not math.isnan (h):
		RATIO = 373.15 / (273.15 + t)
		RHS = -7.90298 * (RATIO - 1)
		RHS += 5.02808 * math.log10 (RATIO)
		RHS += -1.3816e-7 * (math.pow (10, (11.344 * (1 - 1 / RATIO ))) - 1)
		RHS += 8.1328e-3 * (math.pow (10, (-3.49149 * (RATIO - 1))) - 1)
		RHS += math.log10 (1013.246)

		# factor -3 is to adjust units - Vapor Pressure SVP * humidity
		VP = math.pow (10, RHS - 3) * h

		# (2) DEWPOINT = F(Vapor Pressure)
		# log(x) is defined only for strictly positive x
		l = VP / 0.61078
		if not math.isnan (l) and l > 0:
			T = math.log (l)
			ret = (241.88 * T) / (17.558 - T)
	return ret


# FIXME: Stop with SIGHUP
while True:
	try:
		ot = sensoria.sensors["OT"]
		oh = sensoria.sensors["OH"]
		op = sensoria.sensors["OP"]
		#~ print "Found internal thermometer on %s at %s:%s" % (it.server.name, it.server.address, it.server.port)
		print "Sensors found!"

		now = datetime.datetime.now ()
		t = ot.read ().temperature
		h = oh.read ().humidity
		p = op.read ().localPressure
		dp = dewPoint (t, h)
		print "[%s] T=%f H=%f P=%f DP=%f" % (now.strftime ("%Y-%m-%d %H:%M:%S"), t, h, p, dp),



		t_f = celsius2fahrenheit (t)
		p_sea = local2sea (p)
		p_in = mbar2inches (p_sea)
		dp_f = celsius2fahrenheit (dp)
		#~ print "After convs: T=%f H=%f P=%f DP=%f" % (t_f, h, p_in, dp_f)

		params = {
			"action": "updateraw",
			"dateutc": "now",
			"ID": WU_STATION_ID,
			"PASSWORD": WU_STATION_KEY,
			"tempf": t_f,
			"humidity": h,
			"dewptf": dp_f,
			"baromin": p_in
		}

		url = "%s?%s" % (WU_UPLOAD_URL, urllib.urlencode (params))
		#~ print "Uploading data...",
		sys.stdout.flush ()

		try:
			#~ print "Opening %s..." % url
			f = urllib2.urlopen (url)
			reply = f.read ().strip ()
			#~ print "-%s-" % reply
			if reply == "success":
				print "Success!"
				time.sleep (DEFAULT_INTERVAL * 60)
			else:
				print "Failed (Server failure)"
				#print "URL was: %s" % url
				time.sleep (60)	# Retry in a minute
		except urllib2.URLError as ex:
			print "Failed (%s)" % str (ex)
			time.sleep (60) # Retry in a minute
	except (Sensoria.Error, KeyError) as ex:
		print "Cannot get sensors: %s" % str (ex)
		time.sleep (60)		# Retry in a minute
