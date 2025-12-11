
// MesEmulator.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CMesEmulatorApp:
// See MesEmulator.cpp for the implementation of this class
//

class CMesEmulatorApp : public CWinApp
{
public:
	CMesEmulatorApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CMesEmulatorApp theApp;