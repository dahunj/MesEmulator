#pragma once
class CLogFile
{
public:
	CLogFile(void);
	~CLogFile(void);

public:
	void Create_Folder(CString sPath);
	void Save_HandlerLog(CString sLog);
};


extern CLogFile g_objLogFile;