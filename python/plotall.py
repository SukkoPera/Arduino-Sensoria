#!/usr/bin/env python

import os
import sys
import datetime
import argparse
import smtplib
import tempfile
import math

# Here are the email package modules we'll need
from email.mime.image import MIMEImage
from email.mime.multipart import MIMEMultipart

import matplotlib.pyplot as plt
import matplotlib.dates as mdates

import Sensoria

MAIL_FROM = "noreply@sukkology.net"
MAIL_SUBJ = "GRAPHs"
MAIL_SMTP = "mail.sukkology.net"
MAIL_PORT = 25
MAIL_TLS = True
MAIL_USER = "sukkology.net"
MAIL_PASS = ""

DEFAULT_HOURS = 48


parser = argparse.ArgumentParser (description = 'Plot some data')
parser.add_argument ('--output', "-o", dest = 'file', action = 'store',
                     help = "Don't show graph, save to file")
parser.add_argument ('--mailto', "-m", dest = 'mailaddr', action = 'store',
                     help = "Don't show graph, send by e-mail", metavar = "EMAIL_ADDRESS")
parser.add_argument ('--hours', dest = 'hours', type = int, action='store', default = DEFAULT_HOURS,
                     help = 'Show data for last N hours (default: %d)' % DEFAULT_HOURS)
parser.add_argument ('--from', dest = 'from_', type = str, action='store', default = None,
                     help = 'Start date', metavar = "YYYYMMDD")
parser.add_argument ('--to', dest = 'to', type = str, action='store', default = None,
                     help = 'End date', metavar = "YYYYMMDD")

args = parser.parse_args ()

xDateFmt = mdates.DateFormatter ('%d/%m/%Y %H:%M:%S')

db = Sensoria.DB ()
x = []
y = []
yti = []
yv = []
ydht = []
ydht22 = []
yth3 = []
ydallas = []
ydallas2 = []
y35 = []
ybmp = []
ypx = []
ypx2 = []
yh = []
yh2 = []
yh3 = []
ylux = []
ylux2 = []
yscale = []	#yldr
yldr2 = []
yrt = []
yrh = []

if args.from_ is not None or args.to is not None:
	if args.from_ is not None and args.to is not None:
		start = datetime.datetime.strptime (args.from_, "%Y%m%d")
		end = datetime.datetime.strptime (args.to, "%Y%m%d")
		data = db.get_data_between (start, end)
		timeDesc = "from %s to %s" % (start.strftime ("%d/%m/%Y"), end.strftime ("%d/%m/%Y"))
	else:
		print "--from and --to must be used together"
		sys.exit (1)
elif args.hours is not None:
	now = datetime.datetime.now ()
	limit = now - datetime.timedelta (hours = args.hours)
	data = db.get_data_since (limit)
	timeDesc = "over the last %d hours" % args.hours
elif args.hours is not None:
	now = datetime.datetime.now ()
	limit = now - datetime.timedelta (hours = DEFAULT_HOURS)
	data = db.get_data_since (limit)
	timeDesc = "over the last %d hours" % DEFAULT_HOURS

