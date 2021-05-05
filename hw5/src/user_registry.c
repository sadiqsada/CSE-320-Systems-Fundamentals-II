#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "csapp.h"
#include "user_registry.h"
#include "debug.h"

typedef struct user
{
    char *handle;
    int refCount;
    sem_t mutex;
} USER;

typedef struct user_node
{
    USER *user;
    struct user_node *next;
} USERNODE;

typedef struct user_registry
{
    USERNODE *head;
    sem_t mutex;
} USER_REGISTRY;

void test_data_structure(USER_REGISTRY *ureg)
{
    USERNODE *ref = ureg->head;

    while (ref != NULL)
    {
        debug("HANDLE: %s    ", ref->user->handle);
        debug("REFCOUNT: %d\n", ref->user->refCount);

        ref = ref->next;
    }
}

/*
 * Initialize a new user registry.
 *
 * @return the newly initialized USER_REGISTRY, or NULL if initialization
 * fails.
 */
USER_REGISTRY *ureg_init(void)
{
    // malloc the user registry
    USER_REGISTRY *ureg = malloc(sizeof(USER_REGISTRY));

    if (ureg == NULL)
    {
        return NULL;
    }

    // initialize dummy user
    USERNODE *head = malloc(sizeof(USERNODE));

    if (head == NULL)
    {
        return NULL;
    }

    USER *dummyUser = user_create("dummy");

    Sem_init(&ureg->mutex, 0, 1);

    head->user = dummyUser;

    head->next = NULL;

    P(&ureg->mutex);

    ureg->head = head;

    V(&ureg->mutex);

    test_data_structure(ureg);

    return ureg;
}

/*
 * Finalize a user registry, freeing all associated resources.
 *
 * @param cr  The USER_REGISTRY to be finalized, which must not
 * be referenced again.
 */
void ureg_fini(USER_REGISTRY *ureg)
{
    USERNODE *ref = ureg->head;

    P(&ureg->mutex);
    while (ref != NULL)
    {
        USERNODE *temp = ref->next;
        while (ref->user->refCount > 0)
        {
            user_unref(ref->user, "remove reference for current node");
        }
        free(ref->user);
        free(ref);
        ref = temp;
    }
    V(&ureg->mutex);
    sem_destroy(&ureg->mutex);
    free(ureg);
}

USERNODE *search_user(USER_REGISTRY *ureg, char *handle)
{
    // look for user in ureg_registry
    USERNODE *ref = ureg->head; // dummy node

    USERNODE *start = ref->next; // first node after dummy

    while (start != NULL)
    {
        // user with handle <handle> found
        if (strcmp(start->user->handle, handle) == 0)
        {
            return start;
        }

        start = start->next;
    }

    return NULL;
}

USERNODE *search_prev_user(USER_REGISTRY *ureg, char *handle)
{
    // look for user in ureg_registry
    USERNODE *ref = ureg->head; // dummy node

    while (ref->next != NULL)
    {
        // user with handle <handle> found
        if (strcmp(ref->next->user->handle, handle) == 0)
        {
            return ref;
        }

        ref = ref->next;
    }

    return NULL;
}

USER *ureg_register(USER_REGISTRY *ureg, char *handle)
{
    P(&ureg->mutex);
    USERNODE *foundUser = search_user(ureg, handle);
    if (foundUser != NULL)
    {
        // debug("FOUND USER...........");
        // debug("HANDLE: %s\n", foundUser->user->handle);
        // debug("REFCOUNT: %d\n", foundUser->user->refCount);
        user_ref(foundUser->user, "Found a user in the registry.");
        test_data_structure(ureg);
        V(&ureg->mutex);
        return foundUser->user;
    }

    else
    {
        debug("HANDLE NOT FOUND");
    }

    // user does not exist in registry
    USERNODE *ref = ureg->head;

    // look for the end of the list
    while (ref->next != NULL)
    {
        ref = ref->next;
    }

    // initialize new user
    USERNODE *newUsernode = malloc(sizeof(USERNODE));
    if (newUsernode == NULL)
        return NULL;

    USER *newUser = user_create(handle);
    newUsernode->user = newUser;
    newUser->refCount = 2;

    newUsernode->next = NULL;
    ref->next = newUsernode;

    // debug("CREATE USER...........");
    // debug("HANDLE: %s\n", newUser->handle);
    // debug("REFCOUNT: %d\n", newUser->refCount);

    test_data_structure(ureg);

    V(&ureg->mutex);
    return newUser;
}

void ureg_unregister(USER_REGISTRY *ureg, char *handle)
{
    P(&ureg->mutex);
    // look for user in registry
    USERNODE *foundUser = search_user(ureg, handle);
    USERNODE *foundPrevUser = search_prev_user(ureg, handle);

    // user not found
    if (foundUser == NULL)
    {
        return;
    }

    debug("FOUND CURRENT USER.....");
    debug("HANDLE: %s\n", foundUser->user->handle);
    debug("REFCOUNT: %d\n", foundUser->user->refCount);

    debug("FOUND PREVIOUS USER");
    debug("HANDLE: %s\n", foundPrevUser->user->handle);
    debug("REFCOUNT: %d\n", foundPrevUser->user->refCount);

    // user was found in registry
    // decrease refCount
    user_unref(foundUser->user, "user was found for unreffing");

    // delete from ureg linked list
    foundPrevUser->next = foundUser->next;
    free(foundUser);
    V(&ureg->mutex);
}
