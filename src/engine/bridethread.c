/*Copyright (c) 2024 Tristan Wellman*/

#include "bridethread.h"

struct threadData ThreadData;

void startThread(char *ID, BRIDEFUNC function) {
	if(ThreadData.threads[MAXSIMOTHREADS-1] != 0) {
		//log_warn("More threads than recommended are being used! removing threads.");
		pthread_create(&ThreadData.threads[0], NULL, (void *)function, NULL);
		ThreadData.IDs[0] = ID;
		return;
	}
	int i;
	for(i=0;i<MAXSIMOTHREADS;i++) {
		if(ThreadData.IDs[i]!=NULL&&!strcmp(ID, ThreadData.IDs[i])) {
			ThreadData.threads[i] = 0;
			pthread_create(&ThreadData.threads[i], NULL, 
					(void *)function, NULL);	
		}
		if(ThreadData.IDs[i]==NULL ||
				ThreadData.threads[i]==0) {
			ThreadData.IDs[i] = ID;
			pthread_create(&ThreadData.threads[i], NULL, 
					(void *)function, NULL);
			break;
		}
	}
}

void startThreadArg(char *ID, void *function, void *arg) {
	if(ThreadData.threads[MAXSIMOTHREADS-1] != 0) {
		//log_warn("More threads than recommended are being used! removing threads.");
		pthread_create(&ThreadData.threads[0], NULL, (void *)function, arg);
		ThreadData.IDs[0] = ID;
		return;
	}
	int i;
	for(i=0;i<MAXSIMOTHREADS;i++) {
		if(ThreadData.IDs[i]!=NULL&&!strcmp(ID, ThreadData.IDs[i])) {
			ThreadData.threads[i] = 0;
			pthread_create(&ThreadData.threads[i], NULL, 
					(void *)function, arg);	
		}
		if(ThreadData.IDs[i]==NULL ||
				ThreadData.threads[i]==0) {
			ThreadData.IDs[i] = ID;
			pthread_create(&ThreadData.threads[i], NULL, 
					(void *)function, arg);
			break;
		}
	}
}

void finishThread(char *ID) {
	int i;
	for(i=0;i<MAXSIMOTHREADS;i++) {
		if(ThreadData.IDs[i]!=NULL &&
				!strcmp(ID, ThreadData.IDs[i])) {
			pthread_join(ThreadData.threads[i], NULL);
   			ThreadData.IDs[i]=NULL;
		}
	}
};
