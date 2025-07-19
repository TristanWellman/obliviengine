/*Copyright (c) 2025, Tristan Wellman*/
#ifndef OESCRIPT_H
#define OESCRIPT_H
#define OELUALIBSIZE 10 
#define OENORET 0

typedef struct {
	char *filePath;
} OEScript;

typedef struct {
	lua_State *lState;
	struct luaL_Reg *OELib;
	int size, cap;
} OELuaData;

void OEInitLua(OELuaData *data);

#endif
