// CheckPrinterStatus.cpp: определ€ет точку входа дл€ консольного приложени€.
//

#include "stdafx.h"

using namespace std;

#define BUFSIZE 65536

BOOL gFlagExit = FALSE;

BOOL CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
		// Handle the CTRL-C signal. 
	case CTRL_C_EVENT:
		printf("Ctrl-C event\n\n");
		gFlagExit = TRUE;
		return(TRUE);

	default:
		return FALSE;
	}
}

int main()
{
	std::locale rus("rus_rus.866");
	std::wcout.imbue(rus);

	SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);

//	PRINTER_INFO_2*    list;
//	DWORD            cnt = 0;
//	DWORD            sz = 0;
//	DWORD Level = 2;
//	int i = 0;
//	int sl = 0;

/*	HMODULE hL;
	hL = LoadLibrary(L"c:\\lib\\lm__ac.dll");

	void (*Proc)();
	(FARPROC&)Proc = GetProcAddress(hL, "EnumeratePrinter");
	Proc();

	system("PAUSE");
	return 0;*/

/*	char pPrinterName[256];

	EnumPrinters(PRINTER_ENUM_LOCAL, NULL, Level, NULL, 0, &sz, &cnt);

	list = (PRINTER_INFO_2*)malloc(sz);

	EnumPrinters(PRINTER_ENUM_LOCAL, NULL, Level, (LPBYTE)list, sz, &sz, &cnt);

	cout << "Found printer(s): " << cnt << endl << endl;*/

	DWORD pcchBuffer;
	GetDefaultPrinterA(NULL, &pcchBuffer);
	TCHAR *szDefaultPrinter = new TCHAR[pcchBuffer];
	GetDefaultPrinter(szDefaultPrinter, &pcchBuffer);
	wcout << "Default printer: " << szDefaultPrinter << endl;

/*	for (i = 0; i < (int)cnt; i++)
	{
		CharToOem(list[i].pPrinterName, pPrinterName);
		if (0 == wcscmp(defPrinter, list[i].pPrinterName))
		{
			//cout << "Found default printer" << endl;
			cout << pPrinterName << " : " << list[i].Status << endl;
			if (PRINTER_STATUS_TONER_LOW == list[i].Status)
				cout << "PRINTER_STATUS_TONER_LOW" << endl;
		}
	}
	//free(list);*/

	HANDLE hPrinter;
	OpenPrinter(szDefaultPrinter, &hPrinter, NULL);

	PRINTER_INFO_2 *pPrinter;
	DWORD cbBuf(0), cbNeeded(0);
	GetPrinter(hPrinter, 2, NULL, cbBuf, &cbNeeded);
	cbBuf = cbNeeded;

	pPrinter = (PRINTER_INFO_2*)malloc(cbBuf);
	GetPrinter(hPrinter, 2, (LPBYTE)pPrinter, cbBuf, &cbNeeded);
	wcout << "Status: " << hex << pPrinter->Status << L" " << pPrinter->pLocation << endl;

	free(pPrinter);
	delete szDefaultPrinter;

	/*<ESC>%-12345X@PJL <CR><LF>
		@PJL INFO STATUS[<CR>]<LF>
		<ESC>%-12345X*/

