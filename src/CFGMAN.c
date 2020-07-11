/*
 * CFGMAN.c
 * 
 * Configuration Management
 * 
 */

#include "tomlc99/toml.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "HW/SCREEN/SCRNEMDV.h"

toml_table_t* CONFIG;
const char CONFIG_PATH[] = "uvmac-cfg.toml";

/* Load existing config */
static bool Config_TryLoad()
{
	FILE* fp;
	char errbuf[200];

	/* Open the file. */
	fp = fopen("uvmac-cfg.toml", "r");
	if (fp == NULL) {
		return false;
	}

	/* Run the file through the parser. */
	CONFIG = toml_parse_file(fp, errbuf, sizeof(errbuf));
	if (CONFIG == NULL) {
		fclose(fp);
		return false;
	}

	fclose(fp);
	return true;
}

/* Create new configuration */
static bool Config_TryCreate()
{
	/* TODO: implement this. or really, TOML creation in general. */
	char errbuf[200];
	CONFIG = toml_parse("", errbuf, sizeof(errbuf));
	return (CONFIG != NULL);
}

/* Load or create config and prepare for use */
bool Config_TryInit()
{
	bool okay;
	okay = Config_TryLoad();
	if (!okay) { okay = Config_TryCreate(); }
	if (!okay) { return false; }
	
	// Initialize per-device configuration
	okay = Screen_LoadCfg();
	if (!okay) { return false; }
	
	return true;
}

/* Locate a raw value given a table/key pair */
static bool Config_GetRawValue(const char table[], const char key[], toml_raw_t *value)
{
	toml_table_t *table_raw;
	
	/* Locate the table. */
	table_raw = toml_table_in(CONFIG, table);
	if (table_raw == NULL) { return false; }
	
	/* Locate the key */
	*value = toml_raw_in(table_raw, key);
	if (*value == NULL) { return false; }
	return true;
}

/* Check if key exists */
bool Config_KeyExists(const char table[], const char key[])
{
	toml_raw_t value_raw;
	return Config_GetRawValue(table, key, &value_raw);
}

/* Read a boolean */
bool Config_GetBool(const char table[], const char key[], bool *value, bool defaultValue)
{
	toml_raw_t value_raw;
	bool found, okay;
	int result;
	
	found = Config_GetRawValue(table, key, &value_raw);
	if (!found) { 
		// TODO: write default to TOML file
		*value = defaultValue;
		return true;
	}
	
	okay = (toml_rtob(value_raw, &result) == 0);
	*value = result;
	return okay;
}

/* Read an integer */
bool Config_GetInt(const char table[], const char key[], int64_t *value, int64_t defaultValue)
{
	toml_raw_t value_raw;
	bool found;
	
	found = Config_GetRawValue(table, key, &value_raw);
	if (!found) { 
		// TODO: write default to TOML file
		*value = defaultValue;
		return true;
	}
	
	return (toml_rtoi(value_raw, value) == 0);
}

/* Read a double-precision floating point number */
bool Config_GetFloat(const char table[], const char key[], double *value, double defaultValue)
{
	toml_raw_t value_raw;
	bool found;
	
	found = Config_GetRawValue(table, key, &value_raw);
	if (!found) { 
		// TODO: write default to TOML file
		*value = defaultValue;
		return true;
	}
	
	return (toml_rtod(value_raw, value) == 0);
}

/* Read a string value */
bool Config_GetString(const char table[], const char key[], char **value, const char *defaultValue)
{
	toml_raw_t value_raw;
	bool found;
	
	found = Config_GetRawValue(table, key, &value_raw);
	if (!found) { 
		// TODO: write default to TOML file
		int defaultLen = strlen(defaultValue)+1;
		*value = malloc(defaultLen);
		strncpy(*value, defaultValue, defaultLen);
		
		return true;
	}
	
	return (toml_rtos(value_raw, value) == 0);
}
