#!/usr/bin/env python

import time
import datetime
import logging

import Sensoria

class MyAutodiscoveryHandler (Sensoria.AutodiscoveryHandler):
	# ~ def __init__ (self, frame, transducerList):
		# ~ self._transducerList = transducerList

	def onAutodiscoveryStarted (self):
		print ("Autodiscovery started")

	def onTransducersAdded (self, ts):
		for t in ts:
			print "NEW: %s" % t.name

	def onTransducersRemoved (self, ts):
		for t in ts:
			print "DEL: %s" % t.name

def onChangeNotification (t, data):
	print "[%s] %s temp = %.2f, hum = %u%%" % (datetime.datetime.now (), t.name, data.temperature, data.humidity)
	return True


# ~ logging.basicConfig (level = logging.INFO)
logging.basicConfig ()

sensoria = Sensoria.Client ()
# ~ adHandler = MyAutodiscoveryHandler ()
# ~ sensoria.registerHandler (adHandler)
sensoria.enableNotifications ()
# ~ sensoria.enableAutodiscovery ()
sensoria.discover ()


time.sleep (1)

try:
	ow = sensoria.transducers["OW"]
	if not ow.notify (onChangeNotification, Sensoria.PERIODIC, 15):
		print "Enable notification failed"

	# ~ time.sleep (10)
	raw_input ("*** Press Enter to quit ***\n")
	ow.stopNotify (Sensoria.PERIODIC)
except Exception as ex:
	print str (ex)
	raise
