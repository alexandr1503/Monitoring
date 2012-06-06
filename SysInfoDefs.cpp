#include "stdafx.h"
#include "SysInfoDefs.h"
#include "winioctl.h"
#include "windows.h"
#include "SmartDef.h"

BYTE	AttrOutCmd[sizeof(SENDCMDOUTPARAMS) + READ_ATTRIBUTE_BUFFER_SIZE - 1];
BYTE	ThreshOutCmd[sizeof(SENDCMDOUTPARAMS) + READ_THRESHOLD_BUFFER_SIZE - 1];
BYTE	IdOutCmd[sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1];

/*
	IDE Commands using S.M.A.R.T
*/
BOOL DoIDENTIFY(HANDLE, PSENDCMDINPARAMS, PSENDCMDOUTPARAMS, BYTE, BYTE, PDWORD);
BOOL DoEnableSMART(HANDLE, PSENDCMDINPARAMS, PSENDCMDOUTPARAMS, BYTE, PDWORD);
BOOL DoReadAttributesCmd(HANDLE, PSENDCMDINPARAMS, PSENDCMDOUTPARAMS, BYTE );
BOOL DoReadThresholdsCmd(HANDLE, PSENDCMDINPARAMS, PSENDCMDOUTPARAMS, BYTE );
VOID ChangeByteOrder(PCHAR, USHORT);
HANDLE OpenSMART(VOID);
CString DoPrintData(PCHAR, PCHAR, BYTE);
CString DisplayIdInfo(PIDSECTOR, PSENDCMDINPARAMS, BYTE, BYTE, BYTE);
CString PrintIDERegs(PSENDCMDINPARAMS);


CString DetermineIEVer()
{
	CString Ver="Cannot extract version number of Internet Explorer";

	HINSTANCE hBrowser;
	hBrowser=LoadLibrary(_T("shdocvw.dll"));

	if (hBrowser)
	{
		HRESULT hr=S_OK;
		DLLGETVERSIONPROC pDllGetVersion;

		pDllGetVersion=(DLLGETVERSIONPROC) GetProcAddress(hBrowser,_T("DllGetVersion"));

		if (pDllGetVersion)
		{
			DLLVERSIONINFO dvi;
			ZeroMemory(&dvi,sizeof(dvi));
			dvi.cbSize=sizeof(dvi);

			hr= (*pDllGetVersion) (&dvi);

			if (SUCCEEDED(hr))
			{
				Ver.Format("Версия: %ld.%ld, Сборка: %ld",dvi.dwMajorVersion,
										 dvi.dwMinorVersion,
										 dvi.dwBuildNumber);

			}
		}
	}

	FreeLibrary(hBrowser);

	return Ver;
}

CString QueryHardDisks()
{
	int index=0;
	HANDLE hDevice;
	CString Result;
	CString HardDisk;
	DWORD junk;
	DISK_GEOMETRY pdg;

	HardDisk.Format("\\\\.\\PhysicalDrive%d",index);

	hDevice=CreateFile(HardDisk,0,FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,OPEN_EXISTING,0,NULL);

	if (hDevice==INVALID_HANDLE_VALUE)
		Result="Error: Can not Query Hard Disk Drives.";

	while (hDevice!=INVALID_HANDLE_VALUE)
	{
		BOOL bResult=DeviceIoControl(hDevice,IOCTL_DISK_GET_DRIVE_GEOMETRY,
									NULL,0,
									&pdg,sizeof(DISK_GEOMETRY),
									&junk,
									(LPOVERLAPPED) NULL);

		if (bResult)
		{
			CString Temp;
			Temp.Format("--- Жесткий диск № %d ---\r\n",index+1);
			Result+=Temp;

			Temp.Format("Цилиндры: %I64d\r\n",pdg.Cylinders);
			Result+=Temp;

			Temp.Format("Трэков в цилиндре: %ld\r\n",(ULONG) pdg.TracksPerCylinder);
			Result+=Temp;

			Temp.Format("Секторов в трэке: %ld\r\n",(ULONG) pdg.SectorsPerTrack);
			Result+=Temp;

			Temp.Format("Байтов в секторе: %ld\r\n",(ULONG) pdg.BytesPerSector);
			Result+=Temp;

			ULONGLONG DiskSize=pdg.Cylinders.QuadPart * (ULONG) pdg.TracksPerCylinder * 
				(ULONG) pdg.SectorsPerTrack * (ULONG) pdg.BytesPerSector;
			
			Temp.Format("Полный размер: %I64d (Bytes) -> %I64d (MB) -> %I64d (GB)\r\n\r\n",DiskSize,
				DiskSize/1024/1024, DiskSize/1024/1024/1024);
			Result+=Temp;
		}


		CloseHandle(hDevice);

		index++;
		HardDisk.Format("\\\\.\\PhysicalDrive%d",index);
		hDevice=CreateFile(HardDisk,0,FILE_SHARE_READ|FILE_SHARE_WRITE,
							NULL,OPEN_EXISTING,0,NULL);
	}

	CString Temp;
	Temp.Format("Количество жестких дисков: %d\r\n\r\n",index);
	Result=Temp+Result;
	
	return Result;
}

