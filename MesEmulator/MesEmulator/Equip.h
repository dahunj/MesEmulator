#pragma once


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
	CString m_strRecvCmd;

public:
	void Initialize();
	void Terminate();

	BOOL Is_Connected() { return m_bConnected; }
	BOOL Is_HostOnline() { return m_bEquipmentOnline; }
	

};


extern CEquip g_objEquip;