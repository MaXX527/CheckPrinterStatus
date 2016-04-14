// FuckYouLexmark.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"

using namespace std;

HANDLE hPort;

void LogFile(const char *msg)
{
	time_t rawtime;
	struct tm timeinfo;
	char buffer[80];

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);

	strftime(buffer, 80, "%Y-%m-%d %H:%M:%S: ", &timeinfo);

	ofstream logfile("C:\\Zabbix\\log\\fyl.log", ios::out | ios::app);
	logfile << buffer << msg << endl;
	logfile.close();
}

void LogFile(const wchar_t *msg)
{
	time_t rawtime;
	struct tm timeinfo;
	wchar_t buffer[80];

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);

	wcsftime(buffer, 80, _T("%Y-%m-%d %H:%M:%S: "), &timeinfo);

	wofstream logfile(_T("C:\\Zabbix\\log\\fyl.log"), ios::out | ios::app);
	//locale rus("rus_rus.866");
	logfile.imbue(locale("rus_rus.1251"));
	logfile << buffer << msg << endl;
	logfile.close();
}

/*void CharToAnsi(wchar_t *pSrc, char *pDst)
{
	char pBuf[BUFSIZ];
	CharToOem(pSrc, pBuf);
	OemToAnsi(pBuf, pDst);
}*/

