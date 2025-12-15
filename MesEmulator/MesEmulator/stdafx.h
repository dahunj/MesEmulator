
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions




#include "CSClientSocket.h"


#ifdef _DEBUG

#pragma comment(lib, "CSIniFileD.lib")
#pragma comment(lib, "CSClientSocketD.lib")

#else
#pragma comment(lib, "CSIniFileR.lib")
#pragma comment(lib, "CSClientSocketR.lib")

#endif



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC support for ribbons and control bars


extern CString gsCurrentDir;

typedef struct {
	int			nHostPort;
	CString		sEquipId;
	BOOL		bHandlerLog;
	BOOL		bHostLog;
	CString		sErrFile;
	BOOL		bJahwa;
	CString		sOperId;

	CString		sCurrentRecipe;
	int			nRcpCount;
	CString		sRecipList[100];

	int			nPreEquipState;		// 1:init, 2:idle, 3:Setup, 4:Ready, 5:Run, 6:Pause(Down)
	int			nCurEquipState;

	CString		sHandlerLotID[6];
	CString		sHandlerPortID[6];
	CString		sHandlerCMCount[6];

	CString		sBarcode[6][20][40];
	CString		sJudge[6][20][40];
	CString		sNgCode[6][20][40];

	int			nAlarmID;
	CString		sAlarmTxt;
	CString		sGMESData[11];
	CString		sVersion;
	CString		sBodyData[4][50];	// 0:T1M-PC2(1~32), 1:T1S-PC4(1~32), 2:T2M-PC3(1~44), 3:T2S-PC5(1~44)
} GLOVAL_DATA;

extern GLOVAL_DATA gData;