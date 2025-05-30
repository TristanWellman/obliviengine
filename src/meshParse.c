/*Copyright (c) 2025 Tristan Wellman
 *
 * This is for parsing .obj files as well as OpenFOAM mesh vert/face files
 *
 * */

#include <OE/meshParse.h>
#include <OE/bridethread.h>

int checkObjNorm(char *line, OEMesh *mesh) {
	if(line==NULL) return 0;

	if(strstr(line, "vn ")) {
		if(mesh->vertNorms.size>=mesh->vertNorms.cap) {
			mesh->vertNorms.cap+=MAXDATA;
			mesh->vertNorms.data = (float **)realloc(mesh->vertNorms.data, 
					sizeof(float *)*mesh->vertNorms.cap);
		}
		char *vs = strstr(line, "vn") + 2;
		if(vs!=NULL) {
			mesh->vertNorms.data[mesh->vertNorms.size] = calloc(NORMSIZE, sizeof(float));
			int i;
			for(i=0;i<VSIZE;i++) {
				while(vs[0]==' ')vs++;
				char buf[128];
				int j = 0;
				for(;vs[0]!=' '&&vs[0]!='\n';vs++,j++) buf[j]=vs[0];
				buf[j] = '\0';
				mesh->vertNorms.data[mesh->vertNorms.size][i] = atof(buf);
				mesh->vertNorms.total++;
			}
			/*printf("(%f, %f, %f)\n", mesh->vertNorms.data[mesh->vertNorms.size][0],
					mesh->vertNorms.data[mesh->vertNorms.size][1],
					mesh->vertNorms.data[mesh->vertNorms.size][2]);*/
			mesh->vertNorms.size++;
		}
		return 1;
	}
	return 0;
}

int checkObjTex(char *line, OEMesh *mesh) {
	if(line==NULL) return 0;

	if(strstr(line, "vt ")) {
		if(mesh->vertTex.size>=mesh->vertTex.cap) {
			mesh->vertTex.cap+=MAXDATA;
			mesh->vertTex.data = (float **)realloc(mesh->vertTex.data, 
					sizeof(float *)*mesh->vertTex.cap);
		}
		char *vs = strstr(line, "vt") + 2;
		if(vs!=NULL) {
			mesh->vertTex.data[mesh->vertTex.size] = calloc(TEXSIZE, sizeof(float));
			int i;
			for(i=0;i<TEXSIZE;i++) {
				while(vs[0]==' ')vs++;
				char buf[128];
				int j = 0;
				for(;vs[0]!=' '&&vs[0]!='\n';vs++,j++) buf[j]=vs[0];
				buf[j] = '\0';
				mesh->vertTex.data[mesh->vertTex.size][i] = atof(buf);
				mesh->vertTex.total++;
			}
			/*printf("(%f, %f)\n", mesh->vertTex.data[mesh->vertTex.size][0],
					mesh->vertTex.data[mesh->vertTex.size][1]);*/
			mesh->vertTex.size++;
		}
		return 1;
	}
	return 0;
}

