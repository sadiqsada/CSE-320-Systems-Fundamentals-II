#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>

#include "client_registry.h"
#include "csapp.h"
#include "debug.h"
#include "globals.h"

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

void test_client_registry(CLIENT_REGISTRY *cr)
{
    CLIENT_NODE *iter = cr->head;
    while (iter != NULL && iter->client != NULL)
    {
        debug("CLIENT FD: %d CLIENT REFCOUNT: %d NUMCLIENTS: %d\n", iter->client->fd, iter->client->refCount, cr->numClients);
        iter = iter->next;
    }
}

CLIENT_REGISTRY *creg_init()
{
    CLIENT_REGISTRY *cr = malloc(sizeof(CLIENT_REGISTRY));
    if (cr == NULL)
    {
        return NULL;
    }

    Sem_init(&cr->mutex, 0, 1);
    Sem_init(&cr->semaphore, 0, 1);

    P(&cr->mutex);
    cr->head = NULL;
    cr->numClients = 0;
    V(&cr->mutex);

    return cr;
}

void creg_fini(CLIENT_REGISTRY *cr)
{
    CLIENT_NODE *iter = cr->head;

    P(&cr->mutex);
    while (iter != NULL && iter->client != NULL)
    {
        CLIENT_NODE *temp = iter->next;
        free(iter->client);
        free(iter);
        iter = temp;
    }
    V(&cr->mutex);
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

    if (iter == NULL)
    {
        cr->head = clientNode;
    }

    else
    {
        while (iter->next != NULL)
        {
            iter = iter->next;
        }

        // insert the new node into the linked list
        iter->next = clientNode;
    }

    cr->numClients++;

    int numClients = cr->numClients;

    debug("CLIENT REGISTER");

    test_client_registry(cr);

    client_ref(clientNode->client, "increase client ref in client register");

    debug("increasing refCount for client %p (%d -> %d) for new pointer being returned by creg_register", clientNode->client, clientNode->client->refCount - 1, clientNode->client->refCount);

    // unlock the mutex
    V(&cr->mutex);

    if (numClients == 1)
    {
        P(&cr->semaphore);
    }

    return clientNode->client;
}

CLIENT *search_client_registry(CLIENT_REGISTRY *cr, CLIENT *client)
{
    CLIENT_NODE *iter = cr->head;
    while (iter != NULL && iter->client != NULL)
    {
        if (iter->client->fd == client->fd)
        {
            return iter->client;
        }
        iter = iter->next;
    }
    return NULL;
}

CLIENT_NODE *search_client_registry_node(CLIENT_REGISTRY *cr, CLIENT *client)
{
    CLIENT_NODE *iter = cr->head;
    while (iter != NULL)
    {
        if (iter->client->fd == client->fd)
        {
            return iter;
        }
        iter = iter->next;
    }
    return NULL;
}

CLIENT_NODE *search_client_registry_prev(CLIENT_REGISTRY *cr, CLIENT *client)
{
    CLIENT_NODE *iter = cr->head;
    if (iter == NULL)
    {
        return NULL;
    }
    while (iter->next != NULL)
    {
        if (iter->next->client->fd == client->fd)
        {
            return iter;
        }
        iter = iter->next;
    }
    return NULL;
}

int creg_unregister(CLIENT_REGISTRY *cr, CLIENT *client)
{
    // if client doesn't exist yet, error
    P(&cr->mutex);
    debug("CREG UNREGISTER");
    test_client_registry(cr);
    CLIENT *newClient = search_client_registry(cr, client);
    if (newClient == NULL)
    {
        return -1;
    }
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
        debug("Unregister client with fd %d", cr->head->client->fd);
        debug("Total Connected %d", cr->numClients - 1);
        free(cr->head);
        cr->numClients--;
    }

    else
    {
        // find previous node's reference and current ref
        CLIENT_NODE *currClient = search_client_registry_node(cr, client);
        CLIENT_NODE *prevClient = search_client_registry_prev(cr, client);
        prevClient->next = currClient->next;
        cr->numClients--;
        debug("Unregister client with fd %d", currClient->client->fd);
        debug("Total Connected %d", cr->numClients);
    }

    numClients = cr->numClients;
    client_unref(client, "unregistering and unreffing this client");

    V(&cr->mutex);

    if (numClients == 0)
    {
        V(&cr->semaphore);
    }

    return 0;
}

CLIENT **creg_all_clients(CLIENT_REGISTRY *cr)
{
    // initialize array to be returned
    int numClients = cr->numClients;
    CLIENT **clients = malloc((sizeof(CLIENT *) * (numClients + 1)));

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
        // increase ref count
        client_ref(*temp, "increase reference count since array contains pointers to each client obj");
        temp++;
        start = start->next;
    }
    *temp = NULL;
    debug("CREG ALL CLIENTS");
    test_client_registry(cr);
    return clients;
}

void creg_shutdown_all(CLIENT_REGISTRY *cr)
{
    debug("CREG SHUTDOWN ALL");
    test_client_registry(cr);
    P(&cr->mutex);
    CLIENT_NODE *start = cr->head;
    while (start != NULL && start->client != NULL)
    {
        debug("SHUTTING CLIENT FD: %d", start->client->fd);
        shutdown(start->client->fd, 2);
        debug("CLIENT AFTER SHUTDOWN FD:%d REFCOUNT:%d", start->client->fd, start->client->refCount);
        start = start->next;
    }
    V(&cr->mutex);

    P(&cr->semaphore);
    V(&cr->semaphore);
}