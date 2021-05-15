#ifndef config_h
#define config_h

#include "env.cpp"

typedef struct
{
    char *topicName = STR_VALUE(APP_TOPIC_NAME);
    char *topicPath = STR_VALUE(APP_TOPIC_PATH);
    char *host = STR_VALUE(APP_HOST);
    int port = APP_PORT;
    bool enablePortal = APP_ENABLE_PORTAL;
    bool outlet0State = false;
    bool outlet1State = false;
    char *otaPassword = STR_VALUE(APP_OTA_PASSWORD);
} Config;

void logConfig(Config *config);
#endif