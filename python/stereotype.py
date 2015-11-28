class StereoType (object):
	# Please set this to a unique 2-char string
	_IDSTR = None
	
	@classmethod
	def getIdString (cls):
		return cls._IDSTR