int checkObjIndices(char *line, OEMesh *mesh) {
	if(line==NULL) return 0;
	/*we check for '/' since some .obj files contain other 'f' materials*/
	if(strstr(line, "f ")&&strchr(line, '/')) { 		
		if(mesh->indices.size>=mesh->indices.cap) {
			mesh->indices.cap+=MAXDATA;
			mesh->indices.data = (uint16_t **)realloc(mesh->indices.data,
					sizeof(uint16_t *)*mesh->indices.cap);
		}
		if(mesh->normInds.size>=mesh->normInds.cap) {
			mesh->normInds.cap+=MAXDATA;
			mesh->normInds.data = (uint16_t **)realloc(mesh->normInds.data,
					sizeof(uint16_t *)*mesh->normInds.cap);
		}
		if(mesh->texInds.size>=mesh->texInds.cap) {
			mesh->texInds.cap+=MAXDATA;
			mesh->texInds.data = (uint16_t **)realloc(mesh->texInds.data,
					sizeof(uint16_t *)*mesh->texInds.cap);
		}
		char *is = strchr(line, 'f')+1;
		if(is!=NULL) {
			mesh->indices.data[mesh->indices.size] = calloc(ISIZE, sizeof(uint16_t));
			mesh->normInds.data[mesh->normInds.size] = calloc(ISIZE, sizeof(uint16_t));
			mesh->texInds.data[mesh->texInds.size] = calloc(ISIZE, sizeof(uint16_t));
			int j;
			for(j=0;j<ISIZE;j++) {
				while(is[0]==' ')is++;
				char *buf = calloc(256, sizeof(char));
				int i = 0;
				for(;is[0]!=' '&&is[0]!='\n';is++,i++) buf[i]=is[0];
				if(buf==NULL||buf[0]=='\0') continue;
				char *cpy = strdup(buf);
				char *tmp = strtok(cpy, "/");
				mesh->indices.data[mesh->indices.size][j] = atoi(tmp);
				free(cpy);
				/*get the texture coord indexes*/
				while(buf[0]!='/') buf++;
				buf++;
				cpy = strdup(buf);
				char *tex = strtok(cpy, "/");
				mesh->texInds.data[mesh->texInds.size][j] = atoi(tex);
				free(cpy);
				/*get normal indexes*/
				while(buf[0]!='/') buf++;
				buf++;
				mesh->normInds.data[mesh->normInds.size][j] = atoi(buf);
				mesh->normInds.total++;
				mesh->indices.total++;
				mesh->texInds.total++;
			}
			/*printf("(%d, %d, %d, %d)\n", mesh->normInds.data[mesh->normInds.size][0],
					mesh->normInds.data[mesh->normInds.size][1],
					mesh->normInds.data[mesh->normInds.size][2],
					mesh->normInds.data[mesh->normInds.size][3]);*/
			mesh->indices.size++;
			mesh->texInds.size++;
			mesh->normInds.size++;
		}
		return 1;
	}
	return 0;
}

int checkObjVerts(char *line, OEMesh *mesh) {
	if(line==NULL) return 0;

	if(strstr(line, "v ")) {
		if(mesh->verts.size>=mesh->verts.cap) {
			mesh->verts.cap+=MAXDATA;
			mesh->verts.data = (float **)realloc(mesh->verts.data, 
					sizeof(float *)*mesh->verts.cap);
		}
		char *vs = strchr(line, 'v') + 1;
		if(vs!=NULL) {
			mesh->verts.data[mesh->verts.size] = calloc(VSIZE, sizeof(float));
			int i;
			for(i=0;i<VSIZE;i++) {
				while(vs[0]==' ')vs++;
				char buf[128];
				int j = 0;
				for(;vs[0]!=' '&&vs[0]!='\n';vs++,j++) buf[j]=vs[0];
				buf[j] = '\0';
				mesh->verts.data[mesh->verts.size][i] = atof(buf);
				mesh->verts.total++;
			}
			/*printf("(%f, %f, %f)\n", mesh->verts.data[mesh->verts.size][0],
					mesh->verts.data[mesh->verts.size][1],
					mesh->verts.data[mesh->verts.size][2]);*/
			mesh->verts.size++;
		}
		return 1;
	}
	return 0;
}

int getObjLabel(char *line, OEMesh *mesh) {
	if(line[0]=='o') {
		char *name = strchr(line, 'o')+1;
		if(name!=NULL) {
			if(mesh->label!=NULL) return 2;
			while(name[0]==' ') name++;
			int len = strlen(name);
			while(name[len-1]=='\n') name[len-1] = '\0';
			mesh->label = calloc(len+1, sizeof(char));
			strcpy(mesh->label, name);
		}
		return 1;
	}
	return 0;
}

