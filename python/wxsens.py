#!/usr/bin/python

import ConfigParser
import os
import sys
import logging
import datetime
import threading

import wx
from timerpanel import TimerEditDialog
from settingspanel import SettingsEditDialog

import Sensoria

from ObjectListView import ObjectListView, GroupListView, ColumnDefn

# For Python 2.6
def timedelta_total_seconds (timedelta):
	return (timedelta.microseconds + 0.0 + \
		(timedelta.seconds + timedelta.days * 24 * 3600) * 10 ** 6) / 10 ** 6

class Config (object):
	FILENAME = "wxsens.ini"
	SERVER_LIST_SEPARATOR = ','

	def __init__ (self):
		self.winPos = None
		self.winSize = None
		self.transducerUpdateInterval = 5	# s
		self.servers = []
		self.formatStrings = {}
		self.viewDetails = False
		self.groupByGenre = True

	def getFilename (self):
		if "XDG_CONFIG_HOME" in os.environ and len (os.environ["XDG_CONFIG_HOME"]) > 0:
			path = os.environ["XDG_CONFIG_HOME"]
		else:
			path = os.path.expanduser ("~/.config")
		return os.path.join (path, Config.FILENAME)

	def load (self):
		cfgp = ConfigParser.RawConfigParser ()
		cfgp.read (self.getFilename ())
		try:
			self.winPos = (cfgp.getint ('Window', 'x'), cfgp.getint ('Window', 'y'))
			self.winSize = (cfgp.getint ('Window', 'width'), cfgp.getint ('Window', 'height'))
			self.transducerUpdateInterval = cfgp.getint ('Transducers', 'UpdateInterval')
			self.servers = cfgp.get ('Network', 'Servers').split (Config.SERVER_LIST_SEPARATOR)
			for tname, fmt in cfgp.items ('Format'):
				tname = tname.upper ()		# ConfigParser lowercases everything
				self.formatStrings[tname] = fmt
			self.viewDetails = cfgp.getboolean ('View', 'Details')
			self.groupByGenre = cfgp.getboolean ('View', 'Group')
		except ConfigParser.Error as ex:
			# Never mind, we'll just use defaults
			pass

	def save (self):
		cfgp = ConfigParser.RawConfigParser ()
		cfgp.add_section ('Window')
		cfgp.set ('Window', 'x', self.winPos[0])
		cfgp.set ('Window', 'y', self.winPos[1])
		cfgp.set ('Window', 'width', self.winSize[0])
		cfgp.set ('Window', 'height', self.winSize[1])

		cfgp.add_section ('Transducers')
		cfgp.set ('Transducers', 'UpdateInterval', self.transducerUpdateInterval)

		cfgp.add_section ('Network')
		cfgp.set ('Network', 'Servers', Config.SERVER_LIST_SEPARATOR.join (self.servers))

		cfgp.add_section ('Format')
		for tname, fmt in self.formatStrings.iteritems ():
			cfgp.set ('Format', tname, fmt)

		cfgp.add_section ('View')
		cfgp.set ('View', 'Details', self.viewDetails)
		cfgp.set ('View', 'Group', self.groupByGenre)

		# Note that ConfigParser does not support Unicode strings, at least not in 2.6
		with open (self.getFilename (), 'wb') as configfile:
			cfgp.write (configfile)

class TransducerWrapper (object):
	def __init__ (self, sensoria, transducer):
		self._logger = logging.getLogger ('TransducerWrapper')
		self.sensoria = sensoria
		self.transducer = transducer
		self._lastRead = None				# Stereotype from last successful read
		self.updateTime = None				# Time of last successful read
		self.failed = True					# True if last read failed
		self.failMessage = "Not yet read"	# Contains error message if last read failed
		self.lastAttemptTime = None			# Time of last read attempt, same as updateTime if it succeeded
		self.outputFormat = None

	def __repr__ (self):
		return "<TransducerWrapper '%s'>" % self.transducer.name

	def update (self):
		self.lastAttemptTime = datetime.datetime.now ()
		try:
			self._lastRead = self.transducer.read ()
			self.updateTime = self.lastAttemptTime
			self.failed = False
			self.failMessage = None
		except Sensoria.Error as ex:
			self.failed = True
			self.failMessage = str (ex) if len (str (ex)) > 0 else "Unknown error"
			self._logger.error ("Read failed - " + self.failMessage)
		# ~ except KeyError as ex:
			# ~ self.failed = True
			# ~ self.failMessage = "No such transducer: %s" % self.name
			# ~ self._logger.error ("Read failed - " + self.failMessage)

	def enableChangeNotification (self):
		if not self.transducer.notify (self.onChangeNotification, Sensoria.ON_CHANGE):
			print "Enable notification failed"

	def disableChangeNotification (self):
		if not self.transducer.stopNotify (Sensoria.ON_CHANGE):
			print "Disable notification failed"

	def disableAllNotifications (self):
		if not self.transducer.cancelAllNotifications ():
			print "Disable all notifications failed"

	def onChangeNotification (self, data):
		print "Received notification for transducer %s: %s" % (self.name, str (data))
		self._lastRead = data
		self.updateTime = datetime.datetime.now ()

	@property
	def smartName (self):
		return self.transducer.description if self.transducer.description else self.transducer.name

	@property
	def name (self):
		return self.transducer.name

	@property
	def description (self):
		return self.transducer.description

	@property
	def genre (self):
		return self.transducer.genre

	@property
	def stereotype (self):
		return self.transducer.stereotype

	@property
	def version (self):
		return self.transducer.version

	@property
	def server (self):
		return self.transducer.server

	@property
	def lastRead (self):
		"Returns the result of the last reading in a somehow \"smart\" way"
		if self.failed:
			return "ERROR: %s" % self.failMessage
		elif self.outputFormat is not None:
			return self.formatReading (self.outputFormat)
		else:
			return self._lastRead

	@property
	def lastReadRaw (self):
		return self._lastRead

	def write (self, what):
		if self.transducer.genre == Sensoria.ACTUATOR:
			return self.transducer.write (what)
		else:
			return False

	def getValueForFormatWD (self, c):
		if self._lastRead is not None:
			d = {
				"t": self._lastRead.temperature,
				"h": self._lastRead.humidity,
				"l": self._lastRead.localPressure,
				"p": self._lastRead.seaPressure,
				"a": self._lastRead.altitude,
				"x": self._lastRead.lightLux,
				"g": self._lastRead.light10bit
			}

			try:
				return str (d[c])
			except KeyError:
				return None
		else:
			return None

	# Delegates formatting to strftime()
	def getValueForFormatDT (self, c):
		if self._lastRead and self._lastRead.datetime:
			return self._lastRead.datetime.strftime ("%%%c" % c)
		else:
			return None

	def getValueForFormat (self, c):
		d = {
			"WD": self.getValueForFormatWD,
			"DT": self.getValueForFormatDT
		}

		try:
			fn = d[self.transducer.stereotype]
			assert callable (fn)
			return fn (c)
		except KeyError:
			return None

	def formatReading (self, fmt):
		i = 0
		while True:
			try:
				i = fmt.index ('%', i)
				if i + 1 < len (fmt):
					c = fmt[i + 1]
					if c == '%':
						rep = "%"
					else:
						rep = self.getValueForFormat (c)
						if rep is None:
							# Unsupported format character, output as-is
							rep = "%%%c" % c

					fmt = fmt[:i] + rep + fmt[i+2:]
					i += len (rep)
				else:
					# String ends with a single '%', go ahead
					break
			except ValueError as ex:
				break
		return fmt

