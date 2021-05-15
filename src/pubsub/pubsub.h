#ifndef pubsub_h
#define pubsub_h

#include <PubSubClient.h>
#include "config.h"

void publish(PubSubClient *client, Config *config, char *payload);

void subscribe(PubSubClient *client, Config *config);

#endif