void initMeshData(OEMesh *mesh) {
	mesh->verts.cap = MAXDATA;
	mesh->verts.size = 0;
	mesh->verts.total = 0;
	mesh->verts.data = calloc(mesh->verts.cap, sizeof(float*));
	mesh->vertTex.cap = MAXDATA;
	mesh->vertTex.size = 0;
	mesh->vertTex.total = 0;
	mesh->vertTex.data = calloc(mesh->vertTex.cap, sizeof(float*));
	mesh->vertNorms.cap = MAXDATA;
	mesh->vertNorms.size = 0;
	mesh->vertNorms.total = 0;
	mesh->vertNorms.data = calloc(mesh->vertNorms.cap, sizeof(float*));
	mesh->indices.cap = MAXDATA;
	mesh->indices.size = 0;
	mesh->indices.total = 0;
	mesh->indices.data = calloc(mesh->indices.cap, sizeof(uint16_t*));
	mesh->normInds.cap = MAXDATA;
	mesh->normInds.size = 0;
	mesh->normInds.total = 0;
	mesh->normInds.data = calloc(mesh->normInds.cap, sizeof(uint16_t*));
	mesh->texInds.cap = MAXDATA;
	mesh->texInds.size = 0;
	mesh->texInds.total = 0;
	mesh->texInds.data = calloc(mesh->texInds.cap, sizeof(uint16_t*));

	mesh->label = NULL;
}

void *parseFoamFaces(FILE* f, OEFOAMMesh* mesh) {
	/*EXPECTS AN ALREADY OPENED FILE*/
	if (f == NULL) return NULL;
	//OEFOAMMesh *mesh = (OEFOAMMesh *)args_->meshptr;

	char line[2048];
	char* prevLine = calloc(2048, sizeof(char));
	int i, cpyPrev = 1, j, l = 0;
	/*We are using i for line numbers so at some point we can
	 * multithread parsing for larger files.
	 * OpenFOAM point files can have hundreds of thousands of poitns.*/
	for (i = 0; fgets(line, sizeof(line), f) != NULL; i++) {
		/*Look for point count*/
		if (i > 0 && (!strcmp(line, "(\n") || !strcmp(line, "(\r\n")) && prevLine != NULL) {
			/*TODO make the parser multithreaded with the total point size
			 * This can be calculated by just the i offset + 1 and then fseek to a chunk*/
			mesh->faces.cap = atoi(prevLine) + 1;
			mesh->faces.data = (float**)realloc(mesh->faces.data,
				sizeof(float*) * mesh->faces.cap);
			cpyPrev = 0;
			continue;
		}

		if ((line[0]=='4'&&line[1] == '(') && line[1] != '\n') {
			char* lcpy = calloc(strlen(line) + 1, sizeof(char));
			strcpy(lcpy, line);
			lcpy+=2;
			mesh->faces.data[mesh->faces.size] = calloc(ISIZE, sizeof(float));
			for (j = 0; j < ISIZE; j++) {
				while (lcpy[0] == ' ' || lcpy[0] == '(') lcpy++;
				// I want you to know I hate that MSVC does not allow for varied array sizes I.E. buf[strlen(str)]
				char* buf = calloc(strlen(lcpy) + 1, sizeof(char));
				int l = 0;
				for (; lcpy[0] != ' ' && lcpy[0] != '\n' && lcpy[0] != ')';
					lcpy++, l++) buf[l] = lcpy[0];
				buf[l] = '\0';
				mesh->faces.data[mesh->faces.size][j] = atoi(buf);
				mesh->faces.total++;
				free(buf);
			}
			/*printf("(%f, %f, %f, %f)\n", mesh->faces.data[mesh->faces.size][0],
				mesh->faces.data[mesh->faces.size][1],
				mesh->faces.data[mesh->faces.size][2],
				mesh->faces.data[mesh->faces.size][3]);*/
			mesh->faces.size++;
			continue;
		}

		/*check for end of file
		 * All these compares are probably pretty slow*/
		if (!strcmp(line, ")\n") || !strcmp(line, ")") || !strcmp(line, ")\r\n")) break;
		if (cpyPrev) strcpy(prevLine, line);
	}

	free(prevLine);

	/*Turn the faces into indices*/
	mesh->indices.cap = mesh->faces.size;
	mesh->indices.size = 0;
	mesh->indices.total = 0;
	mesh->indices.data = (uint16_t **)realloc(mesh->indices.data, 
		mesh->indices.cap*sizeof(uint16_t *));

	for (j = 0; j < mesh->indices.cap; j++) {
		mesh->indices.data[j] = calloc(ISIZE+2, sizeof(uint16_t));

		mesh->indices.data[j][0] = mesh->faces.data[j][0];
		mesh->indices.data[j][1] = mesh->faces.data[j][1];
		mesh->indices.data[j][2] = mesh->faces.data[j][2];

		mesh->indices.data[j][3] = mesh->faces.data[j][0];
		mesh->indices.data[j][4] = mesh->faces.data[j][2];
		mesh->indices.data[j][5] = mesh->faces.data[j][3];

		mesh->indices.size++;
		mesh->indices.total+=6;
	}

	return NULL;
}