class TransducerList (object):
	def __init__ (self, sensoria, config):
		self._logger = logging.getLogger ('TransducerList')
		self.sensoria = sensoria
		self._config = config
		self._transducers = []
		self._changed = True
		self._lock = threading.RLock ()

	@property
	def transducers (self):
		self._lock.acquire ()
		ret = self._transducers[:]
		self._changed = False
		self._lock.release ()
		return ret

	# Let's support the iterator protocol!
	# See https://docs.python.org/2/library/stdtypes.html#iterator-types
	def __iter__ (self):
		return self._transducers.__iter__ ()

	@property
	def changed (self):
		return self._changed

	@changed.setter
	def changed (self, x):
		raise NotImplementedError

	# Call this before add()/remove() batches
	def lock (self):
		self._lock.acquire ()

	# Call this after add()/remove() batches
	def unlock (self):
		self._lock.release ()

	# t is a Sensoria.Transducer (not a TransducerWrapper)
	def _contains (self, t):
		have = [tt.name for tt in self._transducers]
		return t.name in have

	def add (self, t):
		if not self._contains (t):
			tw = TransducerWrapper (self.sensoria, t)
			self._transducers.append (tw)
			if t.name in self._config.formatStrings:
				tw.outputFormat = self._config.formatStrings[t.name]
			self._changed = True
		else:
			self._logger.error ("Tried to add already-known transducer: %s", t.name)
			tw = None
		return tw

	def remove (self, t):
		tw = filter (lambda tt: tt.name == t.name, self.transducers)
		if len (tw) == 1:
			self._transducers.remove (tw[0])
			self._changed = True
		else:
			self._logger.error ("Tried to remove non-existent transducer: %s", t.name)

	def massUpdate (self):
		for t in self._transducers:
			t.update ()

class AboutInfo (wx.AboutDialogInfo):
	def __init__ (self):
		super (AboutInfo, self).__init__ ()

		self.Name = "wxSensoria"
		self.Version = "0.0.1 Beta"
		self.Copyright = u"\u00A9 2016-2018 SukkoPera"
		self.Description = ("PC Client for the Sensoria project: A quick'n'easy way to create remote sensors and actuators")
		self.WebSite = ("https://github.com/SukkoPera/Arduino-Sensoria", "Project on GitHub")
		self.Developers = ["SukkoPera <software@sukkology.net>"]
		self.License = "GPLv3+"

class ReadOnlyTextCtrl (wx.TextCtrl):
	def __init__ (self, parent, x, text, style):
		super (ReadOnlyTextCtrl, self).__init__ (parent, x, text, style = style | wx.TE_READONLY)
		#~ self.Bind (wx.EVT_TEXT, self.onText)
		self.Bind (wx.EVT_SET_FOCUS, self.onText)

	def onText (self, event):
		# TODO: Find a way to reliably select all text
		#~ print "!"
		#~ self.WriteText (self.GetValue ())
		#~ self.SetInsertionPointEnd ()
		#~ self.SelectAll ()
		event.Skip ()

