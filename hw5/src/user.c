#include "user.h"
#include <stdio.h>

typedef struct user
{
    char *handle;
    int refCount;
} USER;

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
