#include "user.h"
#include "csapp.h"
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

    Sem_init(&newUser->mutex, 0, 1);
    newUser->handle = handle;
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
    if (user->refCount > 0)
    {
        P(&user->mutex);
        user->refCount = user->refCount - 1;
        V(&user->mutex);

        // refCount has reached zero, free user
        if (user->refCount == 0)
        {
            free(user);
            return;
        }
    }

    // refCount is zero, free user
    else if (user->refCount == 0)
    {
        free(user);
    }
}

// gets the handle of user <user>
char *user_get_handle(USER *user)
{
    return user->handle;
}
