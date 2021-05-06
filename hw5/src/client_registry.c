#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>

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
    int numClients;
    sem_t mutex;
    sem_t semaphore;
} CLIENT_REGISTRY;

CLIENT_REGISTRY *creg_init()
{
    CLIENT_REGISTRY *cr = malloc(sizeof(CLIENT_REGISTRY));
    if (cr == NULL)
    {
        return NULL;
    }
    cr->numClients = 0;
    Sem_init(&cr->mutex);
    Sem_init(&cr->semaphore);
    return cr;
}

void creg_fini(CLIENT_REGISTRY *cr)
{
    CLIENT_NODE *iter = cr->head;

    P(&cr->mutex);
    while (iter != NULL)
    {
        CLIENT *temp = iter->next;
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

    cr->numClients++;

    int numClients = cr->numClients;

    if (numClients == 1)
    {
        P(&creg->semaphore);
    }

    // unlock the mutex
    V(&cr->mutex);

    return clientNode;
}

CLIENT *search_client_registry(CLIENT_REGISTRY *cr, CLIENT *client)
{
    CLIENT_NODE *iter = cr->head;
    while (iter != NULL)
    {
        if (iter->client->fd == client->fd)
        {
            return iter->client;
        }
        iter = iter->next;
    }
    return NULL;
}

CLIENT *search_client_registry_prev(CLIENT_REGISTRY *cr, CLIENT *client)
{
    CLIENT_NODE *iter = cr->head;
    while (iter->next != NULL)
    {
        if (iter->next->client->fd == client->fd)
        {
            return iter->client;
        }
        iter = iter->next;
    }
    return NULL;
}

int creg_unregister(CLIENT_REGISTRY *cr, CLIENT *client)
{
    // if client doesn't exist yet, error
    P(&cr->mutex);
    CLIENT *client = search_client_registry(cr, client);
    if (client == NULL)
    {
        return -1;
    }

    // otherwise, decrease referemce count of client by one
    client_unref(client, "unregistering and unreffing this client");

    int numClients = cr->numClients;

    if (numClients == 0)
    {
        V(&cr->mutex);
        return -1;
    }

    // remove client from data structure
    if (numClients == 1)
    {
        // remove the only node
        free(client);
    }

    else
    {
        // find previous node's reference and current ref
        CLIENT *prevClient = search_client_registry_prev(cr, client);
        prevClient->next = client->next;
        free(client);
    }

    cr->numClients--;
    numClients = cr->numClients;

    if (numClients == 0)
    {
        V(&creg->semaphore);
    }
    V(&cr->mutex);

    return 0;
}

CLIENT **creg_all_clients(CLIENT_REGISTRY *cr)
{
    // initialize array to be returned
    int numClients = cr->numClients;
    CLIENT **clients = malloc((sizeof(CLIENT) * numClients) + 1);

    if (clients == NULL)
    {
        return NULL;
    }

    // populate array
    CLIENT **temp = clients;
    CLIENT_NODE *start = cr->head;
    while (start != NULL)
    {
        *temp = start->client;
        temp++;
    }
    *temp = NULL;
    return clients;
}

void creg_shutdown_all(CLIENT_REGISTRY *cr)
{
    P(&cr->mutex);
    CLIENT_NODE *start = cr->head;
    while (start != NULL)
    {
        shutdown(start->client->fd, SHUT_RDWR);
        start = start->next;
    }
    V(&cr->mutex);

    P(&cr->semaphore);
    V(&cr->semaphore);
}