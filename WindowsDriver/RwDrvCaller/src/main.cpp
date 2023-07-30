#include <Windows.h>
#include <stdio.h>

#define RW_DRIVER_ID							"RwDrv"

// type of access: U8 = 0; U16 = 1, U32 =2, ref to READ_REGISTER_BUFFER_UCHAR / USHORT / ULONG
typedef struct {
	DWORD64 physicalAddress;
	DWORD size;
	DWORD access;
	DWORD64 buffer;
} PhysRw_t;

typedef struct {
	DWORD low;
	DWORD pad;
	DWORD reg;
	DWORD high;
} MSRRw_t;

HANDLE hDrv;

DWORD LoadDriver()
{
	TCHAR sDrv[MAX_PATH] = {0};
	GetFullPathName("RwDrv.sys", MAX_PATH, sDrv, NULL);
	
	//check driver exist
	WIN32_FIND_DATA	findData;
	HANDLE hFile = FindFirstFile(sDrv, &findData);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		printf("Driver File Doesn't Exist, errno = %d.\n", GetLastError());
		return -1;
	}

	//establishes a connection to the service control manager
	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(hSCM == NULL)
	{
		printf("Open SCM Failed, errno = %d.\n", GetLastError());
		return -1;
	}
	
	//create a service object and add to SCM
	SC_HANDLE hSrv = CreateService(hSCM, RW_DRIVER_ID, RW_DRIVER_ID, SC_MANAGER_ALL_ACCESS, 
		SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, 
		sDrv, NULL, NULL, NULL, NULL, NULL);
	if(hSrv == NULL)
	{
		if(GetLastError() != ERROR_SERVICE_EXISTS)
		{
			CloseHandle(hSCM);
			printf("Create Service Failed, errno = %d.\n", GetLastError());
			return -1;
		}
	}

	//open service
	hSrv = OpenService(hSCM, RW_DRIVER_ID, SERVICE_ALL_ACCESS);
	if(hSrv == NULL)
	{
		CloseHandle(hSCM);
		printf("Open Service Failed, errno = %d.\n", GetLastError());
		return -1;
	}
	
	//start service
	if(!StartService(hSrv, 0, NULL))
	{
		if(GetLastError() != ERROR_SERVICE_ALREADY_RUNNING)
		{
			CloseHandle(hSCM);
			CloseServiceHandle(hSrv);
			printf("Start Service Failed, errno = %d.\n", GetLastError());
			return -1;
		}
	}

	//create or open device object, see "Win32 Device Namespaces" section 
	//of https://learn.microsoft.com/en-us/windows/win32/fileio/naming-a-file
	//and https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilea
	hDrv = CreateFile("\\\\.\\"RW_DRIVER_ID, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hDrv == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hSCM);
		CloseServiceHandle(hSrv);
		printf("Create File Failed, errno = %d.\n", GetLastError());
		return -1;
	}

	return 0;
}

DWORD UnloadDriver()
{
	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(hSCM == NULL)
	{
		printf("Open SCM Failed, errno = %d.\n", GetLastError());
		return -1;
	}

	SC_HANDLE hSrv = OpenService(hSCM, RW_DRIVER_ID, SERVICE_ALL_ACCESS);
	if(hSrv == NULL)
	{
		CloseHandle(hSCM);
		printf("Open Service Failed, errno = %d.\n", GetLastError());
		return -1;
	}

	SERVICE_STATUS status;
	if (!ControlService(hSrv, SERVICE_CONTROL_STOP, &status)) 
	{
		CloseHandle(hSCM);
		CloseHandle(hDrv);
		printf("Stop Service Failed, errno = %d.\n", GetLastError());
		return -1;
	}

	CloseHandle(hSCM);
	CloseHandle(hDrv);

	return 0;
}

BOOL isElevated() {
	HANDLE hToken = NULL;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
		TOKEN_ELEVATION Elevation;
		DWORD cbSize = sizeof(TOKEN_ELEVATION);
		if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) {
			return Elevation.TokenIsElevated;
		}
	}
	if (hToken) {
		CloseHandle(hToken);
	}
	return FALSE;
}

void readMem(DWORD64 address, const DWORD64* buffer, DWORD size, DWORD access = 0) 
{
	PhysRw_t tPhyMem;
	tPhyMem.physicalAddress = address;
	tPhyMem.size = size;
	tPhyMem.access = access;
	tPhyMem.buffer = (DWORD64)buffer;
	DeviceIoControl(hDrv, 0x222808, &tPhyMem, sizeof(tPhyMem), &tPhyMem, sizeof(tPhyMem), NULL, NULL);
}
void writeMem(DWORD64 address, const DWORD64* buffer, DWORD size, DWORD access = 0) 
{
	PhysRw_t tPhyMem;
	tPhyMem.physicalAddress = address;
	tPhyMem.size = size;
	tPhyMem.access = access;
	tPhyMem.buffer = (DWORD64)buffer;
	DeviceIoControl(hDrv, 0x22280C, &tPhyMem, sizeof(tPhyMem), NULL, 0, NULL, NULL);
}

void readMSR(int reg, LARGE_INTEGER& value) 
{
    MSRRw_t tMSR;
	tMSR.low = 0;
	tMSR.pad = 0;
	tMSR.reg = reg;	
	tMSR.high = 0;

    DeviceIoControl(hDrv, 0x222848, &tMSR, sizeof(tMSR), &tMSR, sizeof(tMSR), NULL, NULL);

	value.LowPart = tMSR.low;
	value.HighPart = tMSR.high;
}
void writeMSR(int reg, LARGE_INTEGER& value) {
    MSRRw_t tMSR;
	ZeroMemory(&tMSR, sizeof(tMSR));
    tMSR.reg = reg;
    tMSR.low = value.LowPart;
    tMSR.high= value.HighPart;
	
    DeviceIoControl(hDrv, 0x22284C, &tMSR, sizeof(tMSR), &tMSR, sizeof(tMSR), NULL, NULL);
}

int main()
{
	//Check UAC
	if(isElevated() == FALSE)
	{
		printf("This program requires run as administrator.\n");
		return 0;
	}

	LoadDriver();

	//Read Mem
	DWORD64 buf[1] = {0};
	readMem(0x0000CCCC, buf, 1);
	/*
	DWORD64 buf[16] = {0};
	readMem(0x0000CCCC, buf, 2, 1);
	*/
	printf("read mem = %x\n", buf[0]);
	
	//Read Msr
	LARGE_INTEGER value;
	readMSR(0x20, value);
	printf("read msr high part = %08x, low part = %08x\n", value.HighPart, value.LowPart);

	UnloadDriver();

	return 0;
}