class InfoBox (wx.Dialog):
	def __init__(self, t):
		super (InfoBox, self).__init__ (None, -1, "%s %s" % ("Sensor" if t.genre == Sensoria.SENSOR else "Actuator", t.name), style = wx.DEFAULT_DIALOG_STYLE | wx.THICK_FRAME | wx.TAB_TRAVERSAL)
		sizer = wx.BoxSizer (wx.VERTICAL)

		gs = wx.FlexGridSizer (12, 2, 10, 5)
		gs.AddMany ([
			(wx.StaticText (self, -1, 'Name:'), 0, wx.ALIGN_CENTER_VERTICAL),
			(ReadOnlyTextCtrl (self, -1, t.name, style = wx.TE_CENTER), 1, wx.EXPAND),
			(wx.StaticText (self, -1, 'Description:'), 0, wx.ALIGN_CENTER_VERTICAL),
			(ReadOnlyTextCtrl (self, -1, t.description if t.description else "N/A", style = wx.TE_CENTER), 1, wx.EXPAND),
			(wx.StaticText (self, -1, 'Version:'), 0, wx.ALIGN_CENTER_VERTICAL),
			(ReadOnlyTextCtrl (self, -1, t.version if t.version else "N/A", style = wx.TE_CENTER), 1, wx.EXPAND),
			(wx.StaticText (self, -1, 'Stereotype:'), 0, wx.ALIGN_CENTER_VERTICAL),
			(ReadOnlyTextCtrl (self, -1, t.stereotype, style = wx.TE_CENTER), 1, wx.EXPAND),
			(wx.StaticText (self, -1, 'Server Name:'), 0, wx.ALIGN_CENTER_VERTICAL),
			(ReadOnlyTextCtrl (self, -1, "%s" % t.server.name, style = wx.TE_CENTER), 1, wx.EXPAND),
			(wx.StaticText (self, -1, 'Server Address:'), 0, wx.ALIGN_CENTER_VERTICAL),
			(ReadOnlyTextCtrl (self, -1, "%s:%u" % (t.server.address, t.server.port), style = wx.TE_CENTER), 1, wx.EXPAND),
			(wx.StaticText (self, -1, 'Last Reading:'), 0, wx.ALIGN_CENTER_VERTICAL),
			(ReadOnlyTextCtrl (self, -1, "%s" % t.lastRead, style = wx.TE_CENTER), 1, wx.EXPAND),
			(wx.StaticText (self, -1, 'Last Reading (Raw):'), 0, wx.ALIGN_CENTER_VERTICAL),
			(ReadOnlyTextCtrl (self, -1, "%s" % t.lastReadRaw, style = wx.TE_CENTER), 1, wx.EXPAND),
			(wx.StaticText (self, -1, 'Last Update:'), 0, wx.ALIGN_CENTER_VERTICAL),
			(ReadOnlyTextCtrl (self, -1, "%s" % t.updateTime.strftime ("%c") if t.updateTime else "N/A", style = wx.TE_CENTER), 1, wx.EXPAND),
			(wx.StaticText (self, -1, 'Last Read Attempt:'), 0, wx.ALIGN_CENTER_VERTICAL),
			(ReadOnlyTextCtrl (self, -1, "%s" % t.lastAttemptTime.strftime ("%c") if t.lastAttemptTime else "N/A", style = wx.TE_CENTER), 1, wx.EXPAND),
			(wx.StaticText (self, -1, 'Failed:'), 0, wx.ALIGN_CENTER_VERTICAL),
			(ReadOnlyTextCtrl (self, -1, "%s" % t.failed, style = wx.TE_CENTER), 1, wx.EXPAND),
			(wx.StaticText (self, -1, 'Fail Message:'), 0, wx.ALIGN_CENTER_VERTICAL),
			(ReadOnlyTextCtrl (self, -1, "%s" % t.failMessage if t.failMessage else "N/A", style = wx.TE_CENTER), 1, wx.EXPAND)
		])
		gs.AddGrowableCol (1, 1)
		sizer.Add (gs, 0, wx.EXPAND | wx.ALL, 30)

		btn = wx.Button (self, wx.ID_CLOSE, 'Close')
		btn.SetDefault ()
		self.SetEscapeId (wx.ID_CLOSE)
		sizer.Add (btn, 0, wx.ALIGN_RIGHT | wx.RIGHT | wx.BOTTOM, 10)

		# Making the dialog fixed-size is pretty tedious...
		w = screenSize = wx.DisplaySize ()[0] / 2
		self.SetSizerAndFit (sizer)
		self.SetMinSize ((w, 200))
		self.Fit ()
		sz = self.GetSize ()
		self.SetSizeHints (minW = sz.GetWidth (), minH = sz.GetHeight (), maxW = sz.GetWidth (), maxH = sz.GetHeight ())

		self.CentreOnParent (wx.BOTH)
		self.SetFocus ()

class ServersBox (wx.Dialog):
	def __init__(self, frame):
		super (ServersBox, self).__init__ (None, -1, "Servers", style = wx.DEFAULT_DIALOG_STYLE | wx.THICK_FRAME | wx.TAB_TRAVERSAL)
		self._frame = frame

		sizer = wx.BoxSizer (wx.VERTICAL)

		self._srvTextCtrl = wx.TextCtrl (self, -1, "", style = wx.TE_MULTILINE)
		self._srvTextCtrl.WriteText ("\n".join (self._frame.config.servers))
		gs = wx.FlexGridSizer (1, 2, 10, 5)
		gs.AddMany ([
			(wx.StaticText (self, -1, "Servers:\n(One per line)"), 0, wx.EXPAND),
			(self._srvTextCtrl, 1, wx.EXPAND)
		])
		gs.AddGrowableCol (1, 1)
		gs.AddGrowableRow (0, 1)
		sizer.Add (gs, 1, wx.EXPAND | wx.ALL, 30)

		btn = wx.Button (self, wx.ID_CLOSE, 'Close')
		sizer.Add (btn, 0, wx.ALIGN_RIGHT | wx.RIGHT | wx.BOTTOM, 10)
		btn.Bind (wx.EVT_BUTTON, self.onClose)

		# Making the dialog fixed-size is pretty tedious...
		self.SetSizer (sizer)
		self.SetMinSize ((500, 300))
		self.Fit ()

		self.CentreOnParent (wx.BOTH)
		self.SetFocus ()

	def onClose (self, event):
		self._frame.config.servers = self._srvTextCtrl.GetValue ().split ()
		self.Close ()

