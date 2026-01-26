/*
C-Ware License

Copyright (c) 2024, Tristan Wellman
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Redistributions of modified source code must append a copyright notice in
   the form of 'Copyright <YEAR> <NAME>' to each modified source file's
   copyright notice, and the standalone license file if one exists.

A "redistribution" can be constituted as any version of the source code
that is intended to comprise some other derivative work of this code. A
fork created for the purpose of contributing to any version of the source
does not constitute a truly "derivative work" and does not require listing.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * */

#ifdef _OE_INC
#include <OE/macky.h>
#else 
#include "macky.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

mky_data globalMackyData; /* to be used as returned pointer to user. */
mky_data *ptr;

mky_data *mky_init(char *filename) {
	if(ptr==NULL) ptr = calloc(1, sizeof(mky_data));
	ptr = &globalMackyData;

	/*allocates all data from config file to the file buffer*/
	if(filename==NULL) return NULL;
	ptr->file = fopen(filename, "r");
	if(ptr->file==NULL) return NULL;
	ptr->fsize = 0;
	ptr->fcap = _MKY_FBSTEP;
	ptr->filebuf = calloc(ptr->fcap, sizeof(char));
	ptr->filebuf[0] = '\0';
	char line[_MKY_FBSTEP];
	while(fgets(line,sizeof(line),ptr->file)!=NULL) {	
		ptr->fsize+=strlen(line);
		if(ptr->fsize<ptr->fcap) {
			strcat(ptr->filebuf, line);
		} else {
			ptr->fcap+=_MKY_FBSTEP;
			ptr->filebuf = (char *)realloc(ptr->filebuf, sizeof(char *)*ptr->fcap);
			strcat(ptr->filebuf, line);
		}
	}
	fclose(ptr->file);
	ptr->file = NULL;
	return ptr;
}

void mky_close(mky_data *data) {
	if(ptr&&data&&ptr==data) {
		if(data->file) fclose(data->file);
		if(data->filebuf) free(data->filebuf);
	}
}

int checkData(char *str, char *data) {
	int j=0;
	char tmpbuf[strlen(str)];
	int cmp=0;
	for(j=0;j<strlen(str);j++) {
		if(str[j]==data[j]) cmp=1;
		else {cmp=0;break;}
	}

	/*printf("%s, %s\n", tmpbuf, str);*/
	if(cmp) return 1;
	return 0;
}

char *extractValue(char *fileBuffer, int index) {
	char *buf = calloc(_MKY_FBSTEP+1, sizeof(char));
	int begin=index;
	/*Work backward to the start of declaration*/
	for(;begin>=0;begin--) {
		/*Make sure we don't go to before last line*/
		if('.'==fileBuffer[begin]) return NULL;

		if(checkData("VALUE", fileBuffer+begin)||
				checkData("FLOAT", fileBuffer+begin)||
				checkData("ARRAY", fileBuffer+begin)) break;
	}

	int valueStartIndex = begin;
	/*Now we can go forward and append to buffer til end of declaration*/
	int i=0,fltThreash=2,curper=0;
	int flt = 0;
	if(checkData("FLOAT", fileBuffer+valueStartIndex)) flt = 1;
	for(;valueStartIndex<=strlen(fileBuffer);valueStartIndex++) {
		if('.'==fileBuffer[valueStartIndex]) {
			curper++;
			if(flt) {
				if(curper>=2) break;
			} 
			else break;
		}
		buf[i]=fileBuffer[valueStartIndex];
		i++;
	}	
	buf[i] = '\0';
	return buf;
}

char *findData (char *section, char *itemName) {
	mky_data *mb = &globalMackyData;
	mky_data modcp; /* modifiable copy for global data */
	memcpy(&modcp, &globalMackyData, sizeof(globalMackyData));
	int i;
	int comment=0;
	int sec=0;
	int datasec=0;
	int item=0;
	for(i=0;i<mb->fsize;i++) {
		/* check comments */
		if(comment==1) {
			if(mb->filebuf[i]!='\n') continue;
			else {comment=0;continue;}
		}
		if(mb->filebuf[i]==';') {comment=1;continue;}
		
		/* check data sections */
		if(sec==1) {
			if('.' == mb->filebuf[i]) sec=0;
			if(checkData(section, mb->filebuf+i)) {datasec=1;sec=0;}
			continue;
		}

		/*Check for requested value & extract it*/
		if(datasec==1) {
			if(checkData("END.", mb->filebuf+i)) break;
			if(checkData(itemName, mb->filebuf+i)) {
				char *value = extractValue(mb->filebuf, i);
				if(value==NULL) return NULL;
				return value;
			}
			continue;
		}

		/* check next set of characters for keywords */
		if(checkData("SECTION", mb->filebuf+i)) sec=1;
		
	}
	return NULL;
}

mky_array mky_getArrayAt(char *section, char *itemName) {
	char *data = findData(section, itemName);
	int i;
	/*Move up to array data*/
	for(i=0;i<strlen(data);i++) {
		if(checkData(itemName, data)) {data += strlen(itemName)+1;break;}
		data++;
	}	
	if(data[0]!='[' || data[strlen(data)-1]!=']') {
		/*printf("%c, %c\n", data[0], data[strlen(data)-1]);*/
		fprintf(stderr, "ERROR:: Invalid array initialization at %s", itemName);
		exit(1);
	}
	data++;data[strlen(data)-1]='\0'; /*get rid of brackets*/
	/*printf("%s\n", data);*/
	char *arrData = calloc(MAX_ARR_SIZE, sizeof(char));
	for(i=0;i<strlen(data);i++) {
		if(data[i]==',') data++;
		if(i>=MAX_ARR_SIZE) break;
		arrData[i]=data[i];
	}
	return (mky_array){(void *)arrData, strlen(arrData)};
}

