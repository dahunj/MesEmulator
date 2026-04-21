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

	static int nLotNo = 0;

	CMesEmulatorDlg *pMainDlg = (CMesEmulatorDlg*)AfxGetMainWnd();
	if (!m_xml.LoadXml(sXmlData) ) 
	{
		CString strLog, strMsg;

		strMsg.Format("[Extract_Xml] CXml Data Load Fail.");
		//pMainDlg->Set_HostMsg(strMsg);

		strLog.Format("%s\n%s", strMsg, sXmlData);
		g_objLogFile.Save_EquipLog(strLog);

		return FALSE;
	}

	CXmlNode node = m_xml.GetRoot();
	m_strStFn = node.GetAttribute("ID");

	if (m_strStFn == "S6F11")
	{
		CXmlNode node = m_xml.GetRoot()->GetChild("ITEM")->GetChild("CEID");
		m_strRcmd = node.GetAttribute("VALUE", "");
		if(m_strRcmd == "20310")
		{
			
			//Set_S6F12_CarrierOutReport();
		}
		else if(m_strRcmd =="20301")
		{
			
			//Set_S6F12_CarrierIDReport();
			//Sleep(10);
			
		}
		else if(m_strRcmd =="40103")
		{
			Set_S2F49_LOT_START();
			Set_S2F49_LOT_MODULE_DATA_DETAIL();
			//Set_S6F12_CarrierIDReport();
			//Sleep(10);

		}
		else if(m_strRcmd =="20106")
		{
			//if(nLotNo >=5) 
			//Set_S6F12_LotIDReport();
			//Set_S2F49_PP_SELECT();

			CXmlNodes nodes = m_xml.GetRoot()->GetChild("ITEM")->GetChild("DVLIST")->GetChildren();
			int nCount = nodes.GetCount();
			CString strName = nodes[2]->GetAttribute("VALUE");			
						
			gData.sHandlerLotID[nLotNo] = strName;
			//gData.sHandlerLotID[nLotNo].Format("TEST-%d", nTemp);
					
			Set_S2F49_LotStart(0);
			//nLotNo++;
		}
		else if(m_strRcmd =="40102") //get PP SELECTED REPORT 
		{
			Set_S7F25();
		}
	}	
	else if(m_strStFn == "S2F50")
	{
		CXmlNode node = m_xml.GetRoot()->GetChild("ITEM")->GetChild("RCMDCP")->GetChild("RCMD");
		m_strRcmd = node.GetAttribute("VALUE", "");
		if(m_strRcmd == "PP_SELECT")
		{
			
		}
		else if(m_strRcmd == "LOT_START")
		{

		}
	}
	else if(m_strStFn == "S7F26")
	{
		CXmlNode node = m_xml.GetRoot()->GetChild("ITEM")->GetChild("PCLIST");
		m_strRcmd = node.GetAttribute("COUNT", "");
		int nTemp = atoi(m_strRcmd);

		CXmlNodes nodes = m_xml.GetRoot()->GetChild("ITEM")->GetChild("PCLIST")->GetChildren();
		int nCount = nodes.GetCount();

		/*for (int i = 0; i < nCount; i++) 
		{
		CString strName = nodes[i]->GetChild("CCODE")->GetAttribute("VALUE");
		CString strData = nodes[i]->GetChild("PPARM")->GetAttribute("VALUE");

		}*/
	/*	if(nCount < nTemp)
		{
			return FALSE;
		}*/
		Set_S2F49_PP_UPLOAD_CONFIRM();

	}
	m_xml.Close();
	return TRUE;
}


void CEquip::Set_S6F12_LotIDReport()
{
	gData.sEquipId = "AVI-TEST";

	CString strSend = "<?xml version=\"1.0\" encoding=\"utf-16\"?>" + CRLF;

	strSend += "<EIF VERSION=\"2.0\" ID=\"S6F12\" NAME=\"Event Report Acknowledge\">" + CRLF;
	strSend += "  <ELEMENT>" + CRLF;
	strSend += "    <EQPID VALUE=\"" + gData.sEquipId + "\" />" + CRLF;
	strSend += "  </ELEMENT>" + CRLF;
	strSend += "  <ITEM>" + CRLF;
	strSend += "    <CEID NAME=\"CEID\" VALUE=\"20106\"/>" + CRLF;
	strSend += "    <RPTID NAME=\"RPTID\" VALUE=\"20106\"/>" + CRLF;
	strSend += "    <ACKC NAME=\"ACKC\" VALUE=\"0\"/>" + CRLF;
	strSend += "  </ITEM>" + CRLF;
	strSend += "</EIF>";

	Send_Command(strSend, FALSE, "S6F12");
}