class DialogSetFormat (wx.Dialog):
	def __init__(self, t):
		super (DialogSetFormat, self).__init__ (None, -1, "Set format for transducer %s" % t.name, style = wx.DEFAULT_DIALOG_STYLE | wx.THICK_FRAME | wx.TAB_TRAVERSAL)
		self._transducer = t

		sizer = wx.BoxSizer (wx.VERTICAL)

		self._fmtTextCtrl = wx.TextCtrl (self, -1, "")
		self._fmtTextCtrl.WriteText (t.outputFormat if t.outputFormat else "")
		gs = wx.FlexGridSizer (1, 2, 10, 5)
		gs.AddMany ([
			(wx.StaticText (self, -1, "Output format:"), 0, wx.ALIGN_CENTER_VERTICAL),
			(self._fmtTextCtrl, 1, wx.EXPAND)
		])
		gs.AddGrowableCol (1, 1)
		gs.AddGrowableRow (0, 1)
		sizer.Add (gs, 1, wx.EXPAND | wx.ALL, 30)

		btnBox = wx.BoxSizer (wx.HORIZONTAL)
		btnCancel = wx.Button (self, wx.ID_CANCEL, '&Cancel')
		btnBox.Add (btnCancel, 0)
		btnOk = wx.Button (self, wx.ID_APPLY, '&OK')
		btnBox.Add (btnOk, 0)
		btnOk.SetDefault ()
		btnOk.Bind (wx.EVT_BUTTON, self.onOk)
		sizer.Add (btnBox, 0, wx.ALIGN_RIGHT | wx.RIGHT | wx.BOTTOM, 10)

		# Making the dialog fixed-size is pretty tedious...
		self.SetSizer (sizer)
		self.SetMinSize ((500, 100))
		self.Fit ()

		self.CentreOnParent (wx.BOTH)
		self.SetFocus ()

	def onOk (self, event):
		fmt = self._fmtTextCtrl.GetValue ()
		if fmt and len (fmt) > 0:
			self._transducer.outputFormat = fmt
		else:
			self._transducer.outputFormat = None
		self.Close ()
		event.Skip ()

class PopupMenuTransducer (wx.Menu):
	def __init__ (self, transducer, addSeparator = False):
		super (PopupMenuTransducer, self).__init__ ()
		self.transducer = transducer

		item = wx.MenuItem (self, wx.ID_REFRESH, "&Update")
		self.AppendItem (item)
		self.Bind (wx.EVT_MENU, self.onUpdate, item)

		item = wx.MenuItem (self, wx.ID_EDIT, "&Set Format...\tCtrl+F")
		self.AppendItem (item)
		self.Bind (wx.EVT_MENU, self.onSetFormat, item)

		item = wx.MenuItem (self, wx.ID_COPY, "&Copy")
		self.AppendItem (item)
		self.Bind (wx.EVT_MENU, self.onCopy, item)

		item = wx.MenuItem (self, wx.ID_REDO, "&Notify on Change")
		self.AppendItem (item)
		self.Bind (wx.EVT_MENU, self.onNotifyChange, item)
		
		item = wx.MenuItem (self, wx.ID_UNDO, "Stop N&otifications on Change")
		self.AppendItem (item)
		self.Bind (wx.EVT_MENU, self.onCancelNotifications, item)
		
		item = wx.MenuItem (self, wx.ID_CANCEL, "C&ancel ALL Notifications")
		self.AppendItem (item)
		self.Bind (wx.EVT_MENU, self.onCancelAllNotifications, item)

		if addSeparator:
			self.AppendSeparator ()

	def onUpdate (self, event):
		print "Shall update %s" % self.transducer.name
		self.transducer.update ()

	def onSetFormat (self, event):
		print "Shall set format for %s" % self.transducer.name
		dlg = DialogSetFormat (self.transducer)
		dlg.ShowModal ()
		dlg.Destroy ()

	def onCopy (self, event):
		if wx.TheClipboard.Open ():
			s = str (self.transducer.lastRead)
			print "Copying to clipboard: '%s'" % s
			dataObj = wx.TextDataObject ()
			dataObj.SetText (s)
			wx.TheClipboard.SetData (dataObj)
			#~ wx.TheClipboard.Flush ()
			wx.TheClipboard.Close ()
		else:
			wx.MessageBox ("Unable to open the clipboard", "Error", wx.ICON_ERROR)

	def onNotifyChange (self, event):
		print "Shall get notifications on change of %s" % self.transducer.name
		self.transducer.enableChangeNotification ()

	def onCancelNotifications (self, event):
		print "Shall cancel notifications on change of %s" % self.transducer.name
		self.transducer.disableChangeNotification ()

	def onCancelAllNotifications (self, event):
		print "Shall cancel all notifications from %s" % self.transducer.name
		self.transducer.disableAllNotifications ()

