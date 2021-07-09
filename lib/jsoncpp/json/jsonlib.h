#ifndef JSONCPP_LIB_H_H
#define JSONCPP_LIB_H_H

#ifdef WIN32

#ifdef JSONCPP_DLL
	#define JSON_API  _declspec(dllexport)
#else
	#define JSON_API  _declspec(dllimport)
#endif

#else
	#define JSON_API
#endif

#endif

