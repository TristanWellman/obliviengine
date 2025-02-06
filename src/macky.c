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

#include <OE/macky.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

mky_data globalMackyData; /* to be used as returned pointer to user. */
mky_data *ptr;

mky_data *mky_init(char *filename) {
	if(ptr==NULL) ptr = (mky_data *)malloc(sizeof(mky_data));
	ptr = &globalMackyData;

	/*allocates all data from config file to the file buffer*/
	if(filename==NULL) return ptr;
	ptr->file = fopen(filename, "r");
	if(ptr->file==NULL) return NULL;
	ptr->filebuf = (char *)malloc(sizeof(char)*1024);
	int BSIZE = 1024;
	int curSize=0;
	char line[1024];
	while(fgets(line,sizeof(line),ptr->file)!=NULL) {	
		curSize+=strlen(line);
		if(curSize<=BSIZE) {
			strcat(ptr->filebuf, line);
		} else {
			ptr->filebuf = (char *)realloc(ptr->filebuf, (sizeof(char)*curSize)*2);
			BSIZE=curSize*2;
			strcat(ptr->filebuf, line);
		}
	}
	/* Shrink down to size of text */
	/*ptr->filebuf = (char *)realloc(ptr->filebuf, sizeof(char)*curSize);*/
	ptr->filebuf+=6;/*skip weird characters*/
	ptr->fsize = curSize-6;
	/*printf("%s", ptr->filebuf);*/
	/*fclose(ptr->file);*/
	return ptr;
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
	char *buf = (char *)malloc(sizeof(char)*1024);
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
	char *arrData = (char *)malloc(sizeof(char)*MAX_ARR_SIZE);
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
	int *final = (int *)malloc(sizeof(int)*arr.array_length);
	int i;
	for(i=0;i<arr.array_length;i++) final[i] = vars[i] - '0';
	arr.array = (void *)final;
	return arr;
}


int mky_getIntAt(char *section, char *itemName) {
	/*mky_getIntAt("ITEM", "Damage");*/
	char *data = findData(section, itemName);
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
	return atoi(data);
}

float mky_getFloatAt(char *section, char *itemName) {
	char *data = findData(section, itemName);
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
	return atof(data);
}

char *mky_getStrAt(char *section, char *itemName) {
	char *data = findData(section, itemName);
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
	int i,item;
	for(i=0;i<strlen(data);i++) {
		if(checkData("FLOAT", data)) {
			fprintf(stderr, "ERROR:: Incorrect data type for Value %s!\n", itemName);
			exit(1);
		}
		if(checkData(itemName,data))
			data+=strlen(itemName)+1;

		if(checkData("TRUE", data)) return MKY_TRUE;

		data++;
	}

	return MKY_FALSE;
}
#else
bool mky_getBoolAt(char *section, char *itemName) {
	char *data = findData(section, itemName);
	int i,item;
	for(i=0;i<strlen(data);i++) {
		if(checkData("FLOAT", data)) {
			fprintf(stderr, "ERROR:: Incorrect data type for Value %s!\n", itemName);
			exit(1);
		}
		if(checkData(itemName,data))
			data+=strlen(itemName)+1;

		if(checkData("TRUE", data)) return true;
		data++;
	}

	return false;
}
#endif


/* This is just for testing*/
/*
int main() {
	mky_data *data = mky_init("test.mky");
	if(data==NULL) {
		printf("Error loading file!\n");
		return 0;
	}

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


	return 0;
}
*/
	
