/*
 * CFGMAN.h
 * 
 * Configuration Management
 * 
 */

/* Load or create config and prepare for use */
#pragma once
#include <stdint.h>

bool Config_TryInit();

/* Check if key exists */
bool Config_KeyExists(const char table[], const char key[]);
/* Read a boolean */
bool Config_GetBool(const char table[], const char key[], bool *value, bool defaultValue);
/* Read an integer */
bool Config_GetInt(const char table[], const char key[], int64_t *value, int64_t defaultValue);
/* Read a double-precision floating point number */
bool Config_GetFloat(const char table[], const char key[], double *value, double defaultValue);
/* Read a string value */
bool Config_GetString(const char table[], const char key[], char **value, const char *defaultValue);

/* List of known tables and keys */
#define CONFIG_SCC_REALPORT(MACPORT)      "SCC_#MACPORT", "RealPort"
#define CONFIG_SCC_REALPORT(MACPORT)      "SCC_#MACPORT", "RealPort"

#define CONFIG_VIDEO_BLACK                "Video", "ColorBlack"
#define CONFIG_VIDEO_WHITE                "Video", "ColorWhite"
#define CONFIG_VIDEO_WIDTH                "Video", "Width"
#define CONFIG_VIDEO_HEIGHT               "Video", "Height"
#define CONFIG_VIDEO_DEPTH                "Video", "Depth"
#define CONFIG_VIDEO_USEHACK              "Video", "UseLargeScreenHack"

#define DISK_PATH(NUM)                    "Disk", "Path#NUM"
