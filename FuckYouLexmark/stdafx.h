// stdafx.h: ���������� ���� ��� ����������� ��������� ���������� ������
// ��� ���������� ������ ��� ����������� �������, ������� ����� ������������, ��
// �� ����� ����������
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>



// TODO: ���������� ����� ������ �� �������������� ���������, ����������� ��� ���������

#include <iostream>
//#include <iomanip>
#include <Windows.h>
#include <thread>
#include <fstream>

struct PrinterInfo
{
	char  Sn[0x0d+1];		// sn ��������
	char  Fw[0x0d+1];		// ������ ��������
	char  cSn[0x0c+1];	// sn ���������
	u_int cPercent;		// ������� ������ � %
	u_int cCapacity;	// ������ ���������
	char  iuSn[0x0c+1];	// sn Imaging Unit
	u_int iuPercent;	// ������� ������� Imaging Unit � %
	u_int iuResource;	// ������ Imaging Unit
};

#define REG_MAX_KEY_LENGTH 255
#define REG_MAX_VALUE_NAME 16383
#define TIMEOUT 30		// ������� ����� �������� 30�

#include <cfgmgr32.h>
#include <setupapi.h>
//#include <devguid.h>

//#pragma comment(lib, "Cfgmgr32.lib")
#pragma comment(lib, "Setupapi.lib")