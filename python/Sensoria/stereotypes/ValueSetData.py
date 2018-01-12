from Sensoria.StereoType import StereoType

"""
EXAMPLE:

V0:10 V1:18 V3:Stringa
"""
class ValueSetData (StereoType):
	UNKNOWN = None
	NVALUES = 10

	_IDSTR = "VS"

	def __init__ (self):
		self.values = [ValueSetData.UNKNOWN] * ValueSetData.NVALUES

	def __eq__ (self, other):
		return isinstance (other, self.__class__) and \
			self.values == other.values

	def unmarshal (self, string):
		ret = True
		d = dict ([p.split (":") for p in string.split (" ")])

		for k, v in d.iteritems ():
			if k[0].upper () == "V" and len (k) == 2:
				lev = int (k[1])
				if lev >= 0 and lev <= ValueSetData.NVALUES:
					self.values[lev] = v		 # Always treat as string
			else:
				print "Bad key: %s" % k
		return ret

	def marshal (self):
		val = ""

		for i in xrange (0, ValueSetData.NVALUES):
			if self.values[i] is not None:
				val += "V%u:%s " % (i, str (self.values[i]))

		return val.rstrip ()

	# Nice :)
	def __repr__ (self):
		return self.marshal ()

if __name__ == "__main__":
	s = "V0:10 V1:18 V3:Stringa"

	vs = ValueSetData ()
	vs.unmarshal (s)
	for i in xrange (0, ValueSetData.NVALUES):
		if vs.values[i] is not None:
			print "- Value %u: %s" % (i, str (vs.values[i]))
	#~ print str (vs)
	s2 = vs.marshal ()
	assert s == s2, "Marshal/Unmarshal mismatch:\n%s\n%s" % (s, s2)
