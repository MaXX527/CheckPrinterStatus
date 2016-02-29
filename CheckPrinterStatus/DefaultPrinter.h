#pragma once

#define DEBUG 0

class CDefaultPrinter
{
private:
	const char *m_SubStrCODE      = "CODE=";
	const char *m_SubStrONLINE    = "ONLINE=";
	const char *m_SubStrDISPLAY   = "DISPLAY=";
	const char *m_SubStrPAGECOUNT = "PAGECOUNT";

	const char *m_CmdID = "\x1B%-12345X@PJL\r\n@PJL INFO ID\r\n\x1B%-12345X\r\n";
	const char *m_CmdPageCount = "\x1B%-12345X@PJL\r\n@PJL INFO PAGECOUNT\r\n\x1B%-12345X\r\n";
	const char *m_CmdStatus    = "\x1B%-12345X@PJL\r\n@PJL INFO STATUS\r\n\x1B%-12345X\r\n";

	HANDLE m_hPort;
	char m_CODE[6] = { 0 };
	char m_ONLINE[6] = { 0 };
	char m_DISPLAY[256] = { 0 };
	wchar_t m_wDISPLAY[256] = { 0 };
	char m_PAGECOUNT[16] = { 0 };
	DWORD m_Printer_Status_Code = 0;

	BOOL GetData(const char *);

	void Init();

public:
	CDefaultPrinter();
	CDefaultPrinter(const wchar_t *,const wchar_t *);
	~CDefaultPrinter();

	const char * GetStatusCode();
	const char * GetPageCount();
	const DWORD GetCode();

	wchar_t *m_Name;
	wchar_t *m_Interface;
};