// Узнать путь принтера по умолчанию с помощью всяких модных SetupDi* и CM_*
bool GetDefaultPrinterPath(wchar_t *path /* Out путь принтера */, size_t *l /* Out длина пути принтера */)
{
	bool ret = false;
	*l = 0;

	HANDLE     hPrinter;
//	DOC_INFO_1 DocInfo;
	DWORD      dwJob(0);
	DWORD      dwBytesWritten(0);
	DWORD      dwBytesRead(0);

	// Принтер по умолчанию
	DWORD pcchBuffer;
	GetDefaultPrinterA(NULL, &pcchBuffer);
	TCHAR *szDefaultPrinter = new TCHAR[pcchBuffer];
	GetDefaultPrinter(szDefaultPrinter, &pcchBuffer);
	if(path != nullptr)
		LogFile(szDefaultPrinter);
	//wcout << _T("szDefaultPrinter = ") << szDefaultPrinter << endl;

	// Параметры принтера
	PRINTER_OPTIONS po;
	ZeroMemory(&po, sizeof PRINTER_OPTIONS);
	po.cbSize = sizeof PRINTER_OPTIONS;
	po.dwFlags = PRINTER_OPTION_NO_CACHE;
	//PRINTER_DEFAULTS PrinterDefaults = { NULL, NULL, PRINTER_ALL_ACCESS };

	DWORD cbNeeded(0);
	//OpenPrinter2(szDefaultPrinter, &hPrinter, &PrinterDefaults, &po);
	OpenPrinter2(szDefaultPrinter, &hPrinter, NULL, &po);

	// Узнать экземпляр принтера, что-то типа USBPRINT\LEXMARKLEXMARK_MX310_SERIES\8&23B21019&0&USB003
	// Можно подсмотреть в HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Print\Printers\Lexmark MX310 Series XL\PnPData\DeviceInstanceId
	GetPrinterDataEx(hPrinter, _T("PnPData"), _T("DeviceInstanceId"), NULL, NULL, 0, &cbNeeded);
	DWORD nSize = cbNeeded;
	wchar_t *szDeviceInstanceId = new wchar_t[cbNeeded];
	GetPrinterDataEx(hPrinter, _T("PnPData"), _T("DeviceInstanceId"), NULL, (LPBYTE)szDeviceInstanceId, nSize, &cbNeeded);
	//wcout << _T("PnPData->DeviceInstanceId = ") << szDeviceInstanceId << endl;
	if (path != nullptr)
		LogFile(szDeviceInstanceId);
	ClosePrinter(hPrinter);

	// Перебор всех устройств класса USBPRINT
	static const GUID GUID_DEVINTERFACE_USBPRINT = { 0x28d78fad,0x5a12,0x11D1,0xae,0x5b,0x00,0x00,0xf8,0x03,0xa8,0xc2 };
	HDEVINFO hDevInfoSet = SetupDiGetClassDevs(&GUID_DEVINTERFACE_USBPRINT, NULL, NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT | DIGCF_DEVICEINTERFACE); // DIGCF_PRESENT - только присутствующие устройства

	SP_DEVINFO_DATA DeviceInfoData;
	ZeroMemory(&DeviceInfoData, sizeof SP_DEVINFO_DATA);
	DeviceInfoData.cbSize = sizeof SP_DEVINFO_DATA;

	SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
	ZeroMemory(&DeviceInterfaceData, sizeof SP_DEVICE_INTERFACE_DATA);
	DeviceInterfaceData.cbSize = sizeof SP_DEVICE_INTERFACE_DATA;

	PSP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData;

	DWORD MemberIndex(0);
	DWORD RequiredSize(0);
	DWORD PropertyRegDataType(0);
//	wchar_t *DeviceInstanceId;

	wchar_t TempBuffer[512];
	ULONG BufferLen = 512;

	SP_DEVINSTALL_PARAMS DeviceInstallParams;
	ZeroMemory(&DeviceInstallParams, sizeof SP_DEVINSTALL_PARAMS);
	DeviceInstallParams.cbSize = sizeof SP_DEVINSTALL_PARAMS;

	DEVINST devInst;

	// Цикл по интерфейсам USBPRINT
	while (SetupDiEnumDeviceInterfaces(hDevInfoSet, NULL, &GUID_DEVINTERFACE_USBPRINT, MemberIndex++, &DeviceInterfaceData))
	{
		SetupDiGetDeviceInterfaceDetail(hDevInfoSet, &DeviceInterfaceData, NULL, 0, &RequiredSize, NULL);
		DeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(RequiredSize);
		DeviceInterfaceDetailData->cbSize = sizeof SP_DEVICE_INTERFACE_DETAIL_DATA;
		SetupDiGetDeviceInterfaceDetail(hDevInfoSet, &DeviceInterfaceData, DeviceInterfaceDetailData, RequiredSize, &RequiredSize, &DeviceInfoData);

		// Родитель USB-принтеров будет "Поддержка USB принтера"
		// Получаем первый дочерний узел
		CM_Get_Child(&devInst, DeviceInfoData.DevInst, 0);
		ZeroMemory(TempBuffer, BufferLen);
		CM_Get_Device_ID(devInst, TempBuffer, BufferLen, 0);
		// Попался нужный экземпляр, получить путь принтера
		// Смотреть где-то тут: HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\DeviceClasses\{28d78fad-5a12-11d1-ae5b-0000f803a8c2}\##?#USB#VID_043D&PID_0227&MI_01#7&1f0b0e8&1&0001#{28d78fad-5a12-11d1-ae5b-0000f803a8c2}\#\SymbolicLink
		// Например SymbolicLink = \\?\USB#VID_043D&PID_0227&MI_01#7&1f0b0e8&1&0001#{28d78fad-5a12-11d1-ae5b-0000f803a8c2}
		if (0 == _wcsicmp(szDeviceInstanceId, TempBuffer))
		{
			*l = wcslen(DeviceInterfaceDetailData->DevicePath) + 1;
			// Первый раз вызов идет с nullptr, чтобы узнать необходимый размер строки
			if(path != nullptr)
				wcscpy_s(path, *l, DeviceInterfaceDetailData->DevicePath);
			ret = true;
		}
		// Здесь нужно получить остальные дочерние узлы, на тот случай, если есть несколько принтеров
		// Хз насколько этот код правильный
		// Но поскольку такой случай маловероятен и все равно DeviceInterfaceDetailData один для всех детей, то забиваем на это
		while (CR_SUCCESS == CM_Get_Sibling(&devInst, devInst, 0))
		{
			ZeroMemory(TempBuffer, BufferLen);
			CM_Get_Device_ID(devInst, TempBuffer, BufferLen, 0);
			if (0 == _wcsicmp(szDeviceInstanceId, TempBuffer))
			{
				*l = wcslen(DeviceInterfaceDetailData->DevicePath) + 1;
				// Первый раз вызов идет с nullptr, чтобы узнать необходимый размер строки
				if (path != nullptr)
					wcscpy_s(path, *l, DeviceInterfaceDetailData->DevicePath);
				ret = true;
			}
		}

		free(DeviceInterfaceDetailData);
	}

	delete szDeviceInstanceId;
	delete szDefaultPrinter;

	return ret;
}