class PopupMenuActuatorRS (PopupMenuTransducer):
	def __init__ (self, transducer):
		super (PopupMenuActuatorRS, self).__init__ (transducer, True)

		# If state is unknown, show all possibilities
		showOn = True
		showOff = True
		if transducer.lastRead is not None and transducer.lastRead.state != Sensoria.stereotypes.RelayData.RelayData.UNKNOWN:
			showOn = transducer.lastRead.state == Sensoria.stereotypes.RelayData.RelayData.OFF
			showOff = not showOn

		if showOn:
			item = wx.MenuItem (self, wx.ID_YES, "Turn O&n")
			self.AppendItem (item)
			self.Bind (wx.EVT_MENU, self.onTurnOn, item)

		if showOff:
			item = wx.MenuItem (self, wx.ID_NO, "Turn O&ff")
			self.AppendItem (item)
			self.Bind (wx.EVT_MENU, self.onTurnOff, item)

	def onTurnOn (self, event):
		print "Shall turn on %s" % self.transducer.name
		d = self.transducer.lastRead
		d.state = Sensoria.stereotypes.RelayData.RelayData.ON
		self.transducer.write (d)

	def onTurnOff (self, event):
		print "Shall turn off %s" % self.transducer.name
		d = self.transducer.lastRead
		d.state = Sensoria.stereotypes.RelayData.RelayData.OFF
		self.transducer.write (d)

class PopupMenuActuatorCR (PopupMenuTransducer):
	def __init__ (self, transducer):
		super (PopupMenuActuatorCR, self).__init__ (transducer, True)

		# If state is unknown, show all possibilities
		showTakeCtrl = True
		showRelCtrl = True
		showOn = True
		showOff = True
		if not transducer.failed and transducer.lastRead is not None:
			if transducer.lastRead.controller == Sensoria.stereotypes.ControlledRelayData.ControlledRelayData.AUTO:
				showTakeCtrl = True
				showRelCtrl = False
				showOn = False
				showOff = False
			elif transducer.lastRead.controller == Sensoria.stereotypes.ControlledRelayData.ControlledRelayData.MANUAL:
				showTakeCtrl = False
				showRelCtrl = True
				if transducer.lastRead.state != Sensoria.stereotypes.RelayData.RelayData.UNKNOWN:
					showOn = transducer.lastRead.state == Sensoria.stereotypes.RelayData.RelayData.OFF
					showOff = not showOn

		if showTakeCtrl:
			item = wx.MenuItem (self, wx.NewId(), "&Take Control")
			self.AppendItem (item)
			self.Bind (wx.EVT_MENU, self.onTakeCtrl, item)

		if showRelCtrl:
			item = wx.MenuItem(self, wx.NewId(), "&Release Control")
			self.AppendItem(item)
			self.Bind(wx.EVT_MENU, self.onReleaseCtrl, item)

		if showOn:
			item = wx.MenuItem (self, wx.NewId(), "Turn O&n")
			self.AppendItem (item)
			self.Bind (wx.EVT_MENU, self.onTurnOn, item)

		if showOff:
			item = wx.MenuItem (self, wx.NewId(), "Turn O&ff")
			self.AppendItem (item)
			self.Bind (wx.EVT_MENU, self.onTurnOff, item)

	def onTakeCtrl (self, event):
		print "Shall take control of %s" % self.transducer.name
		d = self.transducer.lastRead
		d.controller = Sensoria.stereotypes.ControlledRelayData.ControlledRelayData.MANUAL
		self.transducer.write (d)

	def onReleaseCtrl (self, event):
		print "Shall release control of %s" % self.transducer.name
		d = self.transducer.lastRead
		d.controller = Sensoria.stereotypes.ControlledRelayData.ControlledRelayData.AUTO
		self.transducer.write (d)

	def onTurnOn (self, event):
		print "Shall turn on %s" % self.transducer.name
		d = self.transducer.lastRead
		d.state = Sensoria.stereotypes.ControlledRelayData.ControlledRelayData.ON
		self.transducer.write (d)

	def onTurnOff (self, event):
		print "Shall turn off %s" % self.transducer.name
		d = self.transducer.lastRead
		d.state = Sensoria.stereotypes.ControlledRelayData.ControlledRelayData.OFF
		self.transducer.write (d)

class PopupMenuActuatorTC (PopupMenuTransducer):
	def __init__ (self, transducer):
		super (PopupMenuActuatorTC, self).__init__ (transducer, True)

		item = wx.MenuItem (self, wx.ID_PREFERENCES, "&Edit Schedule...")
		self.AppendItem (item)
		self.Bind (wx.EVT_MENU, self.onEdit, item)

	def onEdit (self, event):
		print "Shall edit schedule for %s" % self.transducer.name
		dlg = TimerEditDialog (self.transducer)
		dlg.ShowModal ()
		dlg.Destroy ()

class PopupMenuActuatorVS (PopupMenuTransducer):
	def __init__ (self, transducer):
		super (PopupMenuActuatorVS, self).__init__ (transducer, True)

		item = wx.MenuItem (self, wx.ID_PREFERENCES, "&Edit Settings...")
		self.AppendItem (item)
		self.Bind (wx.EVT_MENU, self.onEdit, item)

	def onEdit (self, event):
		print "Shall edit settings for %s" % self.transducer.name
		dlg = SettingsEditDialog (self.transducer)
		dlg.ShowModal ()
		dlg.Destroy ()

# Map stereotypes to popup menus
stereoTypeToMenu = {
	"RS": PopupMenuActuatorRS,
	"CR": PopupMenuActuatorCR,
	"TC": PopupMenuActuatorTC,
	"VS": PopupMenuActuatorVS
}

