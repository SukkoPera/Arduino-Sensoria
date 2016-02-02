#!/usr/bin/env python

import os
import sys
import datetime
import argparse
import smtplib
import tempfile

# Here are the email package modules we'll need
from email.mime.image import MIMEImage
from email.mime.multipart import MIMEMultipart

import matplotlib.pyplot as plt
import matplotlib.dates as mdates

from db import DB

MAIL_FROM = None
MAIL_SUBJ = "GRAPHs"
MAIL_SMTP = None
MAIL_PORT = 25
MAIL_TLS = True
MAIL_USER = None
MAIL_PASS = None

DEFAULT_HOURS = 48


parser = argparse.ArgumentParser (description = 'Plot some data')
parser.add_argument ('--output', "-o", dest = 'file', action = 'store',
                     help = "Don't show graph, save to file")
parser.add_argument ('--mailto', "-m", dest = 'mailaddr', action = 'store',
                     help = "Don't show graph, send by e-mail", metavar = "EMAIL_ADDRESS")
parser.add_argument ('--hours', dest = 'hours', type = int, action='store', default = DEFAULT_HOURS,
                     help = 'Show data for last N hours (default: %d)' % DEFAULT_HOURS)

args = parser.parse_args ()

xDateFmt = mdates.DateFormatter ('%d/%m/%Y %H:%M:%S')

db = DB ()
x = []
y = []
yv = []
ydht = []
ydht22 = []
ydallas = []
ydallas2 = []
y35 = []
ybmp = []
ypx = []
ypx2 = []
yh = []
yh2 = []
ylux = []
ylux2 = []
yscale = []	#yldr
yldr2 = []
now = datetime.datetime.now ()
limit = now - datetime.timedelta (hours = args.hours)
prev = None
for dt, row in db.get_data_since (limit):
	if prev is not None and dt - prev > datetime.timedelta (hours = 2):
		x.append (dt - datetime.timedelta (hours = 1))
		ydht.append (float ('nan'))
		ydht22.append (float ('nan'))
		ydallas.append (float ('nan'))
		ydallas2.append (float ('nan'))
		ybmp.append (float ('nan'))
		ypx2.append (float ('nan'))
		yh.append (float ('nan'))
		yh2.append (float ('nan'))
		ylux.append (float ('nan'))
		ylux2.append (float ('nan'))
		yscale.append (float ('nan'))
		yldr2.append (float ('nan'))
		ypx.append (float ('nan'))
		y35.append (float ('nan'))
	prev = dt

	x.append (dt)

	if "OH" in row:
		ydht.append (row["OH"].temperature)
		yh.append (row["OH"].humidity)
	else:
		print "No reading for sensor OH in data from %s" % (dt)
		ydht.append (float ('nan'))
		yh.append (float ('nan'))

	if "H2" in row:
		ydht22.append (row["H2"].temperature)
		yh2.append (row["H2"].humidity)
	else:
		print "No reading for sensor H2 in data from %s" % (dt)
		ydht22.append (float ('nan'))
		yh2.append (float ('nan'))

	if "OT" in row:
		ydallas.append (row["OT"].temperature)
	else:
		print "No reading for sensor OT in data from %s" % (dt)
		ydallas.append (float ('nan'))

	if "T2" in row:
		ydallas2.append (row["T2"].temperature)
	else:
		print "No reading for sensor T2 in data from %s" % (dt)
		ydallas2.append (float ('nan'))

	if "T3" in row:
		y35.append (row["T3"].temperature)
	else:
		print "No reading for sensor T3 in data from %s" % (dt)
		y35.append (float ('nan'))

	if "OP" in row:
		ypx.append (row["OP"].localPressure)
		ybmp.append (row["OP"].temperature)
	else:
		print "No reading for sensor OP in data from %s" % (dt)
		ypx.append (float ('nan'))
		ybmp.append (float ('nan'))

	if "P2" in row:
		ypx2.append (row["P2"].localPressure)
		#~ ybmp.append (row["P2"].temperature)
	else:
		print "No reading for sensor P2 in data from %s" % (dt)
		ypx2.append (float ('nan'))

	# Light sensor
	if "OL" in row:
		ylux.append (row["OL"].lightLux)
	else:
		print "No reading for sensor OL in data from %s" % (dt)
		ylux.append (float ('nan'))

	# Visible/Infrared light
	if "OR" in row:
		ylux2.append (row["OR"].lightLux)
	else:
		print "No reading for sensor OR in data from %s" % (dt)
		ylux2.append (float ('nan'))

	# LDR 1
	if "PR" in row:
		yscale.append (row["PR"].light10bit)
		#~ yv.append (row["light_v"])
	else:
		print "No reading for sensor PR in data from %s" % (dt)
		yscale.append (float ('nan'))

	# LDR 2
	if "L2" in row:
		yldr2.append (row["L2"].light10bit)	# Column name is misleading!
	else:
		print "No reading for sensor L2 in data from %s" % (dt)
		yldr2.append (float ('nan'))


fig = plt.figure (1, figsize = (16, 9), dpi = 120)
fig.suptitle ("Weather Data over the last %d hours" % args.hours)

# Temps

# Dew Point
# NOAA
# reference (1) : http://wahiduddin.net/calc/density_algorithms.htm
# reference (2) : http://www.colorado.edu/geography/weather_station/Geog_site/about.htm
# (1) Saturation Vapor Pressure = ESGG(T)

import math

