#ifndef _SYS_INFO_DEFS_H_
#define _SYS_INFO_DEFS_H_

#include "ShlObj.h"
#include "LMCONS.H"
#include "windows.h"
#include "shlwapi.h"
#include "math.h"
#include "WinIoCtl.h"
#include "CPU_Info.h"
//#include "DTWinVer.h"
#include "Winsock2.h"

#pragma comment(lib,"Ws2_32.lib")

//Собственно, запросы. все номера соответствуют названию запроса
#define Q_OS				0
#define Q_COMPUTER_NAME		1
#define Q_IP_ADDRESS		2
#define Q_USER_NAME			3
#define Q_IEXPLORER_VERSION	4
#define Q_RAM_TOTAL			5
#define Q_RAM_FREE			6 
#define Q_HARD_NUMBER		7
//#define Q_PARTITIONS		8 
//#define Q_HARD_SPACE		9
//#define Q_HARD_FREE			10
//#define Q_HARD_USED			11
#define Q_HDD_SMART_COMMAND 8//12
#define Q_CD_DRIVE			9//13
#define Q_RESOLUTION		10//14
#define Q_COLOR_DEPTH		11//15
#define Q_NUM_CPU			12//16
#define Q_CPU_SPEED			13//17
#define Q_CPU_ID			14//18
#define Q_CPU_VENDOR_ID		15//19
#define Q_OS_DIR			16//20
#define Q_SYSTEM_DIR		17//21
#define	Q_DESKTOP_DIR		18//22
#define Q_COOKIES_DIR		19//23
#define Q_STARTUP_DIR		20//24
#define Q_STARTMENU_DIR		21//25
#define Q_FAVORITES_DIR		22//26
#define Q_FONTS_DIR			23//27
#define Q_INET_CACHE_DIR	24//28
#define Q_LOCAL_APP_DIR		25//29
#define Q_MY_DOCUMENTS		26//30
#define Q_PROGRAM_FILES_DIR	27//31
#define Q_PROGRAM_GROUP_DIR 28//32
#define Q_RECENT_DIR		29//33
#define Q_SEND_TO_DIR		30//34
#define Q_TEMPLATES_DIR		31//35
#define Q_ADMIN_TOOLS_DIR	32//36
#define Q_HISTORY_DIR		33//37
#define Q_NET_HOOD_DIR		34//38
#define Q_PERSONAL_DIR		35//39
#define Q_PROFILE_DIR		36//40
#define Q_COM_STARTUP_DIR	37//41	//Common Directories
#define Q_COM_PROG_DIR		38//42	
#define Q_COM_DESKTOP_DIR	39//43	
#define Q_COM_FAVORITES_DIR	40//44
#define Q_COM_STARTMENU_DIR	41//45
#define Q_COM_ADMIN_DIR		42//46
#define Q_COM_APP_DATA_DIR	43//47
#define Q_COM_DOCUMENTS_DIR	44//48
#define Q_COM_TEMPLATES_DIR	45//49

CString DetermineIEVer();
CString QueryHardDisks();
CString QueryDriveSpace();
CString QueryCDDrive();
CString QueryTotalRAM();
CString QueryFreeRAM();
CString QueryUserName();
CString QueryComputerName();
CString QueryCPUInfo();
CString QueryCPUSpeed();
CString QueryCPUID();
CString QueryCPUVendorID();
CString QueryResolution();
CString QueryPixelDepth();
CString QueryOS();
CString QueryLocalIPAddress();
//CString QueryPartitions();
CString QueryDirectory(char* sFolder,int iFolder);
CString GetCommandError();
CString QueryHDDSmartCommand();


#define NUM_OF_QUERIES		50

// Объявление глобальной структуры, чтобы определить данные.

static PCHAR pAttrNames[]=
{
	"No Attribute Here       ", //0
	"Raw Read Error Rate     ", //1
	"Throughput Performance  ", //2
	"Spin Up Time            ", //3
	"Start/Stop Count        ", //4
	"Reallocated Sector Count", //5
	"Read Channel Margin     ", //6
	"Seek Error Rate         ", //7
	"Seek Time Performance   ", //8
	"Power On Hours Count    ", //9
	"Spin Retry Count        ", //10
	"Calibration Retry Count ", //11
	"Power Cycle Count       ", //12
	"(Unknown attribute)     "
};

#define MAX_KNOWN_ATTRIBUTES	12

static char Query[NUM_OF_QUERIES][50]=
{
	"Выберите из списка",						//1
	"Имя компьютера",
	"IP адрес",
	"Имя пользователя",
	"Версия Internet Explorer",					
	"Полная RAM",
	"Свободная оперативная память",
	"Количество жестких дисков",
	//"Partitions",
	//"Объем жеского диска",
	//"Свободный объем жесткого диска",							
	//"Занятый объём жесткого диска",
	"Расширение жесткого диска (S.M.A.R.T.)",
	"Привод DVD",
	"Разрешение монитора",
	"Глубина цвета",
	"Количество ядер",								
	"Частота процессера",
	"Идентификатор процессора",
	"Идентификатор производителя CPU",
	"Каталог ОС",
	"Системный каталог",								
	"Рабочий стол",
	"Папка с куками ",
	"Пуск",
	"Главное меню",
	"Избранное",								
	"Шрифты",
	"Интернет кэш",
	"Приложение",
	"Мои документы",
	"Program Files ",							
	"Program Group Folder",
	"Recent Folder",
	"Send To Folder",
	"Templates Folder",
	"Administrative Tools Folder",
	"History Folder",
	"NetHood Folder",
	"Personal Folder",
	"Profile Folder",
	"Common Startup Folder",						
	"Common Programs Folder",
	"Common Desktop Folder",
	"Common Favorites Folder",
	"Common Start Menu Folder",
	"Common Administrative Tools Folder",
	"Common Application Data Folder",
	"Common Documents Folder",
	"Common Templates Folder"						
};

#define MAX_OF_HARD_DISKS	24
static char HardDiskLetters[MAX_OF_HARD_DISKS][4]={
	"c:\\",	"d:\\",	"e:\\",	"f:\\",	"g:\\",	"h:\\",
	"i:\\",	"j:\\",	"k:\\",	"l:\\",	"m:\\",	"n:\\",
	"o:\\",	"p:\\",	"q:\\",	"r:\\",	"s:\\",	"t:\\",
	"u:\\",	"v:\\",	"w:\\",	"x:\\",	"y:\\",	"z:\\"
};
#endif //_SYS_INFO_DEFS_H_
