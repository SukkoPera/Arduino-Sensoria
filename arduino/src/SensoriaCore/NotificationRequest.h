#include <Sensoria.h>
#include <Arduino.h>
#include <IPAddress.h>
#include "Transducer.h"
#include "common.h"

#ifdef ENABLE_NOTIFICATIONS

class NotificationRequest {
public:
	IPAddress destAddr;
	word destPort;
	NotificationType type;
	Transducer* transducer;

	unsigned long period;	// ms

	/* NT_PRD: millis() when notification was last sent
	 * NT_CHA: millis() when transducer was last polled
	 */
	unsigned long timeLastSent;

	char lastReading[SENSOR_BUF_SIZE];
};

#endif