dp = []
for celsius, humidity in zip (ydallas, yh2):
	RATIO = 373.15 / (273.15 + celsius)
	RHS = -7.90298 * (RATIO - 1)
	RHS += 5.02808 * math.log10 (RATIO)
	RHS += -1.3816e-7 * (math.pow (10, (11.344 * (1 - 1 / RATIO ))) - 1)
	RHS += 8.1328e-3 * (math.pow (10, (-3.49149 * (RATIO - 1))) - 1)
	RHS += math.log10 (1013.246)

	# factor -3 is to adjust units - Vapor Pressure SVP * humidity
	VP = math.pow (10, RHS - 3) * humidity

	# (2) DEWPOINT = F(Vapor Pressure)
	T = math.log (VP / 0.61078)
	dp.append ((241.88 * T) / (17.558 - T))

#~ dpfast = []
#~ for celsius, humidity in zip (ydallas, yh):
    #~ # delta max = 0.6544 wrt dewPoint()
    #~ # reference: http://en.wikipedia.org/wiki/Dew_point
    #~ a = 17.271
    #~ b = 237.7
    #~ temp = (a * celsius) / (b + celsius) + math.log (humidity * 0.01)
    #~ Td = (b * temp) / (a - temp)
    #~ dpfast.append (Td);

plt.subplot (2, 2, 1)
plt.title ("Temperature")
plt.plot (x, ydht, "r--", label = "DHT11")
plt.plot (x, ydht22, "m--", label = "DHT22")
plt.plot (x, ydallas, "b-", label = "Temperature")
plt.plot (x, y35, "c-", label = "LM35")
plt.plot (x, dp, "g-", label = "Dew Point")
#~ plt.plot (x, dpfast, "m-", label = "Dew Point (Fast)")
#~ plt.plot (x, ydallas2, "m-", label = "DS18B20 (2)")
#~ plt.plot (x, ybmp, "g--", label = "BMP180")
#~ ymean = [(xx + y + z) / 3 for xx, y, z in zip (ydht, ydallas, ybmp)]
#~ plt.plot (x, ymean, "k-", label = "Mean")
#~ plt.xlabel ('Time')
plt.ylabel ('Degrees Celsius')
plt.grid (True)
plt.gca ().xaxis.set_major_formatter (xDateFmt)
plt.legend (loc = 'best', fancybox = True, framealpha = 0.5)

# Pressure

# http://www.sandhurstweather.org.uk/barometric.pdf
import math
ELEVATION = 230.0
yslp = [p / math.exp (-ELEVATION / ((t + 273.15) * 29.263)) if p is not None else float ("nan") for p, t in zip (ypx, ybmp)]
#~ print zip (x, yslp)

plt.subplot (2, 2, 2)
plt.title ("Local Pressure")
plt.plot (x, ypx, "b-", label = "Local")
#~ plt.plot (x, ypx2, "g-", label = "Local 2")
plt.plot (x, yslp, "r-", label = "Sea-Level")
#~ plt.xlabel ('Time')
plt.ylabel ('Hectopascals (mbar)')
plt.grid (True)
plt.gca ().xaxis.set_major_formatter (xDateFmt)
plt.legend (loc = 'best', fancybox = True, framealpha = 0.5)


# Humidity
plt.subplot (2, 2, 3)
plt.title ("Humidity")
plt.plot (x, yh, "b-")
plt.plot (x, yh2, "g-")
#~ plt.xlabel ('Time')
plt.ylabel ('Percentage')
plt.grid (True)
plt.gca ().xaxis.set_major_formatter (xDateFmt)


# Light
plt.subplot (2, 2, 4)
plt.title ("Light Level")
plt.plot (x, yscale, "b-", label = "LDR")
plt.plot (x, yldr2, "m-", label = "LDR 2")
#~ plt.xlabel ('Time')
#~ plt.ylabel ('Light Level')
plt.grid (True)
plt.gca ().xaxis.set_major_formatter (xDateFmt)
plt.plot (x, ylux, "r-", label = "Lux")
#~ plt.plot (x, ylux2, "g-", label = "Lux (Vis+IR)")
plt.legend (loc = 'best', fancybox = True, framealpha = 0.5)

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
elif args.mailaddr is not None:
	tempfile = tempfile.mkstemp (suffix = ".png")
	print "Saving temporary graph to %s" % tempfile[1]
	plt.savefig (tempfile[1])

	# Create the container (outer) email message.
	msg = MIMEMultipart()
	msg['Subject'] = MAIL_SUBJ
	msg['From'] = MAIL_FROM
	msg['To'] = args.mailaddr

	# Open the files in binary mode.  Let the MIMEImage class automatically
	# guess the specific image type.
	fp = open (tempfile[1], 'rb')
	img = MIMEImage (fp.read())
	fp.close ()
	msg.attach (img)

	# Clean up
	#~ tempfile[0].close ()
	os.unlink (tempfile[1])

	attempts = 0
	done = False
	while attempts < 3 and not done:
		try:
			attempts += 1
			s = smtplib.SMTP (MAIL_SMTP, MAIL_PORT, timeout = 15)
			#~ s.set_debuglevel (True)
			if MAIL_TLS:
				print "TLS enabled"
				s.starttls ()
			if MAIL_USER is not None and MAIL_PASS is not None:
				print "Attempting login"
				s.login (MAIL_USER, MAIL_PASS)
			s.sendmail (MAIL_FROM, args.mailaddr, msg.as_string ())
			s.quit ()
			done = True
		except Exception, ex:
			print "Attempt %d failed: %s" % (attempts, str (ex))
	if not done:
		print "Too many failed attempts, aborting"
else:
	plt.show ()

sys.exit (0)
