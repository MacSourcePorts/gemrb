/* GemRB - Infinity Engine Emulator
 * Copyright (C) 2013 The GemRB Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 */

#include "globals.h"

#include "InterfaceConfig.h"

#include "Streams/FileStream.h"
#include "System/VFS.h"

namespace GemRB {

InterfaceConfig::InterfaceConfig(int /*argc*/, char** /**argv[]*/)
{
	// currently the base class has no CLI options.
	configVars = new StringMap();
	configVars->init(50, 10);
}

InterfaceConfig::~InterfaceConfig() noexcept
{
	delete configVars;
}

void InterfaceConfig::SetKeyValuePair(const char* key, const char* value)
{
	// lowercase the key so that the key is not case sensitive
	char* keyCopy = strdup(key);
	for (char* c = keyCopy; *c != '\0'; ++c) *c = tolower(*c);
	configVars->set(keyCopy, value);
	free(keyCopy);
}

const char* InterfaceConfig::GetValueForKey(const char* key) const
{
	const char* value = NULL;
	if (key) {
		// lowercase the key so that the key is not case sensitive
		char* keyCopy = strdup(key);
		for (char* c = keyCopy; *c != '\0'; ++c) *c = tolower(*c);
		if (configVars->get(keyCopy)) {
			value = configVars->get(keyCopy)->c_str();
		}
		free(keyCopy);
	}
	return value;
}

CFGConfig::CFGConfig(int argc, char *argv[])
	: InterfaceConfig(argc, argv)
{
	FileStream* config = new FileStream();
	// skip arg0 (it is just gemrb)
	for (int i=1; i < argc; i++) {
		if (stricmp(argv[i], "-c") == 0) {
			const char* filename = argv[++i];

			if (!config->Open(filename)) {
				// Explicitly specified cfg file HAS to be present
				Log(FATAL, "Config", "Failed to open config file \"{}\".", filename);
			}
			isValid = InitWithINIData(config);
		} else if (stricmp(argv[i], "-q") == 0) {
			// quiet mode
			SetKeyValuePair("AudioDriver", "none");
		} else {
			// assume a path was passed, soft force configless startup
			SetKeyValuePair("GamePath", argv[i]);
			isValid = true;
		}
	}
	if (!isValid) {
		// nothing passed in on CLI, so search for gemrb.cfg
		char datadir[_MAX_PATH];
		char path[_MAX_PATH];
		char name[_MAX_PATH];

		// Find basename of this program. It does the same as basename (3),
		// but that's probably missing on some archs
		const char* appName = strrchr(argv[0], PathDelimiter);
		if (appName) {
			appName++;
		} else {
			appName = argv[0];
		}

		strlcpy(name, appName, _MAX_PATH);
		assert(name[0]);

#if TARGET_OS_MAC
		// CopyGemDataPath would give us bundle resources dir
		CopyHomePath(datadir, _MAX_PATH);
		PathAppend(datadir, PACKAGE);
#else
		CopyGemDataPath(datadir, _MAX_PATH);
#endif
		PathJoinExt( path, datadir, name, "cfg" );

#define ATTEMPT_INIT \
if (config->Open(path) \
	&& InitWithINIData(config)) { \
		goto done; \
	}

		ATTEMPT_INIT;

#ifdef SYSCONF_DIR
		PathJoinExt( path, SYSCONF_DIR, name, "cfg" );
		ATTEMPT_INIT
#endif

#ifndef ANDROID
		// Now try ~/.gemrb folder
		CopyHomePath(datadir, _MAX_PATH);
		char confpath[_MAX_PATH] = ".";
		strcat(confpath, name);
		PathJoin(datadir, datadir, confpath, nullptr);
		PathJoinExt( path, datadir, name, "cfg" );
		ATTEMPT_INIT;
#endif
		// Don't try with default binary name if we have tried it already
		if (strcmp( name, PACKAGE ) != 0) {
			PathJoinExt( path, datadir, PACKAGE, "cfg" );

			ATTEMPT_INIT;

#ifdef SYSCONF_DIR
			PathJoinExt( path, SYSCONF_DIR, PACKAGE, "cfg" );
			ATTEMPT_INIT;
#endif
		}
		// if all else has failed try current directory
		PathJoinExt(path, "./", PACKAGE, "cfg");
		ATTEMPT_INIT;
	}
#undef ATTEMPT_INIT
done:
	delete config;
}

bool CFGConfig::InitWithINIData(DataStream* cfgStream)
{
	if (cfgStream == NULL) {
		return false;
	}

	if (isValid) {
		Log(WARNING, "Config", "attempting to replace config values with contents of {}", cfgStream->filename);
	} else {
		Log(MESSAGE, "Config", "attempting to initialize config with {} found at:", cfgStream->filename);
	}
	Log(MESSAGE, "Config", "{}", cfgStream->originalfile);

	isValid = false;
	int lineno = 0;
	char line[1024];
	while (cfgStream->Remains()) {
		char *key, *keyend, *value, *valueend;

		if (cfgStream->ReadLine(line, _MAX_PATH) == -1) {
			break;
		}
		lineno++;

		// skip leading blanks from name
		key = line;
		key += strspn( line, " \t\r\n" );

		// ignore empty or comment lines
		if (*key == '\0' || *key == '#') {
			continue;
		}

		value = strchr( key, '=' );
		if (!value || value == key) {
			Log(WARNING, "Config", "Invalid line {}", lineno);
			continue;
		}

		// trim trailing blanks from name
		keyend = value;
		while (keyend > key && strchr( "= \t", *keyend )) {
			*keyend-- = '\0';
		}

		value++;
		// skip leading blanks
		value += strspn( value, " \t");

		// trim trailing blanks from value
		valueend = value + strlen( value ) - 1;
		while (valueend >= value && strchr( " \t\r\n", *valueend )) {
			*valueend-- = '\0';
		}
		SetKeyValuePair(key, value);
	}
	isValid = true;
	return true;
}

}

