#include <Arduino.h>

#ifndef LOG
#define LOG(...) \
    if (DEBUG)   \
    Serial.println(__VA_ARGS__)
#endif

#define STRINGIZER(arg) #arg
#define STR_VALUE(arg) STRINGIZER(arg)

#ifndef DEBUG
#define DEBUG false
#endif

#ifndef APP_TOPIC_NAME
#define APP_TOPIC_NAME "carlos"
#endif

#ifndef APP_TOPIC_PATH
#define APP_TOPIC_PATH "office/outlets"
#endif

#ifndef APP_HOST
#define APP_HOST "berry.ccs"
#endif

#ifndef APP_PORT
#define APP_PORT 1883
#endif

#ifndef APP_ENABLE_PORTAL
#define APP_ENABLE_PORTAL false
#endif

#ifndef APP_OTA_PASSWORD
#define APP_OTA_PASSWORD ""
#endif
