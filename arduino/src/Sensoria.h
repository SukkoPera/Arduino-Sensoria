#ifndef SENSORIA_H_INCLUDED
#define SENSORIA_H_INCLUDED

// This is just a placeholder header

#define DEFAULT_PORT 9999

#define DEFAULT_BROADCAST_PORT DEFAULT_PORT

#define DEFAULT_NOTIFICATION_PORT 9998

#define MAX_SERVERS 4

#define MAX_TRANSDUCERS 8

#define MAX_NOTIFICATION_REQS 2

#define MAX_SERVER_NAME (16 + 1)

#define MAX_TRANSDUCER_NAME (2 + 1)

#define MAX_TRANSDUCER_DESC (32 + 1)

#define MAX_TRANSDUCER_VER (8 + 1)

#define MAX_FAILURES 3

// ms
#define CLIENT_TIMEOUT 5000

// ms
#define DISCOVERY_INTERVAL 10000

// ms
#define DISCOVERY_TIMEOUT 1500

/* When checking for if transducer readings have changed for notification
 * purposes, only poll at this interval (ms)
 */
#define NOTIFICATION_POLL_INTERVAL 1000

enum NotificationType {
  NT_CHA,     // On change
  NT_PRD,     // Periodic

  NT_UNK      // Used for error reporting
};

#endif