void *parseFoamPoints(FILE *f, OEFOAMMesh *mesh) {
	/*EXPECTS AN ALREADY OPENED FILE*/
	//OEThreadArg *args_ = (OEThreadArg *)args;
	//if (args_->meshptr == NULL || args_->file == NULL) return NULL;
	//FILE *f = (FILE *)args_->file;
	if(f==NULL) return NULL;
	//OEFOAMMesh *mesh = (OEFOAMMesh *)args_->meshptr;

	char line[2048];
	char *prevLine = calloc(2048, sizeof(char));
	int i, cpyPrev=1, j, l=0;
	/*We are using i for line numbers so at some point we can
	 * multithread parsing for larger files.
	 * OpenFOAM point files can have hundreds of thousands of poitns.*/
	for(i=0;fgets(line, sizeof(line), f)!=NULL;i++) {
		/*Look for point count*/
		if(i>0&&(!strcmp(line, "(\n")||!strcmp(line, "(\r\n"))&&prevLine!=NULL) {
			/*TODO make the parser multithreaded with the total point size
			 * This can be calculated by just the i offset + 1 and then fseek to a chunk*/
			mesh->verts.cap = atoi(prevLine)+1;
			mesh->verts.data = (float **)realloc(mesh->verts.data, 
					sizeof(float *)*mesh->verts.cap);
			cpyPrev=0;
			continue;			
		}

		if(line[0]=='('&&line[1]!='\n') {
			char *lcpy = calloc(strlen(line)+1, sizeof(char));
			strcpy(lcpy, line);
			lcpy++;
			mesh->verts.data[mesh->verts.size] = calloc(ISIZE, sizeof(float));
			for(j=0;j<VSIZE;j++) {
				while(lcpy[0]==' '||lcpy[0]=='(') lcpy++;
				// I want you to know I hate that MSVC does not allow for varied array sizes I.E. buf[strlen(str)]
				char *buf = calloc(strlen(lcpy)+1, sizeof(char));
				int l = 0;
				for (; lcpy[0] != ' ' && lcpy[0] != '\n' && lcpy[0] != ')';
					lcpy++, l++) buf[l] = lcpy[0];
				buf[l] = '\0';
				mesh->verts.data[mesh->verts.size][j] = atof(buf);
				mesh->verts.total++;
				free(buf);
			}
			mesh->verts.size++;
			continue;
		}

		/*check for end of file
		 * All these compares are probably pretty slow*/
		if(!strcmp(line, ")\n")||!strcmp(line, ")")||!strcmp(line, ")\r\n")) break;
		if(cpyPrev) strcpy(prevLine, line);
	}

	free(prevLine);

	return NULL;
}

