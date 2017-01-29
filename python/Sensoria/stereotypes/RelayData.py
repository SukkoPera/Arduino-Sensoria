from Sensoria.StereoType import StereoType

class RelayData (StereoType):
	OFF, ON, UNKNOWN = xrange (0, 3)

	_IDSTR = "RS"

	def __init__ (self):
		self.state = RelayData.UNKNOWN

	def marshal (self):
		if self.state == RelayData.ON:
			val = "ON"
		elif self.state == RelayData.OFF:
		    val = "OFF"
		else:
			val = "UNKNOWN"
		return val

	def unmarshal (self, string):
		string = string.upper ()
		if string == "ON":
			self.state = RelayData.ON
		elif string == "OFF":
			self.state = RelayData.OFF
		else:
			self.state = RelayData.UNKNOWN
		return True

	# Nice :)
	def __repr__ (self):
		return self.marshal ()
