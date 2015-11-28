#!/usr/bin/env python

import time
import os
import sys
import smtplib

# Here are the email package modules we'll need
from email.mime.image import MIMEImage
from email.mime.multipart import MIMEMultipart

COMMASPACE = ', '

while True:
	print "GO!"

	#~ ret = os.system ("./plot2.py")
	ret = os.system ("./plotall.py -o /tmp/fullgraph.png")
	if ret != 0:
		sys.exit (1)

	# Create the container (outer) email message.
	msg = MIMEMultipart()
	msg['Subject'] = 'GRAPHs'
	# me == the sender's email address
	# family = the list of all recipients' email addresses
	msg['From'] = "sukkopera@sukkology.net"
	msg['To'] = "sukkopera@sukkology.net"
	msg.preamble = 'Our family reunion'

	# Assume we know that the image files are all in PNG format
	#~ pngfiles = ["/tmp/graph1.png", "/tmp/graph2.png"]
	pngfiles = ["/tmp/fullgraph.png"]
	for file in pngfiles:
		# Open the files in binary mode.  Let the MIMEImage class automatically
		# guess the specific image type.
		fp = open(file, 'rb')
		img = MIMEImage(fp.read())
		fp.close()
		msg.attach(img)

		attempts = 0
		done = False
		while attempts < 3 and not done:
			try:
				attempts += 1
				s = smtplib.SMTP ('mail.sukkology.net', 25, timeout = 10)
				s.set_debuglevel (True)
				s.starttls ()
				s.login ("sukkology.net", "Faith&Devotion")
				s.sendmail ("sukko@sukkology.net", "sukkopera@gmail.com", msg.as_string())
				s.quit ()
				done = True
			except Exception, ex:
				print "Attempt %d failed: %s" % (attempts, str (ex))
		if not done:
			print "Too many failed attempts, aborting"

	print "WAIT!"
	time.sleep (60 * 60)