void CEquip::Set_S2F49_LotStart(int nLotNo)
{
	gData.sEquipId = "AVI-TEST";
	
	CString strSend = "<?xml version=\"1.0\" encoding=\"utf-16\"?>" + CRLF;

	strSend += "<EIF VERSION=\"2.0\" ID=\"S2F49\" NAME=\"Enhanced Remote Command\">" + CRLF;
	strSend += "  <ELEMENT>" + CRLF;
	strSend += "    <EQPID VALUE=\"" + gData.sEquipId + "\" />" + CRLF;
	strSend += "  </ELEMENT>" + CRLF;
	strSend += "  <ITEM>" + CRLF;
	strSend += "    <RCMDCP>" + CRLF;
	strSend += "      <RCMD NAME=\"RCMD\" VALUE=\"LOT_START\" />" + CRLF;
	strSend += "      <CPLIST COUNT=\"5\">" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"TIME\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"20251117001910\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"PROCID\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"AA10599\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"MODEL\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"MAMVSZ0A0A.KM00\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"VENDOR_TYPE\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"DPAMS\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"MODEL_CONFIG_CODE\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"DPAMS - AKMMV\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"LOTID\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\""+ gData.sHandlerLotID[0]+"\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"RECIPEID\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"A34B_DPAMS_REV0\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"TOTALQTY\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"80\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"OPERATORID\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"DPAMS - AKMMV\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "      </CPLIST>" + CRLF;	  
	strSend += "      <RESULT>" + CRLF;
	strSend += "        <CODE NAME=\"CODE\" VALUE=\"\" />" + CRLF;
	strSend += "        <TEXT VALUE=\"CODE\" />" + CRLF;
	strSend += "      </RESULT>" + CRLF;
	strSend += "    </RCMDCP>" + CRLF;
	strSend += "  </ITEM>" + CRLF;
	strSend += "</EIF>";

	Send_Command(strSend, FALSE, "S2F49");
}


void CEquip::Set_S6F12_CarrierIDReport()
{
	gData.sEquipId = "AVI-TEST";

	CString strSend = "<?xml version=\"1.0\" encoding=\"utf-16\"?>" + CRLF;

	strSend += "<EIF VERSION=\"2.0\" ID=\"S6F12\" NAME=\"Event Report Acknowledge\">" + CRLF;
	strSend += "  <ELEMENT>" + CRLF;
	strSend += "    <EQPID VALUE=\"" + gData.sEquipId + "\" />" + CRLF;
	strSend += "  </ELEMENT>" + CRLF;
	strSend += "  <ITEM>" + CRLF;
	strSend += "    <CEID NAME=\"CEID\" VALUE=\"20301\"/>" + CRLF;
	strSend += "    <RPTID NAME=\"RPTID\" VALUE=\"20301\"/>" + CRLF;
	strSend += "    <ACKC NAME=\"ACKC\" VALUE=\"0\"/>" + CRLF;
	strSend += "  </ITEM>" + CRLF;
	strSend += "</EIF>";

	Send_Command(strSend, FALSE, "S6F12");
}


void CEquip::Set_S6F12_CarrierOutReport()
{
	gData.sEquipId = "AVI-TEST";

	CString strSend = "<?xml version=\"1.0\" encoding=\"utf-16\"?>" + CRLF;

	strSend += "<EIF VERSION=\"2.0\" ID=\"S6F12\" NAME=\"Event Report Acknowledge\">" + CRLF;
	strSend += "  <ELEMENT>" + CRLF;
	strSend += "    <EQPID VALUE=\"" + gData.sEquipId + "\" />" + CRLF;
	strSend += "  </ELEMENT>" + CRLF;
	strSend += "  <ITEM>" + CRLF;
	strSend += "    <CEID NAME=\"CEID\" VALUE=\"20310\"/>" + CRLF;
	strSend += "    <RPTID NAME=\"RPTID\" VALUE=\"20310\"/>" + CRLF;
	strSend += "    <ACKC NAME=\"ACKC\" VALUE=\"0\"/>" + CRLF;
	strSend += "  </ITEM>" + CRLF;
	strSend += "</EIF>";

	Send_Command(strSend, FALSE, "S6F12");
}



