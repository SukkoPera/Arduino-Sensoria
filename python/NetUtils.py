#!/usr/bin/env python

import struct
import socket

class NetUtils:
	@staticmethod
	def getLocalIp ():
		# http://stackoverflow.com/questions/166506/finding-local-ip-addresses-using-pythons-stdlib
		return ([l for l in ([ip for ip in socket.gethostbyname_ex(socket.gethostname())[2] if not ip.startswith("127.")][:1], [[(s.connect(('8.8.8.8', 80)), s.getsockname()[0], s.close()) for s in [socket.socket(socket.AF_INET, socket.SOCK_DGRAM)]][0][1]]) if l][0][0])

	@staticmethod
	def ip2int (addr):
		return struct.unpack ("!I", socket.inet_aton (addr))[0]

	@staticmethod
	def int2ip (addr):
		return socket.inet_ntoa(struct.pack ("!I", addr))

	@staticmethod
	def calculateNet (ip, mask):
		network = ip & mask
		broadcast = network | (~mask & 0xFFFFFFFF)	# Don't exceed 32 bits
		firstaddr = network + 1
		lastaddr = broadcast - 1
		return network, broadcast, firstaddr, lastaddr

	@staticmethod
	def calculateNetFromStr (ipstr, maskstr):
		ip = NetUtils.ip2int (ipstr)
		mask = NetUtils.ip2int (maskstr)
		return NetUtils.calculateNet (ip, mask)

	@staticmethod
	def getNetworkIPs (ip, mask):
		network, broadcast, firstaddr, lastaddr = NetUtils.calculateNet (ip, mask)
		for ip in range (firstaddr, lastaddr):
			yield ip

	@staticmethod
	def getNetworkIPsFromStr (ipstr, maskstr):
		ip = NetUtils.ip2int (ipstr)
		mask = NetUtils.ip2int (maskstr)
		return NetUtils.getNetworkIPs (ip, mask)



#network, broadcast, firstaddr, lastaddr = NetUtils.calculateNetFromStr (net, mask)
#print "Network: %s" % NetUtils.int2ip (network)
#print "Broadcast: %s" % NetUtils.int2ip (broadcast)
#print "Hosts: %s-%s" % (NetUtils.int2ip (firstaddr), NetUtils.int2ip (lastaddr))
if __name__ == "__main__":
	net = "192.168.1.0"
	mask = "255.255.255.0"

	for ip in NetUtils.getNetworkIPsFromStr (net, mask):
		print NetUtils.int2ip (ip)