void OEParseMagnitudeTimeStamp(char *path, int timeStamp, OEFOAMMesh *mesh) {
	if(path==NULL||mesh==NULL) return;
	/* Path should look similar to: C:/repos/aburn/usr/modules/NewModule/cubeTest/pitzDaily/1/U */
	FILE *magFile = fopen(path, "r");
	if(magFile==NULL) return;

	struct OEMagnitude defaultMagnitude = {0};
	if(mesh->magnitudeTS==NULL || 
		memcmp(mesh->magnitudeTS, &defaultMagnitude, sizeof(struct OEMagnitude)) == 0) {
		mesh->maxTS = MAXTIMESTAMPS;
		mesh->sizeTS = 0;

		mesh->magnitudeTS = calloc(mesh->maxTS, sizeof(struct OEMagnitude));
	}

	int i, cpyPrev=1, j, l=0;
	struct OEMagnitude *mag = &mesh->magnitudeTS[mesh->sizeTS];
	mesh->sizeTS++;

	mag->timeStamp = timeStamp;
	mag->values.cap = MAXMAGDATA;
	mag->values.size = 0;
	mag->values.total = 0;
	mag->values.data = calloc(mag->values.cap, sizeof(float *));

	char line[2048];
	char *prevLine = calloc(2048, sizeof(char));

	for(i=0;fgets(line, sizeof(line), magFile)!=NULL;i++) {
		/*Look for point count*/
		if(i>0&&(!strcmp(line, "(\n")||!strcmp(line, "(\r\n"))&&prevLine!=NULL) {
			mag->values.cap = atoi(prevLine)+1;
			mag->values.data = (float **)realloc(mag->values.data, 
					sizeof(float *)*mag->values.cap);
			cpyPrev=0;
			continue;			
		}

		if(line[0]=='('&&line[1]!='\n') {
			char *lcpy = calloc(strlen(line)+1, sizeof(char));
			strcpy(lcpy, line);
			lcpy++;
			mag->values.data[mag->values.size] = calloc(ISIZE, sizeof(float));
			for(j=0;j<VSIZE;j++) {
				while(lcpy[0]==' '||lcpy[0]=='(') lcpy++;
				// I want you to know I hate that MSVC does not allow for varied array sizes I.E. buf[strlen(str)]
				char *buf = calloc(strlen(lcpy)+1, sizeof(char));
				int l = 0;
				for (; lcpy[0] != ' ' && lcpy[0] != '\n' && lcpy[0] != ')';
					lcpy++, l++) buf[l] = lcpy[0];
				buf[l] = '\0';
				mag->values.data[mag->values.size][j] = atof(buf);
				mag->values.total++;
				free(buf);
			}
			mag->values.size++;
			continue;
		}

		/*check for end of file
		 * All these compares are probably pretty slow*/
		if(!strcmp(line, ")\n")||!strcmp(line, ")")||!strcmp(line, ")\r\n")) break;
		if(cpyPrev) strcpy(prevLine, line);
	}

	free(prevLine);

}

/*Parse single integer OpenFOAM mesh file 
 *Looks like:
 * 25061
 * (
 * 1
 * 2 . . . 
 */
void parseSingleOFAtoiStream(FILE *f, int **ptr, int *size, int *cap) {
	if(f == NULL) return;

	int i, j, k, cpyPrev=1;
	char line[2048];
	char* prevLine = calloc(2048, sizeof(char));
	for (i = 0; fgets(line, sizeof(line), f) != NULL; i++) {
		/*Look for point count*/
		if (i > 0 && (!strcmp(line, "(\n") || !strcmp(line, "(\r\n")) && prevLine != NULL) {
			*cap = atoi(prevLine) + 1;
			if(*ptr != NULL) free(*ptr);
			*ptr = calloc(*cap, sizeof(int));
			cpyPrev = 0;
			continue;
		}
	
		if(!cpyPrev) {
			char* buf = calloc(strlen(line)+1, sizeof(char));
			for(j=0;line[j]!='\n'&&line[j]!='\r'; j++) buf[j] = line[j];
			(*ptr)[*size] = atoi(buf);
			(*size)++;
			free(buf);
		}

		if(!strcmp(line, ")\n") || !strcmp(line, ")") || !strcmp(line, ")\r\n")) break;
		if(cpyPrev) strcpy(prevLine, line);
	}

	free(prevLine);
}

void parseFoamOwner(FILE *fowner, OEFOAMMesh *mesh) {
	parseSingleOFAtoiStream(fowner, &mesh->owner, &mesh->osize, &mesh->ocap);
}

void parseFoamNeighbor(FILE *fneighbour, OEFOAMMesh *mesh) {
	parseSingleOFAtoiStream(fneighbour, &mesh->neighbour, &mesh->nsize, &mesh->ncap);
}

