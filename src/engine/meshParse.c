/*Copyright (c) 2025 Tristan Wellman
 *
 * This is for parsing .obj files as well as OpenFOAM mesh vert/face files
 *
 * */

#include "meshParse.h"

void checkObjNorm(char *line, OEMesh *mesh) {
	char *tmp = strstr(line, "vn");
	if(tmp!=NULL) {

	}
}

void checkObjTex(char *line, OEMesh *mesh) {
	char *tmp = strstr(line, "vt");
	if(tmp!=NULL) {

	}
}

void checkObjVerts(char *line, OEMesh *mesh) {
	if(line[0]=='v') {
		if(mesh->verts.size>=mesh->verts.cap) {
			mesh->verts.cap+=MAXDATA;
			mesh->verts.data = (double **)realloc(mesh->verts.data, 
					sizeof(double*)*mesh->verts.cap);
		}
		char *vs = strchr(line, 'v');
		if(vs!=NULL) {
			mesh->verts.data[mesh->verts.size] = calloc(VSIZE, sizeof(double));
			int i;
			for(i=0;i<3;i++) {
				char *buf = "";
				for(;vs[0]!=' '&&vs[0]!='\n';vs++) buf+=vs[0];
				mesh->verts.data[mesh->verts.size][i] = atof(buf);
			}
			mesh->verts.size++;
		}
	}
}

void getObjLabel(char *line, OEMesh *mesh) {
	if(line[0]=='o') {
		char *name = strchr(line, 'o');
		if(name!=NULL) {
			while(name[0]==' ') name++;
			mesh->label = calloc(strlen(name)+1, sizeof(char));
			strcpy(mesh->label, name);
		}
	}
}

void OEParseObj(char *file, OEMesh *mesh) {
	if(mesh==NULL) mesh = calloc(1, sizeof(OEMesh));
	
	mesh->f = fopen(file, "r");
	WASSERT(mesh->f!=NULL&&strstr(file, ".obj"), 
			"Failed to open .obj file!");


	mesh->verts.cap = MAXDATA;
	mesh->verts.size = 0;
	mesh->verts.data = calloc(mesh->verts.cap, sizeof(double*));
	mesh->vertTex.cap = MAXDATA;
	mesh->vertTex.size = 0;
	mesh->vertTex.data = calloc(mesh->verts.cap, sizeof(double*));
	mesh->vertNorms.cap = MAXDATA;
	mesh->vertNorms.size = 0;
	mesh->vertNorms.data = calloc(mesh->verts.cap, sizeof(double*));
	mesh->indices.cap = MAXDATA;
	mesh->indices.size = 0;
	mesh->indices.data = calloc(mesh->verts.cap, sizeof(double*));

	char line[2048];
	while(fgets(line, sizeof(line), mesh->f)!=NULL) {
		getObjLabel(line, mesh);
		checkObjVerts(line, mesh);
		checkObjTex(line, mesh);
		checkObjNorm(line, mesh);
	}

}