class MyAutodiscoveryHandler (Sensoria.AutodiscoveryHandler):
	def __init__ (self, frame, transducerList):
		self._frame = frame
		self._transducerList = transducerList

	def onAutodiscoveryStarted (self):
		self._frame.setStatusBar ("Autodiscovery started")
		self._frame.forceRedraw ()

	def onAutodiscoveryCompleted (self):
		self._frame.setStatusBar ("Autodiscovery complete")
		self._frame.forceRedraw ()

	def onTransducersAdded (self, ts):
		if len (ts) == 1:
			self._frame.setStatusBar ("Found new transducer: %s" % ts[0].name)
		else:
			self._frame.setStatusBar ("Found new transducers: %s" % ", ".join (t.name for t in ts))

		self._transducerList.lock ()
		for t in ts:
			print "NEW: %s" % t.name
			tw = self._transducerList.add (t)
			if tw is not None:
				tw.update ()
		self._transducerList.unlock ()
		self._frame.forceRedraw ()
		# ~ self._frame.forceUpdate ()

	def onTransducersRemoved (self, ts):
		if len (ts) == 1:
			self._frame.setStatusBar ("Lost transducer: %s" % ts[0].name)
		else:
			self._frame.setStatusBar ("Lost transducers: %s" % ", ".join (t.name for t in ts))

		self._transducerList.lock ()
		for t in ts:
			print "DEL: %s" % t.name
			self._transducerList.remove (t)
		self._transducerList.unlock ()
		self._frame.forceRedraw ()

class MenuBar (wx.MenuBar):
	MENUITEM_UPD_5SEC = wx.NewId ()
	MENUITEM_UPD_15SEC = wx.NewId ()
	MENUITEM_UPD_30SEC = wx.NewId ()
	MENUITEM_UPD_1MIN = wx.NewId ()
	MENUITEM_UPD_5MIN = wx.NewId ()
	MENUITEM_FORCE_UPD = wx.ID_REFRESH

	itemIntervalMap = {
		MENUITEM_UPD_5SEC: 5,
		MENUITEM_UPD_15SEC: 15,
		MENUITEM_UPD_30SEC: 30,
		MENUITEM_UPD_1MIN: 60,
		MENUITEM_UPD_5MIN: 60 * 5
	}

	def __init__ (self, frame):
		super (MenuBar, self).__init__ ()
		self._logger = logging.getLogger ('MenuBar')
		self._frame = frame

		m1 = wx.Menu ()
		m11 = m1.Append (wx.ID_ADD, "&Servers\tAlt-S", "Configure manual servers")
		self._frame.Bind (wx.EVT_MENU, self.onServers, m11)
		m12 = m1.AppendSeparator ()
		m13 = m1.Append (wx.ID_EXIT, "E&xit\tAlt-X", "Close window and exit program")
		self._frame.Bind (wx.EVT_MENU, self.onQuit, m13)
		self.Append (m1, "&File")

		m2 = wx.Menu ()
		m21 = m2.Append (MenuBar.MENUITEM_UPD_5SEC, "Every &5 seconds", kind = wx.ITEM_RADIO)
		m22 = m2.Append (MenuBar.MENUITEM_UPD_15SEC, "Every 15 s&econds", kind = wx.ITEM_RADIO)
		m23 = m2.Append (MenuBar.MENUITEM_UPD_30SEC, "Every &30 seconds", kind = wx.ITEM_RADIO)
		m24 = m2.Append (MenuBar.MENUITEM_UPD_1MIN, "Every &1 minute", kind = wx.ITEM_RADIO)
		m25 = m2.Append (MenuBar.MENUITEM_UPD_5MIN, "Every 5 minute&s", kind = wx.ITEM_RADIO)
		for m in [m21, m22, m23, m24, m25]:
			id_ = m.GetId ()
			assert id_ in MenuBar.itemIntervalMap
			t = MenuBar.itemIntervalMap[id_]
			if t == self._frame.config.transducerUpdateInterval:
				m.Check ()
			self._frame.Bind (wx.EVT_MENU, self.onUpdateIntervalChanged, m)
		m26 = m2.AppendSeparator ()
		m27 = m2.Append (MenuBar.MENUITEM_FORCE_UPD, "&Now!\tF5")
		self._frame.Bind (wx.EVT_MENU, self.onForceUpdate, m27)
		self.Append (m2, "&Update")

		m3 = wx.Menu ()
		m31 = m3.Append (wx.NewId (), "Transducer &Details\tCtrl+D", kind = wx.ITEM_CHECK)
		if self._frame.config.viewDetails:
			m31.Check ()
		self._frame.Bind (wx.EVT_MENU, self.onViewDetailsToggle, m31)
		m32 = m3.Append (wx.NewId (), "&Group by Genre\tCtrl+G", kind = wx.ITEM_CHECK)
		if self._frame.config.groupByGenre:
			m32.Check ()
		self._frame.Bind (wx.EVT_MENU, self.onGroupToggle, m32)
		self.Append (m3, "&View")

		menu = wx.Menu ()
		m_about = menu.Append (wx.ID_ABOUT, "&About", "Information about this program")
		self._frame.Bind (wx.EVT_MENU, self.onAbout, m_about)
		self.Append (menu, "&Help")

	def onServers (self, event):
		dlg = ServersBox (self._frame)
		dlg.ShowModal ()
		dlg.Destroy ()

	def onAbout (self, event):
		wx.AboutBox (AboutInfo ())

	def onQuit (self, event):
		wx.CallAfter (self._frame.onQuit, event)

	def onUpdateIntervalChanged (self, event):
		assert event.Id in MenuBar.itemIntervalMap
		t = MenuBar.itemIntervalMap[event.Id]
		self._logger.info ("Setting update interval to %u seconds" % t)
		self._frame.config.transducerUpdateInterval = t

	def onForceUpdate (self, event):
		self._frame.update (force = True)

	def onViewDetailsToggle (self, event):
		self._frame.config.viewDetails = not self._frame.config.viewDetails
		self._frame.forceRedraw ()

	def onGroupToggle (self, event):
		self._frame.config.groupByGenre = not self._frame.config.groupByGenre
		self._frame.forceRedraw ()

