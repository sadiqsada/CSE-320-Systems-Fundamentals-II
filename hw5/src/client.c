#include <pthread.h>
#include <stdlib.h>

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

    client->currentUser = newUser;

    // unlock static mutex
    V(&staticMutex);

    return 0;
}