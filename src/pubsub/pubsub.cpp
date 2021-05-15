#include "pubsub.h"

void publish(PubSubClient *client, Config *config, char *payload)
{
    client->publish(String((String)config->topicPath + "/status").c_str(), payload);
    client->publish(String((String)config->topicPath + "/" + (String)config->topicName + "/status").c_str(), payload);
}

void subscribe(PubSubClient *client, Config *config)
{
    client->subscribe(String((String)config->topicPath).c_str());
    client->subscribe(String((String)config->topicPath + "/" + (String)config->topicName).c_str());
}