CString QueryDriveSpace()
{
	ULARGE_INTEGER AvailableToCaller, Disk, Free;
	CString Result, Temp;

	for (int iCounter=0;iCounter<MAX_OF_HARD_DISKS;iCounter++)
	{
		if (GetDriveType(HardDiskLetters[iCounter])==DRIVE_FIXED)
		{
			if (GetDiskFreeSpaceEx(HardDiskLetters[iCounter],&AvailableToCaller,
									&Disk, &Free))
			{
				Temp.Format("*** Жесткий диск: (%s) ***\r\n",HardDiskLetters[iCounter]);
				Result+=Temp;

				Temp.Format("Полный размер: %I64d (MB)\r\n",Disk.QuadPart/1024/1024);
				Result+=Temp;

				ULONGLONG Used=Disk.QuadPart-Free.QuadPart;
				Temp.Format("Занято: %I64d (MB)\r\n",Used/1024/1024);
				Result+=Temp;

				Temp.Format("Свободно: %I64d (MB)\r\n",Free.QuadPart/1024/1024);
				Result+=Temp;
			}

			char Label[256]="";
			char FS[256]="";
			DWORD SerialNumber=0, MaxFileLen=0, FileSysFlag=0;
			
			if (GetVolumeInformation(HardDiskLetters[iCounter], Label, 256, &SerialNumber,
				&MaxFileLen, &FileSysFlag, FS, 256))
			{
				Temp.Format("%X", SerialNumber);
				Temp=CString("Серийный номер: ") + Temp.Left(4) + 
					CString("-") + Temp.Right(4) + CString("\r\n");
				Result+=Temp;
			
				Temp.Format("Максимальный размер файла: %d\r\n", MaxFileLen);
				Result+=Temp;

				Temp.Format("Файловая система: %s\r\n\r\n", FS);
				Result+=Temp;
			}
		}
	}

	return Result;
}

CString QueryCDDrive()
{
	CString Result;
	CString Temp;
	int NumofCDDrive=0;
	BOOL bFlag=FALSE;
	for (int iCounter=0;iCounter<MAX_OF_HARD_DISKS;iCounter++)
	{
		if (GetDriveType(HardDiskLetters[iCounter])==DRIVE_CDROM)
		{
			NumofCDDrive++;
			bFlag=TRUE;
			Temp.Format("CD Drive letter: (%s)\r\n",HardDiskLetters[iCounter]);
			Result+=Temp;
		}
	}

	if (bFlag)
		Temp.Format("Number of CD Drive(s) found: %d\r\n",NumofCDDrive);
	else
		Temp="Нет доступа.\r\n";

	Result=Temp+Result;

	return Result;
}

CString QueryTotalRAM()
{
	CString Response;
	MEMORYSTATUS memoryStatus;
	ZeroMemory(&memoryStatus,sizeof(MEMORYSTATUS));
	memoryStatus.dwLength = sizeof (MEMORYSTATUS);
	
	::GlobalMemoryStatus (&memoryStatus);
	
	CString Temp;
//	Temp.Format("Installed RAM: %ldMB",(DWORD) ceil(memoryStatus.dwTotalPhys/1024/1024));
	Response=Temp;
	
	Temp.Format("\rДоступная память: %ldKB",(DWORD) (memoryStatus.dwAvailPhys/1024));
	Response+=Temp;
	
	Temp.Format("\r\nПроцент занятой RAM: %%%ld",memoryStatus.dwMemoryLoad);
	Response+=Temp;

	return Response;
}

CString QueryFreeRAM()
{
	CString Response;
	MEMORYSTATUS memoryStatus;
	ZeroMemory(&memoryStatus,sizeof(MEMORYSTATUS));
	memoryStatus.dwLength = sizeof (MEMORYSTATUS);
	
	::GlobalMemoryStatus (&memoryStatus);
	
	Response.Format("Свободно: %%%ld",100-memoryStatus.dwMemoryLoad);
	
	return Response;
}

CString QueryUserName()
{
	CString Response;
	char UserName[UNLEN + 1]="";
	DWORD Size=UNLEN+1;
	if (GetUserName(UserName,&Size)!=0)
		Response.Format("Имя пользователя: %s",UserName);
	else
		Response="Error";
	
	return Response;
}

CString QueryComputerName()
{
	CString Response;
	char ComputerName[MAX_COMPUTERNAME_LENGTH+1]="";
	DWORD Size=MAX_COMPUTERNAME_LENGTH+1;
	if (GetComputerName(ComputerName,&Size)!=0)
		Response.Format("Имя компьютера: %s",ComputerName);
	else
		Response="Error";

	return Response;
}

CString QueryCPUInfo()
{
	CString Response;
	SYSTEM_INFO SysInfo;
	int nProcessors = 0;
	
	// Get the number of processors in the system.
	ZeroMemory (&SysInfo, sizeof (SYSTEM_INFO));
	GetSystemInfo (&SysInfo);
	
	
	nProcessors = SysInfo.dwNumberOfProcessors;
	Response.Format("Количество ядер: %d",nProcessors);
	
	return Response;
}

CString QueryCPUSpeed()
{
	CString Response;
	CPUInfo cpu;
	
	Response.Format("Тактовая частота: %d MHz",cpu.GetProcessorClockFrequency());
	return Response;
}

CString QueryCPUID()
{
	CString Response;
	CPUInfo cpu;
	if (cpu.DoesCPUSupportCPUID())
	{
		Response="Производитель: ";
		Response+=cpu.GetVendorID();
		Response+="\r\n Тип: ";
		Response+=cpu.GetTypeID();
		Response+="\r\nСемейство ID: ";
		Response+=cpu.GetFamilyID();
		Response+="\r\nМодель ID: ";
		Response+=cpu.GetModelID();
		Response+="\r\nКод: ";
		Response+=cpu.GetSteppingCode();
		Response+="\r\nБрэнд ID: ";
		Response+=cpu.GetExtendedProcessorName();
	}
	else
	{
		Response="Error: не поддерживает CPUID.";
	}
	
	return Response;
}

CString QueryCPUVendorID()
{
	CString Response;
	CPUInfo cpu;
	if (cpu.DoesCPUSupportCPUID())
	{
		Response+="Производитель: ";
		Response+=cpu.GetVendorID();
		Response+="\r\nБрэнд: ";
		Response+=cpu.GetExtendedProcessorName();
	}
	else
	{
		Response="Error: процкссор не поддерживает  CPUID.";
	}
	
	return Response;
}

CString QueryResolution()
{
	CString Response;
	int iWidth=GetSystemMetrics(SM_CXSCREEN);
	int iHeight=GetSystemMetrics(SM_CYSCREEN);
	
	Response.Format("Разрешение экрана: %dx%d\r\n"
		"Ширина: %d pixels\r\n"
		"Высота: %d pixels",
		iWidth,iHeight,iWidth,iHeight);
	
	return Response;
}