/*	int idx(0);
	GUID ClassGuid;
	while (CR_NO_SUCH_VALUE != CM_Enumerate_Classes(idx, &ClassGuid, 0))
		cout << dec << idx++ << " : " << hex << ClassGuid.Data1 << "-" << ClassGuid.Data2 << "-" << ClassGuid.Data3 << endl;*/
	

	static const GUID GUID_DEVINTERFACE_USBPRINT = { 0x28d78fad,0x5a12,0x11D1,0xae,0x5b,0x00,0x00,0xf8,0x03,0xa8,0xc2 };

	// Ќадеемс€, что принтер только один (((

	HDEVINFO devs;
	SP_DEVINFO_DATA devinfo;
	SP_DEVICE_INTERFACE_DATA devinterface;
	GUID intfce;
	PSP_DEVICE_INTERFACE_DETAIL_DATA_A interface_detail;
	ULONG index(0);
	ULONG requiredLength;

	intfce = GUID_DEVINTERFACE_USBPRINT;
	
	devs = SetupDiGetClassDevs(&intfce, NULL, NULL,	DIGCF_PRESENT | DIGCF_ALLCLASSES | DIGCF_DEVICEINTERFACE);

	ZeroMemory(&devinterface, sizeof(SP_DEVICE_INTERFACE_DATA));
	devinterface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	devinterface.Flags = 0;

	SetupDiEnumDeviceInterfaces(devs, 0, &intfce, index, &devinterface);
	char* interfacename = new char[2048];

	requiredLength = 0;
	SetupDiGetDeviceInterfaceDetailA(devs, &devinterface, NULL, 0, &requiredLength, NULL);

	interface_detail = (SP_DEVICE_INTERFACE_DETAIL_DATA_A*)malloc(requiredLength);
	interface_detail = (PSP_DEVICE_INTERFACE_DETAIL_DATA_A)calloc(1, requiredLength);

	ZeroMemory(interface_detail, sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA));
	interface_detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
	devinfo.cbSize = sizeof(SP_DEVINFO_DATA);
	SetupDiGetDeviceInterfaceDetailA(devs, &devinterface, interface_detail, requiredLength, &requiredLength, &devinfo);

	strcpy_s(interfacename, 0x800, interface_detail->DevicePath);
	cout << "interfacename " << interfacename << endl;


	DWORD pcWritten(0);
	DWORD pcRead(0);
	//const char *pCmd = "<ESC>%-12345X@PJL ECHO Test<CR><LF><ESC>%-12345X<CR><LF>";
	const char *szSubStrCODE = "CODE=";
	const char *szSubStrONLINE = "ONLINE=";
	const char *szSubStrDISPLAY = "DISPLAY=";
	const char *szSubStrPAGECOUNT = "PAGECOUNT";
	const char *pCmd = "\x1B%-12345X@PJL\r\n@PJL INFO PAGECOUNT\r\n\x1B%-12345X\r\n";
	//const char *pCmd = "\x1B%-12345X@PJL\r\n@PJL USTATUSOFF\r\n@PJL USTATUS DEVICE = ON\r\n\x1B%-12345X@PJL\r\n@PJL USTATUS DEVICE\r\n\x1B%-12345X\r\n";

	//std::string cmd = "\x1B%-12345X@PJL INFO PAGECOUNT \n\x1B%-12345X\n";
	//"\\\\?\\USB#VID_043D&PID_0227&MI_01#7&3629265&0&0001#{28d78fad-5a12-11d1-ae5b-0000f803a8c2}"
	HANDLE handle;
	cout << "CreateFile ... ";
	//interfacename
	handle = CreateFileA(interfacename,
		FILE_READ_DATA | FILE_WRITE_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	cout << "ok" << endl;

	if (INVALID_HANDLE_VALUE == handle)
	{
		cout << "Error CreateFile" << endl;
		system("PAUSE");
		return -1;
	}


	DWORD written;
	/*OVERLAPPED Overlapped;
	Overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	WriteFile(handle, pCmd, strlen(pCmd), &written, &Overlapped);
	while(WAIT_TIMEOUT == WaitForSingleObject(Overlapped.hEvent, 100))
	{
		cout << "Wait" << endl;
	}
	GetOverlappedResult(handle, &Overlapped, &written, FALSE);
	cout << "written " << written << endl;*/


	DWORD bytesRead;
	char readBuf[BUFSIZE];
	wchar_t szReadBuf[BUFSIZE];
	ZeroMemory(&readBuf, BUFSIZE);
	ZeroMemory(&szReadBuf, BUFSIZE);
	int i(0);
	char CODE[6] = { 0 };
	char ONLINE[6] = { 0 };
	char DISPLAY[256] = { 0 };
	char PAGECOUNT[16] = { 0 };
	char *found = { 0 };

	//DWORD written;

	cout << "WriteFile ... " << endl;
	if (WriteFile(handle, pCmd, strlen(pCmd), &written, NULL))
		wcout << L"Written " << written << endl << pCmd << endl;
	else
	{
		wcout << L"Error WriteFile: " << GetLastError() << endl;
		CloseHandle(handle);
		system("PAUSE");
		return -1;
	}

	Sleep(2000);	// ѕауза чтобы принтер успел проснутьс€

	cout << "ReadFile ... " << endl;
	while (!gFlagExit)
	{
		ReadFile(handle, readBuf, BUFSIZE, &bytesRead, NULL);
		/*for (i = 0; i < bytesRead; i++)
			cout << hex << (unsigned short)readBuf[i] << endl;*/
			//cout << readBuf[i];
		//cout << endl;
		//cout << "read " << bytesRead << " : ";
		if (strlen(readBuf) > 0)
		{
			cout << "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv" << endl;
			cout << readBuf << endl;
			cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
		}
		//wcout << szReadBuf;
		found = strstr(readBuf, szSubStrCODE);
		if (found)
		{
			ZeroMemory(CODE, 5);
			for (int j = 0; j < 5; j++)
				CODE[j] = found[j + 5];
			cout << " === CODE === " << CODE << endl;
		}

		found = strstr(readBuf, szSubStrONLINE);
		if (found)
		{
			ZeroMemory(&ONLINE, 6);
			if (found[7] == 'T')
				strcpy_s(ONLINE, 6, "TRUE");
			else
				strcpy_s(ONLINE, 6, "FALSE");
			cout << " === ONLINE === " << ONLINE << endl;
		}

		found = strstr(readBuf, szSubStrDISPLAY);
		if (found)
		{
			ZeroMemory(&DISPLAY, 256);
			int j(0);
			while (found[j + 9] != '"')
			{
				DISPLAY[j++] = found[j + 9];
				//cout << j << " " << found[j + 8] << " ";
			}
			//cout << " === DISPLAY === " << DISPLAY << endl;
			int output_size = MultiByteToWideChar(CP_UTF8, 0, DISPLAY, -1, NULL, 0);
			wchar_t *converted_buf = new wchar_t[output_size];
			int size = MultiByteToWideChar(CP_UTF8, 0, DISPLAY, -1, converted_buf, output_size);
			wcout << " === DISPLAY === " << converted_buf << endl;
		}
		found = strstr(readBuf, szSubStrPAGECOUNT);
		if (found)
		{
			ZeroMemory(PAGECOUNT, 16);
			int j(0);
			while (found[j + 11] != '\r')
			{
				PAGECOUNT[j++] = found[j + 11];
			}
			cout << " === PAGECOUNT === " << PAGECOUNT << endl;
		}
		if (0x0a == (unsigned short)readBuf[bytesRead - 2] && 0x0c == (unsigned short)readBuf[bytesRead - 1]) break;

		ZeroMemory(&readBuf, BUFSIZE);
	}

//---------------------------------------------------------------------------------------------------------------------v
	const char *pCmd2 = "\x1B%-12345X@PJL\r\n@PJL INFO STATUS\r\n\x1B%-12345X\r\n";
	WriteFile(handle, pCmd2, strlen(pCmd2), &written, NULL);
	wcout << L"Written " << written << endl << pCmd2 << endl;

	while (!gFlagExit)
	{
		ReadFile(handle, readBuf, BUFSIZE, &bytesRead, NULL);
		/*for (i = 0; i < bytesRead; i++)
		cout << hex << (unsigned short)readBuf[i] << endl;*/
		//cout << readBuf[i];
		//cout << endl;
		//cout << "read " << bytesRead << " : ";
		if (strlen(readBuf) > 0)
		{
			cout << "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv" << endl;
			cout << readBuf << endl;
			cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
		}
		//wcout << szReadBuf;
		found = strstr(readBuf, szSubStrCODE);
		if (found)
		{
			ZeroMemory(CODE, 5);
			for (int j = 0; j < 5; j++)
				CODE[j] = found[j + 5];
			cout << " === CODE === " << CODE << endl;
		}
			found = strstr(readBuf, szSubStrONLINE);
		if (found)
		{
			ZeroMemory(&ONLINE, 6);
			if (found[7] == 'T')
				strcpy_s(ONLINE, 6, "TRUE");
			else
				strcpy_s(ONLINE, 6, "FALSE");
			cout << " === ONLINE === " << ONLINE << endl;
		}
		found = strstr(readBuf, szSubStrDISPLAY);
		if (found)
		{
			ZeroMemory(&DISPLAY, 256);
			int j(0);
			while (found[j + 9] != '"')
			{
				DISPLAY[j++] = found[j + 9];
				//cout << j << " " << found[j + 8] << " ";
			}
			//cout << " === DISPLAY === " << DISPLAY << endl;
			int output_size = MultiByteToWideChar(CP_UTF8, 0, DISPLAY, -1, NULL, 0);
			wchar_t *converted_buf = new wchar_t[output_size];
			int size = MultiByteToWideChar(CP_UTF8, 0, DISPLAY, -1, converted_buf, output_size);
			wcout << " === DISPLAY === " << converted_buf << endl;
		}
		found = strstr(readBuf, szSubStrPAGECOUNT);
		if (found)
		{
			ZeroMemory(PAGECOUNT, 16);
			int j(0);
			while (found[j + 11] != '\r')
			{
				PAGECOUNT[j++] = found[j + 11];
			}
			cout << " === PAGECOUNT === " << PAGECOUNT << endl;
		}


		//if (readBuf[bytesRead-1] == 0xff) break;
		//cout << "oxoc" << hex << (unsigned short)readBuf[bytesRead - 2] << " " << (unsigned short)readBuf[bytesRead - 1] << endl;

		if (0x0a == (unsigned short)readBuf[bytesRead - 2] && 0x0c == (unsigned short)readBuf[bytesRead - 1]) break;
		
		ZeroMemory(&readBuf, BUFSIZE);
//		ZeroMemory(&szReadBuf, BUFSIZE);

		//if (strstr(readBuf, "USTATUS") ) break;
		//if ( !strlen(CODE) 
		
		//if (!strcmp(CODE, "10001")) break;

		// —ообщение должно закончитьс€ на 0x0a 0x0c
		//if (0x0c == (unsigned short)readBuf[bytesRead-1]) break;
	}
//-----------------------------------------------------------------------------------------------------------------------------------^

	CloseHandle(handle);



	system("PAUSE");
	return 0;
}

