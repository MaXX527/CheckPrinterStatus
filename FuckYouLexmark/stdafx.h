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
	char  Sn[0x0d+1];		// sn принтера
	char  Fw[0x0d+1];		// верси€ прошивки
	char  cSn[0x0c+1];	// sn картриджа
	u_int cPercent;		// остаток тонера в %
	u_int cCapacity;	// ресурс картриджа
	char  iuSn[0x0c+1];	// sn Imaging Unit
	u_int iuPercent;	// остаток ресурса Imaging Unit в %
	u_int iuResource;	// ресурс Imaging Unit
};

#define REG_MAX_KEY_LENGTH 255
#define REG_MAX_VALUE_NAME 16383
#define TIMEOUT 30		// ќжидать ответ принтера 30с

#include <cfgmgr32.h>
#include <setupapi.h>
//#include <devguid.h>

//#pragma comment(lib, "Cfgmgr32.lib")
#pragma comment(lib, "Setupapi.lib")