CString QueryPixelDepth()
{
	CString Response;
	DEVMODE DevMode;
	DevMode.dmSize=sizeof(DEVMODE);
	
	if (EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&DevMode))
	{
		CString temp;
		Response.Format("Глубина цвета: %ld Bits per pixel.",DevMode.dmBitsPerPel);
		
		temp.Format("\r\n Ширина, рх: %ld",DevMode.dmPelsWidth);
		Response+=temp;
		
		temp.Format("\r\nВысота, рх: %ld",DevMode.dmPelsHeight);
		Response+=temp;
		
		temp.Format("\r\nЧасстота обновления: %d Hz",DevMode.dmDisplayFrequency);
		Response+=temp;
	}
	else
		Response="Error";
	
	return Response;
}



CString QueryOS()
{
	CString Response;
/*	OS_VERSION_INFO osvi;
	char sText[512]="";
	char sBuf[100]="";
	ZeroMemory(&osvi,sizeof(OS_VERSION_INFO));
	
	if (GetOSVersion(&osvi))
	{                                           
		_stprintf(sText, _T("Emulated OS: "));
		
		switch (osvi.dwEmulatedPlatformId)
		{
		case PLATFORM_DOS:               
			{
				_stprintf(sBuf, _T("Dos"));
				break;
			}
		case PLATFORM_WINDOWS31:         
			{
				_stprintf(sBuf, _T("Windows"));
				break;
			}
		case PLATFORM_WINDOWSFW:         
			{
				_stprintf(sBuf, _T("Windows For Workgroups"));
				break; 
			}
		case PLATFORM_WIN32S:
			{
				_stprintf(sBuf, _T("Win32s"));
				break; 
			}
		case PLATFORM_WINDOWS_CE:
			{
				_stprintf(sBuf, _T("Windows CE"));
				break;
			}
		case PLATFORM_WINDOWS:
			{
				if (IsWindows95(&osvi))
					_stprintf(sBuf, _T("Windows 95"));
				else if (IsWindows95SP1(&osvi))
					_stprintf(sBuf, _T("Windows 95 SP1"));
				else if (IsWindows95OSR2(&osvi))
					_stprintf(sBuf, _T("Windows 95 OSR2"));
				else if (IsWindows98(&osvi))
					_stprintf(sBuf, _T("Windows 98"));
				else if (IsWindows98SP1(&osvi))
					_stprintf(sBuf, _T("Windows 98 SP1"));
				else if (IsWindows98SE(&osvi))
					_stprintf(sBuf, _T("Windows 98 Second Edition"));
				else
					_stprintf(sBuf, _T("Windows ??"));
				break;
			}
		case PLATFORM_NT_WORKSTATION:
			{
				if (IsWindows2000(&osvi))
					_stprintf(sBuf, _T("Windows 2000 Professional"));
				else
					_stprintf(sBuf, _T("Windows NT Workstation"));
				break;
			}
		case PLATFORM_NT_PRIMARY_DOMAIN_CONTROLLER:         
			{
				if (IsWindows2000(&osvi))
					_stprintf(sBuf, _T("Windows 2000 Server (Acting as Primary Domain Controller)"));
				else
					_stprintf(sBuf, _T("Windows NT Server (Acting as Primary Domain Controller)"));
				break;
			}
		case PLATFORM_NT_BACKUP_DOMAIN_CONTROLLER:         
			{
				if (IsWindows2000(&osvi))
					_stprintf(sBuf, _T("Windows 2000 Server (Acting as Backup Domain Controller)"));
				else
					_stprintf(sBuf, _T("Windows NT Server (Acting as Backup Domain Controller)"));
				break;
			}
		case PLATFORM_NT_STAND_ALONE_SERVER:
			{
				if (IsWindows2000(&osvi))
					_stprintf(sBuf, _T("Windows 2000 Server (Acting as Standalone Sever)"));
				else
					_stprintf(sBuf, _T("Windows NT Server (Acting as Standalone Sever)"));
				break;
			}
		case PLATFORM_WINDOWS_TERMINAL_SERVER:
			{
				_stprintf(sBuf, _T("Windows NT Terminal Server"));
				break;
			}
		case PLATFORM_NT_ENTERPRISE_SERVER:
			{
				_stprintf(sBuf, _T("Windows NT Enterprise Edition"));
				break;
			}
		default: 
			{
				_stprintf(sBuf, _T("Unknown OS"));
				break;
			}
		}                     
		_tcscat(sText, sBuf);
		_stprintf(sBuf, _T(" v%d."), osvi.dwEmulatedMajorVersion);
		_tcscat(sText, sBuf);     
		_stprintf(sBuf, _T("%02d "), osvi.dwEmulatedMinorVersion);
		_tcscat(sText, sBuf);                           
		if (osvi.dwEmulatedBuildNumber)
		{
			_stprintf(sBuf, _T("Build:%d"), osvi.dwEmulatedBuildNumber);
			_tcscat(sText, sBuf);           
		}
		if (osvi.wEmulatedServicePack)       
		{
			_stprintf(sBuf, _T(" Service Pack:%d"), osvi.wEmulatedServicePack);
			_tcscat(sText, sBuf);
		}                            
		_tcscat(sText, _T(" \r\n"));        
		
		_stprintf(sBuf, _T("Underlying OS: "));
		_tcscat(sText, sBuf);
		
		switch (osvi.dwUnderlyingPlatformId)
		{
		case PLATFORM_DOS:               
			{
				_stprintf(sBuf, _T("Dos"));
				break;
			}
		case PLATFORM_WINDOWS31:         
			{
				_stprintf(sBuf, _T("Windows"));
				break;
			}
		case PLATFORM_WINDOWSFW:         
			{
				_stprintf(sBuf, _T("Windows For Workgroups"));
				break;
			}
		case PLATFORM_WIN32S:            
			{
				_stprintf(sBuf, _T("Win32s"));
				break;
			}
		case PLATFORM_WINDOWS_CE:            
			{
				_stprintf(sBuf, _T("Windows CE"));
				break;
			}
		case PLATFORM_WINDOWS:           
			{
				if (IsWindows95(&osvi))
					_stprintf(sBuf, _T("Windows 95"));
				else if (IsWindows95SP1(&osvi))
					_stprintf(sBuf, _T("Windows 95 SP1"));
				else if (IsWindows95OSR2(&osvi))
					_stprintf(sBuf, _T("Windows 95 OSR2"));
				else if (IsWindows98(&osvi))
					_stprintf(sBuf, _T("Windows 98"));
				else if (IsWindows98SP1(&osvi))
					_stprintf(sBuf, _T("Windows 98 SP1"));
				else if (IsWindows98SE(&osvi))
					_stprintf(sBuf, _T("Windows 98 Second Edition"));
				else
					_stprintf(sBuf, _T("Windows ??"));
				break;
			}
		case PLATFORM_NT_WORKSTATION:    
			{
				if (IsWindows2000(&osvi))
					_stprintf(sBuf, _T("Windows 2000 Professional"));
				else
					_stprintf(sBuf, _T("Windows NT Workstation"));
				break;
			}
		case PLATFORM_NT_PRIMARY_DOMAIN_CONTROLLER:         
			{
				if (IsWindows2000(&osvi))
					_stprintf(sBuf, _T("Windows 2000 Server (Acting as Primary Domain Controller)"));
				else
					_stprintf(sBuf, _T("Windows NT Server (Acting as Primary Domain Controller)"));
				break;
			}
		case PLATFORM_NT_BACKUP_DOMAIN_CONTROLLER:         
			{
				if (IsWindows2000(&osvi))
					_stprintf(sBuf, _T("Windows 2000 Server (Acting as Backup Domain Controller)"));
				else
					_stprintf(sBuf, _T("Windows NT Server (Acting as Backup Domain Controller)"));
				break;
			}
		case PLATFORM_NT_STAND_ALONE_SERVER:
			{
				if (IsWindows2000(&osvi))
					_stprintf(sBuf, _T("Windows 2000 Server (Acting as Standalone Sever)"));
				else
					_stprintf(sBuf, _T("Windows NT Server (Acting as Standalone Sever)"));
				break;
			}
		case PLATFORM_WINDOWS_TERMINAL_SERVER:
			{
				_stprintf(sBuf, _T("Windows NT Terminal Server"));
				break;
			}
		case PLATFORM_NT_ENTERPRISE_SERVER:
			{
				_stprintf(sBuf, _T("Windows NT Enterprise Edition"));
				break;
			}
		default:                         
			{
				_stprintf(sBuf, _T("Unknown OS"));
				break;
			}
		}                                      
		_tcscat(sText, sBuf);        
		_stprintf(sBuf, _T(" v%d."), osvi.dwUnderlyingMajorVersion);
		_tcscat(sText, sBuf);     
		_stprintf(sBuf, _T("%02d "), osvi.dwUnderlyingMinorVersion);
		_tcscat(sText, sBuf);          
		if (osvi.dwUnderlyingBuildNumber)
		{
			_stprintf(sBuf, _T("Build:%d"), osvi.dwUnderlyingBuildNumber);
			_tcscat(sText, sBuf);
		}
		if (osvi.wUnderlyingServicePack)       
		{
			_stprintf(sBuf, _T(" Service Pack:%d"), osvi.wUnderlyingServicePack);
			_tcscat(sText, sBuf);
		}                    
		_tcscat(sText, _T(" \n"));    
  }
  else
	  _stprintf(sText, _T("Failed in call to GetOSVersion\n"));
 
  Response=sText;
 */ 
  return Response;

  }

