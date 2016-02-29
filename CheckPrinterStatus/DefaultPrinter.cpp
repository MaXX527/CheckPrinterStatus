#include "stdafx.h"
#include "DefaultPrinter.h"

using namespace std;

CDefaultPrinter::CDefaultPrinter()
{
	Init2();
}

CDefaultPrinter::CDefaultPrinter(const wchar_t *name, const wchar_t *iface)
{
	unsigned long long len(0);

	len = sizeof(wchar_t)*(wcslen(name) + 1);
	m_Name = new wchar_t[len];
	wcscpy_s(m_Name, len, name);

	len = sizeof(wchar_t)*(wcslen(iface) + 1);
	m_Interface = new wchar_t[len];
	wcscpy_s(m_Interface, len, iface);
}

CDefaultPrinter::~CDefaultPrinter()
{
	//delete m_Name;
	//delete m_Interface;
	if (INVALID_HANDLE_VALUE != m_hPort)
		CloseHandle(m_hPort);
}

const char * CDefaultPrinter::GetStatusCode()
{
	GetData(m_CmdID); // Только чтобы разбудить принтер
	Sleep(2000);
	if (GetData(m_CmdStatus))
		return m_CODE;
	else
		return nullptr;
}

const char * CDefaultPrinter::GetPageCount()
{
	GetData(m_CmdID); // Только чтобы разбудить принтер
	Sleep(2000);
	if (GetData(m_CmdPageCount))
		return m_PAGECOUNT;
	else
		return nullptr;
}

const DWORD CDefaultPrinter::GetCode()
{
	return m_Printer_Status_Code;
}

void CDefaultPrinter::Init()
{
	// Принтер по умолчанию
	DWORD pcchBuffer;
	GetDefaultPrinter(NULL, &pcchBuffer);
	wchar_t *szDefaultPrinter = new wchar_t[pcchBuffer];
	GetDefaultPrinter(szDefaultPrinter, &pcchBuffer);

	if(DEBUG)
		wcout << "Default printer: " << szDefaultPrinter << endl;

	HANDLE hPrinter;
	OpenPrinter(szDefaultPrinter, &hPrinter, NULL);

	PRINTER_INFO_2 *pPrinter;
	DWORD cbBuf(0), cbNeeded(0);
	GetPrinter(hPrinter, 2, NULL, cbBuf, &cbNeeded);
	cbBuf = cbNeeded;

	pPrinter = (PRINTER_INFO_2*)malloc(cbBuf);
	GetPrinter(hPrinter, 2, (LPBYTE)pPrinter, cbBuf, &cbNeeded);
	
	if (DEBUG)
		wcout << "Status: " << hex << pPrinter->Status << L" " << pPrinter->pLocation << endl;

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

	devs = SetupDiGetClassDevs(&intfce, NULL, NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES | DIGCF_DEVICEINTERFACE);

	ZeroMemory(&devinterface, sizeof(SP_DEVICE_INTERFACE_DATA));
	devinterface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	devinterface.Flags = 0;

	//while (SetupDiEnumDeviceInterfaces(devs, 0, &intfce, index, &devinterface))
	//{
		SetupDiEnumDeviceInterfaces(devs, 0, &intfce, index, &devinterface);
		requiredLength = 0;
		SetupDiGetDeviceInterfaceDetail(devs, &devinterface, NULL, 0, &requiredLength, NULL);

		interface_detail = (SP_DEVICE_INTERFACE_DETAIL_DATA*)malloc(requiredLength);
		interface_detail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)calloc(1, requiredLength);

		ZeroMemory(interface_detail, sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA));
		interface_detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		devinfo.cbSize = sizeof(SP_DEVINFO_DATA);
		SetupDiGetDeviceInterfaceDetail(devs, &devinterface, interface_detail, requiredLength, &requiredLength, &devinfo);

		//index++;

		//wcout << devinfo.ClassGuid.Data1 << devinfo.ClassGuid.Data2 << devinfo.ClassGuid.Data3 << devinfo.ClassGuid.Data4 << endl;
	//}

	unsigned long long len(0);

	len = sizeof(wchar_t)*(wcslen(szDefaultPrinter) + 1);
	m_Name = new wchar_t[len];
	wcscpy_s(m_Name, len, szDefaultPrinter);

	len = sizeof(wchar_t)*(wcslen(interface_detail->DevicePath) + 1);
	m_Interface = new wchar_t[len];
	wcscpy_s(m_Interface, len, interface_detail->DevicePath);

	m_Printer_Status_Code = pPrinter->Status;
	delete szDefaultPrinter;
	free(pPrinter);
	
	if (DEBUG)
		wcout << "Interface name: " << m_Interface << endl;
}

