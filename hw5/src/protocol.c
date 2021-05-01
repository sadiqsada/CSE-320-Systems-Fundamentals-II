#include "protocol.h"
#include "csapp.h"

int proto_send_packet(int fd, CHLA_PACKET_HEADER *hdr, void *payload)
{
    // convert to network byte order
    uint32_t newPayloadLength = htonl(hdr->payload_length);
    uint32_t newMsgId = htonl(hdr->msgid);
    uint32_t newTimestampSec = htonl(hdr->timestamp_sec);
    uint32_t newTimestampNSec = htonl(hdr->timestamp_nsec);

    hdr->payload_length = newPayloadLength;
    hdr->msgid = newMsgId;
    hdr->timestamp_sec = newTimestampSec;
    hdr->timestamp_nsec = newTimestampNSec;

    ssize_t writtenHeaderBytes = rio_writen(fd, hdr, sizeof(hdr));

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
    return 0;
}