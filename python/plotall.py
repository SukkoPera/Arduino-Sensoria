#!/usr/bin/env python

import sys
import datetime
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import argparse

from db import DB

DEFAULT_HOURS = 48


parser = argparse.ArgumentParser (description = 'Plot some data')
parser.add_argument ('--output', "-o", dest = 'file', action = 'store',
                     help = "Don't show graph, save to file")
parser.add_argument ('--hours', dest = 'hours', type = int, action='store', default = DEFAULT_HOURS,
                     help = 'Show data for last N hours (default: %d)' % DEFAULT_HOURS)

args = parser.parse_args ()

xDateFmt = mdates.DateFormatter ('%d/%m/%Y %H:%M:%S')

db = DB ()
x = []
y = []
yv = []
ydht = []
ydallas = []
ydallas2 = []
ybmp = []
ypx = []
ypx2 = []
yh = []
ylux = []
ylux2 = []
yscale = []	#yldr
yldr2 = []
now = datetime.datetime.now ()
limit = now - datetime.timedelta (hours = args.hours)
prev = None
for row in db.get_data_since (limit):
	dt = row["date"]
	if prev is not None and dt - prev > datetime.timedelta (hours = 2):
		x.append (dt - datetime.timedelta (hours = 1))
		ydht.append (float ('nan'))
		ydallas.append (float ('nan'))
		ydallas2.append (float ('nan'))
		ybmp.append (float ('nan'))
		ypx2.append (float ('nan'))
		yh.append (float ('nan'))
		ylux.append (float ('nan'))
		ylux2.append (float ('nan'))
		yscale.append (float ('nan'))
		yldr2.append (float ('nan'))
		ypx.append (float ('nan'))
	prev = dt

	x.append (row["date"])

	# Temps
	ydht.append (row["temp_dht"])
	ydallas.append (row["temp_dallas"])
	ydallas2.append (row["temp_dallas_2"])
	ybmp.append (row["temp_bmp"])

	# Hum
	yh.append (row["hum"])

	# Light
	ylux.append (row["light_lux"])
	#~ ylux2.append (row["light_lux_ir"])
	yscale.append (row["light_v"])
	#~ yldr2.append (row["light_lux_2"])	# Column name is misleading!
	yv.append (row["light_v"])

	# Pressure
	ypx.append (row["local_px"])
	#~ ypx2.append (row["local_px_2"])

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
for celsius, humidity in zip (ydallas, yh):
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
#~ plt.plot (x, ydht, "r--", label = "DHT11")
plt.plot (x, ydallas, "b-", label = "Temperature")
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
#~ plt.xlabel ('Time')
plt.ylabel ('Percentage')
plt.grid (True)
plt.gca ().xaxis.set_major_formatter (xDateFmt)


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
else:
	plt.show ()

sys.exit (0)