void CEquip::Set_S2F3_LINK_REQUEST()
{
	gData.sEquipId = "AVI-TEST";

	CString strSend = "<?xml version=\"1.0\" encoding=\"utf-16\"?>" + CRLF;

	strSend += "<EIF VERSION=\"2.0\" ID=\"S2F3\" NAME=\"Link Test Request\">" + CRLF;
	strSend += "  <ELEMENT>" + CRLF;
	strSend += "    <EQPID VALUE=\"" + gData.sEquipId + "\" />" + CRLF;
	strSend += "  </ELEMENT>" + CRLF;	
	strSend += "</EIF>";

	Send_Command(strSend, FALSE, "S6F12");
}



void CEquip::Set_S2F49_PP_SELECT()
{
	gData.sEquipId = "AVI-TEST";
	CString strSend = "<?xml version=\"1.0\" encoding=\"utf-16\"?>" + CRLF;

	strSend += "<EIF VERSION=\"2.0\" ID=\"S2F49\" NAME=\"Enhanced Remote Command\">" + CRLF;
	strSend += "  <ELEMENT>" + CRLF;
	strSend += "    <EQPID VALUE=\"" + gData.sEquipId + "\" />" + CRLF;
	strSend += "  </ELEMENT>" + CRLF;
	strSend += "  <ITEM>" + CRLF;
	strSend += "    <RCMDCP>" + CRLF;
	strSend += "      <RCMD NAME=\"RCMD\" VALUE=\"PP_SELECT\" />" + CRLF;
	strSend += "      <CPLIST COUNT=\"5\">" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"TIME\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"20251215000000\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"LOTID\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"TEST000000001\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"PROCID\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"TEST001\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"PRODUCTID\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"MAMV.KM00\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"RECIPEID\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"A53B_DPAMS_REV0\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "      </CPLIST>" + CRLF;	  
	strSend += "      <RESULT>" + CRLF;
	strSend += "        <CODE NAME=\"CODE\" VALUE=\"\" />" + CRLF;
	strSend += "        <TEXT VALUE=\"CODE\" />" + CRLF;
	strSend += "      </RESULT>" + CRLF;
	strSend += "    </RCMDCP>" + CRLF;
	strSend += "  </ITEM>" + CRLF;
	strSend += "</EIF>";

	Send_Command(strSend, FALSE, "S2F49");
}


void CEquip::Set_S2F49_PP_UPLOAD_CONFIRM()
{
	gData.sEquipId = "AVI-TEST";

	CString strSend = "<?xml version=\"1.0\" encoding=\"utf-16\"?>" + CRLF;

	strSend += "<EIF VERSION=\"2.0\" ID=\"S2F49\" NAME=\"Enhanced Remote Command\">" + CRLF;
	strSend += "  <ELEMENT>" + CRLF;
	strSend += "    <EQPID VALUE=\"" + gData.sEquipId + "\" />" + CRLF;
	strSend += "  </ELEMENT>" + CRLF;

	strSend += "  <ITEM>" + CRLF;
	strSend += "    <RCMDCP>" + CRLF;
	strSend += "      <RCMD NAME=\"RCMD\" VALUE=\"PP_UPLOAD_CONFIRM\" />" + CRLF;
	strSend += "      <CPLIST COUNT=\"3\">" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"TIME\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"20251215000000\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"LOTID\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"TEST_LOT\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"RECIPEID\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"A53B_DPAMS_REV0\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "      </CPLIST>" + CRLF;
	strSend += "      <RESULT>" + CRLF;
	strSend += "        <CODE NAME=\"CODE\" VALUE=\"\" />" + CRLF;
	strSend += "        <TEXT NAME=\"CODE\" VALUE=\"\" />" + CRLF;
	strSend += "      </RESULT>" + CRLF;
	strSend += "    </RCMDCP>" + CRLF;
	strSend += "  </ITEM>" + CRLF;
	strSend += "</EIF>";

	Send_Command(strSend, FALSE, "S2F49");
}


