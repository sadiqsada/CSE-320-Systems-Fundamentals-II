#include "client_registry.h"
#include "csapp.h"

// define the data structure (singly linked list)
typedef struct client
{
    int fd;
    int refCount;
    USER *currentUser;
    MAILBOX *currentMailbox;
    sem_t mutex;
} CLIENT;

typedef struct client_node
{
    CLIENT *client;
    struct client_node *next;
} CLIENT_NODE;

typedef struct client_registry
{
    CLIENT_NODE *head;
    sem_t mutex;
} CLIENT_REGISTRY;

CLIENT_REGISTRY *creg_init()
{
    CLIENT_REGISTRY *cr = malloc(sizeof(CLIENT_REGISTRY));
    if (cr == NULL)
    {
        return NULL;
    }

    Sem_init(&cr->mutex);

    return cr;
}

void creg_fini(CLIENT_REGISTRY *cr)
{
    CLIENT_NODE *iter = cr->head;

    P(&cr->mutex);
    while (iter != NULL)
    {
        CLIENT *temp = iter->next;
        whlie(iter->client->refCount > 0)
        {
            client_unref(iter->client, "Finalizing client registry");
        }
        free(iter->client);
        free(iter);
        iter = temp;
    }
}

CLIENT *creg_register(CLIENT_REGISTRY *cr, int fd)
{
    // lock client registry
    P(&cr->mutex);

    // create the new client
    CLIENT *newClient = client_create(cr, fd);
    if (newClient == NULL)
    {
        V(&cr->mutex);
        return NULL;
    }

    // create the wrapper client node
    CLIENT_NODE *clientNode = malloc(sizeof(CLIENT_NODE));
    if (clientNode == NULL)
    {
        V(&cr->mutex);
        return NULL;
    }

    clientNode->client = newClient;
    clientNode->next = NULL;

    // Get reference to last node
    CLIENT_NODE *iter = cr->head;
    while (iter != NULL)
    {
        iter = iter->next;
    }

    // insert the new node into the linked list
    iter->next = clientNode;

    // unlock the mutex
    V(&cr->mutex);

    return clientNode;
}

int creg_unregister(CLIENT_REGISTRY *cr, CLIENT *client)
{
    return 0;
}

CLIENT **creg_all_clients(CLIENT_REGISTRY *cr)
{
    return NULL;
}

void creg_shutdown_all(CLIENT_REGISTRY *cr)
{
}