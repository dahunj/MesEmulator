#pragma once

#include "./CXml/Xml.h"

using namespace JWXml;
// CEquip

class CEquip : public CWnd
{
	DECLARE_DYNAMIC(CEquip)

public:
	CEquip();
	virtual ~CEquip();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnClientConnect(WPARAM wConnect, LPARAM lParam);
	afx_msg LRESULT OnClientReceive(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnClientClose(WPARAM wParam, LPARAM lParam);

private:
	CClientSocketCS m_Client;

	BOOL	m_bConnected;
	BOOL	m_bEquipmentOnline;

	CXml	m_xml;

	CString m_strRecvCmd, m_sXMLData;

	int		m_nRecvCmdCount;	// 4Byte
	int		m_nSendCmdCount;

	CString m_strStFn;	// StreamFunction (S1F1, S2F3, S2F31, S2F49, S6F12)
	CString m_strRcmd;	// RCMD Command (START, CANCEL, DATA, PERMIT)

	DWORD	m_dwLastTime;	// 마지막 통신 시간
	CString m_strSetTime;	// Host 설정 시간

	int		m_nS1F4AckNo;	
	CString m_sHostMsg;
	CString m_sLastProcID;

public:
	void Initialize();
	void Terminate();

	BOOL Is_Connected() { return m_bConnected; }
	BOOL Is_HostOnline() { return m_bEquipmentOnline; }
	
	BOOL CEquip::Extract_Xml(CString sXmlData);

private:
	void Get_S6F11_CarrierIDReport();
	void Get_S6F11_CarrierOutReport();

};


extern CEquip g_objEquip;