CString QueryLocalIPAddress()
{
	WORD wVersionRequested;
    WSADATA wsaData;
    char name[255];
    CString IP;
	PHOSTENT hostinfo;
	wVersionRequested = MAKEWORD(2,0);
	
	if (WSAStartup(wVersionRequested, &wsaData)==0)
	{
		if(gethostname(name, sizeof(name))==0)
		{
			if((hostinfo=gethostbyname(name)) != NULL)
			{
				IP = inet_ntoa(*(struct in_addr*)* hostinfo->h_addr_list);
			}
		}
		
		WSACleanup();
	} 
	
	IP=(LPCTSTR) "IP Address: " + IP;
	
	return (IP);
}

CString QueryDirectory(char* sFolder,int iFolder)
{
	CString Response;
	char path[MAX_PATH];
	if (SHGetSpecialFolderPath(NULL,path,iFolder,FALSE))
		Response.Format("%s \r\n   %s\r\n",sFolder,path);
	else
		Response="Error: Folder not found.";

	return Response;
}

CString GetCommandError()
{
	CString Result;

	DWORD dwError=GetLastError();
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
		);
	
	Result.Format("Error: %s\r\n",(char*) lpMsgBuf);
	LocalFree( lpMsgBuf );

	return Result;
}

/*
CString QueryPartitions()
{
	int index=0;
	HANDLE hDevice;
	CString Result;
	CString HardDisk;
	DWORD junk;
	DRIVE_LAYOUT_INFORMATION_EX dli;
	ZeroMemory(&dli, sizeof(dli));
	//dli.

	HardDisk.Format("\\\\.\\PhysicalDrive%d",index);

	hDevice=CreateFile(HardDisk,0,FILE_SHARE_READ,
						NULL,OPEN_EXISTING,0,NULL);

	if (hDevice==INVALID_HANDLE_VALUE)
		Result="Error: Can not Query Partitions.";

	while (hDevice!=INVALID_HANDLE_VALUE)
	{
		BOOL bResult=DeviceIoControl(hDevice,IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
									NULL,0,
									&dli, sizeof(DRIVE_LAYOUT_INFORMATION_EX),
									&junk,
									(LPOVERLAPPED) NULL);

		if (bResult)
		{
			CString Temp;
			Temp.Format("Number of partitions in physical disk %d is %d\r\n",index+1, dli.PartitionCount);
			Result+=Temp;

			for (DWORD iCounter=0;iCounter<dli.PartitionCount;iCounter++)
			{
				Temp.Format("\r\nPartition No.: %d", iCounter);
				Result+=Temp;
				
				switch (dli.PartitionStyle)
				{
				case PARTITION_STYLE_MBR:
					Temp.Format("Partition Style: MBR "
						"(standard AT-style master boot records)\r\n");
					Result+=Temp;
					Temp.Format("Signature: %ld\r\n", dli.Mbr.Signature);
					Result+=Temp;
					break;
					
				case PARTITION_STYLE_GPT:
					Temp.Format("Partition Style: GPT\r\n");
					Result+=Temp;
					USHORT guid[100];
//					StringFromGUID2(dli.Gpt.DiskId, guid, 100);
					Temp.Format("Disk ID: %s\r\n", guid);
					Result+=Temp;
					Temp.Format("Starting byte offset of the first usable block: %I64d\r\n", dli.Gpt.StartingUsableOffset);
					Result+=Temp;
					Temp.Format("Size of the usable blocks on the disk (bytes): %I64d\r\n", dli.Gpt.UsableLength);
					Result+=Temp;
					Temp.Format("Maximum number of partitions that can be defined in the usable block: %d", dli.Gpt.MaxPartitionCount);
					Result+=Temp;					
					break;
					
				case PARTITION_STYLE_RAW:
					Temp.Format("Partition Style: RAW "
						"(Partition not formatted in either of "
						"the recognized formats-MBR or GPT\r\n");
					Result+=Temp;
					break;
					
				default:
					Temp.Format("Not recognized\r\n");
					Result+=Temp;
				}	//switch
			}	//for
		}	//if	
		else
		{
			LPSTR MessageBuffer;

			DWORD dwFormatFlags=FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_IGNORE_INSERTS |
		        FORMAT_MESSAGE_FROM_SYSTEM ;

			FormatMessageA(dwFormatFlags, NULL, GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR) &MessageBuffer, 0, NULL);

			Result+=MessageBuffer;
			LocalFree(MessageBuffer);
		}
		
		CloseHandle(hDevice);
		
		index++;
		HardDisk.Format("\\\\.\\PhysicalDrive%d",index);
		hDevice=CreateFile(HardDisk,0,FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL,OPEN_EXISTING,0,NULL);
	}	//while

	return Result;
}
*/
CString QueryHDDSmartCommand()
{
	HANDLE      		hSMARTIOCTL;
	DWORD       		cbBytesReturned;
	GETVERSIONOUTPARAMS VersionParams;
	SENDCMDINPARAMS 	scip;
	SENDCMDOUTPARAMS	OutCmd;
	BYTE				bDfpDriveMap;
	BYTE				i;
	BYTE				bSuccess;
	BYTE				bIDCmd;		// IDE or ATAPI IDENTIFY cmd

	CString Result;

	char Temp[512]="";

//#ifdef WINDOWS9X

	OSVERSIONINFO os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&os);
	if (os.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	{
		// Version Windows 95 OSR2, Windows 98
		if ((hSMARTIOCTL = CreateFile("\\\\.\\SMARTVSD", 0,0,0,
			CREATE_NEW, 0, 0)) == INVALID_HANDLE_VALUE)
		{
			sprintf(Temp,"Unable to open SMARTVSD, error code: 0x%lX\r\n", GetLastError());
		}
		else
		{
			sprintf(Temp,"SMARTVSD opened successfully\r\n");
		}
		
		Result+=Temp;
	}
	else
	{
		// Windows NT, Windows 2000
		if ((hSMARTIOCTL = CreateFile("\\\\.\\PhysicalDrive0",GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,
			OPEN_EXISTING,0,NULL)) == INVALID_HANDLE_VALUE)
		{
			sprintf(Temp, "Unable to open physical drive, error code: 0x%lX\r\n", GetLastError());
		}
		/*
		else
				{
					sprintf(Temp, "Physical drive opened successfully\r\n");
				}
		*/
		
		Result+=Temp;
	}
	
//#else
//#endif

	if (hSMARTIOCTL != INVALID_HANDLE_VALUE)
    {
		//
		// Get the version, etc of SMART IOCTL
		//
		memset((void*)&VersionParams, 0, sizeof(VersionParams));

		if (DeviceIoControl(hSMARTIOCTL, DFP_GET_VERSION,
			NULL, 
			0,
			&VersionParams,
			sizeof(VersionParams),
			&cbBytesReturned, NULL) )
    {
		sprintf(Temp,"DFP_GET_VERSION returned:\r\n");
		Result+=Temp;
		sprintf(Temp,"\tbVersion        = %d\r\n", VersionParams.bVersion);
		Result+=Temp;
		sprintf(Temp,"\tbRevision       = %d\r\n", VersionParams.bRevision);
		Result+=Temp;
		sprintf(Temp,"\tfCapabilities   = 0x%lx\r\n", VersionParams.fCapabilities);
		Result+=Temp;
		sprintf(Temp,"\tbReserved       = 0x%x\r\n", VersionParams.bReserved);
		Result+=Temp;
		sprintf(Temp,"\tbIDEDeviceMap   = 0x%x\r\n", VersionParams.bIDEDeviceMap);
		Result+=Temp;
		sprintf(Temp,"\tcbBytesReturned = %d\r\n\r\n", cbBytesReturned);
		Result+=Temp;
    }
    else
    {
        sprintf(Temp,"DFP_GET_VERSION failed.\r\n");
		Result+=Temp;
    }

	for (i = 0; i < MAX_IDE_DRIVES; i++)
	{
		//
		// If there is a IDE device at number "i" issue commands
		// to the device.
		//
		if (VersionParams.bIDEDeviceMap >> i & 1)
		{

			//
			// Try to enable SMART so we can tell if a drive supports it.
			// Ignore ATAPI devices.
			//

			if (!(VersionParams.bIDEDeviceMap >> i & 0x10))
			{

				memset(&scip, 0, sizeof(scip));
				memset(&OutCmd, 0, sizeof(OutCmd));

	    		if (DoEnableSMART(hSMARTIOCTL, 
					&scip, 
					&OutCmd, 
					i,
					&cbBytesReturned))
				{
					sprintf(Temp,"SMART Enabled on Drive: %d\r\n", i);
					Result+=Temp;
					//
					// Mark the drive as SMART enabled
					//
					bDfpDriveMap |= (1 << i);
				}
				else
	    		{
					sprintf(Temp,"SMART Enable Command Failed, Drive: %d.\r\n",i);
					Result+=Temp;
					sprintf(Temp," DriverStatus: bDriverError=0x%X, bIDEStatus=0x%X\r\n\r\n", 
	    					OutCmd.DriverStatus.bDriverError, 
	    					OutCmd.DriverStatus.bIDEError);
					Result+=Temp;
	    		}
				sprintf(Temp,"\tcbBytesReturned: %d\r\n\r\n", cbBytesReturned);
				Result+=Temp;
			}


			//
			// Now, get the ID sector for all IDE devices in the system.
			// If the device is ATAPI use the IDE_ATAPI_ID command,
			// otherwise use the IDE_ID_FUNCTION command.
			//
			bIDCmd = (VersionParams.bIDEDeviceMap >> i & 0x10) ? \
				IDE_ATAPI_ID : IDE_ID_FUNCTION;

			memset(&scip, 0, sizeof(scip));
			memset(IdOutCmd, 0, sizeof(IdOutCmd));

			if (DoIDENTIFY(hSMARTIOCTL, 
					&scip, 
					(PSENDCMDOUTPARAMS)&IdOutCmd, 
					bIDCmd,
					i,
					&cbBytesReturned))
	        {
				Result+=
					DisplayIdInfo((PIDSECTOR) ((PSENDCMDOUTPARAMS)IdOutCmd)->bBuffer,
						&scip, bIDCmd, bDfpDriveMap, i);
			}

	       	else
	       		{
	       			sprintf(Temp,"Identify Command Failed on Drive: %d\r\n", i);
					Result+=Temp;
					sprintf(Temp," DriverStatus: bDriverError=0x%X, bIDEStatus=0x%X\r\n\r\n", 
					((PSENDCMDOUTPARAMS)IdOutCmd)->DriverStatus.bDriverError, 
					((PSENDCMDOUTPARAMS)IdOutCmd)->DriverStatus.bIDEError);
					Result+=Temp;
	        	}
	    		sprintf(Temp,"\tcbBytesReturned: %d\r\n\r\n", cbBytesReturned);
				Result+=Temp;
	    }

	}

	//
	// Loop through all possible IDE drives and send commands to the ones that support SMART.
	//
	for (i = 0; i < MAX_IDE_DRIVES; i++)
	{
		if (bDfpDriveMap >> i & 1)
		{

			memset(AttrOutCmd, 0, sizeof(AttrOutCmd));
			memset(ThreshOutCmd, 0, sizeof(ThreshOutCmd));

			if ( !(bSuccess = DoReadAttributesCmd(hSMARTIOCTL, 
					&scip, 
					(PSENDCMDOUTPARAMS)&AttrOutCmd, 
					i)))
			{
				sprintf(Temp,"\r\nSMART Read Attr Command Failed on Drive: %d.\r\n", i);
				sprintf(Temp," DriverStatus: bDriverError=0x%X, bIDEStatus=0x%X\r\n\r\n", 
					((PSENDCMDOUTPARAMS)AttrOutCmd)->DriverStatus.bDriverError, 
					((PSENDCMDOUTPARAMS)AttrOutCmd)->DriverStatus.bIDEError);
			}	
	
			// ReadAttributes worked. Try ReadThresholds.
			else if (!(DoReadThresholdsCmd(hSMARTIOCTL, &scip, 
							(PSENDCMDOUTPARAMS)&ThreshOutCmd, i)))
			{
				sprintf(Temp,"\r\nSMART Read Thrsh Command Failed on Drive: %d.\r\n", i);
				Result+=Temp;
				sprintf(Temp," DriverStatus: bDriverError=0x%X, bIDEStatus=0x%X\r\n\r\n", 
					((PSENDCMDOUTPARAMS)ThreshOutCmd)->DriverStatus.bDriverError, 
					((PSENDCMDOUTPARAMS)ThreshOutCmd)->DriverStatus.bIDEError);
				Result+=Temp;
			}

			//
			// The following report will print if ReadAttributes works.
			// If ReadThresholds works, the report will also show values for
			// Threshold values.
			//
			if (bSuccess)
			{
				Result+=
					DoPrintData((char*) ((PSENDCMDOUTPARAMS) AttrOutCmd)->bBuffer, 
						(char*) ((PSENDCMDOUTPARAMS)ThreshOutCmd)->bBuffer, i);
			}
	
		}
	}

	//
    // Close SMART.
	//
        CloseHandle(hSMARTIOCTL);
    }

	return Result;
}

