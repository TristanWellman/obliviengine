/*Copyright (c) 2024 Tristan Wellman*/

#include <stdio.h>
#include <stdlib.h>
#include <OE/bridethread.h>

struct threadData ThreadData;

#ifdef _WIN32
DWORD WINAPI ThreadWrapper(LPVOID param) {
	BRIDEFUNC *funcP = (BRIDEFUNC *)param;
	BRIDEFUNC func = *funcP;
	free(funcP);
	func();
    return 0;
}
#else
void *ThreadWrapper(void *param) {
	BRIDEFUNC *funcP = (BRIDEFUNC *)param;
	BRIDEFUNC func = *funcP;
	free(funcP);
	func();
	return NULL;
}
#endif

void startThread(char *ID, BRIDEFUNC function) {
	BRIDEFUNC *arg = calloc(1, sizeof(BRIDEFUNC));
	*arg = function;
    if (ThreadData.threads[MAXSIMOTHREADS - 1] != 0) {
#ifdef _WIN32
        ThreadData.threads[0] = CreateThread(NULL, 0, ThreadWrapper, (LPVOID)function, 0, NULL);
#else
        pthread_create(&ThreadData.threads[0], NULL, ThreadWrapper, (void *)arg);
#endif
        ThreadData.IDs[0] = ID;
        return;
    }
    int i;
    for (i = 0; i < MAXSIMOTHREADS; i++) {
        if (ThreadData.IDs[i] != NULL && !strcmp(ID, ThreadData.IDs[i])) {
            ThreadData.threads[i] = 0;
#ifdef _WIN32
            ThreadData.threads[i] = CreateThread(NULL, 0, ThreadWrapper, (LPVOID)function, 0, NULL);
#else
            pthread_create(&ThreadData.threads[i], NULL, ThreadWrapper, (void *)arg);
#endif
        }
        if (ThreadData.IDs[i] == NULL || ThreadData.threads[i] == 0) {
            ThreadData.IDs[i] = ID;
#ifdef _WIN32
            ThreadData.threads[i] = CreateThread(NULL, 0, ThreadWrapper, (LPVOID)function, 0, NULL);
#else
            pthread_create(&ThreadData.threads[i], NULL, ThreadWrapper, (void *)arg);
#endif
            break;
        }
    }
}

void startThreadArg(char *ID, void *function, void *arg) {
    if (ThreadData.threads[MAXSIMOTHREADS - 1] != 0) {
#ifdef _WIN32
        ThreadData.threads[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)function, arg, 0, NULL);
#else
        pthread_create(&ThreadData.threads[0], NULL, function, arg);
#endif
        ThreadData.IDs[0] = ID;
        return;
    }
    int i;
    for (i = 0; i < MAXSIMOTHREADS; i++) {
        if (ThreadData.IDs[i] != NULL && !strcmp(ID, ThreadData.IDs[i])) {
            ThreadData.threads[i] = 0;
#ifdef _WIN32
            ThreadData.threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)function, arg, 0, NULL);
#else
            pthread_create(&ThreadData.threads[i], NULL, function, arg);
#endif
        }
        if (ThreadData.IDs[i] == NULL || ThreadData.threads[i] == 0) {
            ThreadData.IDs[i] = ID;
#ifdef _WIN32
            ThreadData.threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)function, arg, 0, NULL);
#else
            pthread_create(&ThreadData.threads[i], NULL, function, arg);
#endif
            break;
        }
    }
}

void finishThread(char *ID) {
    int i;
    for (i = 0; i < MAXSIMOTHREADS; i++) {
        if (ThreadData.IDs[i] != NULL && !strcmp(ID, ThreadData.IDs[i])) {
#ifdef _WIN32
            WaitForSingleObject(ThreadData.threads[i], INFINITE);
            CloseHandle(ThreadData.threads[i]);
#else
            pthread_join(ThreadData.threads[i], NULL);
#endif
            ThreadData.IDs[i] = NULL;
        }
    }
}

