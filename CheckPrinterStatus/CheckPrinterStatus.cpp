// CheckPrinterStatus.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"

using namespace std;

#define BUFSIZE 65536
#define TIMEOUT 10		// Ожидать ответ принтера 10с

BOOL gFlagExit = FALSE;
CDefaultPrinter *Printer;

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

void ResponsePrinter(char *arg)
{
	map<string, int> args;
	args["code"] = 1;
	args["pc"] = 2;
	args["status"] = 3;

	if (args.end() == args.find(arg))
	{
		cout << "Usage: CheckPrinterStatus [code|pc|status]" << endl;
		exit(EXIT_SUCCESS);
	}

	Printer = new CDefaultPrinter();

	switch (args.find(arg)->second)
	{
	case 1: //Status code
		if (Printer->GetStatusCode())
			cout << Printer->GetStatusCode() << endl;
		break;
	case 2: //Page count
		if (Printer->GetPageCount())
		{
			cout << Printer->GetPageCount() << endl;
			//ofstream pc("pc.txt", ios::out | ios::trunc);
			//pc << Printer->GetPageCount();
			//pc.close();
		}
		break;
	case 3: //Printer code
		cout << Printer->GetCode() << endl;
		break;
	default:
		cout << "Usage: CheckPrinterStatus [code|pc|status]" << endl;
		break;
	}

	delete Printer;
	exit(EXIT_SUCCESS);
}

void Timeout()
{
	clock_t end_time = clock() + TIMEOUT * CLOCKS_PER_SEC;
	while (clock() < end_time) {}
	//cout << "1" << endl;
	exit(EXIT_SUCCESS);
}

void ErrorHandler()
{
	clock_t end_time = clock() + (TIMEOUT - 1) * CLOCKS_PER_SEC;
	while (clock() < end_time) {}
	if (nullptr != Printer)
		delete Printer;
	exit(EXIT_SUCCESS);
}