prev = None
for dt, row in data:
	if prev is not None and dt - prev > datetime.timedelta (hours = 2):
		x.append (dt - datetime.timedelta (hours = 1))
		yti.append (float ('nan'))
		ydht.append (float ('nan'))
		ydht22.append (float ('nan'))
		yth3.append (float ('nan'))
		ydallas.append (float ('nan'))
		ydallas2.append (float ('nan'))
		ybmp.append (float ('nan'))
		ypx2.append (float ('nan'))
		yh.append (float ('nan'))
		yh2.append (float ('nan'))
		yh3.append (float ('nan'))
		ylux.append (float ('nan'))
		ylux2.append (float ('nan'))
		yscale.append (float ('nan'))
		yldr2.append (float ('nan'))
		ypx.append (float ('nan'))
		y35.append (float ('nan'))
		yrt.append (float ('nan'))
		yrh.append (float ('nan'))
	prev = dt

	x.append (dt)

	if "OH" in row:
		ydht.append (row["OH"].temperature)
		yh.append (row["OH"].humidity)
	else:
		#~ print "No reading for sensor OH in data from %s" % (dt)
		ydht.append (float ('nan'))
		yh.append (float ('nan'))

	if "H2" in row:
		ydht22.append (row["H2"].temperature)
		yh2.append (row["H2"].humidity)
	else:
		#~ print "No reading for sensor H2 in data from %s" % (dt)
		ydht22.append (float ('nan'))
		yh2.append (float ('nan'))

	if "IH" in row:
		yth3.append (row["IH"].temperature)
		yh3.append (row["IH"].humidity)
	else:
		#~ print "No reading for sensor IH in data from %s" % (dt)
		yth3.append (float ('nan'))
		yh3.append (float ('nan'))

	if "RH" in row:
		yrt.append (row["RH"].temperature)
		yrh.append (row["RH"].humidity)
	else:
		#~ print "No reading for sensor RH in data from %s" % (dt)
		yrt.append (float ('nan'))
		yrh.append (float ('nan'))

	if "OT" in row:
		ydallas.append (row["OT"].temperature)
	else:
		#~ print "No reading for sensor OT in data from %s" % (dt)
		ydallas.append (float ('nan'))

	if "IT" in row:
		yti.append (row["IT"].temperature)
	else:
		#~ print "No reading for sensor IT in data from %s" % (dt)
		yti.append (float ('nan'))

	if "T2" in row:
		ydallas2.append (row["T2"].temperature)
	else:
		#~ print "No reading for sensor T2 in data from %s" % (dt)
		ydallas2.append (float ('nan'))

	if "T3" in row:
		y35.append (row["T3"].temperature)
	else:
		#~ print "No reading for sensor T3 in data from %s" % (dt)
		y35.append (float ('nan'))

	if "OP" in row:
		ypx.append (row["OP"].localPressure)
		ybmp.append (row["OP"].temperature)
	else:
		#~ print "No reading for sensor OP in data from %s" % (dt)
		ypx.append (float ('nan'))
		ybmp.append (float ('nan'))

	if "P2" in row:
		ypx2.append (row["P2"].localPressure)
		#~ ybmp.append (row["P2"].temperature)
	else:
		#~ print "No reading for sensor P2 in data from %s" % (dt)
		ypx2.append (float ('nan'))

	# Light sensor
	if "OL" in row:
		ylux.append (row["OL"].lightLux)
	else:
		#~ print "No reading for sensor OL in data from %s" % (dt)
		ylux.append (float ('nan'))

	# Visible/Infrared light
	if "OR" in row:
		ylux2.append (row["OR"].lightLux)
	else:
		#~ print "No reading for sensor OR in data from %s" % (dt)
		ylux2.append (float ('nan'))

	# LDR 1
	if "PR" in row:
		yscale.append (row["PR"].light10bit)
		#~ yv.append (row["light_v"])
	else:
		#~ print "No reading for sensor PR in data from %s" % (dt)
		yscale.append (float ('nan'))

	# LDR 2
	if "L2" in row:
		yldr2.append (row["L2"].light10bit)	# Column name is misleading!
	else:
		#~ print "No reading for sensor L2 in data from %s" % (dt)
		yldr2.append (float ('nan'))


fig = plt.figure (1, figsize = (16, 9), dpi = 120)
fig.suptitle ("Weather Data %s" % timeDesc)

# Temps

# We'll need humidity for a few calculations, so choose the one with the most
# samples
notnans = lambda v: len (v) - len (filter (math.isnan, v))
# ~ hs = tuple ((h, notnans (h)) for h in (yh, yh2, yh3))
hs = tuple ((h, notnans (h)) for h in (yh,))		# Only IH is measured outdoors!
hs = sorted (hs, key = lambda t: t[1], reverse = True)
#~ print [h[1] for h in hs]
goodH = hs[0][0]

# Dew Point
# NOAA
# reference (1) : http://wahiduddin.net/calc/density_algorithms.htm
# reference (2) : http://www.colorado.edu/geography/weather_station/Geog_site/about.htm
# (1) Saturation Vapor Pressure = ESGG(T)
dp = []
for celsius, humidity in zip (ydallas, goodH):
	if not math.isnan (celsius) and not math.isnan (humidity):
		RATIO = 373.15 / (273.15 + celsius)
		RHS = -7.90298 * (RATIO - 1)
		RHS += 5.02808 * math.log10 (RATIO)
		RHS += -1.3816e-7 * (math.pow (10, (11.344 * (1 - 1 / RATIO ))) - 1)
		RHS += 8.1328e-3 * (math.pow (10, (-3.49149 * (RATIO - 1))) - 1)
		RHS += math.log10 (1013.246)

		# factor -3 is to adjust units - Vapor Pressure SVP * humidity
		VP = math.pow (10, RHS - 3) * humidity

		# (2) DEWPOINT = F(Vapor Pressure)
		# log(x) is defined only for strictly positive x
		l = VP / 0.61078
		if not math.isnan (l) and l > 0:
			T = math.log (l)
			dp.append ((241.88 * T) / (17.558 - T))
		else:
			dp.append (float ("nan"))
	else:
		dp.append (float ("nan"))

def celsius2fahrenheit (t):
	return float (t) *  9 / 5 + 32

def fahrenheit2celsius (t):
	return (float (t) - 32) * 5 / 9

