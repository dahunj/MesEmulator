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

	void Send_Command(CString sSend, BOOL bReply, CString sStFn, CString sRcmd="");

	void Get_S6F11_CarrierIDReport();
	void Get_S6F11_CarrierOutReport();

	void Get_S7F26();

	void Get_S6F11_LotIDReport();


public:
	void Set_S6F12_LotIDReport();
	void Set_S2F49_LotStart(int nLotNo);

	void Set_S6F12_PP_SELECTED_REPORT();



	void Set_S6F12_CarrierIDReport();
	void Set_S6F12_CarrierOutReport();

	void Set_S2F49_PP_SELECT();
	void Set_S2F49_PP_UPLOAD_CONFIRM();
	void Set_S7F25();

	void Set_S2F49_LOT_START();
	void Set_S2F49_LOT_MODULE_DATA_DETAIL();



	void Set_S2F3_LINK_REQUEST();


};


extern CEquip g_objEquip;