#include "user.h"
#include <stdio.h>

// User Struct
typedef struct user
{
    char *handle;
    int refCount;
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
    newUser->handle = handle;
    newUser->refCount = 0;
    return newUser;
}

// Increases the reference count of user by one
USER *user_ref(USER *user, char *why)
{
    user->refCount = user->refCount + 1;
    return user;
}

// Decreases the reference count of user by one
void user_unref(USER *user, char *why)
{
    // if refcount is bigger than zero, decrement it
    if (user->refCount > 0)
    {
        user->refCount = user->refCount - 1;
    }

    // if refcount has reached zero, free the user
    if (user->refCount == 0)
    {
        free(user);
    }
}

// gets the handle of user <user>
char *user_get_handle(USER *user)
{
    return user->handle;
}
