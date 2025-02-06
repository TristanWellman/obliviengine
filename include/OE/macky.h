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

/**
 * @file macky.h
 * @author Tristan Wellman
 * @brief Header for Macky Lib
 * */

#ifndef MACKY_H
#define MACKY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MKY_ARRLEN(x) \
		(sizeof(x)/sizeof(x[0]))

#define MAX_ARR_SIZE 1024

/**
 * @typedef MKY_ARR
 * @brief void pointer to hold array data
 * */
typedef void (*MKY_ARR)();

#if !defined __STDBOOL_H
/**
 * @typedef MKY_BOOL
 * @brief bool definition for systems without stdbool
 * */
typedef int (MKY_BOOL);
#define MKY_TRUE 1
#define MKY_FALSE 0
#endif

/**
 * @typedef mky_array
 * @brief Struct for holding array data. User must cast void ptr to valid array
 * */
typedef struct mky_array {
	void *array;
	int array_length;
} mky_array;

/**
 * @typedef mky_data
 * @brief Structure holding file buffers and other data
 * */
typedef struct maky_data {
	FILE *file;
	char *filebuf; /* malloc to size of all data in file */
	int fsize;
} mky_data;

/**
 * @brief Initializes all config file data and sends it into memory
 * 
 * @param filename The name of the configuration file
 * */
mky_data *mky_init(char *filename);


/**
 * @brief retrieves and parses int array from section in macky config file
 *
 * @param section The section at which the item resides in the macky file
 * @param itemName Name of ARRAY in macky file
 * */
mky_array mky_getIntArrayAt(char *section, char *itemName);

/**
 * @brief retrieves and parses int from section in macky config file
 *
 * @param section The section at which the item resides in the macky file
 * @param itemName Name of VALUE in macky file
 * */
int mky_getIntAt(char *section, char *itemName);

/**
 * @brief retrieves and parses float from section in macky config file
 *
 * @param section The section at which the item resides in the macky file
 * @param itemName Name of FLOAT in macky file
 * */
float mky_getFloatAt(char *section, char *itemName);

/**
 * @brief retrieves and parses string from section in macky config file
 *
 * @param section The section at which the item resides in the macky file
 * @param itemName Name of VALUE in macky file
 * */
char *mky_getStrAt(char *section, char *itemName);

#if !defined __STDBOOL_H
/**
 * @brief retrieves and parses bool from section in macky config file
 *
 * @param section The section at which the item resides in the macky file
 * @param itemName Name of VALUE in macky file
 * */
MKY_BOOL mky_getBoolAt(char *section, char *itemName);
#else
/**
 * @brief retrieves and parses bool from section in macky config file
 *
 * @param section The section at which the item resides in the macky file
 * @param itemName Name of VALUE in macky file
 * */

bool mky_getBoolAt(char *section, char *itemName);
#endif

#endif
