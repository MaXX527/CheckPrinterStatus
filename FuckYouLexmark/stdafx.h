// stdafx.h: включаемый файл дл€ стандартных системных включаемых файлов
// или включаемых файлов дл€ конкретного проекта, которые часто используютс€, но
// не часто измен€ютс€
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>



// TODO: ”становите здесь ссылки на дополнительные заголовки, требующиес€ дл€ программы

#include <iostream>
//#include <iomanip>
#include <Windows.h>
#include <thread>
#include <fstream>

struct PrinterInfo
{
	char Sn[13];
	u_int Percent;
	u_int Capacity;
};

#define REG_MAX_KEY_LENGTH 255
#define REG_MAX_VALUE_NAME 16383
#define TIMEOUT 10		// ќжидать ответ принтера 10с

//#include <cfgmgr32.h>
//#include <setupapi.h>
//#include <devguid.h>

//#pragma comment(lib, "Cfgmgr32.lib")
//#pragma comment(lib, "Setupapi.lib")