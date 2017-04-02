from Sensoria.StereoType import StereoType

class ControlledRelayData (StereoType):
	OFF, ON = xrange (0, 2)
	AUTO, MANUAL = xrange (0, 2)
	UNKNOWN = -1

	_IDSTR = "CR"

	def __init__ (self):
		self.state = ControlledRelayData.UNKNOWN
		self.controller = ControlledRelayData.AUTO

	def __eq__ (self, other):
		return other is not None and \
			self.state == other.state and \
			self.controller == other.controller

	def unmarshal (self, string):
		string = string.upper ()
		d = dict ([p.split (":") for p in string.split (" ")])
		if "S" in d and "C" in d:
			s = d["S"]
			if s == "ON":
				self.state = ControlledRelayData.ON
			elif s == "OFF":
				self.state = ControlledRelayData.OFF
			else:
				self.state = ControlledRelayData.UNKNOWN
			#~ print self.state

			c = d["C"]
			if c == "AUT":
				self.controller = ControlledRelayData.AUTO
			elif c == "MAN":
				self.controller = ControlledRelayData.MANUAL
			else:
				self.controller = ControlledRelayData.UNKNOWN
			#~ print self.controller

			return True
		else:
			return False

	def marshal (self):
		if self.state == ControlledRelayData.ON:
			val = "S:ON"
		elif self.state == ControlledRelayData.OFF:
		    val = "S:OFF"
		else:
			val = "S:UNK"

		val += " "

		if self.controller == ControlledRelayData.AUTO:
			val += "C:AUT"
		elif self.controller == ControlledRelayData.MANUAL:
			val += "C:MAN"
		else:
			val += "C:UNK"

		return val

	# Nice :)
	def __repr__ (self):
		return self.marshal ()
