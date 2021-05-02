#include "protocol.h"
#include "csapp.h"
#include <stdlib.h>

int proto_send_packet(int fd, CHLA_PACKET_HEADER *hdr, void *payload)
{
    // convert to network byte order
    uint32_t newPayloadLength = ntohl(hdr->payload_length);

    ssize_t writtenHeaderBytes = rio_writen(fd, hdr, sizeof(CHLA_PACKET_HEADER));

    if (writtenHeaderBytes == -1)
        return -1;

    if (newPayloadLength != 0)
    {
        ssize_t writtenPayloadBytes = rio_writen(fd, payload, newPayloadLength);

        if (writtenPayloadBytes == -1)
        {
            return -1;
        }
    }
    return 0;
}

int proto_recv_packet(int fd, CHLA_PACKET_HEADER *hdr, void **payload)
{
    // read header using rio_readn library func
    ssize_t readHeaderBytes = rio_readn(fd, (void *)hdr, sizeof(CHLA_PACKET_HEADER));

    if (readHeaderBytes <= 0)
        return -1;

    uint32_t newPayloadLength = ntohl(hdr->payload_length);
    // if payload length is nonzero, read in payloadd
    if (newPayloadLength != 0)
    {
        *payload = malloc(newPayloadLength);
        ssize_t readPayloadBytes = rio_readn(fd, *payload, newPayloadLength);

        if (readPayloadBytes <= 0)
        {
            return -1;
        }
    }
    return 0;
}
