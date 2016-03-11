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

void CharToAnsi(wchar_t *pSrc, char *pDst)
{
	char pBuf[BUFSIZ];
	CharToOem(pSrc, pBuf);
	OemToAnsi(pBuf, pDst);
}

/*void LogFileW(const wchar_t *msg)
{
	time_t rawtime;
	struct tm timeinfo;
	wchar_t buffer[80];

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);

	wcsftime(buffer, 80, _T("%Y-%m-%d %H:%M:%S: "), &timeinfo);
	
	ofstream logfile("C:\\Zabbix\\log\\fyl.log", ios::out | ios::app);
	logfile << buffer << msg << endl;
	logfile.close();
}*/

// Если за время TIMEOUT-1 программа не завершилась, значит произошла какая-то ошибка. Пытаемся закрыть порт и выйти
void ErrorHandler()
{
	clock_t end_time = clock() + (TIMEOUT - 1) * CLOCKS_PER_SEC;
	while (clock() < end_time) { Sleep(100); }
	LogFile("Аварийное завершение: ErrorHandler()");
	CloseHandle(hPort);
	exit(EXIT_SUCCESS);
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
}

// Если не удалось получить ответ за TIMEOUT секунд, убиться нафиг
void Timeout()
{
	clock_t end_time = clock() + TIMEOUT * CLOCKS_PER_SEC;
	while (clock() < end_time) { Sleep(100); }
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

	while (ReadFile(hPort, readBuf, BUFSIZ, &bytesRead, NULL))
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

		if (0x0a == (unsigned short)readBuf[bytesRead - 2] && 0x0c == (unsigned short)readBuf[bytesRead - 1]) break; // Конец сообщения
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
	if (pPrinter->Status != 0) ErrorHandler();

	char szBuf[BUFSIZ];
	ZeroMemory(szBuf, BUFSIZ);
	CharToAnsi(szDefaultPrinter, szBuf);
	LogFile(szBuf);
	ZeroMemory(szBuf, BUFSIZ);
	_itoa_s(pPrinter->Status, szBuf, BUFSIZ, 10);
	LogFile(szBuf);

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
	char szPortA[REG_MAX_VALUE_NAME];
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

		//if(PortNumber > 0 	// Нашли просто какой-то порт, т.к. агент заббикса запускается не от локального пользователя и у него другой принтер по умолчанию (((
		//	&& ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegControl, 0, KEY_READ, &hkControl)) // Проверяем, что есть раздел DeviceClasses\\{28d78fad-5a12-11d1-ae5b-0000f803a8c2}\\$achKey$\\#\\Control
		
		// Поиск порта подходит, если запускаемся от локального пользователя. Если от админа или system - надо проверять значение \\#\Control\Linked == 1
		if (0 == wcscmp(pPrinter->pPortName, szPort)) // Найденный порт совпал с портом принтера по умолчанию, значит нашли то что надо
		{
			RegGetValue(HKEY_LOCAL_MACHINE, szRegSymbolicLink, _T("SymbolicLink"), RRF_RT_ANY, NULL, NULL, &cbData); // DeviceClasses\\{28d78fad-5a12-11d1-ae5b-0000f803a8c2}\\$achKey$\\#
			vData = (VOID*)malloc(cbData);
			RegGetValue(HKEY_LOCAL_MACHINE, szRegSymbolicLink, _T("SymbolicLink"), RRF_RT_ANY, NULL, vData, &cbData); // DeviceClasses\\{28d78fad-5a12-11d1-ae5b-0000f803a8c2}\\$achKey$\\#
			memcpy(szInterface, vData, cbData);
			free(vData);
			flagExit = TRUE;
			CharToOem(szPort, szPortA);
			LogFile(szPortA);
		}
		RegCloseKey(hkDeviceParameters);
	}
	RegCloseKey(hkResult);

	char *pDst = new char[wcslen(szInterface) + 1];
	CharToOem(szInterface, pDst);
	LogFile(pDst);
	delete pDst;

	// Вот такие байты отсылает утилита Lexmark, чтобы получить данные от МФУ. Что они означают - полное ХЗ
	//byte b0[] = { 0xA5, 0x00, 0x07, 0x50, 0xE0, 0xD9, 0x00, 0x0E, 0x0E, 0x04 };

	const byte b1[] = { 0xA5, 0x00, 0x10, 0x80, 0xA4, 0x5B, 0xA4, 0x5B, 0x10, 0xEF, 0xA4, 0x5B, 0x11, 0xEE, 0x00, 0xFF, 0x13, 0xEC, 0x2E };
	const byte b2[] = { 0xA5, 0x00, 0x07, 0x50, 0xE0, 0x21, 0x01, 0x00, 0x00, 0x00 };
	//const byte b2[] = { 0xA5, 0x00, 0x04, 0x50, 0xE0, 0xE7, 0x02 };

	//wcout << szInterface << endl;

	hPort = CreateFile(szInterface, FILE_READ_DATA | FILE_WRITE_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hPort) ErrorHandler();

	//Init();

	// Получение остатка тонера

	DWORD BytesWritten;
	DWORD BytesRead;

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
		ReadFile(hPort, answer, BUFSIZE, &BytesRead, NULL);
		for (size_t i = 0; i < BytesRead; i++)
		{
			if (0xa5 == answer[i])
			{
				CountA5++;
				i += 4;
			}
			else
				AllAnswer[AllCount++] = answer[i];
		}

		Flag = answer[3];

		//if (!(Flag & flagContinue)) break;

		//if ((BytesRead > 0) && (0x2c == answer[BytesRead - 1])) break;	// Последний байт в ответе обычно 0x2c
		//if (47 == CountA5) break;										// Для страховки проверяем количество пакетов
		//if (0 == BytesRead) break;										// Прочитано 0 байт, наверное хватит
		//wcout << BytesRead << " " << CountA5 << endl;
	}

	// Записываем все муть, пришедшую с принтера, в файл log\answer.bin для последующего анализа
	ofstream answerbin("C:\\Zabbix\\log\\answer.bin", ios::out | ios::trunc | ios::binary);
	for (u_int i = 0; i < AllCount; i++)
		answerbin.put(AllAnswer[i]);
	answerbin.close();
	LogFile("Файл answer.bin записан");

	PrinterInfo Pi;
	ZeroMemory(&Pi, sizeof PrinterInfo);

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
	memcpy(Pi.Sn,  AllAnswer +  75, 0x0d);
	memcpy(Pi.Fw,  AllAnswer + 258, 0x0d);
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
	free(pPrinter);
	CloseHandle(hPort);

	LogFile("Стоп GetTonerLeft()");
	exit(EXIT_SUCCESS);
}

int main()
{
	std::locale rus("rus_rus.866");
	std::wcout.imbue(rus);

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

