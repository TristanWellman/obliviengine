/*Copyright (c) 2025 Tristan Wellman
 *
 * This is for parsing .obj files as well as OpenFOAM mesh vert/face files
 *
 * */

#include "meshParse.h"

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
			mesh->vertNorms.data[mesh->vertNorms.size] = calloc(VSIZE, sizeof(float));
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
	char *tmp = strstr(line, "vt");
	if(tmp!=NULL) {
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
		char *is = strchr(line, 'f')+1;
		if(is!=NULL) {
			mesh->indices.data[mesh->indices.size] = calloc(ISIZE, sizeof(uint16_t));
			int j;
			for(j=0;j<ISIZE;j++) {
				while(is[0]==' ')is++;
				char buf[128];
				int i = 0;
				for(;is[0]!=' '&&is[0]!='\n';is++,i++) buf[i]=is[0];
				char *tmp = strtok(buf, "/");
				mesh->indices.data[mesh->indices.size][j] = atoi(tmp);
				mesh->indices.total++;
			}
			/*printf("(%d, %d, %d, %d)\n", mesh->indices.data[mesh->indices.size][0],
					mesh->indices.data[mesh->indices.size][1],
					mesh->indices.data[mesh->indices.size][2],
					mesh->indices.data[mesh->indices.size][3]);*/
			mesh->indices.size++;
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

void OEParseObj(char *file, OEMesh *mesh) {
	if(mesh==NULL) mesh = calloc(1, sizeof(OEMesh));
	
	mesh->f = fopen(file, "r");
	/*check for .obj & OpenFOAM file types*/
	WASSERT(mesh->f!=NULL&&(strstr(file, ".obj")||
					strstr(file, "points")||strstr(file, "faces")), 
			"Failed to open .obj file!");

	mesh->verts.cap = MAXDATA;
	mesh->verts.size = 0;
	mesh->verts.total = 0;
	mesh->verts.data = calloc(mesh->verts.cap, sizeof(float*));
	mesh->vertTex.cap = MAXDATA;
	mesh->vertTex.size = 0;
	mesh->vertTex.total = 0;
	mesh->vertTex.data = calloc(mesh->verts.cap, sizeof(float*));
	mesh->vertNorms.cap = MAXDATA;
	mesh->vertNorms.size = 0;
	mesh->vertNorms.total = 0;
	mesh->vertNorms.data = calloc(mesh->verts.cap, sizeof(float*));
	mesh->indices.cap = MAXDATA;
	mesh->indices.size = 0;
	mesh->indices.total = 0;
	mesh->indices.data = calloc(mesh->verts.cap, sizeof(uint16_t*));

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

	if(mesh->label!=NULL) {
		char buf[strlen(mesh->label)+128];
		sprintf(buf, "Successfully loaded model: %s", mesh->label);
		WLOG(INFO, buf);
	}

}

/*scales all verts by s*/
void scaleMesh(OEMesh *mesh, float s) {
	if(mesh==NULL&&mesh->verts.data==NULL) return;
	int i,j;
	for(i=0;i<mesh->verts.size;i++) {
		for(j=0;j<VSIZE;j++) mesh->verts.data[i][j]*=s;
	}
}


