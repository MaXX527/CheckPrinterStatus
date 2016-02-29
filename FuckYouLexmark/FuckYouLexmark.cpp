// FuckYouLexmark.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"

using namespace std;

HANDLE hPort;

// Если за время TIMEOUT-1 программа не завершилась, значит произошла какая-то ошибка. Пытаемся закрыть порт и выйти
void ErrorHandler()
{
	clock_t end_time = clock() + (TIMEOUT - 1) * CLOCKS_PER_SEC;
	while (clock() < end_time) {}
	CloseHandle(hPort);
	exit(EXIT_SUCCESS);
}

// Какая-то ошибка. Печатаем 1
/*void ErrorHandler(const char * code)
{
	CloseHandle(hPort);
	cout << code;
	exit(EXIT_SUCCESS);
}*/

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
	while (clock() < end_time) { }
	exit(EXIT_SUCCESS);
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
		if(PortNumber > 0 	// Нашли просто какой-то порт, т.к. агент заббикса запускается не от локального пользователя и у него другой принтер по умолчанию (((
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

	// Вот такие байты отсылает утилита Lexmark, чтобы получить данные от МФУ. Что они означают - полное ХЗ
	//byte b0[] = { 0xA5, 0x00, 0x07, 0x50, 0xE0, 0xD9, 0x00, 0x0E, 0x0E, 0x04 };

	const byte b1[] = { 0xA5, 0x00, 0x10, 0x80, 0xA4, 0x5B, 0xA4, 0x5B, 0x10, 0xEF, 0xA4, 0x5B, 0x11, 0xEE, 0x00, 0xFF, 0x13, 0xEC, 0x2E };
	const byte b2[] = { 0xA5, 0x00, 0x07, 0x50, 0xE0, 0x21, 0x01, 0x00, 0x00, 0x00 };

	//wcout << szInterface << endl;

	hPort = CreateFile(szInterface, FILE_READ_DATA | FILE_WRITE_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hPort) ErrorHandler();

	//Init();

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

	while (ReadFile(hPort, answer, BUFSIZE, &BytesRead, NULL))
	{
		for (size_t i = 0; i < BytesRead; i++)
		{
			AllAnswer[AllCount++] = answer[i];
			if (0xa5 == answer[i]) CountA5++;
		}

		//if ((BytesRead > 0) && (0x2c == answer[BytesRead - 1])) break;	// Последний байт в ответе обычно 0x2c
		if (47 == CountA5) break;										// Для страховки проверяем количество пакетов
		//if (0 == BytesRead) break;										// Прочитано 0 байт, наверное хватит
		//wcout << BytesRead << " " << CountA5 << endl;
	}

	// Ищем строку "Black Toner" в ответе, рядом будут серийный номер картриджа, емкость и остаток тонера в %
	const size_t BlackTonerLen = 11;
	byte BlackToner[] = { 0x42, 0x6c, 0x61, 0x63, 0x6b, 0x20, 0x54, 0x6f, 0x6e, 0x65, 0x72 };
	const size_t BlackTonerSNLen = 13;
	char BlackTonerSN[BlackTonerSNLen];

	PrinterInfo Pi;
	ZeroMemory(&Pi, sizeof PrinterInfo);

	for (u_int i = 0; i < AllCount - BlackTonerLen; i++)
	{
		for (u_int j = 0; j < BlackTonerLen; j++)
		{
			if (j == BlackTonerLen - 1) // Нашли строку
			{
				memcpy(Pi.Sn, AllAnswer + i + j + 16, BlackTonerSNLen - 1);
				BlackTonerSN[BlackTonerSNLen - 1] = '\0';

				Pi.Percent = static_cast<u_int>(AllAnswer[i + 91]);

				Pi.Capacity = (((u_int)AllAnswer[i + 44] << 8) + (u_int)AllAnswer[i + 45]);
			}
			if (AllAnswer[i + j] != BlackToner[j])
				break;
		}
	}

	CloseHandle(hPort);
	
	if (Pi.Capacity > 0) // Что-то узнали
	{
		cout << Pi.Percent * Pi.Capacity / 100;
		//ofstream left("left.txt", ios::out | ios::trunc);
		//left << Pi.Percent * Pi.Capacity / 100;
		//left.close();
		//system("PAUSE");
		exit(EXIT_SUCCESS);
	}
	//system("PAUSE");
}

int main()
{
	std::locale rus("rus_rus.866");
	std::wcout.imbue(rus);

	thread timeout(Timeout);
	thread printer(GetTonerLeft);
	thread errorhandler(ErrorHandler);
	
	errorhandler.join();
	timeout.join();
	printer.join();

	return 0;
}

