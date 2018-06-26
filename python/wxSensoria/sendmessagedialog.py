#!/usr/bin/env python

import copy

import wx
import wx.grid

import Sensoria
from Sensoria.stereotypes.InstantMessageData import InstantMessageData

class SendMessageDialog (wx.Dialog):
	def __init__(self, t):
		super (SendMessageDialog, self).__init__ (None, title = "Send Message", style = wx.DEFAULT_DIALOG_STYLE | wx.TAB_TRAVERSAL)

		self.transducer = t

		sizer = wx.BoxSizer (wx.VERTICAL)

		gs = wx.FlexGridSizer (rows = 3, cols = 2, vgap = 3, hgap = 5)
		gs.Add (wx.StaticText (self, -1, 'Recipient:'), proportion = 1, flag = wx.ALIGN_CENTER_VERTICAL)
		self._tc_recipient = wx.TextCtrl (self, -1)
		self._tc_recipient.Bind (wx.EVT_TEXT, self.onTextEdit)
		gs.Add (self._tc_recipient, 1, wx.EXPAND)
		gs.Add (wx.StaticText (self, -1, 'Subject:'), 1, wx.ALIGN_CENTER_VERTICAL)
		self._tc_subject = wx.TextCtrl (self, -1)
		self._tc_subject.Bind (wx.EVT_TEXT, self.onTextEdit)
		gs.Add (self._tc_subject, 1, wx.EXPAND)
		gs.Add (wx.StaticText (self, -1, 'Text:'), 2, wx.ALIGN_CENTER_VERTICAL)
		self._tc_body = wx.TextCtrl (self, -1)
		self._tc_body.Bind (wx.EVT_TEXT, self.onTextEdit)
		gs.Add (self._tc_body, 1, wx.EXPAND)
		gs.AddGrowableRow (idx = 2, proportion = 1)

		gs.AddGrowableCol (1, 1)
		sizer.Add (gs, 1, wx.EXPAND | wx.ALL, 30)

		btnBox = wx.BoxSizer (wx.HORIZONTAL)
		self._btnCancel = wx.Button (self, wx.ID_CANCEL, '&Cancel')		# Using ID_CANCEL automatically closes dialog
		# ~ self._btnCancel.SetDefault ()
		btnBox.Add (self._btnCancel, 0)
		self._btnOk = wx.Button (self, wx.ID_OK, '&OK')
		btnBox.Add (self._btnOk, 0)
		self._btnOk.Bind (wx.EVT_BUTTON, self.onOk)
		sizer.Add (btnBox, 0, wx.ALIGN_RIGHT | wx.RIGHT | wx.BOTTOM, 10)

		# Update OK button status for inital conditions
		self.onTextEdit (None)

		# ~ w = wx.DisplaySize ()[0] / 3
		self.SetSizer (sizer)
		# ~ self.SetMinSize ((w, w))
		# ~ self.Fit ()
		# ~ sz = self.GetSize ()
		# ~ self.SetSizeHints (minW = sz.GetWidth (), minH = sz.GetHeight ())

		self.CentreOnParent (wx.BOTH)
		self.SetFocus ()

	def onTextEdit (self, event):
		if not self._tc_recipient.IsEmpty () and \
			not self._tc_subject.IsEmpty () and \
			not self._tc_body.IsEmpty ():
			self._btnOk.Enable ()
			self._btnOk.SetDefault ()
		else:
			self._btnOk.Disable ()
			self._btnCancel.SetDefault ()

	def onOk (self, event):
		print "Sending Message"
		im = InstantMessageData ()
		im.recipient = self._tc_recipient.GetValue ()
		im.subject = self._tc_subject.GetValue ()
		im.body = self._tc_body.GetValue ()

		try:
			self.transducer.write (im)
		except Sensoria.Error as ex:
			reason = str (ex)
			if reason is not None:
				wx.MessageBox ("Unable to send message\n\n%s" % reason, "Error", wx.ICON_ERROR)
			else:
				wx.MessageBox ("Unable to send message", "Error", wx.ICON_ERROR)

		self.Close ()
		event.Skip ()
