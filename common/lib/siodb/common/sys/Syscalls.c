#include "Syscalls.h"

// CRT headers
#include <assert.h>
#include <stdlib.h>

// System headers
#include <pthread.h>
#include <syscall.h>

static pthread_key_t g_tidKey;
static int g_tidKeyCreateResult = -1;
static int g_initCount;

void initSyscalls(void)
{
    if (g_initCount++ == 0) g_tidKeyCreateResult = pthread_key_create(&g_tidKey, &free);
}

void finalizeSyscalls(void)
{
    assert(g_initCount > 0);
    if (--g_initCount == 0) {
        if (!g_tidKeyCreateResult) pthread_key_delete(g_tidKey);
    }
}

pid_t gettid(void)
{
    // Cached TID key was not created, fallaback to system call.
    if (g_tidKeyCreateResult) return syscall(SYS_gettid);

    // Return cached TID, if available.
    pid_t* cachedTid = pthread_getspecific(g_tidKey);
    if (cachedTid) return *cachedTid;

    // Obtain TID using system call and attempt to cache it.
    const pid_t tid = syscall(SYS_gettid);
    cachedTid = malloc(sizeof(tid));
    if (cachedTid) {
        *cachedTid = tid;
        if (pthread_setspecific(g_tidKey, cachedTid)) free(cachedTid);
    }

    return tid;
}
