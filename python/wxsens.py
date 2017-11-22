#!/usr/bin/python

import ConfigParser
import sys

import wx, wx.html

import Sensoria

from ObjectListView import GroupListView, ColumnDefn

class Config (object):
	def __init__ (self):
		self.winPos = None
		self.winSize = None

class TransducerWrapper (object):
	def __init__ (self, t):
		self.transducer = t
		self.lastRead = None

	def update (self):
		self.lastRead = self.transducer.read ()

	def write (self, what):
		if self.transducer.genre == Sensoria.ACTUATOR:
			return self.transducer.write (what)

	@property
	def name (self):
		return self.transducer.name

	@property
	def description (self):
		return self.transducer.description
		return self.transducer.name

	@property
	def genre (self):
		return self.transducer.genre

	@property
	def stereotype (self):
		return self.transducer.stereotype

class TransducerList (object):
	def __init__ (self, sensoria):
		self._sensoria = sensoria
		self.transducers = [TransducerWrapper (t) for t in self._sensoria.transducers.values ()]

	def massUpdate (self):
		for t in self.transducers:
			t.update ()


aboutText = """<p>A quick'n'easy way to create remote sensors and actuators<br/><br/>
Project on <a href="https://github.com/SukkoPera/Arduino-Sensoria">github</a></p>"""

class HtmlWindow(wx.html.HtmlWindow):
	def __init__(self, parent, id, size=(600,400)):
		wx.html.HtmlWindow.__init__(self,parent, id, size=size)
		if "gtk2" in wx.PlatformInfo:
			self.SetStandardFonts()

	def OnLinkClicked(self, link):
		wx.LaunchDefaultBrowser(link.GetHref())

class AboutBox(wx.Dialog):
	def __init__(self):
		wx.Dialog.__init__(self, None, -1, "About Sensoria",
			style=wx.DEFAULT_DIALOG_STYLE|wx.THICK_FRAME|wx.RESIZE_BORDER|
				wx.TAB_TRAVERSAL)
		hwin = HtmlWindow(self, -1, size=(400,200))
		vers = {}
		vers["python"] = sys.version.split()[0]
		vers["wxpy"] = wx.VERSION_STRING
		hwin.SetPage(aboutText % vers)
		btn = hwin.FindWindowById(wx.ID_OK)
		irep = hwin.GetInternalRepresentation()
		hwin.SetSize((irep.GetWidth()+25, irep.GetHeight()+10))
		self.SetClientSize(hwin.GetSize())
		self.CentreOnParent(wx.BOTH)
		self.SetFocus()

class PopupMenuTransducer (wx.Menu):
	def __init__ (self, transducer, addSeparator = False):
		super (PopupMenuTransducer, self).__init__ ()
		self.transducer = transducer

		item = wx.MenuItem (self, wx.ID_REFRESH, "&Update")
		self.AppendItem (item)
		self.Bind (wx.EVT_MENU, self.onUpdate, item)

		if addSeparator:
			self.AppendSeparator ()

	def onUpdate (self, event):
		print "Shall update %s" % self.transducer.name
		self.transducer.update ()

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
		if transducer.lastRead is not None:
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

# Map stereotypes to popup menus
stereoTypeToMenu = {
	"RS": PopupMenuActuatorRS,
	"CR": PopupMenuActuatorCR
}

