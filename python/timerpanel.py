#!/usr/bin/env python

import datetime
import copy

import wx
import wx.grid

import Sensoria
from Sensoria.stereotypes.TimeControlData import TimeControlData

class TimerPanel (wx.grid.Grid):
	NHOURS = 24
	SLOTS_PER_HOUR = 1
	NSLOTS = NHOURS * SLOTS_PER_HOUR
	NDAYS = 7
	DAY_NAMES = ["Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"]
	COLORS = [wx.NamedColour ("white"), wx.NamedColour ("yellow"), wx.NamedColour ("orange"), wx.NamedColour ("red")]

	def __init__ (self, parent):
		super (TimerPanel, self).__init__ (parent)

		self._schedule = [[[0 for i in range (TimerPanel.SLOTS_PER_HOUR)] for i in range (TimerPanel.NHOURS)] for i in range (TimerPanel.NDAYS)]

		# GUI stuff
		self.CreateGrid (TimerPanel.NDAYS, TimerPanel.NSLOTS)
		self.EnableEditing (False)
		self.SetCellHighlightPenWidth (0)		# Disable cursor
		t = datetime.datetime (1900, 1, 1, 0, 0)
		for i in xrange (0, TimerPanel.NSLOTS):
			self.SetColLabelValue (i, t.strftime ("%H"))
			t += datetime.timedelta (minutes = 60 / TimerPanel.SLOTS_PER_HOUR)
		for n, day in enumerate (TimerPanel.DAY_NAMES):
			self.SetRowLabelValue (n, day)
		self.AutoSizeLabels ()
		self.AutoSize ()
		self.DisableDragRowSize ()
		self.DisableDragColSize ()

		self.Bind (wx.grid.EVT_GRID_CELL_LEFT_CLICK, self.onCellLeftClick)
		self.Bind (wx.grid.EVT_GRID_CELL_RIGHT_CLICK, self.onCellRightClick)

	@property
	def schedule (self):
		return self._schedule

	@schedule.setter
	def schedule (self, s):
		self._schedule = copy.deepcopy (s)		# Copy, don't modify original

		# Update view
		for i in xrange (0, TimerPanel.NDAYS):
			for j in xrange (0, TimerPanel.NHOURS):
				t = self._schedule[i][j]
				assert len (t) == TimerPanel.SLOTS_PER_HOUR
				for k in xrange (0, TimerPanel.SLOTS_PER_HOUR):
					v = t[k]
					assert v >= 0 and v < len (TimerPanel.COLORS)
					self.SetCellBackgroundColour (i, j + k, self.COLORS[v])
		self.Refresh ()

	def AutoSizeLabels (self):
		# Common setup.

		devContext = wx.ScreenDC()
		devContext.SetFont(self.GetLabelFont())

		# First do row labels.

		maxWidth = 0
		curRow = self.GetNumberRows() - 1
		while curRow >= 0:
			curWidth = devContext.GetTextExtent("M%s"%(self.GetRowLabelValue(curRow)))[0]
			if curWidth > maxWidth:
					maxWidth = curWidth
			curRow = curRow - 1
		self.SetRowLabelSize(maxWidth)

		# Then column labels.

		maxHeight = 0
		curCol = self.GetNumberCols() - 1
		while curCol >= 0:
			(w,h,d,l) = devContext.GetFullTextExtent(self.GetColLabelValue(curCol))
			curHeight = h + d + l + 4
			if curHeight > maxHeight:
					maxHeight = curHeight
			curCol = curCol - 1
		self.SetColLabelSize(maxHeight)

	def onCellLeftClick (self, evt):
		r = evt.GetRow ()
		c = evt.GetCol ()
		#~ print "OnCellLeftClick: (%d,%d) %s" % (r, c, evt.GetPosition())

		old = self._schedule[r][c][0]
		new = (old + 1) % len (TimerPanel.COLORS)
		self._schedule[r][c][0] = new

		self.SetCellBackgroundColour (r, c, self.COLORS[new])
		self.Refresh ()
		self.ClearSelection ()
		#~ evt.Veto ()


	def onCellRightClick (self, evt):
		r = evt.GetRow ()
		c = evt.GetCol ()
		#~ print "onCellRightClick: (%d,%d) %s" % (r, c, evt.GetPosition())

		old = self.schedule[r][c][0]
		new = (old + len (TimerPanel.COLORS) - 1) % len (TimerPanel.COLORS)
		self.schedule[r][c][0] = new

		self.SetCellBackgroundColour (r, c, self.COLORS[new])
		self.Refresh ()
		self.ClearSelection ()
		#~ evt.Veto ()
		#~ evt.Skip ()

class TimerEditDialog (wx.Dialog):
	def __init__(self, t):
		super (TimerEditDialog, self).__init__ (None, title = "Edit Timer %s" % t.name, style = wx.DEFAULT_DIALOG_STYLE | wx.THICK_FRAME | wx.TAB_TRAVERSAL)

		self.transducer = t

		sizer = wx.BoxSizer (wx.VERTICAL)

		self._tPanel = TimerPanel (self)
		self._tPanel.schedule = self.transducer.lastRead.schedule
		sizer.Add (self._tPanel, 0, wx.EXPAND | wx.ALL, 30)

		btnBox = wx.BoxSizer (wx.HORIZONTAL)
		btnCancel = wx.Button (self, wx.ID_CANCEL, '&Cancel')		# Using ID_CANCEL automatically closes dialog
		btnBox.Add (btnCancel, 0)
		btnOk = wx.Button (self, wx.ID_OK, '&OK')
		btnBox.Add (btnOk, 0)
		btnOk.SetDefault ()
		btnOk.Bind (wx.EVT_BUTTON, self.onOk)
		sizer.Add (btnBox, 0, wx.ALIGN_RIGHT | wx.RIGHT | wx.BOTTOM, 10)

		# Making the dialog fixed-size is pretty tedious...
		self.SetSizerAndFit (sizer)
		self.SetSize (self._tPanel.GetSize ())
		#~ self.SetMinSize ((500, 200))
		self.Fit ()
		sz = self.GetSize ()
		self.SetSizeHints (minW = sz.GetWidth (), minH = sz.GetHeight (), maxW = sz.GetWidth (), maxH = sz.GetHeight ())

		self.CentreOnParent (wx.BOTH)
		self.SetFocus ()

	def onOk (self, event):
		print "Saving Timer Data"
		tc = TimeControlData ()
		tc.schedule = self._tPanel.schedule
		try:
			self.transducer.write (tc)
		except Sensoria.Error as ex:
			wx.MessageBox ("Unable to set timer schedule: %s" % str (ex), "Error")
		self.Close ()
		event.Skip ()
