from Sensoria.StereoType import StereoType

class MotionData (StereoType):
	"""
		Stereotype: MD (Motion Detection Data)

		Output is either MOTION or NO_MOTION with obvious meaning.
	"""

	_IDSTR = "MD"

	def __init__ (self):
		self.motionDetected = None

	def __eq__ (self, other):
		return isinstance (other, self.__class__) and \
			self.motionDetected == other.motionDetected

	def marshal (self):
		if self.motionDetected is None:
			val = "UNKNOWN"
		elif self.motionDetected:
			val = "MOTION"
		else:
		    val = "NO_MOTION"
		return val

	def unmarshal (self, string):
		string = string.upper ()
		if string == "MOTION":
			self.motionDetected = True
		elif string == "NO_MOTION":
			self.motionDetected = False
		else:
			self.motionDetected = None
		return True

	# Nice :)
	def __repr__ (self):
		return self.marshal ()