mky_array mky_getIntArrayAt(char *section, char *itemName) {
	mky_array arr = mky_getArrayAt(section, itemName);
	char *vars = (char *)arr.array;
	int *final = calloc(arr.array_length, sizeof(int));
	int i;
	for(i=0;i<arr.array_length;i++) final[i] = vars[i] - '0';
	arr.array = (void *)final;
	return arr;
}


int mky_getIntAt(char *section, char *itemName) {
	/*mky_getIntAt("ITEM", "Damage");*/
	char *data = findData(section, itemName);
	char *dataptr = data; /*for free later*/
	if(!data) return 0;
	if(data==NULL) {
		fprintf(stdout, "MKY:WARN:: Could not find data %s - %s\n", section, itemName);
		return -1;
	}
	int i,item;
	for(i=0;i<strlen(data);i++) {
		if(checkData("FLOAT", data)) {
			fprintf(stderr, "ERROR:: Incorrect data type for Value %s!\n", itemName);
			exit(1);
		}
		if(checkData(itemName,data)) {
			data+=strlen(itemName)+1;
			break;
		}
		data++;
	}
	int res = atoi(data);
	free(dataptr); dataptr = NULL;
	return res;
}

float mky_getFloatAt(char *section, char *itemName) {
	char *data = findData(section, itemName);
	char *dataptr = data;
	if(!data) return 0;
	int i,item;
	for(i=0;i<strlen(data);i++) {
		if(checkData("VALUE", data)) {
			fprintf(stderr, "ERROR:: Incorrect data type for Float %s!\n", itemName);
			exit(1);
		}
		if(checkData(itemName,data)) {
			data+=strlen(itemName)+1;
			break;
		}
		data++;
	}
	float res = atof(data);
	free(dataptr); dataptr = NULL;
	return res;
}

char *mky_getStrAt(char *section, char *itemName) {
	char *data = findData(section, itemName);
	if(!data) return NULL;
	int i,item;
	for(i=0;i<strlen(data);i++) {
		if(checkData("FLOAT", data)) {
			fprintf(stderr, "ERROR:: Incorrect data type for Value %s!\n", itemName);
			exit(1);
		}
		if(checkData(itemName,data)) {
			data+=strlen(itemName)+1;
			break;
		}
		data++;
	}
	return data;
}

#if !defined __STDBOOL_H
MKY_BOOL mky_getBoolAt(char *section, char *itemName) {
	char *data = findData(section, itemName);
	if(!data) return MKY_FALSE;
	int i,item;
	for(i=0;i<strlen(data);i++) {
		if(checkData("FLOAT", data)) {
			fprintf(stderr, "ERROR:: Incorrect data type for Value %s!\n", itemName);
			exit(1);
		}
		if(checkData(itemName,data))
			data+=strlen(itemName)+1;

		if(checkData("TRUE", data)) {
			free(data); data = NULL;
			return MKY_TRUE;
		}
		data++;
	}
	free(data); data = NULL;
	return MKY_FALSE;
}
#else
bool mky_getBoolAt(char *section, char *itemName) {
	char *data = findData(section, itemName);
	if(!data) return MKY_FALSE;
	int i,item;
	for(i=0;i<strlen(data);i++) {
		if(checkData("FLOAT", data)) {
			fprintf(stderr, "ERROR:: Incorrect data type for Value %s!\n", itemName);
			exit(1);
		}
		if(checkData(itemName,data))
			data+=strlen(itemName)+1;

		if(checkData("TRUE", data)) {
			free(data); data = NULL;
			return true;
		}
		data++;
	}
	free(data); data = NULL;
	return false;
}
#endif


/* This is just for testing*/

/*int main() {
	mky_data *data = mky_init("test.mky");
	if(data==NULL) {
		printf("Error loading file!\n");
		return 0;
	}
	printf("- - -\n%s\n- - -\n", data->filebuf);

	char *name = mky_getStrAt("ITEM", "Name");
	int damage = mky_getIntAt("ITEM", "Damage");
	float Rarity = mky_getFloatAt("DROP", "Rarity");
	MKY_BOOL chestDrop = mky_getBoolAt("DROP", "ChestDrop");

	printf("Name: %s\nDamage: %d\nRarity: %f\nChest Drop: %d\n", 
			name, damage, Rarity, chestDrop);


	mky_array tmp = mky_getIntArrayAt("ITEM", "test");
	int *array = (int *)tmp.array;
	int i;
	for(i=0;i<tmp.array_length;i++) {
		printf("%d, ", array[i]);
	}
	free(tmp.array);

	mky_close(data);

	data = mky_init("test.mky");
	if(data==NULL) {
		printf("Error loading file!\n");
		return 0;
	}
	name = mky_getStrAt("ITEM", "Name");
	printf("\nReopened file Name: %s\n", name);


	return 0;
}*/

	