void CDefaultPrinter::Init2()
{
	// Принтер по умолчанию
	DWORD pcchBuffer;
	GetDefaultPrinter(NULL, &pcchBuffer);
	wchar_t *szDefaultPrinter = new wchar_t[pcchBuffer];
	GetDefaultPrinter(szDefaultPrinter, &pcchBuffer);

	if (DEBUG)
		wcout << "Default printer: " << szDefaultPrinter << endl;

	HANDLE hPrinter;
	OpenPrinter(szDefaultPrinter, &hPrinter, NULL);

	PRINTER_INFO_2 *pPrinter;
	DWORD cbBuf(0), cbNeeded(0);
	GetPrinter(hPrinter, 2, NULL, cbBuf, &cbNeeded);
	cbBuf = cbNeeded;

	pPrinter = (PRINTER_INFO_2*)malloc(cbBuf);
	GetPrinter(hPrinter, 2, (LPBYTE)pPrinter, cbBuf, &cbNeeded);
	// Адрес? порта принтера
	//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\DeviceClasses\{28d78fad-5a12-11d1-ae5b-0000f803a8c2}
	HKEY	hkResult;
	HKEY	hkDeviceParameters;
	HKEY	hkControl;
	TCHAR	achClass[MAX_PATH] = TEXT("");
	TCHAR	achKey[REG_MAX_KEY_LENGTH];
	DWORD	cbName;
	DWORD	cchClassName = MAX_PATH;
	DWORD	cSubKeys = 0;
	DWORD	cbMaxSubKey;
	DWORD	cchMaxClass;
	DWORD	cValues;
	DWORD	cchMaxValue;
	DWORD	cbMaxValueData;
	DWORD	cbSecurityDescriptor;
	FILETIME ftLastWriteTime;

	const TCHAR *szRegDeviceClass = _T("SYSTEM\\CurrentControlSet\\Control\\DeviceClasses\\{28d78fad-5a12-11d1-ae5b-0000f803a8c2}");
	TCHAR szRegDeviceParameters[REG_MAX_VALUE_NAME];
	TCHAR szRegSymbolicLink[REG_MAX_VALUE_NAME];
	TCHAR szRegControl[REG_MAX_VALUE_NAME];
	TCHAR szInterface[REG_MAX_VALUE_NAME];
	VOID *vData;
	TCHAR szBaseName[REG_MAX_VALUE_NAME];
	DWORD PortNumber;
	TCHAR szPort[REG_MAX_VALUE_NAME];
	DWORD cbData(0);
	DWORD Type(0);

	BOOL flagExit = FALSE;

	ZeroMemory(szRegDeviceParameters, REG_MAX_VALUE_NAME);
	ZeroMemory(szRegSymbolicLink, REG_MAX_VALUE_NAME);
	ZeroMemory(szRegControl, REG_MAX_VALUE_NAME);
	ZeroMemory(szInterface, REG_MAX_VALUE_NAME);
	ZeroMemory(szBaseName, REG_MAX_VALUE_NAME);
	ZeroMemory(szPort, REG_MAX_VALUE_NAME);

	RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegDeviceClass, 0, KEY_READ, &hkResult); // DeviceClasses\\{28d78fad-5a12-11d1-ae5b-0000f803a8c2}
	RegQueryInfoKey(
		hkResult,				// key handle
		achClass,               // buffer for class name 
		&cchClassName,          // size of class string 
		NULL,                   // reserved 
		&cSubKeys,              // number of subkeys 
		&cbMaxSubKey,           // longest subkey size 
		&cchMaxClass,           // longest class string 
		&cValues,               // number of values for this key 
		&cchMaxValue,           // longest value name 
		&cbMaxValueData,        // longest value data 
		&cbSecurityDescriptor,  // security descriptor 
		&ftLastWriteTime		// last write time
		);
	int ret(0);
	for (u_int i = 0; i < cSubKeys, !flagExit; i++)
	{
		cbName = REG_MAX_KEY_LENGTH;
		RegEnumKeyEx(hkResult, i, achKey, &cbName, NULL, NULL, NULL, &ftLastWriteTime);

		wcscpy_s(szRegDeviceParameters, REG_MAX_VALUE_NAME, szRegDeviceClass);
		wcscat_s(szRegDeviceParameters, REG_MAX_VALUE_NAME, _T("\\"));
		wcscat_s(szRegDeviceParameters, REG_MAX_VALUE_NAME, achKey);
		wcscat_s(szRegDeviceParameters, REG_MAX_VALUE_NAME, _T("\\#"));
		wcscpy_s(szRegSymbolicLink, REG_MAX_VALUE_NAME, szRegDeviceParameters);
		wcscpy_s(szRegControl, REG_MAX_VALUE_NAME, szRegDeviceParameters);
		wcscat_s(szRegControl, REG_MAX_VALUE_NAME, _T("\\Control"));
		wcscat_s(szRegDeviceParameters, REG_MAX_VALUE_NAME, _T("\\Device Parameters"));

		RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegDeviceParameters, 0, KEY_READ, &hkDeviceParameters); // DeviceClasses\\{28d78fad-5a12-11d1-ae5b-0000f803a8c2}\\$achKey$\\#\\Device Parameters

		PortNumber = 0;
		RegGetValue(HKEY_LOCAL_MACHINE, szRegDeviceParameters, _T("Base Name"), RRF_RT_ANY, NULL, NULL, &cbData); // DeviceClasses\\{28d78fad-5a12-11d1-ae5b-0000f803a8c2}\\$achKey$\\#\\Device Parameters
		vData = (VOID*)malloc(cbData);
		RegGetValue(HKEY_LOCAL_MACHINE, szRegDeviceParameters, _T("Base Name"), RRF_RT_ANY, NULL, vData, &cbData); // DeviceClasses\\{28d78fad-5a12-11d1-ae5b-0000f803a8c2}\\$achKey$\\#\\Device Parameters
		memcpy(szBaseName, vData, cbData);
		free(vData);
		PortNumber = 0;
		RegGetValue(HKEY_LOCAL_MACHINE, szRegDeviceParameters, _T("Port Number"), RRF_RT_ANY, NULL, &PortNumber, &cbData); // DeviceClasses\\{28d78fad-5a12-11d1-ae5b-0000f803a8c2}\\$achKey$\\#\\Device Parameters

		wsprintf(szPort, _T("%s%03d"), szBaseName, PortNumber);

		//if (0 == wcscmp(pPrinter->pPortName, szPort)) // Найденный порт совпал с портом принтера по умолчанию, значит нашли то что надо
		if (PortNumber > 0 	// Нашли просто какой-то порт, т.к. агент заббикса запускается не от локального пользователя и у него другой принтер по умолчанию (((
			&& ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegControl, 0, KEY_READ, &hkControl)) // Проверяем, что есть раздел DeviceClasses\\{28d78fad-5a12-11d1-ae5b-0000f803a8c2}\\$achKey$\\#\\Control
		{
			RegGetValue(HKEY_LOCAL_MACHINE, szRegSymbolicLink, _T("SymbolicLink"), RRF_RT_ANY, NULL, NULL, &cbData); // DeviceClasses\\{28d78fad-5a12-11d1-ae5b-0000f803a8c2}\\$achKey$\\#
			vData = (VOID*)malloc(cbData);
			RegGetValue(HKEY_LOCAL_MACHINE, szRegSymbolicLink, _T("SymbolicLink"), RRF_RT_ANY, NULL, vData, &cbData); // DeviceClasses\\{28d78fad-5a12-11d1-ae5b-0000f803a8c2}\\$achKey$\\#
			memcpy(szInterface, vData, cbData);
			free(vData);
			flagExit = TRUE;
		}
		RegCloseKey(hkDeviceParameters);
	}
	RegCloseKey(hkResult);

	size_t len(0);
	len = sizeof(TCHAR) * (wcslen(szInterface) + 1);
	m_Interface = new TCHAR[len];
	wcscpy_s(m_Interface, len, szInterface);

	if (DEBUG)
		wcout << "Interface name: " << m_Interface << endl;
}