void GetStructure6()
{
	DWORD nSize(64);
	TCHAR *szComputerName = new TCHAR[64];
	GetComputerName(szComputerName, &nSize);
	wcout << "Comp name " << szComputerName << " " << nSize << endl;
	delete szComputerName;

	DWORD pcchBuffer;
	GetDefaultPrinter(NULL, &pcchBuffer);
	wchar_t *szDefaultPrinter = new wchar_t[pcchBuffer];
	GetDefaultPrinter(szDefaultPrinter, &pcchBuffer);

	HANDLE hPrinter;
	OpenPrinter(szDefaultPrinter, &hPrinter, NULL);

	PRINTER_INFO_2 *pPrinter;
	JOB_INFO_2 *pJobInfo;
	DWORD cbBuf(0), cbNeeded(0), nReturned(0);
	DWORD dwChange;

	//HANDLE chObj = FindFirstPrinterChangeNotification(hPrinter, PRINTER_CHANGE_JOB, 0, NULL);

	PORT_INFO_2 *portInfo2;
	EnumPorts(NULL, 2, NULL, 0, &cbNeeded, &nReturned);
	portInfo2 = (PORT_INFO_2*)malloc(cbNeeded);
	ZeroMemory(portInfo2, cbNeeded);
	EnumPorts(NULL, 2, (LPBYTE)portInfo2, cbNeeded, &cbNeeded, &nReturned);

	for (int i = 0; i < nReturned; i++)
		wcout << "portInfo2 " << portInfo2[i].fPortType << ", " << portInfo2[i].pPortName << ", " << portInfo2[i].pMonitorName << endl;

	while (!gFlagExit)
	{
		GetPrinter(hPrinter, 2, NULL, cbBuf, &cbNeeded);

		pPrinter = (PRINTER_INFO_2*)malloc(cbNeeded);
		ZeroMemory(pPrinter, cbNeeded);
		GetPrinter(hPrinter, 2, (LPBYTE)pPrinter, cbNeeded, &cbNeeded);

		wcout << _T("Printer ") << szDefaultPrinter << _T(", Status ") << hex << pPrinter->Status << dec << endl;

		if ((pPrinter->Status & PRINTER_STATUS_PAPER_OUT) == PRINTER_STATUS_PAPER_OUT)
			wcout << _T("No Paper") << endl;
		if ((pPrinter->Status & PRINTER_STATUS_TONER_LOW) == PRINTER_STATUS_TONER_LOW)
			wcout << _T("Near Paper") << endl;
		if ((pPrinter->Status & PRINTER_STATUS_OUTPUT_BIN_FULL) == PRINTER_STATUS_OUTPUT_BIN_FULL)
			wcout << _T("Ticket Out") << endl;
		if ((pPrinter->Status & PRINTER_STATUS_DOOR_OPEN) == PRINTER_STATUS_DOOR_OPEN)
			wcout << _T("No Cover") << endl;
		if ((pPrinter->Status & PRINTER_STATUS_USER_INTERVENTION) == PRINTER_STATUS_USER_INTERVENTION)
			wcout << _T("Cut Error") << endl;
		if ((pPrinter->Status & PRINTER_STATUS_PAPER_JAM) == PRINTER_STATUS_PAPER_JAM)
			wcout << _T("Paper Jam") << endl;
		if ((pPrinter->Status & PRINTER_STATUS_WAITING) == PRINTER_STATUS_WAITING)
			wcout << _T("Overtemperature") << endl;
		if ((pPrinter->Status & PRINTER_STATUS_NO_TONER) == PRINTER_STATUS_NO_TONER)
			wcout << _T("Voltage Error") << endl;
		if ((pPrinter->Status & PRINTER_STATUS_PRINTING) == PRINTER_STATUS_PRINTING)
			wcout << _T("Spooling") << endl;
		if ((pPrinter->Status & PRINTER_STATUS_OFFLINE) == PRINTER_STATUS_OFFLINE)
			wcout << _T("Offline") << endl;
		

		dwChange = WaitForPrinterChange(hPrinter, PRINTER_CHANGE_ALL);
		wcout << "dwChange = " << hex << dwChange << dec << endl;
		//FindNextPrinterChangeNotification(chObj, &dwChange, NULL, NULL);

		EnumJobs(hPrinter, 0, pPrinter->cJobs, 2, NULL, 0, &cbNeeded, &nReturned);
		pJobInfo = (JOB_INFO_2*)malloc(cbNeeded);
		ZeroMemory(pJobInfo, cbNeeded);
		EnumJobs(hPrinter, 0, pPrinter->cJobs, 2, (LPBYTE)pJobInfo, cbNeeded, &cbNeeded, &nReturned);

		for (int i = 0; i < nReturned; i++)
		{
			if(pJobInfo[i].pStatus != NULL)
				wcout << "JobId " << pJobInfo[i].JobId << ", document " << pJobInfo[i].pDocument << ", pStatus " << pJobInfo[i].pStatus << ", status " << hex << pJobInfo[i].Status << dec << endl;
			else
				wcout << "JobId " << pJobInfo[i].JobId << ", document " << pJobInfo[i].pDocument << ", status " << hex << pJobInfo[i].Status << dec << endl;
			
			if ((pJobInfo[i].Status & JOB_STATUS_BLOCKED_DEVQ) == JOB_STATUS_BLOCKED_DEVQ)
				wcout << _T("JOB_STATUS_BLOCKED_DEVQ") << endl;
			if ((pJobInfo[i].Status & JOB_STATUS_COMPLETE) == JOB_STATUS_COMPLETE)
				wcout << _T("JOB_STATUS_COMPLETE") << endl;
			if ((pJobInfo[i].Status & JOB_STATUS_DELETED) == JOB_STATUS_DELETED)
				wcout << _T("JOB_STATUS_COMPLETE") << endl;
			if ((pJobInfo[i].Status & JOB_STATUS_DELETING) == JOB_STATUS_DELETING)
				wcout << _T("JOB_STATUS_DELETING") << endl;
			if ((pJobInfo[i].Status & JOB_STATUS_ERROR) == JOB_STATUS_ERROR)
				wcout << _T("JOB_STATUS_ERROR") << endl;
			if ((pJobInfo[i].Status & JOB_STATUS_OFFLINE) == JOB_STATUS_OFFLINE)
				wcout << _T("JOB_STATUS_OFFLINE") << endl;
			if ((pJobInfo[i].Status & JOB_STATUS_PAPEROUT) == JOB_STATUS_PAPEROUT)
				wcout << _T("JOB_STATUS_PAPEROUT") << endl;
			if ((pJobInfo[i].Status & JOB_STATUS_PAUSED) == JOB_STATUS_PAUSED)
				wcout << _T("JOB_STATUS_PAUSED") << endl;
			if ((pJobInfo[i].Status & JOB_STATUS_PRINTED) == JOB_STATUS_PRINTED)
				wcout << _T("JOB_STATUS_PRINTED") << endl;
			if ((pJobInfo[i].Status & JOB_STATUS_PRINTING) == JOB_STATUS_PRINTING)
				wcout << _T("JOB_STATUS_PRINTING") << endl;
			if ((pJobInfo[i].Status & JOB_STATUS_RENDERING_LOCALLY) == JOB_STATUS_RENDERING_LOCALLY)
				wcout << _T("JOB_STATUS_RENDERING_LOCALLY") << endl;
			if ((pJobInfo[i].Status & JOB_STATUS_RESTART) == JOB_STATUS_RESTART)
				wcout << _T("JOB_STATUS_RESTART") << endl;
			if ((pJobInfo[i].Status & JOB_STATUS_RETAINED) == JOB_STATUS_RETAINED)
				wcout << _T("JOB_STATUS_RETAINED") << endl;
			if ((pJobInfo[i].Status & JOB_STATUS_SPOOLING) == JOB_STATUS_SPOOLING)
				wcout << _T("JOB_STATUS_SPOOLING") << endl;
			if ((pJobInfo[i].Status & JOB_STATUS_USER_INTERVENTION) == JOB_STATUS_USER_INTERVENTION)
				wcout << _T("JOB_STATUS_USER_INTERVENTION") << endl;
		}

		wcout << endl;
		//Sleep(1000);

		free(pPrinter);
		free(pJobInfo);
	}
/*
No Paper PRINTER_STATUS_PAPER_OUT
Near Paper End PRINTER_STATUS_TONER_LOW
Ticket Out PRINTER_STATUS_OUTPUT_BIN_FULL
No Cover PRINTER_STATUS_DOOR_OPEN
Cut Error PRINTER_STATUS_USER_INTERVENTION
Paper Jam PRINTER_STATUS_PAPER_JAM
Overtemperature PRINTER_STATUS_WAITING
Voltage Error PRINTER_STATUS_NO_TONER
Spooling PRINTER_STATUS_PRINTING
*/

/*	if ((pPrinter->dwStatus & PRINTER_STATUS_PAPER_OUT) == PRINTER_STATUS_PAPER_OUT)
		wcout << _T("No Paper") << endl;
	if ((pPrinter->dwStatus & PRINTER_STATUS_TONER_LOW) == PRINTER_STATUS_TONER_LOW)
		wcout << _T("Near Paper") << endl;
	if ((pPrinter->dwStatus & PRINTER_STATUS_OUTPUT_BIN_FULL) == PRINTER_STATUS_OUTPUT_BIN_FULL)
		wcout << _T("Ticket Out") << endl;
	if ((pPrinter->dwStatus & PRINTER_STATUS_DOOR_OPEN) == PRINTER_STATUS_DOOR_OPEN)
		wcout << _T("No Cover") << endl;
	if ((pPrinter->dwStatus & PRINTER_STATUS_USER_INTERVENTION) == PRINTER_STATUS_USER_INTERVENTION)
		wcout << _T("Cut Error") << endl;
	if ((pPrinter->dwStatus & PRINTER_STATUS_PAPER_JAM) == PRINTER_STATUS_PAPER_JAM)
		wcout << _T("Paper Jam") << endl;
	if ((pPrinter->dwStatus & PRINTER_STATUS_WAITING) == PRINTER_STATUS_WAITING)
		wcout << _T("Overtemperature") << endl;
	if ((pPrinter->dwStatus & PRINTER_STATUS_NO_TONER) == PRINTER_STATUS_NO_TONER)
		wcout << _T("Voltage Error") << endl;
	if ((pPrinter->dwStatus & PRINTER_STATUS_PRINTING) == PRINTER_STATUS_PRINTING)
		wcout << _T("Spooling") << endl;*/

//	free(pPrinter);
//	delete szDefaultPrinter;
}

