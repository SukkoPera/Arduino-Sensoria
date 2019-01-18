#!/usr/bin/env python

import copy

import wx
import wx.grid

import Sensoria
from Sensoria.stereotypes.ValueSetData import ValueSetData

class SettingsEditDialog (wx.Dialog):
	NSETTINGS = ValueSetData.NVALUES

	def __init__(self, t):
		super (SettingsEditDialog, self).__init__ (None, title = "Edit Settings for %s" % t.name, style = wx.DEFAULT_DIALOG_STYLE | wx.TAB_TRAVERSAL)

		self.transducer = t

		sizer = wx.BoxSizer (wx.VERTICAL)

		self._textCtrls = []
		gs = wx.FlexGridSizer (SettingsEditDialog.NSETTINGS, 2, 10, 5)
		for i in xrange (0, SettingsEditDialog.NSETTINGS):
			if t.lastReadRaw is not None and i < len (t.lastReadRaw.values) and t.lastReadRaw.values[i]:
				s = str (t.lastReadRaw.values[i])
			else:
				s = ""

			tc = wx.TextCtrl (self, -1, s)
			self._textCtrls.append (tc)

			gs.Add (wx.StaticText (self, -1, 'Value %u:' % i), 0, wx.ALIGN_CENTER_VERTICAL)
			gs.Add (tc, 1, wx.EXPAND)

		gs.AddGrowableCol (1, 1)
		sizer.Add (gs, 0, wx.EXPAND | wx.ALL, 30)

		btnBox = wx.BoxSizer (wx.HORIZONTAL)
		btnCancel = wx.Button (self, wx.ID_CANCEL, '&Cancel')		# Using ID_CANCEL automatically closes dialog
		btnBox.Add (btnCancel, 0)
		btnOk = wx.Button (self, wx.ID_OK, '&OK')
		btnBox.Add (btnOk, 0)
		btnOk.SetDefault ()
		btnOk.Bind (wx.EVT_BUTTON, self.onOk)
		sizer.Add (btnBox, 0, wx.ALIGN_RIGHT | wx.RIGHT | wx.BOTTOM, 10)

		w = wx.DisplaySize ()[0] / 3
		self.SetSizer (sizer)
		self.SetMinSize ((w, 200))
		self.Fit ()
		sz = self.GetSize ()
		self.SetSizeHints (minW = sz.GetWidth (), minH = sz.GetHeight ())

		self.CentreOnParent (wx.BOTH)
		self.SetFocus ()

	def onOk (self, event):
		print "Saving Settings"
		vs = ValueSetData ()
		for i, tc in enumerate (self._textCtrls):
			v = tc.GetValue ()
			# FIXME: How to tell if this should be None or an empty string?
			if len (v) > 0:
				vs.values[i] = v

		try:
			self.transducer.write (vs)
		except Sensoria.Error as ex:
			reason = str (ex)
			if reason is not None:
				wx.MessageBox ("Unable to save settings\n\n%s" % reason, "Error", wx.ICON_ERROR)
			else:
				wx.MessageBox ("Unable to save settings", "Error", wx.ICON_ERROR)
		self.Close ()
		event.Skip ()