void OEParseFOAMObj(char *path, OEFOAMMesh *mesh) {
	if(mesh==NULL) mesh = calloc(1, sizeof(OEFOAMMesh));
	mesh->magnitudeTS = NULL;

	char *points = calloc(strlen(path)+128, sizeof(char));
	char *faces = calloc(strlen(path)+128, sizeof(char));
	char *owner = calloc(strlen(path) + 128, sizeof(char));
	char *neighbour = calloc(strlen(path) + 128, sizeof(char));
	sprintf(points, "%s/points", path);
	sprintf(faces, "%s/faces", path);
	sprintf(owner, "%s/owner", path);
	sprintf(neighbour, "%s/neighbour", path);

	FILE *fpoints = fopen(points, "r");
	FILE *ffaces = fopen(faces, "r");
	FILE *fowner = fopen(owner, "r");
	FILE *fneighbour = fopen(neighbour, "r");

	//OEThreadArg pargs = {(void *)fpoints, (void *)mesh};
	//OEThreadArg fargs = {(void *)ffaces, (void *)mesh};
	
	//startThreadArg("FOAMPoints", parseFoamPoints, (void *)&pargs);
	//startThreadArg("FOAMFaces", parseFoamFaces, (void *)&fargs);

	mesh->verts.cap = MAXDATA;
	mesh->verts.size = 0;
	mesh->verts.total = 0;
	mesh->verts.data = calloc(mesh->verts.cap, sizeof(float*));
	mesh->faces.cap = MAXDATA;
	mesh->faces.size = 0;
	mesh->faces.total = 0;
	mesh->faces.data = calloc(mesh->verts.cap, sizeof(float*));
	mesh->indices.cap = MAXDATA;
	mesh->indices.size = 0;
	mesh->indices.total = 0;
	mesh->indices.data = calloc(mesh->indices.cap, sizeof(uint16_t*));
	mesh->osize = 0;
	mesh->nsize = 0;
	mesh->ocap = 0;
	mesh->ncap = 0;
	/*Size gets allocated after size is found in the file*/
	mesh->owner = NULL;
	mesh->neighbour = NULL;

	parseFoamPoints(fpoints, mesh);
	parseFoamFaces(ffaces, mesh);

	parseFoamOwner(fowner, mesh);
	parseFoamNeighbor(fneighbour, mesh);

	free(points);
	free(faces);
	free(owner);
	free(neighbour);

	fclose(ffaces);
	fclose(fpoints);
	fclose(fowner);
	fclose(fneighbour);
}

void OEParseObj(char *name, char *file, OEMesh *mesh) {
	if(mesh==NULL) mesh = calloc(1, sizeof(OEMesh));
	
	mesh->f = fopen(file, "r");
	/*check for .obj & OpenFOAM file types*/
	WASSERT(mesh->f!=NULL&&(strstr(file, ".obj")||
					strstr(file, "points")||strstr(file, "faces")), "Failed to open .obj file: %s", file);


	initMeshData(mesh);	


	char line[2048];
	while(fgets(line, sizeof(line), mesh->f)!=NULL) {
		int n = getObjLabel(line, mesh);
		if(n) continue;
		if(n==2) break;
		if(checkObjVerts(line, mesh)) continue;
		if(checkObjTex(line, mesh)) continue;
		if(checkObjNorm(line, mesh)) continue;
		if(checkObjIndices(line, mesh)) continue;
	}

	mesh->label = name;
	if(mesh->label!=NULL) {
		// I want you to know I hate that MSVC does not allow for varied array sizes I.E. buf[strlen(str)]
		char *buf = calloc(strlen(mesh->label)+128, sizeof(char));
		sprintf(buf, "Successfully loaded model: %s", mesh->label);
		WLOG(INFO, buf);
		free(buf);
	}
	fclose(mesh->f);

}

/*scales all verts by s*/
void scaleMesh(OEMesh *mesh, float s) {
	if(mesh==NULL&&mesh->verts.data==NULL) return;
	int i,j;
	for(i=0;i<mesh->verts.size;i++) {
		for(j=0;j<VSIZE;j++) mesh->verts.data[i][j]*=s;
	}
}