class AutoStatusBar (wx.StatusBar):
	DEFAULT_DURATION = 3

	def __init__ (self, parent):
		super (AutoStatusBar, self).__init__ (parent)
		self.msgs = []
		self.curMsg = -1

		self.timer = wx.Timer (self)
		self.Bind (wx.EVT_TIMER, self.refresh, self.timer)
		self.timer.Start (1000)

	def push (self, msg, duration = DEFAULT_DURATION):
		# Tuple items are: message, display duration in seconds, display start datetime
		m = (msg, duration, datetime.datetime.now ())
		self.msgs.append (m)
		wx.CallAfter (self.refresh, None)		# Dummy argument

	def refresh (self, event):
		changed = False
		delenda = []
		for msg in filter (lambda m: m[2] is not None, self.msgs):
			# Message has been displayed, check if it should be removed
			delTime = msg[2] + datetime.timedelta (seconds = msg[1])
			delta = datetime.datetime.now () - delTime
			if timedelta_total_seconds (delta) >= 0:
				delenda.append (msg)
		for msg in delenda:
			self.msgs.remove (msg)
			changed = True

		if len (self.msgs) > 0:
			msg = self.msgs[-1]
			self.SetStatusText (msg[0])
		elif changed:
			self.SetStatusText ("")

class Frame (wx.Frame):
	EVT_SET_FORMAT = wx.NewId ()
	EVT_COPY = wx.NewId ()

	def __init__(self):
		self.config = Config ()
		self.config.load ()

		super (Frame, self).__init__ (None, title = "wxSensoria", pos = self.config.winPos, size = self.config.winSize)
		self.Bind (wx.EVT_CLOSE, self.onQuit)

		menuBar = MenuBar (self)
		self.SetMenuBar (menuBar)

		# Set up accelerator table, used for popup menu shortcuts
		accTable = [
			(wx.ACCEL_CTRL, ord ('C'), Frame.EVT_COPY),
			(wx.ACCEL_CTRL, ord ('F'), Frame.EVT_SET_FORMAT),
		]
		self.Bind (wx.EVT_MENU, self.onCopy, id = Frame.EVT_COPY)
		self.Bind (wx.EVT_MENU, self.onSetFormat, id = Frame.EVT_SET_FORMAT)
		self.SetAcceleratorTable (wx.AcceleratorTable (accTable))

		self._statusBar = AutoStatusBar (self)
		self.SetStatusBar (self._statusBar)

		self._panel = wx.Panel (self)
		self._box = wx.BoxSizer (wx.VERTICAL)
		self._lc = self._makeListView (self._panel)
		self._box.Add (self._lc, 1, wx.ALL | wx.EXPAND)

		self._panel.SetSizer (self._box)
		self._panel.Layout ()

		#~ self._sensoria = Sensoria.Client (servers = ["localhost"], autodiscover = False)
		self._sensoria = Sensoria.Client (autodiscInterval = 60 * 3)
		self._sensoria.enableNotifications ()
		self.transducerList = TransducerList (self._sensoria, self.config)
		self._lc.SetObjects (self.transducerList.transducers)
		self._adHandler = MyAutodiscoveryHandler (self, self.transducerList)
		self._sensoria.registerHandler (self._adHandler)

		self._lastTransducerUpdate = None

		self.currentListViewType = (self.config.viewDetails, self.config.groupByGenre)
		# ~ self._redrawLock = threading.RLock ()
		self._updateLock = threading.RLock ()
		self.timer = wx.Timer (self)
		self.Bind (wx.EVT_TIMER, self.update, self.timer)
		self.timer.Start (1000)

	# These handle accelerator presses that trigger context menu actions
	# I don't really like the code duplication here...
	def onSetFormat (self, event):
		t = self._lc.GetSelectedObject ()
		if t is not None:
			print "Shall set format for %s" % t.name
			dlg = DialogSetFormat (t)
			dlg.ShowModal ()
			dlg.Destroy ()

	def onCopy (self, event):
		t = self._lc.GetSelectedObject ()
		if t is not None:
			if wx.TheClipboard.Open ():
				s = str (t.lastRead)
				print "Copying to clipboard: '%s'" % s
				dataObj = wx.TextDataObject ()
				dataObj.SetText (s)
				wx.TheClipboard.SetData (dataObj)
				#~ wx.TheClipboard.Flush ()
				wx.TheClipboard.Close ()
			else:
				wx.MessageBox ("Unable to open the clipboard", "Error", wx.ICON_ERROR)

	def _makeListView (self, parent):
		if self.config.groupByGenre:
			listView = GroupListView (parent, wx.ID_ANY, style = wx.LC_REPORT | wx.SUNKEN_BORDER, useAlternateBackColors = False)
		else:
			listView = ObjectListView (parent, wx.ID_ANY, style = wx.LC_REPORT | wx.SUNKEN_BORDER, useAlternateBackColors = False)

		listView.rowFormatter = self.rowFormatter
		listView.SetEmptyListMsg ("No transducers found")

		# COLUMNS:
		# Normal: DR
		# Grp: D(G)R
		# Det: DNGSR
		# Det+Grp: DN(G)SR
		cols = ([
			ColumnDefn ("Description", "left", 220, "smartName"),
			ColumnDefn ("Reading", "right", 100, "lastRead", isSpaceFilling = True),
		])

		if self.config.viewDetails:
			cols.insert (1, ColumnDefn ("Name", "center", 50, "name"))
			if self.config.groupByGenre:
				# This col is only used for grouping, use 0 width and make entries empty to hide it
				cols.insert (2, ColumnDefn ("Genre", "center", 0, "genre", maximumWidth = 0, stringConverter = lambda g: "", groupKeyConverter = lambda g: "Sensors" if g == Sensoria.SENSOR else "Actuators"))
			else:
				cols.insert (2, ColumnDefn ("Genre", "center", 100, "genre", stringConverter = lambda g: "Sensor" if g == Sensoria.SENSOR else "Actuator"))
			cols.insert (3, ColumnDefn ("Stereo", "center", 50, "stereotype"))
		elif self.config.groupByGenre:
				# This col is only used for grouping, use 0 width and make entries empty to hide it
				cols.insert (2, ColumnDefn ("Genre", "center", 50, "genre", maximumWidth = 0, stringConverter = lambda g: "", groupKeyConverter = lambda g: "Sensors" if g == Sensoria.SENSOR else "Actuators"))

		listView.SetColumns (cols)

		if self.config.groupByGenre:
			listView.showItemCounts = False
			listView.SetShowGroups = True
			listView.alwaysGroupByColumnIndex = 3			# col 0 is expansion icon
			listView.SetSortColumn (listView.columns[1])
		else:
			listView.SetSortColumn (listView.columns[0])

		listView.Bind (wx.EVT_LIST_ITEM_ACTIVATED, self.onItemDoubleClicked)
		listView.Bind (wx.EVT_LIST_ITEM_RIGHT_CLICK, self.onItemRightClicked)

		return listView

	@staticmethod
	def rowFormatter (listItem, t):
		if t.failed:
			listItem.SetTextColour (wx.RED)

	# This is safe to be called from other threads
	def forceRedraw (self):
		wx.CallAfter (self.redraw, None)	# Dummy argument

	# This is safe to be called from other threads
	def forceUpdate (self):
		wx.CallAfter (self.update, force = True)

	def update (self, event = None, force = False):
		self._updateLock.acquire ()
		if force or self._lastTransducerUpdate is None or timedelta_total_seconds (datetime.datetime.now () - self._lastTransducerUpdate) >= self.config.transducerUpdateInterval:
			print "Updating (%s)" % ("forced" if force else "periodic")
			self.transducerList.massUpdate ()
			self._lastTransducerUpdate = datetime.datetime.now ()
		self._updateLock.release ()
		self.redraw ()

	def redraw (self, event = None):
		# ~ self._redrawLock.acquire ()
		print "Redrawing"

		if self.currentListViewType != (self.config.viewDetails, self.config.groupByGenre):
			print "Changing ListView"
			self._lc = self._makeListView (self._panel)
			self.currentListViewType = (self.config.viewDetails, self.config.groupByGenre)
			self._lc.SetObjects (self.transducerList.transducers)
			self._box.Remove (0)
			self._box.Add (self._lc, 1, wx.ALL | wx.EXPAND)
			self._panel.Layout ()
		elif self.transducerList.changed or self.currentListViewType != (self.config.viewDetails, self.config.groupByGenre):
			print "Transducer list changed"
			self._lc.SetObjects (self.transducerList.transducers)
			#~ self.transducerList.changed = False		# This is done transparently by the above call
			self.currentListViewType = (self.config.viewDetails, self.config.groupByGenre)
		else:
			self._lc.RefreshObjects (self.transducerList.transducers)

		# ~ self._redrawLock.release ()

	# This is safe to be called from other threads
	def setStatusBar (self, msg, duration = None):
		if duration is not None:
			self._statusBar.push (msg, duration)
		else:
			# Use default interval
			self._statusBar.push (msg)


	def onQuit (self, event):
		self.config.winPos = self.GetPosition ()
		self.config.winSize = self.GetSize ()
		#~ print "Window is %ux%u at %u,%u" % (self.config.winPos[0], self.config.winPos[1], self.config.winSize[0], self.config.winSize[1])
		self.config.formatStrings = {}
		for t in self.transducerList:
			if t.outputFormat:
				self.config.formatStrings[t.name] = t.outputFormat
		self.config.save ()
		self.Destroy ()

	def onItemDoubleClicked (self, event):
		t = self._lc.GetSelectedObject ()
		if t is not None:
			dlg = InfoBox (t)
			dlg.ShowModal ()
			dlg.Destroy ()

	def onItemRightClicked (self, event):
		t = self._lc.GetSelectedObject ()
		if t is not None:
			menu = None
			if t.stereotype in stereoTypeToMenu:
				menuClass = stereoTypeToMenu[t.stereotype]
				menu = menuClass (t)
			else:
				menu = PopupMenuTransducer (t)

			if menu is not None:
				self.PopupMenu (menu, event.GetPoint ())
				menu.Destroy ()		# wx.Menu objects need to be explicitly destroyed

logging.basicConfig (level = logging.DEBUG)
app = wx.App ()   # Error messages go to popup window
top = Frame ()
top.Show ()
app.MainLoop ()
