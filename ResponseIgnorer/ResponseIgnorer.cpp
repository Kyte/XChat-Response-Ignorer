/*														*
 *	Blah blah Copyright Mauricio Salinas Abarca 2011	*
 *	Kindari you better be grateful						*
 *														*/

#include "precompiled.h"
#include <algorithm>
#include <map>
#include <string>
#include <cctype>
#include "ResponseIgnorer.h"
#include "xchat-plugin.h"

#define PNAME "ResponseIgnorer"
#define PDESC "Ignores messages meant for people who've been /ignored."
#define PVERSION "1.0.3"
/*	0.1.0 <-- Got it to load on XChat		*
 *	0.2.0 <-- Figured out a command hook	*
 *	0.3.0 <-- Grabbed /ignore data			*
 *	0.4.0 <-- /ignore acknowledged			*
 *	0.5.0 <-- /unignore added				*
 *	????									*
 *	1.0.0 <-- Holy shit it's filtering		*/

using std::string;
using std::map;
using std::pair;

static xchat_plugin* ph;
map<string, int> ignoreMap;
map<string, ignoreFlags> flagMap;

int isnotalnum(int c) { return !std::isalnum(c); }

string reduceToNick(const string& mask) {
	string nick(mask);

	string::iterator it;
	for (it = nick.begin(); it != nick.end() && isnotalnum(*it); it++);
	nick.erase(std::remove_if(nick.begin(), it, isnotalnum), it);
	for (it; it != nick.end() && isalnum(*it); it++);
	nick.erase(it, nick.end());

	return nick;
}

void addToMap(string mask, int flags) {
	ignoreMap[reduceToNick(mask)] = flags;
}

static int ignore_cb(char* words[], char* words_eol[], void*) {
	map<string, ignoreFlags>::iterator found;
	string mask(words[2]);
	int flags = ig_None;

	for (size_t i = 3; words[i][0] != '\0'; i++) {
		string flag(words[i]);
		std::transform(flag.begin(), flag.end(), flag.begin(), ::toupper);

		found = flagMap.find(flag);
		if (found != flagMap.end()) flags |= found->second;
	}
	addToMap(mask, flags);

	return XCHAT_EAT_NONE;
}

static int unignore_cb(char* words[], char**, void*) {
	string mask(words[2]);
	map<string, int>::iterator found = ignoreMap.find(reduceToNick(mask));
	if (found != ignoreMap.end()) ignoreMap.erase(found);

	return XCHAT_EAT_NONE;
}

static int server_cb(char* words[], char* words_eol[], void*) {
	string targetNick = reduceToNick(string(words[4]));

	for each (pair<string, int> pair in ignoreMap) {
		if ((xchat_nickcmp(ph, pair.first.c_str(), targetNick.c_str()) == 0) && ((pair.second & (ig_Private | ig_Channel)) != ig_None))
			return XCHAT_EAT_XCHAT;
	}

	return XCHAT_EAT_NONE;
}

static int ignoreList_cb(char**, char**, void*) {
	xchat_print(ph, "--- TRACKED IGNORES ---");
	for each (pair<string, int> pair in ignoreMap) {
		xchat_printf(ph, "%s - %d", pair.first.c_str(), pair.second);
	}

	return XCHAT_EAT_ALL;
}

void fillFlagMap() {
	flagMap["PRIV"] = ig_Private;
	flagMap["CHAN"] = ig_Channel;
	flagMap["NOTI"] = ig_Notice;
	flagMap["CTCP"] = ig_CTCP;
	flagMap["DCC"] = ig_DCC;
	flagMap["INVI"] = ig_Invite;
	flagMap["ALL"] = ig_All;
	flagMap["NOSAVE"] = ig_NoSave;
}

int xchat_plugin_init(xchat_plugin* plugin_handle,
					  char** plugin_name,
					  char** plugin_desc,
					  char** plugin_version,
					  char*) {
	/* we need to save this for use with any xchat_* functions */
	ph = plugin_handle;

	/* tell xchat our info */
	*plugin_name = PNAME;
	*plugin_desc = PDESC;
	*plugin_version = PVERSION;

	fillFlagMap();

	xchat_list* list = xchat_list_get(ph, "ignore");
	if (list) {
		while (xchat_list_next(ph, list)) {
			addToMap(string(xchat_list_str(ph, list, "mask")), xchat_list_int(ph, list, "flags"));
		}
	}
	xchat_list_free(ph, list);

	xchat_hook_server(ph, "PRIVMSG", XCHAT_PRI_NORM, server_cb, NULL);
	xchat_hook_command(ph, "IgnoreList", XCHAT_PRI_NORM, ignoreList_cb, "Returns the tracked ignore list", NULL);
	xchat_hook_command(ph, "Unignore", XCHAT_PRI_NORM, unignore_cb, "Usage: UNIGNORE <mask> [QUIET]", NULL);
	xchat_hook_command(ph, "Ignore", XCHAT_PRI_NORM, ignore_cb, "\nUsage: IGNORE <mask> <types..> <options..>\n"
		"mask - host mask to ignore, eg: *!*@*.aol.com\n"
		"types - types of data to ignore, one or all of:\n"
		"\tPRIV, CHAN, NOTI, CTCP, DCC, INVI, ALL\n"
		"options - NOSAVE, QUIET", NULL);

	xchat_print(ph, "ResponseIgnorer loaded successfully!\n");

	return 1; /* return 1 for success */
}

void xchat_plugin_get_info(char **name, char **desc, char **version, void**) {
	*name = PNAME;
	*desc = PDESC;
	*version = PVERSION;
}