#include <Sensoria.h>
#include <SensoriaCore/Communicator.h>
#include <SensoriaCore/common.h>
#include <SensoriaCore/utils.h>
#include <SensoriaCore/debug.h>

class ByteAddress: public SensoriaAddress {
public:
	byte addr;		// 0 = Broadcast, 1-255 = Valid client addresses
	boolean inUse;

	char* toString (char* buf, byte size) const override {
		if (size >= 4)
			utoa ((unsigned int) addr, buf, 10);
		else
			buf[0] = '\0';
		return buf;
	}

protected:
	virtual bool equalTo (const SensoriaAddress& otherBase) const override {
		const ByteAddress& other = static_cast<const ByteAddress&> (otherBase);
		return addr == other.addr;
	}

	virtual void clone (const SensoriaAddress& otherBase) override {
		const ByteAddress& other = static_cast<const ByteAddress&> (otherBase);
		addr = other.addr;
	}

	// Default copy/assignment operators should be fine
};

/******************************************************************************/


class SensoriaSerialCommunicator: public SensoriaCommunicator {
private:
	static const byte N_ADDRESSES = 6;
	ByteAddress addressPool[N_ADDRESSES];

	static const byte BUF_SIZE = 64;

  Stream *serial;

  byte myAddr;

  static const byte BROADCAST_ADDRESS = 0;

  char *readSerialString () {
    static char buf[BUF_SIZE];
    static int i = 0;

    char *ret = NULL;

    while (serial -> available ()) {
        char c = serial -> read ();
        switch (c) {
          case '\r':
            // Ignore
            break;
          case '\n':
            // End of string, process
            buf[i] = '\0';	// This will always be possible
            ret = buf;
            i = 0;
            break;
          case -1:
            // No char available to read
            break;
          default:
            // Got new char, append
            if (i < BUF_SIZE - 1)
							buf[i++] = c;
            break;
      }
    }

    return ret;
  }

	boolean receiveGeneric (char*& str, byte& senderAddr, byte& destAddr) {
		// Assume we'll receive nothing
		boolean ret = false;

		if ((str = readSerialString ())) {
			char *p[3];
			if (splitString (str, p, 3) == 3) {
				int n = atoi (p[0]);
				if (n > 0 && n <= 255) {		// Sender cannot be broadcast address
					senderAddr = n;

					n = atoi (p[1]);
					if (n >= 0 && n <= 255) {
						destAddr = n;

						str = p[2];

#if 0
						DPRINT (F("Received packet of size "));
						DPRINT (packetSize);
						DPRINT (F(" from "));
						DPRINT (senderAddr);
						DPRINT (F(" to "));
						DPRINT (destAddr);
						DPRINT (F(": \""));
						DPRINT (str);
						DPRINTLN (F("\""));
#endif

						ret = true;
					} else {
						DPRINTLN (F("Invalid destination address"));
					}
				} else {
					DPRINTLN (F("Invalid source address"));
				}
			} else {
				DPRINTLN (F("Received malformed message"));
			}
		}

		return ret;
	}

	boolean sendGeneric (const char *str, byte destAddr) {
		return serial -> print (myAddr) && serial -> print (' ') &&
		    serial -> print (destAddr) && serial -> print (' ') &&
		    serial -> print (str);
	}

public:
  SensoriaSerialCommunicator () {
  }

  bool begin (Stream& _serial, byte addr) {
		serial = &_serial;
		myAddr = addr;

		return myAddr != BROADCAST_ADDRESS;
  }

	virtual SensoriaAddress* getAddress () override {
		SensoriaAddress* ret = NULL;

#if 0
		byte cnt = 0;
		for (byte i = 0; i < N_ADDRESSES && !ret; i++) {
			if (!addressPool[i].inUse)
				++cnt;
		}
		DPRINT (F("Addresses not in use: "));
		DPRINTLN (cnt);
#endif

		for (byte i = 0; i < N_ADDRESSES && !ret; i++) {
			if (!addressPool[i].inUse) {
				addressPool[i].inUse = true;
				ret = &(addressPool[i]);
			}
		}

		return ret;
	}

	virtual void releaseAddress (SensoriaAddress* addr) override {
		for (byte i = 0; i < N_ADDRESSES; i++) {
			if (&(addressPool[i]) == addr) {
				addressPool[i].inUse = false;
			}
		}
	}

#ifdef ENABLE_NOTIFICATIONS
  virtual SensoriaAddress* getNotificationAddress (const SensoriaAddress* client) {
		ByteAddress* bAddr = reinterpret_cast<ByteAddress*> (getAddress ());
		if (bAddr) {
			const ByteAddress& clientBAddr = *reinterpret_cast<const ByteAddress*> (client);

			bAddr -> addr = clientBAddr.addr;
		}

		return bAddr;
	}
#endif

	virtual boolean receiveCmd (char*& cmd, SensoriaAddress* client) override {
		ByteAddress& bAddr = *const_cast<ByteAddress*> (reinterpret_cast<const ByteAddress*> (client));

		byte destAddr;
		return receiveGeneric (cmd, bAddr.addr, destAddr) && bAddr.addr != myAddr &&
		       (destAddr == myAddr || destAddr == BROADCAST_ADDRESS);
	}

	virtual SendResult reply (const char* reply, const SensoriaAddress* client) override {
		ByteAddress& bAddr = *const_cast<ByteAddress*> (reinterpret_cast<const ByteAddress*> (client));
		return sendGeneric (reply, bAddr.addr) ? SEND_OK : SEND_ERR;
	}

	virtual boolean notify (const char* notification, const SensoriaAddress* client) override {
		ByteAddress& bAddr = *const_cast<ByteAddress*> (reinterpret_cast<const ByteAddress*> (client));
		return sendGeneric (notification, bAddr.addr) ? SEND_OK : SEND_ERR;
	}

	SendResult sendCmd (const char* cmd, const SensoriaAddress* server, char*& reply) override {
		ByteAddress& bAddr = *const_cast<ByteAddress*> (reinterpret_cast<const ByteAddress*> (server));

		SendResult res = sendGeneric (cmd, bAddr.addr) ? SEND_OK : SEND_ERR;
		if (res > 0) {
			unsigned long start = millis ();

			while (millis () - start < CLIENT_TIMEOUT) {
				byte srcAddr, destAddr;
				if (receiveGeneric (reply, srcAddr, destAddr) &&
				    srcAddr == bAddr.addr && destAddr == myAddr) {
					// Got something
					break;
				}
			}

			if (millis () - start >= CLIENT_TIMEOUT)
				res = SEND_TIMEOUT;
		}

		return res;
	}

	virtual SendResult broadcast (const char* cmd) override {
		(void) cmd;

		return SEND_ERR;
	}

  virtual boolean receiveBroadcastReply (char*& reply, SensoriaAddress*& sender, unsigned int timeout) override {
		(void) reply;
		(void) sender;
		(void) timeout;

		return false;
	}

#ifdef ENABLE_NOTIFICATIONS
	virtual boolean receiveNotification (char*& notification) override {
		(void) notification;

		return false;
	}
#endif
};