// Вариант опеределения пути принтера прямо из реестра
bool GetDefaultPrinterPathFromRegistry(wchar_t *path /* Out путь принтера */, size_t *l /* Out длина пути принтера */)
{
	// Принтер по умолчанию
	DWORD pcchBuffer;
	GetDefaultPrinter(NULL, &pcchBuffer);
	wchar_t *szDefaultPrinter = new wchar_t[pcchBuffer];
	GetDefaultPrinter(szDefaultPrinter, &pcchBuffer);

	HANDLE hPrinter;
	OpenPrinter(szDefaultPrinter, &hPrinter, NULL);

	PRINTER_INFO_2 *pPrinter;
	DWORD cbBuf(0), cbNeeded(0);
	GetPrinter(hPrinter, 2, NULL, cbBuf, &cbNeeded);
	cbBuf = cbNeeded;
	pPrinter = (PRINTER_INFO_2*)malloc(cbBuf);
	GetPrinter(hPrinter, 2, (LPBYTE)pPrinter, cbBuf, &cbNeeded);

	// Если статус принтера не 0, лучше его не трогать. 0 по идее означает готов
	//if (pPrinter->Status != 0) ErrorHandler();
	char szBuf[BUFSIZ];
	ZeroMemory(szBuf, BUFSIZ);
	_itoa_s(pPrinter->Status, szBuf, BUFSIZ, 10);
	if (path != nullptr)
	{
		LogFile(szDefaultPrinter);
		LogFile(szBuf);
	}

	// Адрес? порта принтера
	//HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\DeviceClasses\{28d78fad-5a12-11d1-ae5b-0000f803a8c2}
	HKEY	hkResult;
	HKEY	hkDeviceParameters;
	//HKEY	hkControl;
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
	//char szPortA[REG_MAX_VALUE_NAME];
	DWORD cbData(0);
	DWORD Type(0);

	bool flagExit = false;

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

		//if(PortNumber > 0 	// Нашли просто какой-то порт, т.к. агент заббикса запускается не от локального пользователя и у него другой принтер по умолчанию (((
		//	&& ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegControl, 0, KEY_READ, &hkControl)) // Проверяем, что есть раздел DeviceClasses\\{28d78fad-5a12-11d1-ae5b-0000f803a8c2}\\$achKey$\\#\\Control

		// Поиск порта подходит, если запускаемся от локального пользователя. Если от админа или system - надо проверять значение \\#\Control\Linked == 1
		if (0 == wcscmp(pPrinter->pPortName, szPort)) // Найденный порт совпал с портом принтера по умолчанию, значит нашли то что надо
		{
			RegGetValue(HKEY_LOCAL_MACHINE, szRegSymbolicLink, _T("SymbolicLink"), RRF_RT_ANY, NULL, NULL, &cbData); // DeviceClasses\\{28d78fad-5a12-11d1-ae5b-0000f803a8c2}\\$achKey$\\#
			vData = (VOID*)malloc(cbData);
			RegGetValue(HKEY_LOCAL_MACHINE, szRegSymbolicLink, _T("SymbolicLink"), RRF_RT_ANY, NULL, vData, &cbData); // DeviceClasses\\{28d78fad-5a12-11d1-ae5b-0000f803a8c2}\\$achKey$\\#
			memcpy(szInterface, vData, cbData);
			*l = wcslen(szInterface) + 1;
			free(vData);
			flagExit = true;
			//CharToOem(szPort, szPortA);
			if (path != nullptr)
			{
				wcscpy_s(path, *l, szInterface);
				LogFile(szPort);
			}
		}
		RegCloseKey(hkDeviceParameters);
	}
	RegCloseKey(hkResult);
	//free(pPrinter);
	return flagExit;
}

