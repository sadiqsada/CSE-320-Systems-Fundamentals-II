#include "protocol.h"
#include "csapp.h"

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
    ssize_t readHeaderBytes = rio_readn(fd, hdr, sizeof(CHLA_PACKET_HEADER));

    if (readHeaderBytes == -1)
        return -1;

    if (hdr->payload_length != 0)
    {
        uint32_t newPayloadLength = ntohl(hdr->payload_length);
        ssize_t readPayloadBytes = rio_readn(fd, payload, newPayloadLength);

        if (readPayloadBytes == -1)
        {
            return -1;
        }
    }
    return 0;
}