BOOL DoIDENTIFY(HANDLE hSMARTIOCTL, PSENDCMDINPARAMS pSCIP,
	PSENDCMDOUTPARAMS pSCOP, BYTE bIDCmd, BYTE bDriveNum, PDWORD lpcbBytesReturned)
{
	//
	// Set up data structures for IDENTIFY command.
	//

	pSCIP->cBufferSize = IDENTIFY_BUFFER_SIZE;

	pSCIP->irDriveRegs.bFeaturesReg = 0;
	pSCIP->irDriveRegs.bSectorCountReg = 1;
	pSCIP->irDriveRegs.bSectorNumberReg = 1;
	pSCIP->irDriveRegs.bCylLowReg = 0;
	pSCIP->irDriveRegs.bCylHighReg = 0;

	//
	// Compute the drive number.
	//
	pSCIP->irDriveRegs.bDriveHeadReg = 0xA0 | ((bDriveNum & 1) << 4); 

	//
	// The command can either be IDE identify or ATAPI identify.
	//
	pSCIP->irDriveRegs.bCommandReg = bIDCmd;
	pSCIP->bDriveNumber = bDriveNum;
	pSCIP->cBufferSize = IDENTIFY_BUFFER_SIZE;

    return (DeviceIoControl(hSMARTIOCTL, DFP_RECEIVE_DRIVE_DATA,
             	(LPVOID)pSCIP, sizeof(SENDCMDINPARAMS) - 1,
               	(LPVOID)pSCOP, sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1,
               	lpcbBytesReturned, NULL) );
}