# Perceived temperature (AKA Heat Index)
# https://en.wikipedia.org/wiki/Heat_index
def perceivedTemp (tempList, humList):
	pt1 = []
	pt2 = []
	pt3 = []
	for tc, h in zip (tempList, humList):
		if not math.isnan (tc) and not math.isnan (h):
			t = celsius2fahrenheit (tc)		# American formulas... :(

			if t >= 80 and h >= 40:
				# Within +-1.3 degF.
				c1 = -42.379
				c2 = 2.04901523
				c3 = 10.14333127
				c4 = -0.22475541
				c5 = -6.83783e-3
				c6 = -5.481717e-2
				c7 = 1.22874e-3
				c8 = 8.5282e-4
				c9 = -1.99e-6
				hi1 = c1 + c2 * t + c3 * h + c4 * t * h + c5 * t * t + c6 * h * h + c7 * t * t * h + c8 * t * h * h + c9 * t * t * h * h
				hi1 = fahrenheit2celsius (hi1)
				pt1.append (hi1)

				# Within 3 degrees of the NWS (US National Weather Service???)
				# master table for all humidities from 0 to 80% and all temperatures
				# between 70 and 115 degF and all heat indexes < 150 degF
				# (Whatever that means)
				c1 = 0.363445176
				c2 = 0.988622465
				c3 = 4.777114035
				c4 = -0.114037667
				c5 = -0.000850208
				c6 = -0.020716198
				c7 = 0.000687678
				c8 = 0.000274954
				c9 = 0
				hi2 = c1 + c2 * t + c3 * h + c4 * t * h + c5 * t * t + c6 * h * h + c7 * t * t * h + c8 * t * h * h + c9 * t * t * h * h
				hi2 = fahrenheit2celsius (hi2)
				pt2.append (hi2)

				# Who knows???
				c1 = 16.923
				c2 = 0.185212
				c3 = 5.37941
				c4 = -0.100254
				c5 = 9.41695e-3
				c6 = 7.28898e-3
				c7 = 3.45372e-4
				c8 = -8.14971e-4
				c9 = 1.02102e-5
				c10 = -3.8646e-5
				c11 = 2.91583e-5
				c12 = 1.42721e-6
				c13 = 1.97483e-7
				c14 = -2.18429e-8
				c15 = 8.43296e-10
				c16 = -4.81975e-11
				hi3 = c1 + c2 * t + c3 * h + c4 * t * h + c5 * t * t + c6 * h * h + c7 * t * t * h + c8 * t * h * h + \
						c9 * t * t * h * h + c10 * t * t * t + c11 * h * h * h + c12 * t * t * t * h + c13 * t * h * h * h + \
						c14 * t * t * t * h * h + c15 * t * t * h * h * h + c16 * t * t * t * h * h * h
				hi3 = fahrenheit2celsius (hi3)
				pt3.append (hi3)
			else:
				pt1.append (tc)
				pt2.append (tc)
				pt3.append (tc)
		else:
			pt1.append (float ("nan"))
			pt2.append (float ("nan"))
			pt3.append (float ("nan"))

	return pt1, pt2, pt3

dpfast = []
for celsius, humidity in zip (ydallas, goodH):
    # delta max = 0.6544 wrt dewPoint()
    # reference: http://en.wikipedia.org/wiki/Dew_point
    a = 17.271
    b = 237.7
    temp = (a * celsius) / (b + celsius) + math.log (humidity * 0.01)
    Td = (b * temp) / (a - temp)
    dpfast.append (Td);

pt1, pt2, pt3 = perceivedTemp (ydallas, goodH)
pti1, pti2, pti3 = perceivedTemp (yth3, yh3)
ptr1, ptr2, ptr3 = perceivedTemp (yrt, yrh)

plt.subplot (2, 2, 1)
plt.title ("Temperature")
#plt.plot (x, ydht, "y--", label = "DHT22")
#plt.plot (x, ydht22, "g", label = "DHT22")
#plt.plot (x, ybmp, "m", label = "BMP180")
plt.plot (x, ydallas, "b", label = "Outdoor")
#plt.plot (x, y35, "c", label = "LM35")
plt.plot (x, yth3, "r", label = "Indoor")
plt.plot (x, pti1, "r")
plt.fill_between (x, yth3, pti1, facecolor = 'r', alpha = 0.5)
plt.plot (x, yrt, "c", label = "Sleeping Room")
plt.plot (x, ptr1, "c")
plt.fill_between (x, yrt, ptr1, facecolor = 'c', alpha = 0.5)
#plt.plot (x, yti, "b--", label = "Indoor")
plt.plot (x, dp, "g--", label = "Dew Point")
plt.plot (x, pt1, "b")
#plt.plot (x, pt2, "g--", label = "Heat Index 2")
#plt.plot (x, pt3, "b--", label = "Heat Index 3")
plt.fill_between (x, ydallas, pt1, facecolor = 'b', alpha = 0.5)
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
plt.title ("Atmospheric Pressure")
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
plt.title ("Relative Humidity")
plt.plot (x, yh, "b", label = "Outdoor")
#plt.plot (x, yh2, "g-", label = "Outdoor")
plt.plot (x, yh3, "r", label = "Indoor")
plt.plot (x, yrh, "c", label = "Sleeping Room")
#~ plt.xlabel ('Time')
plt.ylabel ('Percentage')
plt.grid (True)
plt.gca ().xaxis.set_major_formatter (xDateFmt)
plt.legend (loc = 'best', fancybox = True, framealpha = 0.5)

# Light
plt.subplot (2, 2, 4)
plt.title ("Light Level")
plt.plot (x, yscale, "b-", label = "LDR")
#~ plt.plot (x, yldr2, "m-", label = "LDR 2")
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
