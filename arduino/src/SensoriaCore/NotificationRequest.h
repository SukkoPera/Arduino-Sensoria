#include <Sensoria.h>
#include <Arduino.h>
#include <IPAddress.h>
#include "Transducer.h"
#include "common.h"

#ifdef ENABLE_NOTIFICATIONS

class NotificationRequest {
public:
	SensoriaAddress* destAddr;
	NotificationType type;
	Transducer* transducer;
	byte ttl;

	unsigned long period;	// ms

	/* NT_PRD: millis() when notification was last sent
	 * NT_CHA: millis() when transducer was last polled
	 */
	unsigned long timeLastSent;

	NotificationRequest& operator= (const NotificationRequest& other) {
		destAddr = other.destAddr;
		type = other.type;
		transducer = other.transducer;
		period = other.period;
		timeLastSent = other.timeLastSent;

		return *this;
	}
};

#endif