//[00:29:40 147] [<-] 0000082837341<?xml version="1.0" encoding="utf-16"?>
//	<EIF VERSION="2.0" ID="S2F49" NAME="Enhanced Remote Command">
//	<ELEMENT>
//	<EQPID VALUE="AVI-00413" />
//	</ELEMENT>
//	<ITEM>
//	<RCMDCP>
//	<RCMD NAME="RCMD" VALUE="PP_UPLOAD_CONFIRM" />
//	<CPLIST COUNT="3">
//	<CP>
//	<CPNAME NAME="CPNAME" VALUE="TIME" />
//	<CPVAL NAME="CPVAL" VALUE="20251210002939" />
//	</CP>
//	<CP>
//	<CPNAME NAME="CPNAME" VALUE="LOTID" />
//	<CPVAL NAME="CPVAL" VALUE="GPSZ0A0BFC91EYA" />
//	</CP>
//	<CP>
//	<CPNAME NAME="CPNAME" VALUE="RECIPEID" />
//	<CPVAL NAME="CPVAL" VALUE="A53B_DPAMS_REV0" />
//	</CP>
//	</CPLIST>
//	<RESULT>
//	<CODE NAME="CODE" VALUE="" />
//	<TEXT VALUE="" />
//	</RESULT>
//	</RCMDCP>
//	</ITEM>
//	</EIF>



//------------------
//[00:32:04 747] [->] 0000050272441<?xml version="1.0" encoding="utf-16"?>
//	<EIF VERSION="2.0" ID="S6F11" NAME="Event Report">
//	<ELEMENT>
//	<EQPID VALUE="AVI-00413" />
//	</ELEMENT>
//	<ITEM>
//	<CEID NAME="CEID" VALUE="40103" />
//	<RPTID NAME="RPTID" VALUE="40103" />
//	<DVLIST COUNT="4">
//	<DV NAME="TIME" VALUE="20251210003204" />
//	<DV NAME="LOTID" VALUE="GPSZ0A0BFC91GYA" />
//	<DV NAME="RECIPEID" VALUE="A53B_DPAMS_REV0" />
//	<DV NAME="OPERATORID" VALUE="113447" />
//	</DVLIST>
//	</ITEM>
//	</EIF>
//	[00:32:04 790] [<-] 0000031072441<?xml version="1.0" encoding="utf-16"?>
//	<EIF VERSION="2.0" ID="S6F12" NAME="Event Report Acknowledge">
//	<ELEMENT>
//	<EQPID VALUE="AVI-00413" />
//	</ELEMENT>
//	<ITEM>
//	<CEID NAME="CEID" VALUE="40103" />
//	<RPTID NAME="RPTID" VALUE="40103" />
//	<ACKC NAME="ACKC" VALUE="0" />
//	</ITEM>
//	</EIF>
//	[00:32:05 087] [<-] 0000165839171<?xml version="1.0" encoding="utf-16"?>
//	<EIF VERSION="2.0" ID="S2F49" NAME="Enhanced Remote Command">
//	<ELEMENT>
//	<EQPID VALUE="AVI-00413" />
//	</ELEMENT>
//	<ITEM>
//	<RCMDCP>
//	<RCMD NAME="RCMD" VALUE="TRAY_LOT_START" />
//	<CPLIST COUNT="10">
//	<CP>
//	<CPNAME NAME="CPNAME" VALUE="TIME" />
//	<CPVAL NAME="CPVAL" VALUE="20251210003205" />
//	</CP>
//	<CP>
//	<CPNAME NAME="CPNAME" VALUE="PROCID" />
//	<CPVAL NAME="CPVAL" VALUE="AA10152" />
//	</CP>
//	<CP>
//	<CPNAME NAME="CPNAME" VALUE="MODEL" />
//	<CPVAL NAME="CPVAL" VALUE="MAMVSZ0A0B.KM00" />
//	</CP>
//	<CP>
//	<CPNAME NAME="CPNAME" VALUE="VENDOR_TYPE" />
//	<CPVAL NAME="CPVAL" VALUE="DPAMS" />
//	</CP>
//	<CP>
//	<CPNAME NAME="CPNAME" VALUE="MODEL_CONFIG_CODE" />
//	<CPVAL NAME="CPVAL" VALUE="DPAMS - Compeq" />
//	</CP>
//	<CP>
//	<CPNAME NAME="CPNAME" VALUE="LOTID" />
//	<CPVAL NAME="CPVAL" VALUE="GPSZ0A0BFC91GYA" />
//	</CP>
//	<CP>
//	<CPNAME NAME="CPNAME" VALUE="TRAYID" />
//	<CPVAL NAME="CPVAL" VALUE="" />
//	</CP>
//	<CP>
//	<CPNAME NAME="CPNAME" VALUE="RECIPEID" />
//	<CPVAL NAME="CPVAL" VALUE="A53B_DPAMS_REV0" />
//	</CP>
//	<CP>
//	<CPNAME NAME="CPNAME" VALUE="TOTALQTY" />
//	<CPVAL NAME="CPVAL" VALUE="40" />
//	</CP>
//	<CP>
//	<CPNAME NAME="CPNAME" VALUE="OPERATORID" />
//	<CPVAL NAME="CPVAL" VALUE="113447" />
//	</CP>
//	</CPLIST>
//	</RCMDCP>
//	</ITEM>
//	</EIF>


