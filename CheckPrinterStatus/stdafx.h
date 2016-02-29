// stdafx.h: включаемый файл дл€ стандартных системных включаемых файлов
// или включаемых файлов дл€ конкретного проекта, которые часто используютс€, но
// не часто измен€ютс€
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include <iostream>
#include <locale>

#include <Windows.h>
#include <winsplp.h>
#include <winspool.h>

//#include <fstream>

//#include <Cfgmgr32.h>
//#pragma comment(lib, "WinSpool.lib")
//#pragma comment(lib, "Cfgmgr32.lib")

#include <SetupAPI.h>

#pragma comment(lib, "Setupapi.lib")

//#include <usbprint.h>

#include "DefaultPrinter.h"

#include <map>
#include <ctime>
#include <thread>
#include <fstream>

#define REG_MAX_KEY_LENGTH 255
#define REG_MAX_VALUE_NAME 16383

// TODO: ”становите здесь ссылки на дополнительные заголовки, требующиес€ дл€ программы
