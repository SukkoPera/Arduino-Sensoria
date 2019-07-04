#!/usr/bin/env python

import server
import time

from Sensoria.stereotypes.InstantMessageData import InstantMessageData


PUSHBULLET_API_KEY = ""
PUSHBULLET_DEV_IDEN = ""

# Import smtplib for the actual sending function
import smtplib

# Import the email modules we'll need
from email.mime.text import MIMEText

class EMailNotifier (server.Actuator):
	def __init__ (self, name, description = "", sender = "sensoria@localhost", server = "localhost"):
		super (EMailNotifier, self).__init__ (name, InstantMessageData.getIdString (), description)
		self.sender = sender
		self.server = server

	def write (self, data):
		assert isinstance (data, InstantMessageData)
		msg = MIMEText (data.body)
		msg['To'] = data.recipient
		if data.subject is not None:
			msg['Subject'] = data.subject
		msg['From'] = self.sender

		# Send the message via our own SMTP server, but don't include the
		# envelope header.
		s = smtplib.SMTP (self.server)
		s.sendmail (self.sender, [data.recipient], msg.as_string ())
		s.quit ()

		return True, "Notification Sent"

	# Read is not supported

################################################################################

import requests

class PushBulletClient (object):
	BASE_URL = "https://api.pushbullet.com/v2/pushes"

	def __init__ (self, _key, _dev):
		self.key = _key
		self.dev = _dev

	def sendNotification (self, title, body):
		params = {
			"title": title,
			"body": body,
			"type": "note",
			"device_iden": self.dev
		}

		headers = {
			"Access-Token": self.key
		}
		response = requests.post (PushBulletClient.BASE_URL, json = params, headers = headers)

		return response.status_code >= 200 and response.status_code < 300

class SmartphoneNotifier (server.Actuator):
	def __init__ (self, name, description, apiKey, devIden):
		super (SmartphoneNotifier, self).__init__ (name, InstantMessageData.getIdString (), description)
		self._pb = PushBulletClient (apiKey, devIden)

	def write (self, data):
		# Recipient is not supported, FIXME somehow
		assert isinstance (data, InstantMessageData)
		if self._pb.sendNotification (data.subject, data.body):
			ret = True, "Notification Sent"
		else:
			ret = False, "Notification failed"

		return ret

	# Read is not supported

import daemon

with daemon.DaemonContext ():
	st = server.Clock ("$T", "System Clock")
	se = EMailNotifier ("$E", "System E-Mail Notifier")
	sn = SmartphoneNotifier ("$N", "System Smartphone Notifier", PUSHBULLET_API_KEY, PUSHBULLET_DEV_IDEN)
	listener = server.CommandListener ("SensoriaSystem")
	listener.register_sensor (st)
	listener.register_sensor (se)
	listener.register_sensor (sn)
	listener.start ()

	while True:
		time.sleep (1)