/*#define BUFSIZE 256
BOOL TestReadPrinterWithJob(LPSTR szPrinterName)
{
	HANDLE hPrinter = NULL;
	HANDLE hPrinterJob = NULL;
	DWORD dwBytesRead;
	LPVOID lpBytes = NULL;
	DOC_INFO_1 dc;
	DWORD jobid;
	TCHAR jobStr[100];

	// Open a handle to the printer.
	OpenPrinter(szPrinterName, &hPrinter, NULL);
	// We can't read from a printer handle, but we can read from
	// a printer job handle, So the trick is to create a Job using 
	// StartDocPrinter, then open a handle to the printer job...
	ZeroMemory(&dc, sizeof(DOC_INFO_1));
	dc.pDocName = "Dummy job";
	jobid = StartDocPrinter(hPrinter, 1, (LPSTR)&dc); // start a Doc 
	// Open handle to the printer job...
	wsprintf(jobStr, "%s,Job %i", szPrinterName, jobid);
	OpenPrinter(jobStr, &hPrinterJob, NULL)
	// Allocate a buffer to read printer data into...
	lpBytes = (LPVOID)malloc(BUFSIZE);
	// Try ReadPrinter...
	ReadPrinter(hPrinterJob, lpBytes, BUFSIZE, &dwBytesRead);
	// Clean up...
	ClosePrinter(hPrinterJob);

	EndDocPrinter(hPrinter); // end the doc 
	ClosePrinter(hPrinter);
}*/

