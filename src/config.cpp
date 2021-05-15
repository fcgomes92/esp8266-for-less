#include "config.h"

void logConfig(Config *config)
{
    LOG("### ENABLE PORTAL: " + (String)config->enablePortal);
    LOG("### BROKER HOST: " + (String)config->host);
    LOG("### PORT: " + (String)config->port);
    LOG("### TOPIC NAME: " + (String)config->topicName);
    LOG("### TOPIC PATH: " + (String)config->topicPath);
    LOG("### Size: " + (String)sizeof(*config));
}
