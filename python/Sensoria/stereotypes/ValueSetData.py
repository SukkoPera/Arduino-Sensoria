from Sensoria.StereoType import StereoType

"""
EXAMPLE:

V0:10 V1:18 V3:Stringa
"""
class ValueSetData (StereoType):
	UNKNOWN = None
	NVALUES = 10

	_IDSTR = "VS"

	# By titan (Slightly modified)
	# https://stackoverflow.com/questions/18092354/python-split-string-without-splitting-escaped-character#21107911
	@staticmethod
	def escape_split (s, delim = " "):
		i, res, buf = 0, [], ''
		while True:
			j, e = s.find(delim, i), 0
			if j < 0:  # end reached
				return res + [buf + s[i:]]  # add remainder
			while j - e and s[j - e - 1] == '\\':
				e += 1  # number of escapes
			d = e // 2  # number of double escapes
			if e != d * 2:  # odd number of escapes
				buf += s[i:j - d - 1] + s[j]  # add the escaped char
				i = j + 1  # and skip it
				continue  # add more to buf
			res.append(buf + s[i:j])
			i, buf = j + len(delim), ''  # start after delim

	def __init__ (self):
		self.values = [ValueSetData.UNKNOWN] * ValueSetData.NVALUES

	def __eq__ (self, other):
		return isinstance (other, self.__class__) and \
			self.values == other.values

	def unmarshal (self, string):
		ret = True
		parts = ValueSetData.escape_split (string, " ")
		d = dict ([p.split (":") for p in parts])
		for k, v in d.iteritems ():
			if k[0].upper () == "V" and len (k) == 2:
				lev = int (k[1])
				if lev >= 0 and lev <= ValueSetData.NVALUES:
					v = v.decode ("string_escape")		# Escape backslashes
					self.values[lev] = v		 # Always treat as string
			else:
				print "Bad key: %s" % k
				ret = False
		return ret

	def marshal (self):
		val = ""
		for i, v in enumerate (self.values):
			if v is not None:
				if type (v) is str or type (v) is unicode:
					v = str (self.values[i])			# No Unicode on the line!
					v = v.encode ("string_escape")		# Backslash-escape characters that need it
					v = v.replace (" ", "\\ ")			# Additionally escape spaces
				else:
					v = str (self.values[i])
				val += "V%u:%s " % (i, v)
		return val[:-1]

	# Nice :)
	def __repr__ (self):
		return self.marshal ()

if __name__ == "__main__":
	s = r"V0:10 V1:1.8\ \\ V3:String V4:String\ with\ spaces\ and\ \\\ backslash\ \\"

	vs = ValueSetData ()
	vs.unmarshal (s)
	for i in xrange (0, ValueSetData.NVALUES):
		if vs.values[i] is not None:
			print "- Value %u: %s" % (i, str (vs.values[i]))
	#~ print str (vs)
	s2 = vs.marshal ()
	assert s == s2, "Marshal/Unmarshal mismatch:\n'%s'\n'%s'" % (s, s2)