BOOL CDefaultPrinter::GetData(const char* pCmd)
{
	if(DEBUG)
		cout << pCmd << endl;
	//wcscpy_s(m_Interface, 90, L"\\\\?\\USB#VID_043D&PID_0227&MI_01#7&2b402743&5&0001#{28d78fad-5a12-11d1-ae5b-0000f803a8c2}");
	//\\\\?\\USB#VID_043D&PID_0227&MI_01#7&2b402743&5&0001#{28d78fad-5a12-11d1-ae5b-0000f803a8c2}
	m_hPort = CreateFile(m_Interface, FILE_READ_DATA | FILE_WRITE_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,	NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (INVALID_HANDLE_VALUE == m_hPort)
	{
		if (DEBUG)
			cout << "Error CreateFile" << endl;
		if(DEBUG) system("PAUSE");
		return FALSE;
	}
	
	DWORD bytesWritten(0), bytesRead(0);
	char readBuf[BUFSIZ];
	ZeroMemory(&readBuf, BUFSIZ);
	int i(0);
	char *found = { 0 };

	if(DEBUG)
		cout << "WriteFile ... ";
	if(!WriteFile(m_hPort, pCmd, (DWORD)strlen(pCmd), &bytesWritten, NULL))
	{
		if(DEBUG)
			wcout << L"Error WriteFile: " << GetLastError() << endl;
		CloseHandle(m_hPort);
		if(DEBUG) system("PAUSE");
		return FALSE;
	}
	if (DEBUG)
		cout << "ok" << endl;

	//Sleep(1000);	// Пауза 1с чтобы принтер успел проснуться

	if (DEBUG)
		cout << "ReadFile ... ";
	while (ReadFile(m_hPort, readBuf, BUFSIZ, &bytesRead, NULL))
	{
		/*if (strlen(readBuf) > 0)
		{
			cout << "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv" << endl;
			cout << readBuf << endl;
			cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
		}*/
		found = strstr(readBuf, m_SubStrCODE);
		if (found)
		{
			ZeroMemory(m_CODE, 5);
			for (int j = 0; j < 5; j++)
				m_CODE[j] = found[j + 5];
			if (DEBUG)
				cout << " === CODE === " << m_CODE << endl;
		}

		found = strstr(readBuf, m_SubStrONLINE);
		if (found)
		{
			ZeroMemory(&m_ONLINE, 6);
			if (found[7] == 'T')
				strcpy_s(m_ONLINE, 6, "TRUE");
			else
				strcpy_s(m_ONLINE, 6, "FALSE");
			if (DEBUG)
				cout << " === ONLINE === " << m_ONLINE << endl;
		}
		found = strstr(readBuf, m_SubStrDISPLAY);
		if (found)
		{
			ZeroMemory(&m_DISPLAY, 256);
			int j(0);
			while (found[j + 9] != '"')
				m_DISPLAY[j++] = found[j + 9];
			int output_size = MultiByteToWideChar(CP_UTF8, 0, m_DISPLAY, -1, NULL, 0);
			int size = MultiByteToWideChar(CP_UTF8, 0, m_DISPLAY, -1, m_wDISPLAY, output_size);
			if (DEBUG)
				wcout << " === DISPLAY === " << m_wDISPLAY << endl;
		}
		found = strstr(readBuf, m_SubStrPAGECOUNT);
		if (found)
		{
			ZeroMemory(m_PAGECOUNT, 16);
			int j(0);
			while (found[j + 11] != '\r')
				m_PAGECOUNT[j++] = found[j + 11];
			if (DEBUG)
				cout << " === PAGECOUNT === " << m_PAGECOUNT << endl;
		}

		if (0x0a == (unsigned short)readBuf[bytesRead - 2] && 0x0c == (unsigned short)readBuf[bytesRead - 1]) break; // Конец сообщения
		ZeroMemory(&readBuf, BUFSIZ);
	}
	if (DEBUG)
		cout << "ok" << endl;
	CloseHandle(m_hPort);
	return TRUE;
}
