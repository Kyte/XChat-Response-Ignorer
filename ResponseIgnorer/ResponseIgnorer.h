#pragma once

enum ignoreFlags : int {
	ig_None = 0,
	ig_Private = 1,
	ig_Notice = 2,
	ig_Channel = 4,
	ig_CTCP = 8,
	ig_Invite = 16,
	ig_UnIgnore = 32,
	ig_NoSave = 64,
	ig_DCC = 128,
	ig_All = 159 /* All but NoSave & UnIgnore */
};