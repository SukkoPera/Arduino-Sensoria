#include <Sensoria.h>
#include <SensoriaCore/Communicator.h>
#include <SensoriaCore/common.h>

class ByteAddress: public SensoriaAddress {
public:
	uint8_t addr;
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

public:
  SensoriaSerialCommunicator () {
  }

  void begin (Stream& _serial) {
    serial = &_serial;
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

  virtual SensoriaAddress* getNotificationAddress (const SensoriaAddress* client) {
		return NULL;
	}

	// Functions for servers
	virtual boolean receiveCmd (char*& cmd, SensoriaAddress* client) override {
		bool ret = false;
		if ((cmd = readSerialString ())) {
			ret = true;
		}

		return ret;
	}

	virtual SendResult reply (const char* reply, const SensoriaAddress* client) override {
		SendResult res = SEND_ERR;
		if (serial -> print (reply)) {
			res = SEND_OK;
		}

		return res;
	}

	virtual boolean notify (const char* notification, const SensoriaAddress* client) override {
		return false;
	}

	SendResult sendCmd (const char* cmd, const SensoriaAddress* server, char*& reply) override {
		SendResult res = SEND_ERR;
		if (serial -> print (cmd)) {
			res = SEND_OK;
		}

		return res;
	}

	virtual SendResult broadcast (const char* cmd) override {
		return SEND_ERR;
	}

  virtual boolean receiveBroadcastReply (char*& reply, SensoriaAddress*& sender, unsigned int timeout) override {
		return false;
	}

	virtual boolean receiveNotification (char*& notification) override {
		return false;
	}
};