/****************************************************************************
*
* DisplayIdInfo
*
* Display the contents of the ID buffer
*
****************************************************************************/
CString DisplayIdInfo(PIDSECTOR pids, PSENDCMDINPARAMS pSCIP, BYTE bIDCmd, BYTE bDfpDriveMap, BYTE bDriveNum)
{
	BYTE	szOutBuffer[41];
	CString Result;

	if (bIDCmd == IDE_ID_FUNCTION)
	{
		char Temp[512]="";
		sprintf(Temp,"Drive %d is an IDE Hard drive%s\r\n", bDriveNum, (
			bDfpDriveMap >> bDriveNum & 1) ? " that supports SMART" : "");

		Result+=Temp;
		sprintf(Temp,"\t#Cylinders: %d, #Heads: %d, #Sectors per Track: %d\r\n",
				pids->wNumCyls, 
				pids->wNumHeads, 
				pids->wSectorsPerTrack);
		Result+=Temp;

		PrintIDERegs(pSCIP);

	}
	else
	{
		char Temp[512]="";
		sprintf(Temp,"Drive %d is an ATAPI device.\r\n", bDriveNum); 
		Result+=Temp;
	}

	//
	// Change the WORD array to a BYTE
	// array
	//
	ChangeByteOrder(pids->sModelNumber, 
		sizeof pids->sModelNumber);

	memset(szOutBuffer,0, sizeof(szOutBuffer));
	strncpy((char*) szOutBuffer, pids->sModelNumber, sizeof(pids->sModelNumber));

	char Temp[512]="";
	sprintf(Temp, "\tModel number: %s\r\n", szOutBuffer);
	Result+=Temp;


	//
	// Change the WORD array to a BYTE
	// array
	//
	ChangeByteOrder(pids->sFirmwareRev, 
		sizeof pids->sFirmwareRev);

	memset(szOutBuffer,0, sizeof(szOutBuffer));
	strncpy((char*) szOutBuffer, pids->sFirmwareRev, sizeof(pids->sFirmwareRev));

	sprintf(Temp, "\tFirmware rev: %s\r\n", szOutBuffer);
	Result+=Temp;

	//
	// Change the WORD array to a BYTE
	// array
	//
	ChangeByteOrder(pids->sSerialNumber, 
		sizeof pids->sSerialNumber);

	memset(szOutBuffer,0, sizeof(szOutBuffer));
	strncpy((char*) szOutBuffer, pids->sSerialNumber, sizeof(pids->sSerialNumber));

	sprintf(Temp, "\tSerial number: %s\r\n",szOutBuffer);
	Result+=Temp;

	return Result;
}

