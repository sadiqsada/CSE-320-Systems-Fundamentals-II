#include "user.h"
#include "csapp.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>

// User Struct
typedef struct user
{
    char *handle;
    int refCount;
    sem_t mutex;
} USER;

// creates a user with handle <handle>
USER *user_create(char *handle)
{
    // if handle is null, can't create user
    // return null
    if (handle == NULL)
    {
        return NULL;
    }

    USER *newUser = malloc(sizeof(USER));

    if (newUser == NULL)
    {
        return NULL;
    }

    newUser->handle = malloc(strlen(handle) + 1);

    if (newUser->handle == NULL)
    {
        return NULL;
    }

    Sem_init(&newUser->mutex, 0, 1);
    strcpy(newUser->handle, handle);
    // newUser->handle = handle;
    newUser->refCount = 1;
    return newUser;
}

// Increases the reference count of user by one
USER *user_ref(USER *user, char *why)
{
    P(&user->mutex);
    user->refCount = user->refCount + 1;
    V(&user->mutex);
    return user;
}

// Decreases the reference count of user by one
void user_unref(USER *user, char *why)
{
    // if refcount is bigger than zero, decrement it
    debug("USER HANDLE: %s\n", user->handle);
    debug("USER REFCOUNT: %d\n", user->refCount);
    P(&user->mutex);
    if (user->refCount > 0)
    {
        user->refCount = user->refCount - 1;
        V(&user->mutex);

        // refCount has reached zero, free user
        if (user->refCount == 0)
        {
            sem_destroy(&user->mutex);
            free(user->handle);
            return;
        }
    }

    debug("USER HANDLE: %s\n", user->handle);
    debug("USER REFCOUNT: %d\n", user->refCount);
}

// gets the handle of user <user>
char *user_get_handle(USER *user)
{
    if (user == NULL)
    {
        return NULL;
    }
    return user->handle;
}