// Если за время TIMEOUT-1 программа не завершилась, значит произошла какая-то ошибка. Пытаемся закрыть порт и выйти
void ErrorHandler()
{
	clock_t end_time = clock() + (TIMEOUT - 1) * CLOCKS_PER_SEC;
	while (clock() < end_time) { Sleep(100); }
	LogFile("Аварийное завершение: ErrorHandler()");
	CancelIoEx(hPort, NULL);
	CloseHandle(hPort);
	exit(EXIT_SUCCESS);
}

// После каждого обмена передается ReadBreak, принтер ответчает то же самое
void ReadBreak()
{
	DWORD BytesWritten;
	const byte b1[] = { 0xa5, 0x00, 0x0c, 0x50, 0x03, 0x04, 0x52, 0x65, 0x61, 0x64, 0x42, 0x72, 0x65, 0x61, 0x6b };
	
	byte answer[BUFSIZ];

	if (!WriteFile(hPort, (LPCVOID)b1, sizeof b1, &BytesWritten, NULL)) ErrorHandler();

	ReadFile(hPort, answer, BUFSIZ, &BytesWritten, NULL);

	LogFile(_T("End ReadBreak()"));
}

// С таких байт начинается обмен сообщениями, на всякий случай тоже их отправим
void Init()
{
	DWORD BytesWritten;
	const byte b1[] = { 0xA5, 0x00, 0x10, 0x80, 0xA4, 0x5B, 0xA4, 0x5B, 0x10, 0xEF, 0xA4, 0x5B, 0x11, 0xEE, 0x00, 0xFF, 0x13, 0xEC, 0x2E };
	const byte b2[] = { 0xA5, 0x00, 0x07, 0x50, 0xE0, 0xD9, 0x00, 0x0E, 0x0E, 0x04 };
	byte answer[BUFSIZ];

	if (!WriteFile(hPort, (LPCVOID)b1, sizeof b1, &BytesWritten, NULL)) ErrorHandler();

	if (!WriteFile(hPort, (LPCVOID)b2, sizeof b2, &BytesWritten, NULL)) ErrorHandler();

	ReadFile(hPort, answer, BUFSIZ, &BytesWritten, NULL);

	LogFile(_T("End Init()"));
}

// Если не удалось получить ответ за TIMEOUT секунд, убиться нафиг
void Timeout()
{
	clock_t end_time = clock() + TIMEOUT * CLOCKS_PER_SEC;
	while (clock() < end_time) { Sleep(100); }
	CancelIoEx(hPort, NULL);
	LogFile("Аварийное завершение: Timeout()");
	exit(EXIT_SUCCESS);
}