/*{

//byte bCmd[] = {'\033', '-', '1', '2', '3', '4', '5', 'X', '\r', '\n', '@', 'P', 'J', 'L', ' ', 'E', 'C', 'H', 'O', ' ', 'T', 'e', 's', 't', '\r', '\n', '\033', '-', '1', '2', '3', '4', '5', 'X', '\0' };
//byte bCmd[] = { '\x1b','\x25','\x2d','\x31','\x32','\x33','\x34','\x35','\x58','\x40','\x50','\x4a','\x4c','\x0d','\x0a','\x40','\x50','\x4a','\x4c','\x20','\x49','\x4e','\x46','\x4f','\x20','\x53','\x54','\x41','\x54','\x55','\x53','\x0d','\x0a','\x1b','\x25','\x2d','\x31','\x32','\x33','\x34','\x35','\x58','\x0d','\x0a' };
cout << pCmd << endl;

DWORD cbBuf = strlen(pCmd) + 1; // strlen(pCmd);
//DWORD cbBuf = sizeof pCmd/sizeof byte;

cout << "cbBuf = " << cbBuf << endl;

//char *pBuf = new char[cbBuf];

//pReadBuf = { 0 };
//strcpy_s(pBuf, cbBuf, pCmd);
//cout << pBuf << endl;

DOC_INFO_1A docInfo;
ZeroMemory(&docInfo, sizeof DOC_INFO_1A);
docInfo.pDocName = (LPSTR)"Test";
docInfo.pOutputFile = NULL;
docInfo.pDatatype = (LPSTR)"RAW";

char *jobStr = new char[256];
DWORD pcchBuffer;
GetDefaultPrinterA(NULL, &pcchBuffer);
char *szDefaultPrinter = new char[pcchBuffer];
GetDefaultPrinterA(szDefaultPrinter, &pcchBuffer);
cout << "Default printer: " << szDefaultPrinter << endl;
HANDLE hPrinter;
OpenPrinterA(szDefaultPrinter, &hPrinter, NULL);


//SendRecvBidiData();


StartPagePrinter(hPrinter);
DWORD jobid = StartDocPrinterA(hPrinter, 1, (LPBYTE)&docInfo);
sprintf_s(jobStr, 256, "%s,Job %i", szDefaultPrinter, jobid);
cout << jobStr << endl;

BOOL ret;
DWORD lastErrorCode;
ret = WritePrinter(hPrinter, (LPVOID)pCmd, cbBuf, &pcWritten);
lastErrorCode = GetLastError();
cout << "Error code: " << lastErrorCode << endl;
if(ret)
cout << "WritePrinter: " << ret << " " << pcWritten << endl;





HANDLE hPrinterJob;
OpenPrinterA(jobStr, &hPrinterJob, NULL);

//	WritePrinter(hPrinterJob, (LPVOID)pCmd, cbBuf, &pcWritten);

ZeroMemory(&docInfo, sizeof DOC_INFO_1A);
docInfo.pDocName = "Test read";
docInfo.pDatatype = "RAW";
docInfo.pOutputFile = NULL;


//	StartDocPrinterA(hPrinterJob, 1, (LPBYTE)&docInfo);
//	StartPagePrinter(hPrinterJob);

//	for (int i = 0; i < 5; i++)
//	{
EndPagePrinter(hPrinter);
EndDocPrinter(hPrinter);

//Sleep(20000);

char pReadBuf[BUFSIZE];
ret = ReadPrinter(hPrinterJob, (LPVOID)pReadBuf, BUFSIZE, &pcRead);
lastErrorCode = GetLastError();
cout << "Error code: " << lastErrorCode << endl;

if (ret)
cout << "ReadPrinter: " << ret << " " << pcRead << endl;

cout << pReadBuf << endl;



ret = ReadPrinter(hPrinterJob, (LPVOID)pReadBuf, BUFSIZE, &pcRead);
lastErrorCode = GetLastError();
cout << "Error code: " << lastErrorCode << endl;

if (ret)
cout << "ReadPrinter: " << ret << " " << pcRead << endl;

cout << pReadBuf << endl;



//	}


//	EndPagePrinter(hPrinterJob);
//	EndDocPrinter(hPrinterJob);

ClosePrinter(hPrinterJob);

ClosePrinter(hPrinter);


delete jobStr;
delete szDefaultPrinter;
//delete pReadBuf;
//delete pBuf;
}*/