/****************************************************************************
*
* DoEnableSMART
*
* FUNCTION: Send a SMART_ENABLE_SMART_OPERATIONS command to the drive
* bDriveNum = 0-3
*
****************************************************************************/
BOOL DoEnableSMART(HANDLE hSMARTIOCTL, PSENDCMDINPARAMS pSCIP, PSENDCMDOUTPARAMS pSCOP, BYTE bDriveNum, PDWORD lpcbBytesReturned)
{
	//
	// Set up data structures for Enable SMART Command.
	//
	pSCIP->cBufferSize = 0;

	pSCIP->irDriveRegs.bFeaturesReg = SMART_ENABLE_SMART_OPERATIONS;
	pSCIP->irDriveRegs.bSectorCountReg = 1;
	pSCIP->irDriveRegs.bSectorNumberReg = 1;
	pSCIP->irDriveRegs.bCylLowReg = SMART_CYL_LOW;
	pSCIP->irDriveRegs.bCylHighReg = SMART_CYL_HI;

	//
	// Compute the drive number.
	//
	pSCIP->irDriveRegs.bDriveHeadReg = 0xA0 | ((bDriveNum & 1) << 4); 
	pSCIP->irDriveRegs.bCommandReg = IDE_EXECUTE_SMART_FUNCTION;
	pSCIP->bDriveNumber = bDriveNum;

        return ( DeviceIoControl(hSMARTIOCTL, DFP_SEND_DRIVE_COMMAND,
                (LPVOID)pSCIP, sizeof(SENDCMDINPARAMS) - 1,
                (LPVOID)pSCOP, sizeof(SENDCMDOUTPARAMS) - 1,
                lpcbBytesReturned, NULL) );
}

/****************************************************************************
*
* DoReadAttributesCmd
*
* FUNCTION: Send a SMART_READ_ATTRIBUTE_VALUES command to the drive
* bDriveNum = 0-3
*
****************************************************************************/
BOOL DoReadAttributesCmd(HANDLE hSMARTIOCTL, PSENDCMDINPARAMS pSCIP, PSENDCMDOUTPARAMS pSCOP, BYTE bDriveNum)
{
DWORD	cbBytesReturned;

	//
	// Set up data structures for Read Attributes SMART Command.
	//

	pSCIP->cBufferSize = READ_ATTRIBUTE_BUFFER_SIZE;

	pSCIP->irDriveRegs.bFeaturesReg = SMART_READ_ATTRIBUTE_VALUES;
	pSCIP->irDriveRegs.bSectorCountReg = 1;
	pSCIP->irDriveRegs.bSectorNumberReg = 1;
	pSCIP->irDriveRegs.bCylLowReg = SMART_CYL_LOW;
	pSCIP->irDriveRegs.bCylHighReg = SMART_CYL_HI;

	//
	// Compute the drive number.
	//
	pSCIP->irDriveRegs.bDriveHeadReg = 0xA0 | ((bDriveNum & 1) << 4); 
	pSCIP->irDriveRegs.bCommandReg = IDE_EXECUTE_SMART_FUNCTION;
	pSCIP->bDriveNumber = bDriveNum;
        return ( DeviceIoControl(hSMARTIOCTL, DFP_RECEIVE_DRIVE_DATA,
                (LPVOID)pSCIP, sizeof(SENDCMDINPARAMS) - 1,
                (LPVOID)pSCOP, sizeof(SENDCMDOUTPARAMS) + READ_ATTRIBUTE_BUFFER_SIZE - 1,
                &cbBytesReturned, NULL) );
}

