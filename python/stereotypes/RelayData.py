from stereotype import StereoType

class RelayData (StereoType):
	_IDSTR = "RS"
	
	def __init__ (self, state):
		self.state = state

	def pack (self):
		if self.state.upper () == "ON" or int (self.state) > 0:
			val = "ON"
		else:
		    val = "OFF"
		return val
