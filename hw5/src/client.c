#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#include "csapp.h"
#include "client.h"
#include "globals.h"

static sem_t staticMutex;
static pthread_once_t once = PTHREAD_ONCE_INIT;

typedef struct client_registry
{
    CLIENT_NODE *head;
} CLIENT_REGISTRY;

typedef struct client_node
{
    CLIENT *client;
    struct client_node *next;
} CLIENT_NODE;

// definition of Client
typedef struct client
{
    int fd;
    int refCount;
    USER *currentUser;
    MAILBOX *currentMailbox;
    sem_t mutex;
} CLIENT;

void initialize_mutex()
{
    sem_init(&staticMutex, 0, 1);
}

CLIENT *client_create(CLIENT_REGISTRY *creg, int fd)
{
    CLIENT *client = malloc(sizeof(CLIENT));
    client->fd = fd;
    client->refCount = 1;
    client->currentUser = NULL;
    client->currentMailbox = NULL;
    Sem_init(&client->mutex, 0, 1);
    Pthread_once(&once, initialize_mutex);
}

CLIENT *client_ref(CLIENT *client, char *why)
{
    P(&client->mutex);
    client->refCount = client->refCount + 1;
    V(&client->mutex);
    return client;
}

void *client_unref(CLIENT *client, char *why)
{
    P(&client->mutex);
    client->refCount = client->refCount - 1;
    V(&client->mutex);
    if (client->refCount == 0)
    {
        free(client);
    }
}

// Searches for a client with handle <handle> in client_registry
CLIENT *search_client_handle(char *handle)
{
    CLIENT_NODE *ref = client_registry->head;
    while (ref != NULL)
    {
        if ((strcmp(ref->client->currentUser->handle), handle) == 0)
        {
            return ref->client;
        }

        ref = ref->next;
    }

    return NULL;
}

int client_login(CLIENT *client, char *handle)
{
    // lock static mutex
    P(&staticMutex);

    // client is already logged in
    if (client->currentUser != NULL)
    {
        V(&staticMutex);
        return -1;
    }

    // find whether the handle is already logged in
    CLIENT *foundClient = search_client_handle(handle);

    if (foundClient != NULL)
    {
        // client found in registry
        V(&staticMutex);
        return -1;
    }

    USER *newUser = ureg_register(user_registry, handle);
    MAILBOX *newMailbox = mb_init(handle);

    P(client->mutex);

    client->currentUser = newUser;
    client->currentMailbox = newMailbox;

    V(client->mutex);

    // unlock static mutex
    V(&staticMutex);

    return 0;
}

int client_logout(CLIENT *client)
{
    // client is not logged in
    if (client->currentUser == NULL)
    {
        return -1;
    }

    P(client->mutex);
    client->currentMailbox = NULL;
    client->currentUser = NULL;
    V(client->mutex);

    client_unref(client, "logging out client - decrease refCount");
    return 0;
}

USER *client_get_user(CLIENT *client, int no_ref)
{
    if (no_ref == 0)
    {
        return client->currentUser;
    }

    else
    {
        user_ref(client, "Getting user pointer with no-ref = 0");
        return client->currentUser;
    }
}

MAILBOX *client_get_mailbox(CLIENT *client, int no_ref)
{
    if (no_ref == 0)
    {
        return client->currentMailbox;
    }

    else
    {
        mb_ref(client->currentMailbox, "Getting mailbox pointer with no-ref = 0");
        return client->currentMailbox;
    }
}

int client_get_fd(CLIENT *client)
{
    return client->fd;
}

int client_send_packet(CLIENT *user, CHLA_PACKET_HEADER *pkt, void *data)
{
    P(&staticMutex);
    int success = proto_send_packet(user->fd, pkt, data);
    V(&staticMutex);
    return success;
}

int client_send_ack(CLIENT *client, uint32_t msgid, void *data, size_t datalen)
{
    // get current time = packet send time
    uint32_t sec;
    uint32_t nsec;
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);
    sec = spec.tv_sec;
    nsec = spec.tv_nsec;

    // populate the header for sending
    CHLA_PACKET_HEADER *packet = malloc(sizeof(CHLA_PACKET_HEADER));
    packet->type = CHLA_ACK_PKT;
    packet->payload_length = (uint32_t)datalen;
    packet->msgid = msgid;
    packet->timestamp_sec = sec;
    packet->timestamp_sec = nsec;

    // send the packet
    int success = client_send_packet(client, packet, data);

    return success;
}
