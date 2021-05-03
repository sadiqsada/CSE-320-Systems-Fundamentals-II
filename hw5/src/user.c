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
    Sem_init(&newUser->mutex, 0, 1);
    newUser->handle = handle;
    newUser->refCount = 0;
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
    P(&user->mutex);
    if (user->refCount > 0)
    {
        user->refCount = user->refCount - 1;
    }

    // if refcount has reached zero, free the user
    if (user->refCount == 0)
    {
        free(user);
    }
    V(&user->mutex);
}

// gets the handle of user <user>
char *user_get_handle(USER *user)
{
    return user->handle;
}
