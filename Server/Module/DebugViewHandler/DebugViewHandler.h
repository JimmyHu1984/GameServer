#ifndef DEBUG_VIEW_HANDLER_H
#define DEBUG_VIEW_HANDLER_H
#pragma once

#define DebugString(log){\
	std::stringstream ss;\
	ss << log;\
	OutputDebugStringA(ss.str().c_str());\
}\

#endif