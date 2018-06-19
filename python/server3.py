#!/usr/bin/env python

import server
import time

from Sensoria.stereotypes.TimeControlData import TimeControlData
from Sensoria.stereotypes.InstantMessageData import InstantMessageData

class TemperatureSensor (server.TemperatureSensor):
	def __init__ (self):
		super (TemperatureSensor, self).__init__ ("HD", "Heater Temperature")

class HeaterController (server.ControlledRelayActuator):
	def __init__ (self):
		super (HeaterController, self).__init__ ("HC", "Heater Controller")

#TL1:10 TL2:18 TL3:21
class HeaterTimer (server.TimedActuator):
	def __init__ (self):
		super (HeaterTimer, self).__init__ ("HT", "Heater Timer")
		initData = TimeControlData ()
		initData.unmarshal ("PMO:000000001000000003222110 PTU:000000001000000003222110 PWE:000000001000000003222110 PTH:000000001000000003222110 PFR:000000001000000003222111 PSA:000000000322222222222211 PSU:000000000322222222222210")
		ok, msg = self.write (initData)
		print msg
		assert ok

class HeaterSettings (server.ValueSetActuator):
	def __init__ (self):
		super (HeaterSettings, self).__init__ ("HS", "Heater Settings")
		self.levels = [10, 18, 21]

	@property
	def values (self):
		return self.levels

	@values.setter
	def values (self, v):
		self.levels = v[0:3]

################################################################################


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


hd = TemperatureSensor ()
hc = HeaterController ()
ht = HeaterTimer ()
hs = HeaterSettings ()
ck = server.Clock ("$T", "System Clock")
nt = EMailNotifier ("$N", "E-Mail Notifier")
listener = server.CommandListener ("HeatingSystem")
listener.register_sensor (hd)
listener.register_sensor (hc)
listener.register_sensor (ht)
listener.register_sensor (hs)
listener.register_sensor (ck)
listener.register_sensor (nt)
listener.start ()

while True:
	time.sleep (1)
