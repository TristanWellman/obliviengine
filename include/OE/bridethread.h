/*Copyright (c) 2024 Tristan Wellman*/

#ifndef BRIDETHREAD_H
#define BRIDETHREAD_H

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "log.h"
#ifdef _WIN32
#include <windows.h>
typedef HANDLE THREAD_TYPE;
#else
#include <pthread.h>
typedef pthread_t THREAD_TYPE;
#endif

#define MAXSIMOTHREADS 30

typedef void (*BRIDEFUNC)();

typedef struct {
  void *arg1;
  void *arg2;
} sampleArgStruct;

struct threadData {
	THREAD_TYPE threads[MAXSIMOTHREADS];
	char *IDs[MAXSIMOTHREADS];
};

void startThread(char *ID, BRIDEFUNC function);
void startThreadArg(char *ID, void *function, void *arg);
void finishThread(char *ID);

#endif