void TestDC()
{
	DWORD pcchBuffer;
	GetDefaultPrinter(nullptr, &pcchBuffer);
	wchar_t *szDefaultPrinter = new wchar_t[pcchBuffer];
	GetDefaultPrinter(szDefaultPrinter, &pcchBuffer);

	HANDLE hPrinter;
	OpenPrinter(szDefaultPrinter, &hPrinter, nullptr);

	PRINTER_INFO_2 *pPrinter;
	DWORD cbNeeded(0);
	GetPrinterW(hPrinter, 2, nullptr, 0, &cbNeeded);
	pPrinter = (PRINTER_INFO_2*)malloc(cbNeeded);
	GetPrinter(hPrinter, 2, (LPBYTE)pPrinter, cbNeeded, &cbNeeded);
	HDC printerDC = CreateDC(_T("WINSPOOL"), pPrinter->pPrinterName, nullptr, nullptr);

	u_long Query = PASSTHROUGH;
	wcout << printerDC << endl;
	int escRet = ExtEscape(printerDC, QUERYESCSUPPORT, sizeof u_long, (LPCSTR)&Query, 0, nullptr);
	wcout << _T("escRet = ") << escRet << endl;

	delete szDefaultPrinter;
	free(pPrinter);

	return;
}

int main(int argc, char *argv[])
{
	std::locale rus("rus_rus.866");
	std::wcout.imbue(rus);

	SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);

	/*map<string, int> args;
	args["code"] = 1;
	args["pc"] = 2;
	args["status"] = 3;*/

	//GetStructure6();

	TestDC();

	system("PAUSE");
	return 0;

	if (argc != 2)
	{
		cout << "Usage: " << argv[0] << " [code|pc|status]" << endl;
		if (DEBUG) system("PAUSE");
		return 0;
	}

