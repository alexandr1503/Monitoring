/*
NTPort Library
Copyright (c) 1997-2005 Hai Li, Zeal SoftStudio.
E-Mail: support@zealsoft.com
Web: http://www.zealsoft.com
*/
#ifndef NTPort

#define NTPort

#ifndef WINAPI
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Returns a value indicating the last operation is successful or not.
int WINAPI GetLastState(char *s);

// Enable your application to read or write specific ports.
void WINAPI EnablePorts(WORD PortStart, WORD PortStop);

// Disable your application to read or write specific ports.
void WINAPI DisablePorts(WORD PortStart, WORD PortStop);

// Returns a value indicates whether the application is running under Windows NT/2000/XP/Server 2003 system.
BOOL WINAPI IsWinNT();

// Returns a value indicates whether the application is running under 64-bit Windows system.
BOOL APIENTRY IsWin64();

// Returns a value from specific ports.
WORD WINAPI Inp(WORD PortNum);
WORD WINAPI Inport(WORD PortNum);
WORD WINAPI Inpw(WORD PortNum);
WORD WINAPI InportW(WORD PortNum);
DWORD WINAPI Inpd(WORD PortNum);
DWORD WINAPI InportD(WORD PortNum);

// Write a value to specific ports.
void WINAPI Outp(WORD PortNum, WORD Data);
void WINAPI Outport(WORD PortNum, WORD Data);
void WINAPI Outpw(WORD PortNum, WORD Data);
void WINAPI OutportW(WORD PortNum, WORD Data);
void WINAPI Outpd(WORD PortNum, DWORD Data);
void WINAPI OutportD(WORD PortNum, DWORD Data);

// Set the registration information.
void WINAPI LicenseInfo(LPSTR sUserName, DWORD dwKey);

// Returns the version of NTPort Library.
WORD WINAPI GetNTPortVersion();

// Set the setting of fast mode
void WINAPI SetFastMode(BOOL bOption);

// Get the current setting of fast mode
BOOL WINAPI GetFastMode();

// Get the base address of LPT port
WORD WINAPI GetLPTPortAddress(WORD PortNo);

static const int ERROR_NONE = 0;
static const int ERROR_DRIVER = 2;
static const int ERROR_SCM_CANT_CONNECT = 9998;

#ifdef __cplusplus
}
#endif

#endif // NTPORT