/****************************************************************************
*
* DoReadThresholdsCmd
*
* FUNCTION: Send a SMART_READ_ATTRIBUTE_THRESHOLDS command to the drive
* bDriveNum = 0-3
*
****************************************************************************/
BOOL DoReadThresholdsCmd(HANDLE hSMARTIOCTL, PSENDCMDINPARAMS pSCIP, PSENDCMDOUTPARAMS pSCOP, BYTE bDriveNum)
{
DWORD	cbBytesReturned;

	//
	// Set up data structures for Read Thresholds SMART Command.
	//

	pSCIP->cBufferSize = READ_THRESHOLD_BUFFER_SIZE;

	pSCIP->irDriveRegs.bFeaturesReg = SMART_READ_ATTRIBUTE_THRESHOLDS;
	pSCIP->irDriveRegs.bSectorCountReg = 1;
	pSCIP->irDriveRegs.bSectorNumberReg = 1;
	pSCIP->irDriveRegs.bCylLowReg = SMART_CYL_LOW;
	pSCIP->irDriveRegs.bCylHighReg = SMART_CYL_HI;

	//
	// Compute the drive number.
	//
	pSCIP->irDriveRegs.bDriveHeadReg = 0xA0 | ((bDriveNum & 1) << 4); 
	pSCIP->irDriveRegs.bCommandReg = IDE_EXECUTE_SMART_FUNCTION;
	pSCIP->bDriveNumber = bDriveNum;


        return ( DeviceIoControl(hSMARTIOCTL, DFP_RECEIVE_DRIVE_DATA,
                (LPVOID)pSCIP, sizeof(SENDCMDINPARAMS) - 1,
                (LPVOID)pSCOP, sizeof(SENDCMDOUTPARAMS) + READ_THRESHOLD_BUFFER_SIZE - 1,
                &cbBytesReturned, NULL) );
}

/****************************************************************************
*
* DoPrintData
*
* FUNCTION: Display the SMART Attributes and Thresholds
*
****************************************************************************/
CString DoPrintData(PCHAR pAttrBuffer, PCHAR pThrsBuffer, BYTE bDriveNum)
{
	int	i;
	PDRIVEATTRIBUTE	pDA;
	PATTRTHRESHOLD	pAT;
	BYTE Attr;
	CString Result;
	
	//
	// Print the drive number
	//
	char Temp[512]="";
	sprintf(Temp,"\r\nData for Drive Number %d\n", bDriveNum);
	Result+=Temp;
	//
	// Print the revisions of the data structures
	//
	sprintf(Temp, "Attribute Structure Revision          Threshold Structure Revision\r\n");
	Result+=Temp;

	sprintf(Temp,"             %d                                      %d\r\n\r\n", 
				(WORD)pAttrBuffer[0], 
				(WORD)pThrsBuffer[0]);

	Result+=Temp;
	//
	// Print the header and loop through the structures, printing
	// the structures when the attribute ID is known.
	//
	sprintf(Temp, "   -Attribute Name-      -Attribute Value-     -Threshold Value-\r\n");
	Result+=Temp;

	pDA = (PDRIVEATTRIBUTE)&pAttrBuffer[2];
	pAT = (PATTRTHRESHOLD)&pThrsBuffer[2];

	for (i = 0; i < NUM_ATTRIBUTE_STRUCTS; i++)
	{
		Attr = pDA->bAttrID;
		if (Attr)
		{
			if (Attr > MAX_KNOWN_ATTRIBUTES)
				Attr = MAX_KNOWN_ATTRIBUTES+1;
			sprintf(Temp,"%2X %-29s%d%20c%d\r\n", 
					pDA->bAttrID,
					pAttrNames[Attr], 
					pDA->bAttrValue, 
					' ',
					pAT->bWarrantyThreshold );
			Result+=Temp;
		}
		pDA++;
		pAT++;
	}

	return Result;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
VOID ChangeByteOrder(PCHAR szString, USHORT uscStrSize)
{
	USHORT	i;
	CHAR	temp;

	for (i = 0; i < uscStrSize; i+=2)
	{
		temp = szString[i];
		szString[i] = szString[i+1];
		szString[i+1] = temp;
	}
}

//---------------------------------------------------------------------
// Display contents of IDE hardware registers reported by SMART
//---------------------------------------------------------------------
CString PrintIDERegs(PSENDCMDINPARAMS pscip)
{
	char Temp[512]="";
	CString Result;

	sprintf(Temp,"\tIDE TASK FILE REGISTERS:\r\n");
	Result+=Temp;

	sprintf(Temp,"\t\tbFeaturesReg     = 0x%X\r\n", pscip->irDriveRegs.bFeaturesReg);
	Result+=Temp;
	sprintf(Temp,"\t\tbSectorCountReg  = 0x%X\r\n", pscip->irDriveRegs.bSectorCountReg);
	Result+=Temp;
	sprintf(Temp,"\t\tbSectorNumberReg = 0x%X\r\n", pscip->irDriveRegs.bSectorNumberReg);
	Result+=Temp;
	sprintf(Temp,"\t\tbCylLowReg       = 0x%X\r\n", pscip->irDriveRegs.bCylLowReg);
	Result+=Temp;
	sprintf(Temp,"\t\tbCylHighReg      = 0x%X\r\n", pscip->irDriveRegs.bCylHighReg);
	Result+=Temp;
	sprintf(Temp,"\t\tbDriveHeadReg    = 0x%X\r\n", pscip->irDriveRegs.bDriveHeadReg); 
	Result+=Temp;
	sprintf(Temp,"\t\tStatus           = 0x%X\r\n", pscip->irDriveRegs.bCommandReg);
	Result+=Temp;

	return Result;
}
