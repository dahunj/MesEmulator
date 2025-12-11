// Equip.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "MesEmulator.h"
#include "Equip.h"


#define	EQUIP_IP	"127.0.0.1"
#define EQUIP_PORT	11000		// Equip Handler Port


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

	Set_OperUpdate(gData.sOperID);
	Set_EquipState(2);	//Idle
	g_objLogFile.Save_MesAgentLog("MesAgent Connected");
	return 0;
}

LRESULT CEquip::OnClientClose(WPARAM wParam, LPARAM lParam)
{
	m_bConnected = FALSE;
	m_bHostOnline = FALSE;
	m_Client.Close_Socket();
	g_objLogFile.Save_MesAgentLog("MesAgent Disconnected");
	return 0;
}

LRESULT CEquip::OnClientReceive(WPARAM wParam, LPARAM lParam)
{
	BYTE byRecv[1025] = { 0 };	// Buffer 1024, Last 0x00
	int nLen = m_Client.Read_Socket(byRecv);

	CString strRecvSocket, strLog;
	strRecvSocket.Format("%s", byRecv);
	m_strRecvCmd += strRecvSocket;

	while (!m_strRecvCmd.IsEmpty()) {
		int nStart = m_strRecvCmd.Find("@");
		int nEnd = m_strRecvCmd.Find("\n");

		if (nEnd < 0) break;	// 버퍼에 들어오는 중...

		if (nStart < 0 || nStart > nEnd) {
			strLog.Format("[<-] : <<Error>> %s : Start(%d), End(%d)", m_strRecvCmd, nStart, nEnd);
			g_objLogFile.Save_MesAgentLog(strLog);
			m_strRecvCmd.Delete(0, nEnd + 1);	// 쓰레기값이 채워져 있어서...
			continue;
		}

		CString strRecv = m_strRecvCmd.Mid(nStart + 1, nEnd - nStart - 1);
		m_strRecvCmd.Delete(0, nEnd + 1);

		// Inspector Log ////////////////////////////////////////////////////////////
		strLog.Format("[<-] : %s", strRecv);
		g_objLogFile.Save_MesAgentLog(strLog);
		/////////////////////////////////////////////////////////////////////////////

		char chSep = ',';
		CString strCmd, strOp;

		AfxExtractSubString(strCmd, strRecv, 0, chSep);
		AfxExtractSubString(strOp, strRecv, 1, chSep);

		CString strArg[10];
		for (int i = 0; i < 5; i++) AfxExtractSubString(strArg[i], strRecv, i + 2, chSep);

		if (strCmd == "CONTROL") {
			if (strOp == "STATE") Get_ControlState(strArg[0]);

		} else if (strCmd == "LOT") {
			if (strOp == "START")  Get_LotStart(strArg[0], strArg[1], strArg[2], strArg[3], strArg[4]);
			if (strOp == "CANCEL") Get_LotCancel(strArg[0], strArg[1],  strArg[2]);

		} else if (strCmd == "TIME") {
			if (strOp == "UPDATE") Get_TimeSync();

		} else if (strCmd == "RECIPE") {
			if (strOp == "REQUEST") Get_RecipeList(strArg[0]);
			if (strOp == "SELECT")	Get_PPSelect(strArg[0], strArg[1]);
			if (strOp == "FAIL")	Get_PPSelectFail(strArg[0], strArg[1], strArg[2], strArg[3]);

		} else if (strCmd == "CM") {
			if (strOp == "RESULT") Get_CmResult(strArg[0], strArg[1],  strArg[2], strArg[3], strArg[4]);
			if (strOp == "FAIL")   Get_CmFail(strArg[0], strArg[1],  strArg[2], strArg[3]);

		} else if (strCmd == "HOST") {
			if (strOp == "MESSAGE") Get_HostMessage(strArg[0]);

		} else if (strCmd == "MGZ") {
			if (strOp == "CONFIRM") Get_MGZConfirm(strArg[0]);
			if (strOp == "CANCEL")  Get_MGZCancel(strArg[0], strArg[1],  strArg[2]);

		} else if (strCmd == "CARRIER") {
			if (strOp == "CONFIRM") Get_CarrierConfirm(strArg[0]);
			if (strOp == "CANCEL")  Get_CarrierCancel(strArg[0], strArg[1],  strArg[2]);

		} else if (strCmd == "MODULE") {
			if (strOp == "DATA") Get_ModuleData(strRecv);

		}
	}

	return 0;
}
