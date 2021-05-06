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
    return NULL;
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