// Отправка в принтер команд PJL
bool GetData(const char* pCmd)
{
	const char *SubStrCODE = "CODE=";
	const char *SubStrONLINE = "ONLINE=";
	const char *SubStrDISPLAY = "DISPLAY=";
	const char *SubStrPAGECOUNT = "PAGECOUNT";
	char CODE[6] = { 0 };
	char ONLINE[6] = { 0 };
	char DISPLAY[256] = { 0 };
	wchar_t wDISPLAY[256] = { 0 };
	char PAGECOUNT[16] = { 0 };
	DWORD Printer_Status_Code = 0;

	DWORD bytesWritten(0), bytesRead(0);
	char readBuf[BUFSIZ];
	ZeroMemory(&readBuf, BUFSIZ);
	int i(0);
	char *found = { 0 };

	if (!WriteFile(hPort, pCmd, (DWORD)strlen(pCmd), &bytesWritten, NULL)) return false;

//	while (ReadFile(hPort, readBuf, BUFSIZ, &bytesRead, NULL))
	ReadFile(hPort, readBuf, BUFSIZ, &bytesRead, NULL);
	{
		found = strstr(readBuf, SubStrCODE);
		if (found)
		{
			ZeroMemory(CODE, 5);
			for (int j = 0; j < 5; j++)
				CODE[j] = found[j + 5];
		}

		found = strstr(readBuf, SubStrONLINE);
		if (found)
		{
			ZeroMemory(&ONLINE, 6);
			if (found[7] == 'T')
				strcpy_s(ONLINE, 6, "TRUE");
			else
				strcpy_s(ONLINE, 6, "FALSE");
		}
		found = strstr(readBuf, SubStrDISPLAY);
		if (found)
		{
			ZeroMemory(&DISPLAY, 256);
			int j(0);
			while (found[j + 9] != '"')
				DISPLAY[j++] = found[j + 9];
			int output_size = MultiByteToWideChar(CP_UTF8, 0, DISPLAY, -1, NULL, 0);
			int size = MultiByteToWideChar(CP_UTF8, 0, DISPLAY, -1, wDISPLAY, output_size);
		}
		found = strstr(readBuf, SubStrPAGECOUNT);
		if (found)
		{
			ZeroMemory(PAGECOUNT, 16);
			int j(0);
			while (found[j + 11] != '\r')
				PAGECOUNT[j++] = found[j + 11];
		}

		//if (0x0a == (unsigned short)readBuf[bytesRead - 2] && 0x0c == (unsigned short)readBuf[bytesRead - 1]) break; // Конец сообщения
		ZeroMemory(&readBuf, BUFSIZ);
	}
	LogFile("GetData()");

	if (strlen(PAGECOUNT) > 0)
	{
		ofstream pc("C:\\Zabbix\\log\\pc.txt", ios::out | ios::trunc);
		pc << PAGECOUNT;
		pc.close();
		LogFile("Файл pc.txt записан");
	}
	return true;
}

// Получает целое число из четырех байт в массиве AllAnswer
int GetIntFromAllAnswer(const byte b[], const u_int s)
{
	return ((u_int)b[s] << 8 * 3) + ((u_int)b[s + 1] << 8 * 2) + ((u_int)b[s + 2] << 8) + (u_int)b[s + 3];
}