/*	CDefaultPrinter *Printer = new CDefaultPrinter();

	switch (args.find(argv[1])->second)
	{
	case 1: //Status code
		if(Printer->GetStatusCode())
			cout << Printer->GetStatusCode() << endl;
		break;
	case 2: //Page count
		if(Printer->GetPageCount())
			cout << Printer->GetPageCount() << endl;
		break;
	case 3: //Printer code
		cout << Printer->GetCode() << endl;
		break;
	default:
		cout << "Usage: " << argv[0] << "[code|pc|status]" << endl;
		break;
	}
		
	delete Printer;*/

	u_int TonerLeft(0);
	ifstream left("C:\\Zabbix\\left.txt", ios::in);
	left >> TonerLeft;
	left.close();
	if(0 == TonerLeft)
		return 0;

	thread printer(ResponsePrinter, argv[1]);
	thread timeout(Timeout);
	thread errorhandler(ErrorHandler);

	printer.join();
	timeout.join();
	errorhandler.join();

	if(DEBUG) system("PAUSE");
	return 0;
}

/*	// Принтер по умолчанию
DWORD pcchBuffer;
GetDefaultPrinter(NULL, &pcchBuffer);
wchar_t *szDefaultPrinter = new wchar_t[pcchBuffer];
GetDefaultPrinter(szDefaultPrinter, &pcchBuffer);
//wcout << "Default printer: " << szDefaultPrinter << endl;

HANDLE hPrinter;
OpenPrinter(szDefaultPrinter, &hPrinter, NULL);

PRINTER_INFO_2 *pPrinter;
DWORD cbBuf(0), cbNeeded(0);
GetPrinter(hPrinter, 2, NULL, cbBuf, &cbNeeded);
cbBuf = cbNeeded;

pPrinter = (PRINTER_INFO_2*)malloc(cbBuf);
GetPrinter(hPrinter, 2, (LPBYTE)pPrinter, cbBuf, &cbNeeded);
//wcout << "Status: " << hex << pPrinter->Status << L" " << pPrinter->pLocation << endl;

// Найти имя интерфейса. Надеемся, что принтер только один (((
static const GUID GUID_DEVINTERFACE_USBPRINT = { 0x28d78fad, 0x5a12, 0x11D1, 0xae, 0x5b, 0x00, 0x00, 0xf8, 0x03, 0xa8, 0xc2 };

HDEVINFO devs;
SP_DEVINFO_DATA devinfo;
SP_DEVICE_INTERFACE_DATA devinterface;
GUID intfce;
PSP_DEVICE_INTERFACE_DETAIL_DATA interface_detail;
ULONG index(0);
ULONG requiredLength;

intfce = GUID_DEVINTERFACE_USBPRINT;

devs = SetupDiGetClassDevs(&intfce, NULL, NULL,	DIGCF_PRESENT | DIGCF_ALLCLASSES | DIGCF_DEVICEINTERFACE);

ZeroMemory(&devinterface, sizeof(SP_DEVICE_INTERFACE_DATA));
devinterface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
devinterface.Flags = 0;

SetupDiEnumDeviceInterfaces(devs, 0, &intfce, index, &devinterface);
//wchar_t* interfacename = new wchar_t[BUFSIZE];

requiredLength = 0;
SetupDiGetDeviceInterfaceDetail(devs, &devinterface, NULL, 0, &requiredLength, NULL);

interface_detail = (SP_DEVICE_INTERFACE_DETAIL_DATA*)malloc(requiredLength);
interface_detail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)calloc(1, requiredLength);

ZeroMemory(interface_detail, sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA));
interface_detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
devinfo.cbSize = sizeof(SP_DEVINFO_DATA);
SetupDiGetDeviceInterfaceDetail(devs, &devinterface, interface_detail, requiredLength, &requiredLength, &devinfo);

//wcscpy_s(interfacename, 0x800, interface_detail->DevicePath);
//cout << "interfacename " << interfacename << endl;*/

//CDefaultPrinter *Printer = new CDefaultPrinter(szDefaultPrinter, interface_detail->DevicePath);


//char *Buf = new char(BUFSIZ);
//strcpy_s(Buf, BUFSIZ, Printer->GetCode());
//cout << "Status = " << Printer->GetPageCount() << endl;

/*	// Собственно получение информации от принтера
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
handle = CreateFile(Printer->m_Interface,
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

Sleep(2000);	// Пауза чтобы принтер успел проснуться

cout << "ReadFile ... " << endl;
while (!gFlagExit)
{
ReadFile(handle, readBuf, BUFSIZE, &bytesRead, NULL);
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

// Сообщение должно закончиться на 0x0a 0x0c
//if (0x0c == (unsigned short)readBuf[bytesRead-1]) break;
}
//-----------------------------------------------------------------------------------------------------------------------------------^

CloseHandle(handle);
*/

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