void CEquip::Set_S2F49_LOT_START()
{
	gData.sEquipId = "AVI-TEST";

	CString strSend = "<?xml version=\"1.0\" encoding=\"utf-16\"?>" + CRLF;

	strSend += "<EIF VERSION=\"2.0\" ID=\"S2F49\" NAME=\"Enhanced Remote Command\">" + CRLF;
	strSend += "  <ELEMENT>" + CRLF;
	strSend += "    <EQPID VALUE=\"" + gData.sEquipId + "\" />" + CRLF;
	strSend += "  </ELEMENT>" + CRLF;

	strSend += "  <ITEM>" + CRLF;
	strSend += "    <RCMDCP>" + CRLF;
	strSend += "      <RCMD NAME=\"RCMD\" VALUE=\"LOT_START\" />" + CRLF;
	strSend += "      <CPLIST COUNT=\"10\">" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"TIME\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"20251215000000\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"PROCID\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"AA10152\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"MODEL\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"MAMVSZ0A0B.KM00\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"VENDOR_TYPE\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"DPAMS\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"MODEL_CONFIG_CODE\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"DPAMS - Compeq\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"LOTID\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"TEST000000001\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"TRAYID\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"MAMVSZ0A0B.KM00\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"RECIPEID\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"A34B_DPAMS_REV0\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"TOTALQTY\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"40\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"OPERATORID\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"113447\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "      </CPLIST>" + CRLF;
	strSend += "      <RESULT>" + CRLF;
	strSend += "        <CODE NAME=\"CODE\" VALUE=\"\" />" + CRLF;
	strSend += "        <TEXT NAME=\"CODE\" VALUE=\"\" />" + CRLF;
	strSend += "      </RESULT>" + CRLF;
	strSend += "    </RCMDCP>" + CRLF;
	strSend += "  </ITEM>" + CRLF;
	strSend += "</EIF>";

	Send_Command(strSend, FALSE, "S2F49");
}




void CEquip::Set_S2F49_LOT_MODULE_DATA_DETAIL()
{
	gData.sEquipId = "AVI-TEST";

	CString strSend = "<?xml version=\"1.0\" encoding=\"utf-16\"?>" + CRLF;

	strSend += "<EIF VERSION=\"2.0\" ID=\"S2F49\" NAME=\"Enhanced Remote Command\">" + CRLF;
	strSend += "  <ELEMENT>" + CRLF;
	strSend += "    <EQPID VALUE=\"" + gData.sEquipId + "\" />" + CRLF;
	strSend += "  </ELEMENT>" + CRLF;

	strSend += "  <ITEM>" + CRLF;
	strSend += "    <RCMDCP>" + CRLF;
	strSend += "      <RCMD NAME=\"RCMD\" VALUE=\"LOT_MODULE_DATA_DETAIL\" />" + CRLF;
	strSend += "      <CPLIST COUNT=\"10\">" + CRLF;
	strSend += "        <CP>" + CRLF;
	strSend += "          <CPNAME NAME=\"CPNAME\" VALUE=\"TIME\" />" + CRLF;
	strSend += "          <CPVAL NAME=\"CPVAL\" VALUE=\"20251215000000\" />" + CRLF;
	strSend += "        </CP>" + CRLF;
	strSend += "      </CPLIST>" + CRLF;
	strSend += "      <RESULT>" + CRLF;
	strSend += "        <CODE NAME=\"CODE\" VALUE=\"\" />" + CRLF;
	strSend += "        <TEXT NAME=\"CODE\" VALUE=\"\" />" + CRLF;
	strSend += "      </RESULT>" + CRLF;
	strSend += "    </RCMDCP>" + CRLF;
	strSend += "  </ITEM>" + CRLF;
	strSend += "</EIF>";

	Send_Command(strSend, FALSE, "S2F49");
}


