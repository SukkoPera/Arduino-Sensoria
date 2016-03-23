class StereoType (object):
	# Please set this to a unique 2-char string
	_IDSTR = None

	def getData (self):
		ret = {}
		for attr in filter (lambda x: not x.startswith ("_"), dir (self)):
			a = getattr (self, attr)
			if a is not None and not callable (a):
				ret[attr] = a
		return ret

	@classmethod
	def getIdString (cls):
		return cls._IDSTR

	@classmethod
	def unmarshalStatic (cls, output):
		st = cls ()
		st.unmarshal (output)
		return st

	def unmarshal (self, string):
		raise NotImplementedError

	def marshal (self):
		raise NotImplementedError
