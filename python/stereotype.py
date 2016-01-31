class StereoType (object):
	# Please set this to a unique 2-char string
	_IDSTR = None

	def __repr__ (self):
		ret = {}
		for attr in filter (lambda x: not x.startswith ("_"), dir (self)):
			a = getattr (self, attr)
			if a is not None and not callable (a):
				ret[attr] = a
		return " ".join ("%s:%s" % (k, v) for k, v in ret.iteritems ())

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
		wd = cls ()
		wd.unmarshal (output)
		return wd