class Frame (wx.Frame):
	def __init__(self):
		self.config = Config ()
		self._readConfig ()

		super (Frame, self).__init__ (None, title = "Sensoria", pos = self.config.winPos, size = self.config.winSize)
		self.Bind (wx.EVT_CLOSE, self.onQuit)

		menuBar = wx.MenuBar ()
		menu = wx.Menu ()
		m_exit = menu.Append (wx.ID_EXIT, "E&xit\tAlt-X", "Close window and exit program.")
		self.Bind (wx.EVT_MENU, self.onQuit, m_exit)
		menuBar.Append (menu, "&File")
		menu = wx.Menu ()
		m_about = menu.Append (wx.ID_ABOUT, "&About", "Information about this program")
		self.Bind (wx.EVT_MENU, self.onAbout, m_about)
		menuBar.Append (menu, "&Help")
		self.SetMenuBar (menuBar)

		self.statusbar = self.CreateStatusBar ()

		panel = wx.Panel (self)
		box = wx.BoxSizer (wx.VERTICAL)
		self._lc = GroupListView (panel, wx.ID_ANY, style = wx.LC_REPORT | wx.SUNKEN_BORDER, useAlternateBackColors = False)
		self._lc.showItemCounts = False

		self._lc.SetEmptyListMsg ("No transducers found")
		self._lc.SetColumns ([
			ColumnDefn ("Description", "left", 220, "description"),
			ColumnDefn ("Name", "center", 50, "name"),
			# This col is only used for grouping, use 0 width and make entries empty to hide it
			ColumnDefn ("Genre", "center", 50, "genre", maximumWidth = 0, stringConverter = lambda g: "", groupKeyConverter = lambda g: "Sensors" if g == Sensoria.SENSOR else "Actuators"),
			ColumnDefn ("Stereo", "center", 50, "stereotype"),
			ColumnDefn ("Reading", "right", 100, "lastRead", isSpaceFilling = True),
		])
		self._lc.SetShowGroups = True
		self._lc.alwaysGroupByColumnIndex = 3			# col 0 is expansion icon
		self._lc.SetSortColumn (self._lc.columns[1])

		self._lc.Bind (wx.EVT_LIST_ITEM_ACTIVATED, self.onItemDoubleClicked)
		self._lc.Bind (wx.EVT_LIST_ITEM_RIGHT_CLICK, self.onItemRightClicked)
		box.Add (self._lc, 1, wx.ALL | wx.EXPAND)

		panel.SetSizer(box)
		panel.Layout()

		#~ self._sensoria = Sensoria.Client (servers = ["localhost"], autodiscover = False)
		self._sensoria = Sensoria.Client ()
		self.transducerList = TransducerList (self._sensoria)
		self._lc.SetObjects (self.transducerList.transducers)

		self.lastRead = {}
		self.timer = wx.Timer (self)
		self.Bind (wx.EVT_TIMER, self.update, self.timer)
		self.timer.Start (30000)

	def update (self, event):
		print "Updating"
		self.transducerList.massUpdate ()
		self._lc.RefreshObjects (self.transducerList.transducers)

	def onItemDoubleClicked (self, event):
		t = self._lc.GetSelectedObject ()
		if t is not None:
			t.update ()
			self._lc.RefreshObjects (self.transducerList.transducers)

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

	def onAbout (self, event):
		dlg = AboutBox ()
		dlg.ShowModal ()
		dlg.Destroy ()

	def onQuit (self, event):
		self.config.winPos = self.GetPosition ()
		self.config.winSize = self.GetSize ()
		#~ print "Window is %ux%u at %u,%u" % (self.config.winPos[0], self.config.winPos[1], self.config.winSize[0], self.config.winSize[1])
		self._saveConfig ()
		self.Destroy ()

	def _readConfig (self):
		cfgp = ConfigParser.RawConfigParser ()
		cfgp.read ('/tmp/cfg.ini')
		try:
			self.config.winPos = (cfgp.getint ('Window', 'x'), cfgp.getint ('Window', 'y'))
			self.config.winSize = (cfgp.getint ('Window', 'width'), cfgp.getint ('Window', 'height'))
		except ConfigParser.Error as ex:
			# Never mind, we'll just use defaults
			pass

	def _saveConfig (self):
		cfgp = ConfigParser.RawConfigParser ()
		cfgp.add_section ('Window')
		cfgp.set ('Window', 'x', self.config.winPos[0])
		cfgp.set ('Window', 'y', self.config.winPos[1])
		cfgp.set ('Window', 'width', self.config.winSize[0])
		cfgp.set ('Window', 'height', self.config.winSize[1])
		with open ('/tmp/cfg.ini', 'wb') as configfile:
			cfgp.write (configfile)


app = wx.App ()   # Error messages go to popup window
top = Frame ()
top.Show ()
app.MainLoop ()
