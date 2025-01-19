/*Copyright (c) 2024 Tristan Wellman*/

#ifndef BRIDETHREAD_H
#define BRIDETHREAD_H

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "log.h"

#define MAXSIMOTHREADS 30

typedef void (*BRIDEFUNC)();

typedef struct {
  void *arg1;
  void *arg2;
} sampleArgStruct;

struct threadData {
	pthread_t threads[MAXSIMOTHREADS];
	char *IDs[MAXSIMOTHREADS];
};

void startThread(char *ID, BRIDEFUNC function);
void startThreadArg(char *ID, void *function, void *arg);
void finishThread(char *ID);

#endif
