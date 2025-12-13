// Equip.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "MesEmulator.h"
#include "Equip.h"
#include "LogFile.h"
#include "MesEmulatorDlg.h"


#define	EQUIP_IP	"127.0.0.1"
#define EQUIP_PORT	11000		// Equip Handler Port

const char STX = 0x02;
const char ETX = 0x03;
const CString CRLF = "\r\n";

CEquip g_objEquip;
// CEquip

IMPLEMENT_DYNAMIC(CEquip, CWnd)

CEquip::CEquip()
{
	m_bConnected = FALSE;
	m_bEquipmentOnline = FALSE;
	m_strRecvCmd = "";
}

CEquip::~CEquip()
{
}


BEGIN_MESSAGE_MAP(CEquip, CWnd)
	ON_MESSAGE(UM_CLIENT_CONNECT, OnClientConnect)
	ON_MESSAGE(UM_CLIENT_RECEIVE, OnClientReceive)
	ON_MESSAGE(UM_CLIENT_CLOSE, OnClientClose)
END_MESSAGE_MAP()



// CEquip 메시지 처리기입니다.


void CEquip::Initialize()
{
	if (m_bConnected) return;
	
	m_bConnected = m_Client.Open_Socket(EQUIP_IP, EQUIP_PORT, this);	
	Sleep(1000);

}

void CEquip::Terminate()
{
	m_bConnected = FALSE;
	m_bEquipmentOnline = FALSE;
	m_Client.Close_Socket();

	//g_objLogFile.Save_MesAgentLog("MesAgent Terminate.");	Sleep(500);
}


LRESULT CEquip::OnClientConnect(WPARAM wConnect, LPARAM lParam)
{
	m_bConnected = (BOOL)wConnect;
	if (!m_bConnected) return 0;

	//
	g_objLogFile.Save_EquipLog("Equip Connected");
	return 0;
}

LRESULT CEquip::OnClientClose(WPARAM wParam, LPARAM lParam)
{
	m_bConnected = FALSE;
	m_bEquipmentOnline = FALSE;
	m_Client.Close_Socket();
	g_objLogFile.Save_EquipLog("Equip Disconnected");
	return 0;
}

LRESULT CEquip::OnClientReceive(WPARAM wParam, LPARAM lParam)
{
	BYTE byRecv[1025] = { 0 };	// Buffer 1024, Last 0x00
	int nLen = m_Client.Read_Socket(byRecv);

	CString strRecvSocket, strLog;
	strRecvSocket.Format("%s", byRecv);
	m_strRecvCmd += strRecvSocket;

	while (!m_strRecvCmd.IsEmpty()) 
	{
		int nStart = m_strRecvCmd.Find(STX);
		int nEnd = m_strRecvCmd.Find(ETX);

		if (nEnd < 0) break;	// 버퍼에 들어오는 중...

		if (nStart < 0 || nStart > nEnd) 
		{
			strLog.Format("[OnClientReceive] <<Error>> - Start(%d), End(%d).\n%s", nStart, nEnd, m_strRecvCmd);
			g_objLogFile.Save_EquipLog(strLog);
			m_strRecvCmd.Delete(0, nEnd + 1);	// 쓰레기값이 채워져 있어서...
			continue;
		}
		
		CString strRecv = m_strRecvCmd.Mid(nStart + 1, nEnd - nStart - 1);
		m_strRecvCmd.Delete(0, nEnd + 1);

		// Inspector Log ////////////////////////////////////////////////////////////
		strLog.Format("[<-] : %s", strRecv);
		g_objLogFile.Save_EquipLog(strLog);
		/////////////////////////////////////////////////////////////////////////////

		m_nRecvCmdCount = atoi(strRecv.Mid(8, 4));	// 4Byte

		CString strXml = strRecv.Right(strRecv.GetLength() - 13);
		if (!Extract_Xml(strXml)) return 0;

	}

	return 0;
}



BOOL CEquip::Extract_Xml(CString sXmlData)
{
	int k=0;
	m_strStFn = m_strRcmd = "";	// 초기화

	CMesEmulatorDlg *pMainDlg = (CMesEmulatorDlg*)AfxGetMainWnd();
	if (!m_xml.LoadXml(sXmlData) ) {
		CString strLog, strMsg;

		strMsg.Format("[Extract_Xml] CXml Data Load Fail.");
		//pMainDlg->Set_HostMsg(strMsg);

		strLog.Format("%s\n%s", strMsg, sXmlData);
		g_objLogFile.Save_EquipLog(strLog);

		return FALSE;
	}

	CXmlNode node = m_xml.GetRoot();
	m_strStFn = node.GetAttribute("ID");

	
	m_xml.Close();
	return TRUE;
}