void CEquip::Set_S7F25()
{
	gData.sEquipId = "AVI-TEST";

	CString strSend = "<?xml version=\"1.0\" encoding=\"utf-16\"?>" + CRLF;

	strSend += "<EIF VERSION=\"2.0\" ID=\"S7F25\" NAME=\"Formatted Process Program Request\">" + CRLF;
	strSend += "  <ELEMENT>" + CRLF;
	strSend += "    <EQPID VALUE=\"" + gData.sEquipId + "\" />" + CRLF;
	strSend += "  </ELEMENT>" + CRLF;	
	strSend += "  <ITEM>" + CRLF;
	strSend += "    <PPID NAME=\"PPID\" VALUE=\"A53B_DPAMS_REV0\" />" + CRLF;
	strSend += "    <LOTID NAME=\"LOTID\" VALUE=\"TEST-LOT\" />" + CRLF;
	strSend += "  </ITEM>" + CRLF;	
	strSend += "</EIF>";

	Send_Command(strSend, FALSE, "S7F25");
}

//
//[00:29:39 968] [<-] 0000030337331<?xml version="1.0" encoding="utf-16"?>
//	<EIF VERSION="2.0" ID="S7F25" NAME="Formatted Process Program Request">
//	<ELEMENT>
//	<EQPID VALUE="AVI-00413" />
//	</ELEMENT>
//	<ITEM>
//	<PPID NAME="PPID" VALUE="A53B_DPAMS_REV0" />
//	<LOTID NAME="LOTID" VALUE="GPSZ0A0BFC91EYA" />
//	</ITEM>
//	</EIF>
//

void CEquip::Send_Command(CString sSend, BOOL bReply, CString sStFn, CString sRcmd)
{
	CString strLog, strMsg, strSendSocket, strTemp;

	int nLen = sSend.GetLength();

	if (!bReply) m_nSendCmdCount < 9999 ? m_nSendCmdCount++ : m_nSendCmdCount = 1;
	int nCount = (bReply ? m_nRecvCmdCount : m_nSendCmdCount);

	strSendSocket.Format("%c%08d%04d1%s%c", STX, nLen, nCount, sSend, ETX);

	char chSend[2000] = { 0 };	// Max 2000
	int nLength = strSendSocket.GetLength();

	if (nLength > 2000) 
	{
		int nSendCount = strSendSocket.GetLength() / 2000 + 1;
		for (int i = 0; i < nSendCount; i++)
		{
			strTemp = strSendSocket.Mid(i * 2000, 2000);
			memset(chSend, 0x00, sizeof(char) * 2000);
			memcpy(chSend, strTemp, strTemp.GetLength());
			int nLenTemp = strTemp.GetLength();
			if (!m_Client.Write_Socket((BYTE*)chSend, nLenTemp)) return;
		}
	} 
	else
	{
		memcpy(chSend, (LPSTR)(LPCSTR)strSendSocket, nLength);
		if (!m_Client.Write_Socket((BYTE*)chSend, nLength)) return;
	}

	// Host Log //////////////////////////////////////////////////////////////////
	strLog.Format("[->] %08d%04d1%s", nLen, nCount, sSend);
	g_objLogFile.Save_EquipLog(strLog);

	strMsg.Format("%s : %s,%s", strLog.Left(18), sStFn, sRcmd);
	CMesEmulatorDlg *pMainDlg = (CMesEmulatorDlg*)AfxGetMainWnd();
	//pMainDlg->Set_HostMsg(strMsg);
}