// Самое интересное - пытаемся узнать, на сколько страниц осталось тонера в картридже
void GetTonerLeft()
{
	wchar_t *szInterfaceNew = nullptr;
	size_t requiredLength(0);
	if (GetDefaultPrinterPath(nullptr, &requiredLength))
	{
		szInterfaceNew = new wchar_t[requiredLength];
		GetDefaultPrinterPath(szInterfaceNew, &requiredLength);
	}
	else
	{
		LogFile("Не удалось узнать путь принтера, пробуем GetDefaultPrinterPathFromRegistry()");
		if (GetDefaultPrinterPathFromRegistry(nullptr, &requiredLength))
		{
			szInterfaceNew = new wchar_t[requiredLength];
			GetDefaultPrinterPathFromRegistry(szInterfaceNew, &requiredLength);
		}
	}
	// Путь не удалось узнать никакими средствами, выходим
	if (szInterfaceNew == nullptr)
	{
		LogFile("Не удалось узнать путь принтера никакими средствами");
		ErrorHandler();
	}

	//char *pDst = new char[requiredLength];
	//CharToOem(szInterfaceNew, pDst);
	LogFile(szInterfaceNew);
	//delete pDst;

	// Вот такие байты отсылает утилита Lexmark, чтобы получить данные от МФУ. Что они означают - полное ХЗ
	//byte b0[] = { 0xA5, 0x00, 0x07, 0x50, 0xE0, 0xD9, 0x00, 0x0E, 0x0E, 0x04 };

	const byte b1[] = { 0xA5, 0x00, 0x10, 0x80, 0xA4, 0x5B, 0xA4, 0x5B, 0x10, 0xEF, 0xA4, 0x5B, 0x11, 0xEE, 0x00, 0xFF, 0x13, 0xEC, 0x2E };
	const byte b2[] = { 0xA5, 0x00, 0x07, 0x50, 0xE0, 0x21, 0x01, 0x00, 0x00, 0x00 };
	//const byte b2[] = { 0xA5, 0x00, 0x04, 0x50, 0xE0, 0xE7, 0x02 };

	//wcout << szInterface << endl;

	hPort = CreateFile(szInterfaceNew, FILE_READ_DATA | FILE_WRITE_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hPort) ErrorHandler();

	// Вроде бы это первое сообщение, которое посылается принтеру
	Init();

	// Получение остатка тонера
	DWORD BytesWritten(0);
	DWORD BytesRead(0);

	if (!WriteFile(hPort, (LPCVOID)b1, sizeof b1, &BytesWritten, NULL)) ErrorHandler();
	//wcout << "Send buffer 1" << endl;
	if (!WriteFile(hPort, (LPCVOID)b2, sizeof b2, &BytesWritten, NULL)) ErrorHandler();
	//wcout << "Send buffer 2" << endl;

	const size_t BUFSIZE(0x200);
	byte answer[BUFSIZE];
	//
	int countToner(0);

	u_int PacketNumber(0);

	byte AllAnswer[65536];
	ZeroMemory(AllAnswer, 65536);
	u_int AllCount(0);
	// Ответы начинаются с байта 0xa5, на наше сообщение их придет 47 штук
	u_int CountA5(0);

	u_int TonerPercent(0);

	// В каждом пакете байт 3 - это флаг. Если бит 5 установлен в 1, то будет еще один пакет.
	// Т.о. если пятый бит во флаге сброшен, это конец передачи
	const u_int flagContinue = 1 << 5;
	byte Flag(flagContinue);

	while ((Flag & flagContinue))
	{
		if ((BytesRead > 0) && (0x01 == answer[BytesRead - 2]) && (0x2c == answer[BytesRead - 1]))	// Последние байты в ответе обычно 0x01 0x2c
		{
			LogFile("Flag=1, но пришли байты 0x01 0x2c");
			break;
		}

		ReadFile(hPort, answer, BUFSIZE, &BytesRead, NULL);
		for (size_t i = 0; i < BytesRead; i++)
		{
			if (0xa5 == answer[i]) // Пропускаем заголовок
			{
				CountA5++;
				i += 4;
			}
			else
				AllAnswer[AllCount++] = answer[i];
		}

		Flag = answer[3];

		//if (!(Flag & flagContinue)) break;

		
		//if (47 == CountA5) break;										// Для страховки проверяем количество пакетов
		//if (0 == BytesRead) break;										// Прочитано 0 байт, наверное хватит
		//wcout << BytesRead << " " << CountA5 << endl;
	}

	// Отправить принтеру "ReadBreak", тоже непонятно зачем
	ReadBreak();

	// Записываем все муть, пришедшую с принтера, в файл log\answer.bin для последующего анализа
	ofstream answerbin("C:\\Zabbix\\log\\answer.bin", ios::out | ios::trunc | ios::binary);
	for (u_int i = 0; i < AllCount; i++)
		answerbin.put(AllAnswer[i]);
	answerbin.close();
	LogFile("Файл answer.bin записан");

	if (AllCount < 0x40)	// Если ответ слишком короткий, выходим. Все равно ничего полезного там нет
	{
		LogFile("Ответ меньше 0x40 байт ((");
		ErrorHandler();
	}

	PrinterInfo Pi;
	ZeroMemory(&Pi, sizeof PrinterInfo);

	// Ищем "Lexmark MX310 Series", рядом серийный номер принтера и версия прошивки
	// На самом деле таких строк м.б. больше одной, поэтому дополнительно проверяем 3-й и 4-й байты перед с/н и прошивкой
	const size_t LexmarkMX310Len = 20;
	const char *szLexmarkMX310 = "Lexmark MX310 Series";
	byte LexmarkMX310[LexmarkMX310Len];
	for (u_int i = 0; i < LexmarkMX310Len; i++)
		LexmarkMX310[i] = (byte)szLexmarkMX310[i];

	// Возможно, в ответе вообще не будет с/н и версии прошивки, на всякий случай присвоим какие-то значения
	strcpy_s(Pi.Sn, 8, "UNKNOWN");
	strcpy_s(Pi.Fw, 8, "UNKNOWN");

	for (u_int i = 0; i < AllCount - LexmarkMX310Len; i++)
	{
		for (u_int j = 0; j < LexmarkMX310Len; j++)
		{
			if (j == LexmarkMX310Len - 1) // Нашли строку
			{
				if((i - 50) > 0 && 0x01 == AllAnswer[i-50] && 0x02 == AllAnswer[i-49])	// Перед с/н идут байты 01 02 (00 0D) - длина
					memcpy(Pi.Sn, AllAnswer + i - 46, 0x0d);
				if (0x0C == AllAnswer[i + 133] && 0x02 == AllAnswer[i + 134])	// Перед прошивкой идут байты 0C 02 (00 0D) - длина
					memcpy(Pi.Fw, AllAnswer + i + 137, 0x0d);
			}
			if (AllAnswer[i + j] != LexmarkMX310[j])
				break;
		}
	}

	// Ищем строку "Black Toner" в ответе, рядом будут серийный номер картриджа, емкость и остаток тонера в %
	const size_t BlackTonerLen = 11;
	byte BlackToner[] = { 0x42, 0x6c, 0x61, 0x63, 0x6b, 0x20, 0x54, 0x6f, 0x6e, 0x65, 0x72 };
	const size_t BlackTonerSNLen = 13;
	char BlackTonerSN[BlackTonerSNLen];

	for (u_int i = 0; i < AllCount - BlackTonerLen; i++)
	{
		for (u_int j = 0; j < BlackTonerLen; j++)
		{
			if (j == BlackTonerLen - 1) // Нашли строку
			{
				memcpy(Pi.cSn, AllAnswer + i + j + 16, BlackTonerSNLen - 1);
				BlackTonerSN[BlackTonerSNLen - 1] = '\0';

				Pi.cPercent = GetIntFromAllAnswer(AllAnswer, i + 83); //((u_int)AllAnswer[i + 83] << 8 * 3) + ((u_int)AllAnswer[i + 84] << 8 * 2) + ((u_int)AllAnswer[i + 85] << 8) + (u_int)AllAnswer[i + 86];

				Pi.cCapacity = GetIntFromAllAnswer(AllAnswer, i + 42); //((u_int)AllAnswer[i + 42] << 8*3) + ((u_int)AllAnswer[i + 43] << 8*2) + ((u_int)AllAnswer[i + 44] << 8) + (u_int)AllAnswer[i + 45];
			}
			if (AllAnswer[i + j] != BlackToner[j])
				break;
		}
	}

	// Ищем строку "Imaging Unit" в ответе, рядом будут серийный номер юнита, емкость и остаток ресурса в %
	const size_t ImagingUnitLen = 12;
	byte ImagingUnit[] = { 0x49, 0x6d, 0x61, 0x67, 0x69, 0x6e, 0x67, 0x20, 0x55, 0x6e, 0x69, 0x74 };
	const size_t ImagingUnitSNLen = 13;
	char ImagingUnitSN[ImagingUnitSNLen];

	for (u_int i = 0; i < AllCount - ImagingUnitLen; i++)
	{
		for (u_int j = 0; j < ImagingUnitLen; j++)
		{
			if (j == ImagingUnitLen - 1) // Нашли строку
			{
				memcpy(Pi.iuSn, AllAnswer + i + j + 5, ImagingUnitSNLen - 1);
				ImagingUnitSN[ImagingUnitSNLen - 1] = '\0';

				Pi.iuPercent = GetIntFromAllAnswer(AllAnswer, i + 73); //((u_int)AllAnswer[i + 73] << 8 * 3) + ((u_int)AllAnswer[i + 74] << 8 * 2) + ((u_int)AllAnswer[i + 75] << 8) + ((u_int)AllAnswer[i + 76]);

				Pi.iuResource = GetIntFromAllAnswer(AllAnswer, i + 32); //((u_int)AllAnswer[i + 32] << 8 * 3) + ((u_int)AllAnswer[i + 33] << 8 * 2) + ((u_int)AllAnswer[i + 34] << 8) + ((u_int)AllAnswer[i + 35]);
			}
			if (AllAnswer[i + j] != ImagingUnit[j])
				break;
		}
	}

	if (Pi.cCapacity > 0) // Что-то узнали
	{
		//cout << Pi.Percent * Pi.Capacity / 100;
		ofstream left("C:\\Zabbix\\log\\left.txt", ios::out | ios::trunc);
		left << Pi.cPercent * Pi.cCapacity / 100;
		left.close();
		LogFile("Файл left.txt записан");
		//system("PAUSE");
		//exit(EXIT_SUCCESS);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Получение данных от принтера. Надеемся, что нужные сведения всегда с одинаковыми адресами ((
	// Надежды не оправдались, адреса всегда меняются
	//memcpy(Pi.Sn,  AllAnswer +  75, 0x0d);
	//memcpy(Pi.Fw,  AllAnswer + 258, 0x0d);
	//memcpy(Pi.cSn, AllAnswer + 911, 0x0c);
	//Pi.cCapacity  = ((u_int)AllAnswer[0x39f] << 8 * 3) + ((u_int)AllAnswer[0x3a0] << 8 * 2) + ((u_int)AllAnswer[0x3a1] << 8 * 1) + ((u_int)AllAnswer[0x3a2]);
	//memcpy(&Pi.cCapacity, AllAnswer + 0x39f, 4);
	//Pi.cPercent   = ((u_int)AllAnswer[0x3c8] << 8 * 3) + ((u_int)AllAnswer[0x3c9] << 8 * 2) + ((u_int)AllAnswer[0x3ca] << 8 * 1) + ((u_int)AllAnswer[0x3cb]);
	//memcpy(Pi.iuSn, AllAnswer + 0x401, 0x0c);
	//Pi.iuResource = ((u_int)AllAnswer[0x411] << 8 * 3) + ((u_int)AllAnswer[0x412] << 8 * 2) + ((u_int)AllAnswer[0x413] << 8 * 1) + ((u_int)AllAnswer[0x414]);
	//Pi.iuPercent  = ((u_int)AllAnswer[0x43a] << 8 * 3) + ((u_int)AllAnswer[0x43b] << 8 * 2) + ((u_int)AllAnswer[0x43c] << 8 * 1) + ((u_int)AllAnswer[0x43d]);

	ofstream answertxt("C:\\Zabbix\\log\\answer.txt", ios::out | ios::trunc);
	answertxt << "mfusn\t" << Pi.Sn   << endl << "mfufw\t"  << Pi.Fw         << endl
		      << "carsn\t" << Pi.cSn  << endl << "carres\t" << Pi.cCapacity  << endl << "carleft\t" << Pi.cPercent << endl
		      << "iusn\t"  << Pi.iuSn << endl << "iures\t"  << Pi.iuResource << endl << "iuleft\t"  << Pi.iuPercent;
	answertxt.close();
	LogFile("Файл answer.txt записан");
	///////////////////////////////////////////////////////////////////////////////////////////////

	// Получение счетчика страниц PAGECOUNT
	const char *CmdID = "\x1B%-12345X@PJL\r\n@PJL INFO ID\r\n\x1B%-12345X\r\n";
	const char *CmdPageCount = "\x1B%-12345X@PJL\r\n@PJL INFO PAGECOUNT\r\n\x1B%-12345X\r\n";
	const char *CmdStatus = "\x1B%-12345X@PJL\r\n@PJL INFO STATUS\r\n\x1B%-12345X\r\n";

	if (!GetData(CmdID)) ErrorHandler();	// Запрос ID чтобы разбудить принтер, если он спит
	Sleep(2000);							// Пауза чтобы принтер успел проснуться
	if (!GetData(CmdPageCount)) ErrorHandler();

	//system("PAUSE");
	delete szInterfaceNew;
	CloseHandle(hPort);

	LogFile("Стоп GetTonerLeft()");
	exit(EXIT_SUCCESS);
}

int main()
{
	locale rus("rus_rus.866");
	cout.imbue(rus);
	wcout.imbue(rus);

	LogFile("================================================================================");
	LogFile("Старт");

	thread timeout(Timeout);
	thread printer(GetTonerLeft);
	thread errorhandler(ErrorHandler);
	
	errorhandler.join();
	timeout.join();
	printer.join();

	LogFile("Стоп main()");

	return 0;
}

