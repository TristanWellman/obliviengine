/*Copyright (c) 2025, Tristan Wellman
 * They got a pretty alright manual: https://www.lua.org/manual/5.4/manual.html
 * I also followed some of this: https://lucasklassmann.com/blog/2019-02-02-embedding-lua-in-c/
 * */
#include <OE/OE.h>
#include <OE/OEScript.h>

int LGetPosition(lua_State *L);
int LSetObjectPosition(lua_State *L);
int LRotateObject(lua_State *L);

void pushOEFunc(OELuaData *data, struct luaL_Reg func) {
	if(data->size>=data->cap) {
		data->cap += OELUALIBSIZE;
		data->OELib = (struct luaL_Reg *)realloc(data->OELib, sizeof(struct luaL_Reg)*data->cap);
	}
	/*data->OELib[data->size].name = calloc(strlen(func.name)+1, sizeof(char));
	strcpy(data->OELib[data->size].name, func.name);
	data->OELib[data->size].func = func.func;*/
	data->OELib[data->size] = (struct luaL_Reg){strdup(func.name), func.func};
	data->size++;
	if(data->size<data->cap) data->OELib[data->size] = (struct luaL_Reg){NULL, NULL};
}


void OEInitLua(OELuaData *data) {
	data->lState = NULL;
	data->size = 0;
	data->cap = OELUALIBSIZE;
	data->OELib = calloc(data->cap, sizeof(struct luaL_Reg));
	data->lState = luaL_newstate();
	WASSERT(data->lState!=NULL, "Failed to initialize Lua!");
	luaL_openlibs(data->lState);

	lua_newtable(data->lState);	
	pushOEFunc(data, (struct luaL_Reg){"setObjectPosition", LSetObjectPosition});
	pushOEFunc(data, (struct luaL_Reg){"rotateObject", LRotateObject});
	luaL_setfuncs(data->lState, data->OELib, 0);
	lua_setglobal(data->lState, "OE");
}

void OEStopLua(OELuaData *data) {
	if(data) lua_close(data->lState);
	/*int i;
	for(i=0;i<data->size;i++) free(data->OELib[i].name);
	free(data->OELib);*/
	data->lState = NULL;
}

/*
 * OE Library functions
 * */
int LSetObjectPosition(lua_State *L) {
	char *id = luaL_checkstring(L, 1);
	Object *obj = OEGetObjectFromName(id);
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	float z = luaL_checknumber(L, 4);
	vec3 pos;
	vec3_dup(pos, (vec3){x, y, z});
	OESetObjectPosition(id, pos);
	return V3SIZE;
}

int LRotateObject(lua_State *L) {
	char *id = luaL_checkstring(L, 1);
	float deg = luaL_checknumber(L, 2);
	OERotateObject(id, deg);
	return 1;
}
