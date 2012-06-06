#include "stdafx.h"
#include "cpu_info.h"

CPUInfo::CPUInfo ()
{
	// Нужно убедиться, что процессор поддерживает CPUID
	if (DoesCPUSupportCPUID ()) {
		// Получаемым данные от процессора
		RetrieveCPUIdentity ();
		RetrieveCPUFeatures ();
		if (!RetrieveCPUClockSpeed ()) RetrieveClassicalCPUClockSpeed ();

		// пытаемся получить кэш информации
		if (!RetrieveCPUCacheDetails ()) RetrieveClassicalCPUCacheDetails ();

		// Дополнительные данные от процессора
		if (!RetrieveExtendedCPUIdentity ()) RetrieveClassicalCPUIdentity ();
		RetrieveExtendedCPUFeatures ();

		// Теперь попробуем получить серийный номер (если возможно)
		RetrieveProcessorSerialNumber ();
	}
}

CPUInfo::~CPUInfo ()
{
}


//         общие функции

char * CPUInfo::GetVendorString ()
{
	// возврат строки поставщика
	return ChipID.Vendor;
}

char * CPUInfo::GetVendorID ()
{
	// возврат ID поставщика
	switch (ChipManufacturer) {
		case Intel:
			return "Intel Corporation";
		case AMD:
			return "Advanced Micro Devices";
		case NSC:
			return "National Semiconductor";
		case Cyrix:
			return "Cyrix Corp., VIA Inc.";
		case NexGen:
			return "NexGen Inc., Advanced Micro Devices";
		case IDT:
			return "IDT\\Centaur, Via Inc.";
		case UMC:
			return "United Microelectronics Corp.";
		case Rise:
			return "Rise";
		case Transmeta:
			return "Transmeta";
		default:
			return "Unknown Manufacturer";
	}
}

char * CPUInfo::GetTypeID ()
{
	
	//Вернуться идентификатор типа процессора.
	char * szTypeID = new char [32];
	itoa (ChipID.Type, szTypeID, 10);
	return szTypeID;
}

char * CPUInfo::GetFamilyID ()
{
	// возврат семейства процессора
	char * szFamilyID = new char [32];
	itoa (ChipID.Family, szFamilyID, 10);
	return szFamilyID;
}

char * CPUInfo::GetModelID ()
{
	//возврат модели процессора
	char * szModelID = new char [32];
	itoa (ChipID.Model, szModelID, 10);
	return szModelID;
}

char * CPUInfo::GetSteppingCode ()
{
	//возврат кода активизации процессора
	char * szSteppingCode = new char [32];
	itoa (ChipID.Revision, szSteppingCode, 10);
	return szSteppingCode;
}

char * CPUInfo::GetExtendedProcessorName ()
{
	//возврат имени процессора
	return ChipID.ProcessorName;
}

char * CPUInfo::GetProcessorSerialNumber ()
{
	// возврат серийного номера процессора в 16ричной системе: xxxx-xxxx-xxxx-xxxx-xxxx-xxxx.
	return ChipID.SerialNumber;
}

int CPUInfo::GetLogicalProcessorsPerPhysical ()
{
	//возврат логического процессора на физическом
	return Features.ExtendedFeatures.LogicalProcessorsPerPhysical;
}

int CPUInfo::GetProcessorClockFrequency ()
{
	// вернуть частоту процессора
	if (Speed != NULL)
		return Speed->CPUSpeedInMHz;
	else 
		//ошибка
		return -1;
}

int CPUInfo::GetProcessorAPICID ()
{
	// Return the APIC ID.
	return Features.ExtendedFeatures.APIC_ID;
}

int CPUInfo::GetProcessorCacheXSize (DWORD dwCacheID)
{
	//возврат размера кэша
	switch (dwCacheID) {
		case L1CACHE_FEATURE:
			return Features.L1CacheSize;

		case L2CACHE_FEATURE:
			return Features.L2CacheSize;

		case L3CACHE_FEATURE:
			return Features.L3CacheSize;
	}

	//ошибка
	return -1;
}

bool CPUInfo::DoesCPUSupportFeature (DWORD dwFeature)
{
	bool bHasFeature = false;

	// проверка ММХ инструкций
	if (((dwFeature & MMX_FEATURE) != 0) && Features.HasMMX) bHasFeature = true;

	// проверка ММХ инструкций
	if (((dwFeature & MMX_PLUS_FEATURE) != 0) && Features.ExtendedFeatures.HasMMXPlus) bHasFeature = true;

	// проверка SSE инструкций
	if (((dwFeature & SSE_FEATURE) != 0) && Features.HasSSE) bHasFeature = true;

	// проверка SSE FP инструкций
	if (((dwFeature & SSE_FP_FEATURE) != 0) && Features.HasSSEFP) bHasFeature = true;

	// проверка SSE ММХ инструкций
	if (((dwFeature & SSE_MMX_FEATURE) != 0) && Features.ExtendedFeatures.HasSSEMMX) bHasFeature = true;

	// проверка SSE2 инструкций
	if (((dwFeature & SSE2_FEATURE) != 0) && Features.HasSSE2) bHasFeature = true;

	// проверка 3 Dnow! инструкций
	if (((dwFeature & AMD_3DNOW_FEATURE) != 0) && Features.ExtendedFeatures.Has3DNow) bHasFeature = true;

	// проверка 3Dnow+ инструкций
	if (((dwFeature & AMD_3DNOW_PLUS_FEATURE) != 0) && Features.ExtendedFeatures.Has3DNowPlus) bHasFeature = true;

	// проверка IA64 инструкций
	if (((dwFeature & IA64_FEATURE) != 0) && Features.HasIA64) bHasFeature = true;

	// Check for MP capable.
	
	if (((dwFeature & MP_CAPABLE) != 0) && Features.ExtendedFeatures.SupportsMP) bHasFeature = true;

	//проверка серийного номера процессора
	if (((dwFeature & SERIALNUMBER_FEATURE) != 0) && Features.HasSerial) bHasFeature = true;

	// проверка APIC в порцессоре
	if (((dwFeature & APIC_FEATURE) != 0) && Features.HasAPIC) bHasFeature = true;

	// Проверка CMOV инструкций
	if (((dwFeature & CMOV_FEATURE) != 0) && Features.HasCMOV) bHasFeature = true;

	// Проверка MTRR инструкций
	if (((dwFeature & MTRR_FEATURE) != 0) && Features.HasMTRR) bHasFeature = true;

	// проверка кэша 1 уровня
	if (((dwFeature & L1CACHE_FEATURE) != 0) && (Features.L1CacheSize != -1)) bHasFeature = true;

	// проверка кэша 2 уровня
	if (((dwFeature & L2CACHE_FEATURE) != 0) && (Features.L2CacheSize != -1)) bHasFeature = true;

	// проверка кэша 3 уровня
	if (((dwFeature & L3CACHE_FEATURE) != 0) && (Features.L3CacheSize != -1)) bHasFeature = true;

	// проверяем ACPI возможности
	if (((dwFeature & ACPI_FEATURE) != 0) && Features.HasACPI) bHasFeature = true;

	// проверка тепловой поддержки монитора
	if (((dwFeature & THERMALMONITOR_FEATURE) != 0) && Features.HasThermal) bHasFeature = true;

	// Проверка датчика температуры диода поддержки
	if (((dwFeature & TEMPSENSEDIODE_FEATURE) != 0) && Features.ExtendedFeatures.PowerManagement.HasTempSenseDiode) bHasFeature = true;

	//Проверка частоты поддержки ID.
	if (((dwFeature & FREQUENCYID_FEATURE) != 0) && Features.ExtendedFeatures.PowerManagement.HasFrequencyID) bHasFeature = true;

	// Проверка напряжения ID 
	if (((dwFeature & VOLTAGEID_FREQUENCY) != 0) && Features.ExtendedFeatures.PowerManagement.HasVoltageID) bHasFeature = true;

	return bHasFeature;
}

//			Частные функции


bool __cdecl CPUInfo::DoesCPUSupportCPUID ()
{
	int CPUIDPresent = 0;

#ifdef _WIN32 
	
	// Use SEH to determine CPUID presence
	//Использование SEH, чтобы определить наличие CPUID
    __try {
        _asm {
#ifdef CPUID_AWARE_COMPILER
 			push eax
			push ebx
			push ecx
			push edx
#endif
			; <<CPUID>> 
            mov eax, 0
			CPUID_INSTRUCTION

#ifdef CPUID_AWARE_COMPILER
			pop edx
			pop ecx
			pop ebx
			pop eax
#endif
        }
    }

	
	__except (1) {
        //Остановка! класс снова пытается использовать CPUID!
        CPUIDPresent = false;
		return false;
    }
#else
    	// "Правильный" способ, который не работает в некоторых версиях ОС Windows
	__try {
		_asm {
			pushfd                      ; save EFLAGS to stack.
			pop     eax                 ; store EFLAGS in eax.
			mov     edx, eax            ; save in ebx for testing later.
			xor     eax, 0200000h       ; switch bit 21.
			push    eax                 ; copy "changed" value to stack.
			popfd                       ; save "changed" eax to EFLAGS.
			pushfd
			pop     eax
			xor     eax, edx            ; See if bit changeable.
			jnz     short cpuid_present ; if so, mark 
			mov     eax, -1             ; CPUID not present - disable its usage
			jmp     no_features

	cpuid_present:
			mov		eax, 0				; CPUID capable CPU - enable its usage.
			
	no_features:
			mov     CPUIDPresent, eax	; Save the value in eax to a variable.
		}
	}
	
	// A generic catch-all just to be sure...
	__except (1) {
		// Stop the class from trying to use CPUID again!
        CPUIDPresent = false;
		return false;
    }
		
#endif

	
	//Вернуть истинно и указать есть поддержка или нет
	return (CPUIDPresent == 0) ? true : false;
}

bool __cdecl CPUInfo::RetrieveCPUFeatures ()
{
	int CPUFeatures = 0;
	int CPUAdvanced = 0;

	// обнаружение CPUID инфы
	__try {
		_asm {
#ifdef CPUID_AWARE_COMPILER
 			; we must push/pop the registers <<CPUID>> writes to, as the
			; optimiser doesn't know about <<CPUID>>, and so doesn't expect
			; these registers to change.
			push eax
			push ebx
			push ecx
			push edx
#endif
			; <<CPUID>> 
			
			mov eax,1
			CPUID_INSTRUCTION
			mov CPUFeatures, edx
			mov CPUAdvanced, ebx

#ifdef CPUID_AWARE_COMPILER
			pop edx
			pop ecx
			pop ebx
			pop eax
#endif
		}
	}

	__except (1) {
		return false;
	}

	// Получаем инфу от процессора
	Features.HasFPU =		((CPUFeatures & 0x00000001) != 0);		// FPU Present --> Bit 0
	Features.HasTSC =		((CPUFeatures & 0x00000010) != 0);		// TSC Present --> Bit 4
	Features.HasAPIC =		((CPUFeatures & 0x00000200) != 0);		// APIC Present --> Bit 9
	Features.HasMTRR =		((CPUFeatures & 0x00001000) != 0);		// MTRR Present --> Bit 12
	Features.HasCMOV =		((CPUFeatures & 0x00008000) != 0);		// CMOV Present --> Bit 15
	Features.HasSerial =	((CPUFeatures & 0x00040000) != 0);		// Serial Present --> Bit 18
	Features.HasACPI =		((CPUFeatures & 0x00400000) != 0);		// ACPI Capable --> Bit 22
    Features.HasMMX =		((CPUFeatures & 0x00800000) != 0);		// MMX Present --> Bit 23
	Features.HasSSE =		((CPUFeatures & 0x02000000) != 0);		// SSE Present --> Bit 25
	Features.HasSSE2 =		((CPUFeatures & 0x04000000) != 0);		// SSE2 Present --> Bit 26
	Features.HasThermal =	((CPUFeatures & 0x20000000) != 0);		// Thermal Monitor Present --> Bit 29
	Features.HasIA64 =		((CPUFeatures & 0x40000000) != 0);		// IA64 Present --> Bit 30

	// Получить расширенные возможности SSE, если доступно.
	if (Features.HasSSE) {
		
		// Attempt to __try some SSE FP instructions.
		__try {
			_asm {
				_emit 0x0f
	    		_emit 0x56
	    		_emit 0xc0	
			}

			Features.HasSSEFP = true;
	    }
	  
		__except (1) {
	    	// нельзя определить
			Features.HasSSEFP = false;
		}
	} else {
		//ошибка. нельзя уст-ть доп. опред. ссе
		Features.HasSSEFP = false;
	}

	// получаем спец. возм. интела
	if (ChipManufacturer == Intel) {
		Features.ExtendedFeatures.SupportsHyperthreading =	((CPUFeatures &	0x10000000) != 0);	// Intel specific: Hyperthreading --> Bit 28
		Features.ExtendedFeatures.LogicalProcessorsPerPhysical = (Features.ExtendedFeatures.SupportsHyperthreading) ? ((CPUAdvanced & 0x00FF0000) >> 16) : 1;
		
		if ((Features.ExtendedFeatures.SupportsHyperthreading) && (Features.HasAPIC)){
			// Retrieve APIC information if there is one present.
			Features.ExtendedFeatures.APIC_ID = ((CPUAdvanced & 0xFF000000) >> 24);
		}
	}

	return true;
}

bool __cdecl CPUInfo::RetrieveCPUIdentity ()
{
	int CPUVendor[3];
	int CPUSignature;

	//обнаруживаем CPUID информации.
	__try {
		_asm {
#ifdef CPUID_AWARE_COMPILER
 			; we must push/pop the registers <<CPUID>> writes to, as the
			; optimiser doesn't know about <<CPUID>>, and so doesn't expect
			; these registers to change.
			push eax
			push ebx
			push ecx
			push edx
#endif
			; <<CPUID>>
			; eax = 0 --> eax: maximum value of CPUID instruction.
			;			  ebx: part 1 of 3; CPU signature.
			;			  edx: part 2 of 3; CPU signature.
			;			  ecx: part 3 of 3; CPU signature.
			mov eax, 0
			CPUID_INSTRUCTION
			mov CPUVendor[0 * TYPE int], ebx
			mov CPUVendor[1 * TYPE int], edx
			mov CPUVendor[2 * TYPE int], ecx

			; <<CPUID>> 
			; eax = 1 --> eax: CPU ID - bits 31..16 - unused, bits 15..12 - type, bits 11..8 - family, bits 7..4 - model, bits 3..0 - mask revision
			;			  ebx: 31..24 - default APIC ID, 23..16 - logical processsor ID, 15..8 - CFLUSH chunk size , 7..0 - brand ID
			;			  edx: CPU feature flags
			mov eax,1
			CPUID_INSTRUCTION
			mov CPUSignature, eax

#ifdef CPUID_AWARE_COMPILER
			pop edx
			pop ecx
			pop ebx
			pop eax
#endif
		}
	}

	__except (1) {
		return false;
	}

	//Возврат информации
	memcpy (ChipID.Vendor, &(CPUVendor[0]), sizeof (int));
	memcpy (&(ChipID.Vendor[4]), &(CPUVendor[1]), sizeof (int));
	memcpy (&(ChipID.Vendor[8]), &(CPUVendor[2]), sizeof (int));
	ChipID.Vendor[12] = '\0';

	
	// Пытаемся получить производителя
	if (strcmp (ChipID.Vendor, "GenuineIntel") == 0)		ChipManufacturer = Intel;				// Intel Corp.
	else if (strcmp (ChipID.Vendor, "UMC UMC UMC ") == 0)	ChipManufacturer = UMC;					// United Microelectronics Corp.
	else if (strcmp (ChipID.Vendor, "AuthenticAMD") == 0)	ChipManufacturer = AMD;					// Advanced Micro Devices
	else if (strcmp (ChipID.Vendor, "AMD ISBETTER") == 0)	ChipManufacturer = AMD;					// Advanced Micro Devices (1994)
	else if (strcmp (ChipID.Vendor, "CyrixInstead") == 0)	ChipManufacturer = Cyrix;				// Cyrix Corp., VIA Inc.
	else if (strcmp (ChipID.Vendor, "NexGenDriven") == 0)	ChipManufacturer = NexGen;				// NexGen Inc. (now AMD)
	else if (strcmp (ChipID.Vendor, "CentaurHauls") == 0)	ChipManufacturer = IDT;					// IDT/Centaur (now VIA)
	else if (strcmp (ChipID.Vendor, "RiseRiseRise") == 0)	ChipManufacturer = Rise;				// Rise
	else if (strcmp (ChipID.Vendor, "GenuineTMx86") == 0)	ChipManufacturer = Transmeta;			// Transmeta
	else if (strcmp (ChipID.Vendor, "TransmetaCPU") == 0)	ChipManufacturer = Transmeta;			// Transmeta
	else if (strcmp (ChipID.Vendor, "Geode By NSC") == 0)	ChipManufacturer = NSC;					// National Semiconductor
	else													ChipManufacturer = UnknownManufacturer;	// Unknown manufacturer

	// получить семейство процессора
	ChipID.ExtendedFamily =		((CPUSignature & 0x0FF00000) >> 20);	// Bits 27..20 Used
	ChipID.ExtendedModel =		((CPUSignature & 0x000F0000) >> 16);	// Bits 19..16 Used
	ChipID.Type =				((CPUSignature & 0x0000F000) >> 12);	// Bits 15..12 Used
	ChipID.Family =				((CPUSignature & 0x00000F00) >> 8);		// Bits 11..8 Used
	ChipID.Model =				((CPUSignature & 0x000000F0) >> 4);		// Bits 7..4 Used
	ChipID.Revision =			((CPUSignature & 0x0000000F) >> 0);		// Bits 3..0 Used

	return true;
}

bool __cdecl CPUInfo::RetrieveCPUCacheDetails ()
{
	int L1Cache[4] = { 0, 0, 0, 0 };
	int L2Cache[4] = { 0, 0, 0, 0 };

	// проверяем работоспособность
	if (RetrieveCPUExtendedLevelSupport (0x80000005)) {
		__try {
			_asm {
#ifdef CPUID_AWARE_COMPILER
 				; we must push/pop the registers <<CPUID>> writes to, as the
				; optimiser doesn't know about <<CPUID>>, and so doesn't expect
				; these registers to change.
				push eax
				push ebx
				push ecx
				push edx
#endif
				; <<CPUID>>
				; eax = 0x80000005 --> eax: L1 cache information - Part 1 of 4.
				;					   ebx: L1 cache information - Part 2 of 4.
				;					   edx: L1 cache information - Part 3 of 4.
				;			 		   ecx: L1 cache information - Part 4 of 4.
				mov eax, 0x80000005
				CPUID_INSTRUCTION
				mov L1Cache[0 * TYPE int], eax
				mov L1Cache[1 * TYPE int], ebx
				mov L1Cache[2 * TYPE int], ecx
				mov L1Cache[3 * TYPE int], edx

#ifdef CPUID_AWARE_COMPILER
			pop edx
			pop ecx
			pop ebx
			pop eax
#endif
			}
		}

		__except (1) {
			return false;
		}

		// Сохранение кэш данных L1 размер (в КБ) из ECX: биты 31 .. 24, а также кэш данных размером от EDX: биты 31 .. 24.
		Features.L1CacheSize = ((L1Cache[2] & 0xFF000000) >> 24);
		Features.L1CacheSize += ((L1Cache[3] & 0xFF000000) >> 24);
	} else {
		// Хранить -1, чтобы указать кэш не может быть запрос.
		Features.L1CacheSize = -1;
	}

	// проверяем работоспособность
	if (RetrieveCPUExtendedLevelSupport (0x80000006)) {
		__try {
			_asm {
#ifdef CPUID_AWARE_COMPILER
 				; we must push/pop the registers <<CPUID>> writes to, as the
				; optimiser doesn't know about <<CPUID>>, and so doesn't expect
				; these registers to change.
				push eax
				push ebx
				push ecx
				push edx
#endif
				; <<CPUID>>
				; eax = 0x80000006 --> eax: L2 cache information - Part 1 of 4.
				;					   ebx: L2 cache information - Part 2 of 4.
				;					   edx: L2 cache information - Part 3 of 4.
				;			 		   ecx: L2 cache information - Part 4 of 4.
				mov eax, 0x80000006
				CPUID_INSTRUCTION
				mov L2Cache[0 * TYPE int], eax
				mov L2Cache[1 * TYPE int], ebx
				mov L2Cache[2 * TYPE int], ecx
				mov L2Cache[3 * TYPE int], edx

#ifdef CPUID_AWARE_COMPILER
				pop edx
				pop ecx
				pop ebx
				pop eax
#endif			
			}
		}

		__except (1) {
			return false;
		}

		// Сохранение кэша L2 размером (в KB) из ECX: биты 31 .. 16.
		Features.L2CacheSize = ((L2Cache[2] & 0xFFFF0000) >> 16);
	} else {
		//Хранить -1, чтобы указать кэш не может быть запрос.
		Features.L2CacheSize = -1;
	}
	
	// не можем проверить L3.
	Features.L3CacheSize = -1;

	//вернуть фолс если всё плохо
	return ((Features.L1CacheSize == -1) && (Features.L2CacheSize == -1)) ? false : true;
}

bool __cdecl CPUInfo::RetrieveClassicalCPUCacheDetails ()
{
	int TLBCode = -1, TLBData = -1, L1Code = -1, L1Data = -1, L1Trace = -1, L2Unified = -1, L3Unified = -1;
	int TLBCacheData[4] = { 0, 0, 0, 0 };
	int TLBPassCounter = 0;
	int TLBCacheUnit = 0;

	do {
		__try {
			_asm {
#ifdef CPUID_AWARE_COMPILER
 				; we must push/pop the registers <<CPUID>> writes to, as the
				; optimiser doesn't know about <<CPUID>>, and so doesn't expect
				; these registers to change.
				push eax
				push ebx
				push ecx
				push edx
#endif
				; <<CPUID>>
				; eax = 2 --> eax: TLB and cache information - Part 1 of 4.
				;			  ebx: TLB and cache information - Part 2 of 4.
				;			  ecx: TLB and cache information - Part 3 of 4.
				;			  edx: TLB and cache information - Part 4 of 4.
				mov eax, 2
				CPUID_INSTRUCTION
				mov TLBCacheData[0 * TYPE int], eax
				mov TLBCacheData[1 * TYPE int], ebx
				mov TLBCacheData[2 * TYPE int], ecx
				mov TLBCacheData[3 * TYPE int], edx

#ifdef CPUID_AWARE_COMPILER
				pop edx
				pop ecx
				pop ebx
				pop eax
#endif
			}
		}

		__except (1) {
			return false;
		}

		int bob = ((TLBCacheData[0] & 0x00FF0000) >> 16);

		// Процесс верн. TLB и кэш-памяти.
		for (int nCounter = 0; nCounter < TLBCACHE_INFO_UNITS; nCounter ++) {
			
			//ну жно определить с каим блоком мы работаем
			switch (nCounter) {
				
				// eax: bits 8..15 : bits 16..23 : bits 24..31
				case 0: TLBCacheUnit = ((TLBCacheData[0] & 0x0000FF00) >> 8); break;
				case 1: TLBCacheUnit = ((TLBCacheData[0] & 0x00FF0000) >> 16); break;
				case 2: TLBCacheUnit = ((TLBCacheData[0] & 0xFF000000) >> 24); break;

				// ebx: bits 0..7 : bits 8..15 : bits 16..23 : bits 24..31
				case 3: TLBCacheUnit = ((TLBCacheData[1] & 0x000000FF) >> 0); break;
				case 4: TLBCacheUnit = ((TLBCacheData[1] & 0x0000FF00) >> 8); break;
				case 5: TLBCacheUnit = ((TLBCacheData[1] & 0x00FF0000) >> 16); break;
				case 6: TLBCacheUnit = ((TLBCacheData[1] & 0xFF000000) >> 24); break;

				// ecx: bits 0..7 : bits 8..15 : bits 16..23 : bits 24..31
				case 7: TLBCacheUnit = ((TLBCacheData[2] & 0x000000FF) >> 0); break;
				case 8: TLBCacheUnit = ((TLBCacheData[2] & 0x0000FF00) >> 8); break;
				case 9: TLBCacheUnit = ((TLBCacheData[2] & 0x00FF0000) >> 16); break;
				case 10: TLBCacheUnit = ((TLBCacheData[2] & 0xFF000000) >> 24); break;

				// edx: bits 0..7 : bits 8..15 : bits 16..23 : bits 24..31
				case 11: TLBCacheUnit = ((TLBCacheData[3] & 0x000000FF) >> 0); break;
				case 12: TLBCacheUnit = ((TLBCacheData[3] & 0x0000FF00) >> 8); break;
				case 13: TLBCacheUnit = ((TLBCacheData[3] & 0x00FF0000) >> 16); break;
				case 14: TLBCacheUnit = ((TLBCacheData[3] & 0xFF000000) >> 24); break;

				// иначе фолс
				default: return false;
			}

			//Теперь обрабатывать полученные единицы, чтобы увидеть, что это значит
			switch (TLBCacheUnit) {
				case 0x00: break;
				case 0x01: STORE_TLBCACHE_INFO (TLBCode, 4); break;
				case 0x02: STORE_TLBCACHE_INFO (TLBCode, 4096); break;
				case 0x03: STORE_TLBCACHE_INFO (TLBData, 4); break;
				case 0x04: STORE_TLBCACHE_INFO (TLBData, 4096); break;
				case 0x06: STORE_TLBCACHE_INFO (L1Code, 8); break;
				case 0x08: STORE_TLBCACHE_INFO (L1Code, 16); break;
				case 0x0a: STORE_TLBCACHE_INFO (L1Data, 8); break;
				case 0x0c: STORE_TLBCACHE_INFO (L1Data, 16); break;
				case 0x10: STORE_TLBCACHE_INFO (L1Data, 16); break;			// <-- FIXME: IA-64 Only
				case 0x15: STORE_TLBCACHE_INFO (L1Code, 16); break;			// <-- FIXME: IA-64 Only
				case 0x1a: STORE_TLBCACHE_INFO (L2Unified, 96); break;		// <-- FIXME: IA-64 Only
				case 0x22: STORE_TLBCACHE_INFO (L3Unified, 512); break;
				case 0x23: STORE_TLBCACHE_INFO (L3Unified, 1024); break;
				case 0x25: STORE_TLBCACHE_INFO (L3Unified, 2048); break;
				case 0x29: STORE_TLBCACHE_INFO (L3Unified, 4096); break;
				case 0x39: STORE_TLBCACHE_INFO (L2Unified, 128); break;
				case 0x3c: STORE_TLBCACHE_INFO (L2Unified, 256); break;
				case 0x40: STORE_TLBCACHE_INFO (L2Unified, 0); break;		// <-- FIXME: No integrated L2 cache (P6 core) or L3 cache (P4 core).
				case 0x41: STORE_TLBCACHE_INFO (L2Unified, 128); break;
				case 0x42: STORE_TLBCACHE_INFO (L2Unified, 256); break;
				case 0x43: STORE_TLBCACHE_INFO (L2Unified, 512); break;
				case 0x44: STORE_TLBCACHE_INFO (L2Unified, 1024); break;
				case 0x45: STORE_TLBCACHE_INFO (L2Unified, 2048); break;
				case 0x50: STORE_TLBCACHE_INFO (TLBCode, 4096); break;
				case 0x51: STORE_TLBCACHE_INFO (TLBCode, 4096); break;
				case 0x52: STORE_TLBCACHE_INFO (TLBCode, 4096); break;
				case 0x5b: STORE_TLBCACHE_INFO (TLBData, 4096); break;
				case 0x5c: STORE_TLBCACHE_INFO (TLBData, 4096); break;
				case 0x5d: STORE_TLBCACHE_INFO (TLBData, 4096); break;
				case 0x66: STORE_TLBCACHE_INFO (L1Data, 8); break;
				case 0x67: STORE_TLBCACHE_INFO (L1Data, 16); break;
				case 0x68: STORE_TLBCACHE_INFO (L1Data, 32); break;
				case 0x70: STORE_TLBCACHE_INFO (L1Trace, 12); break;
				case 0x71: STORE_TLBCACHE_INFO (L1Trace, 16); break;
				case 0x72: STORE_TLBCACHE_INFO (L1Trace, 32); break;
				case 0x77: STORE_TLBCACHE_INFO (L1Code, 16); break;			// <-- FIXME: IA-64 Only
				case 0x79: STORE_TLBCACHE_INFO (L2Unified, 128); break;
				case 0x7a: STORE_TLBCACHE_INFO (L2Unified, 256); break;
				case 0x7b: STORE_TLBCACHE_INFO (L2Unified, 512); break;
				case 0x7c: STORE_TLBCACHE_INFO (L2Unified, 1024); break;
				case 0x7e: STORE_TLBCACHE_INFO (L2Unified, 256); break;
				case 0x81: STORE_TLBCACHE_INFO (L2Unified, 128); break;
				case 0x82: STORE_TLBCACHE_INFO (L2Unified, 256); break;
				case 0x83: STORE_TLBCACHE_INFO (L2Unified, 512); break;
				case 0x84: STORE_TLBCACHE_INFO (L2Unified, 1024); break;
				case 0x85: STORE_TLBCACHE_INFO (L2Unified, 2048); break;
				case 0x88: STORE_TLBCACHE_INFO (L3Unified, 2048); break;	// <-- FIXME: IA-64 Only
				case 0x89: STORE_TLBCACHE_INFO (L3Unified, 4096); break;	// <-- FIXME: IA-64 Only
				case 0x8a: STORE_TLBCACHE_INFO (L3Unified, 8192); break;	// <-- FIXME: IA-64 Only
				case 0x8d: STORE_TLBCACHE_INFO (L3Unified, 3096); break;	// <-- FIXME: IA-64 Only
				case 0x90: STORE_TLBCACHE_INFO (TLBCode, 262144); break;	// <-- FIXME: IA-64 Only
				case 0x96: STORE_TLBCACHE_INFO (TLBCode, 262144); break;	// <-- FIXME: IA-64 Only
				case 0x9b: STORE_TLBCACHE_INFO (TLBCode, 262144); break;	// <-- FIXME: IA-64 Only
				
				default: return false;
			}
		}

		// увеличение счетчика TLB
		TLBPassCounter ++;
	
	} while ((TLBCacheData[0] & 0x000000FF) > TLBPassCounter);

	// теперь есть max TLB, L1, L2, и L3 
	if ((L1Code == -1) && (L1Data == -1) && (L1Trace == -1)) Features.L1CacheSize = -1;
	else if ((L1Code == -1) && (L1Data == -1) && (L1Trace != -1)) Features.L1CacheSize = L1Trace;
	else if ((L1Code != -1) && (L1Data == -1)) Features.L1CacheSize = L1Code;
	else if ((L1Code == -1) && (L1Data != -1)) Features.L1CacheSize = L1Data;
	else if ((L1Code != -1) && (L1Data != -1)) Features.L1CacheSize = L1Code + L1Data;
	else Features.L1CacheSize = -1;

	// теперь есть max TLB, L1, L2, и L3 
	if (L2Unified == -1) Features.L2CacheSize = -1;
	else Features.L2CacheSize = L2Unified;

	// теперь есть max TLB, L1, L2, и L3 
	if (L3Unified == -1) Features.L3CacheSize = -1;
	else Features.L3CacheSize = L3Unified;

	return true;
}

bool __cdecl CPUInfo::RetrieveCPUClockSpeed ()
{
	// Прежде всего, убедитесь в том, RDTSC (0x0F, 0x31), инструкция не поддерживается.
	if (!Features.HasTSC) return false;

	// Получем тактовую частотут
	Speed = new CPUSpeed ();
	if (Speed == NULL) return false;

	return true;
}

bool __cdecl CPUInfo::RetrieveClassicalCPUClockSpeed ()
{
	LARGE_INTEGER liStart, liEnd, liCountsPerSecond;
	double dFrequency, dDifference;

	// Попытка запуска счетчика тактов.
	QueryPerformanceCounter (&liStart);

	__try {
		_asm {
			mov eax, 0x80000000
			mov ebx, CLASSICAL_CPU_FREQ_LOOP
			Timer_Loop: 
			bsf ecx,eax
			dec ebx
			jnz Timer_Loop
		}	
	}

	__except (1) {
		return false;
	}

	// Попытка запуска счетчика тактов.
	QueryPerformanceCounter (&liEnd);

	// Получить разницу ... Примечание: Это секунды ....
	QueryPerformanceFrequency (&liCountsPerSecond);
	dDifference = (((double) liEnd.QuadPart - (double) liStart.QuadPart) / (double) liCountsPerSecond.QuadPart);

	// высчитывае частоту.
	if (ChipID.Family == 3) {
		// 80386 .... за времени кажись 115 циклов!
		dFrequency = (((CLASSICAL_CPU_FREQ_LOOP * 115) / dDifference) / 1048576);
	} else if (ChipID.Family == 4) {
		// 80486 ....   47 циклов!
		dFrequency = (((CLASSICAL_CPU_FREQ_LOOP * 47) / dDifference) / 1048576);
	} else if (ChipID.Family == 5) {
		//время 43 цикла
		dFrequency = (((CLASSICAL_CPU_FREQ_LOOP * 43) / dDifference) / 1048576);
	}
	
	// сохраняем частоту.
	Features.CPUSpeed = (int) dFrequency;

	return true;
}

bool __cdecl CPUInfo::RetrieveCPUExtendedLevelSupport (int CPULevelToCheck)
{
	int MaxCPUExtendedLevel = 0;

	//  CPUID поддерживается различными поставщиками, начиная со следующих моделей процессоров:
	//
	//		Manufacturer & Chip Name			|		Family		 Model		Revision
	//
	//		AMD K6, K6-2						|		   5		   6			x		
	//		Cyrix GXm, Cyrix III "Joshua"		|		   5		   4			x
	//		IDT C6-2							|		   5		   8			x
	//		VIA Cyrix III						|		   6		   5			x
	//		Transmeta Crusoe					|		   5		   x			x
	//		Intel Pentium 4						|		   f		   x			x
	//

	// проверка на наличие
	if (ChipManufacturer == AMD) {
		if (ChipID.Family < 5) return false;
		if ((ChipID.Family == 5) && (ChipID.Model < 6)) return false;
	} else if (ChipManufacturer == Cyrix) {
		if (ChipID.Family < 5) return false;
		if ((ChipID.Family == 5) && (ChipID.Model < 4)) return false;
		if ((ChipID.Family == 6) && (ChipID.Model < 5)) return false;
	} else if (ChipManufacturer == IDT) {
		if (ChipID.Family < 5) return false;
		if ((ChipID.Family == 5) && (ChipID.Model < 8)) return false;
	} else if (ChipManufacturer == Transmeta) {
		if (ChipID.Family < 5) return false;
	} else if (ChipManufacturer == Intel) {
		if (ChipID.Family < 0xf) return false;
	}

	__try {
		_asm {
#ifdef CPUID_AWARE_COMPILER
 			; we must push/pop the registers <<CPUID>> writes to, as the
			; optimiser doesn't know about <<CPUID>>, and so doesn't expect
			; these registers to change.
			push eax
			push ebx
			push ecx
			push edx
#endif
			; <<CPUID>> 
			; eax = 0x80000000 --> eax: maximum supported extended level
			mov eax,0x80000000
			CPUID_INSTRUCTION
			mov MaxCPUExtendedLevel, eax

#ifdef CPUID_AWARE_COMPILER
			pop edx
			pop ecx
			pop ebx
			pop eax
#endif
		}
	}


	__except (1) {
		return false;
	}

	//проверить уровни
	int nLevelWanted = (CPULevelToCheck & 0x7FFFFFFF);
	int nLevelReturn = (MaxCPUExtendedLevel & 0x7FFFFFFF);

	// проверка при условии...
	if (nLevelWanted > nLevelReturn) return false;

	return true;
}

bool __cdecl CPUInfo::RetrieveExtendedCPUFeatures ()
{
	int CPUExtendedFeatures = 0;

	//не интел 
	if (ChipManufacturer == Intel) return false;

	if (!RetrieveCPUExtendedLevelSupport (0x80000001)) return false;

	__try {
		_asm {
#ifdef CPUID_AWARE_COMPILER
 			; we must push/pop the registers <<CPUID>> writes to, as the
			; optimiser doesn't know about <<CPUID>>, and so doesn't expect
			; these registers to change.
			push eax
			push ebx
			push ecx
			push edx
#endif
			; <<CPUID>> 
			; eax = 0x80000001 --> eax: CPU ID - bits 31..16 - unused, bits 15..12 - type, bits 11..8 - family, bits 7..4 - model, bits 3..0 - mask revision
			;					   ebx: 31..24 - default APIC ID, 23..16 - logical processsor ID, 15..8 - CFLUSH chunk size , 7..0 - brand ID
			;					   edx: CPU feature flags
			mov eax,0x80000001
			CPUID_INSTRUCTION
			mov CPUExtendedFeatures, edx

#ifdef CPUID_AWARE_COMPILER
			pop edx
			pop ecx
			pop ebx
			pop eax
#endif
		}
	}

	__except (1) {
		return false;
	}

	//получаем доп возможности проца
	Features.ExtendedFeatures.Has3DNow =		((CPUExtendedFeatures & 0x80000000) != 0);	// 3DNow Present --> Bit 31.
	Features.ExtendedFeatures.Has3DNowPlus =	((CPUExtendedFeatures & 0x40000000) != 0);	// 3DNow+ Present -- > Bit 30.
	Features.ExtendedFeatures.HasSSEMMX =		((CPUExtendedFeatures & 0x00400000) != 0);	// SSE MMX Present --> Bit 22.
	Features.ExtendedFeatures.SupportsMP =		((CPUExtendedFeatures & 0x00080000) != 0);	// MP Capable -- > Bit 19.
	

	if (ChipManufacturer == AMD) {
		Features.ExtendedFeatures.HasMMXPlus =	((CPUExtendedFeatures &	0x00400000) != 0);	// AMD specific: MMX-SSE --> Bit 22
	}

	
	if (ChipManufacturer == Cyrix) {
		Features.ExtendedFeatures.HasMMXPlus =	((CPUExtendedFeatures &	0x01000000) != 0);	// Cyrix specific: Extended MMX --> Bit 24
	}

	return true;
}

bool __cdecl CPUInfo::RetrieveProcessorSerialNumber ()
{
	int SerialNumber[3];

	// проверка на сер. номер
	if (!Features.HasSerial) return false;

	
	__try {
		_asm {
#ifdef CPUID_AWARE_COMPILER
 			; we must push/pop the registers <<CPUID>> writes to, as the
			; optimiser doesn't know about <<CPUID>>, and so doesn't expect
			; these registers to change.
			push eax
			push ebx
			push ecx
			push edx
#endif
			; <<CPUID>>
			; eax = 3 --> ebx: top 32 bits are the processor signature bits --> NB: Transmeta only ?!?
			;			  ecx: middle 32 bits are the processor signature bits
			;			  edx: bottom 32 bits are the processor signature bits
			mov eax, 3
			CPUID_INSTRUCTION
			mov SerialNumber[0 * TYPE int], ebx
			mov SerialNumber[1 * TYPE int], ecx
			mov SerialNumber[2 * TYPE int], edx

#ifdef CPUID_AWARE_COMPILER
			pop edx
			pop ecx
			pop ebx
			pop eax
#endif
		}
	}

	//
	__except (1) {
		return false;
	}

	// процес вернул инфу
	sprintf (ChipID.SerialNumber, "%.2x%.2x-%.2x%.2x-%.2x%.2x-%.2x%.2x-%.2x%.2x-%.2x%.2x",
			 ((SerialNumber[0] & 0xff000000) >> 24),
			 ((SerialNumber[0] & 0x00ff0000) >> 16),
			 ((SerialNumber[0] & 0x0000ff00) >> 8),
			 ((SerialNumber[0] & 0x000000ff) >> 0),
			 ((SerialNumber[1] & 0xff000000) >> 24),
			 ((SerialNumber[1] & 0x00ff0000) >> 16),
			 ((SerialNumber[1] & 0x0000ff00) >> 8),
			 ((SerialNumber[1] & 0x000000ff) >> 0),
			 ((SerialNumber[2] & 0xff000000) >> 24),
			 ((SerialNumber[2] & 0x00ff0000) >> 16),
			 ((SerialNumber[2] & 0x0000ff00) >> 8),
			 ((SerialNumber[2] & 0x000000ff) >> 0));

	return true;
}

bool __cdecl CPUInfo::RetrieveCPUPowerManagement ()
{	
	int CPUPowerManagement = 0;

	// проверяем работоспособность
	if (!RetrieveCPUExtendedLevelSupport (0x80000007)) {
		Features.ExtendedFeatures.PowerManagement.HasFrequencyID = false;
		Features.ExtendedFeatures.PowerManagement.HasVoltageID = false;
		Features.ExtendedFeatures.PowerManagement.HasTempSenseDiode = false;
		return false;
	}


	__try {
		_asm {
#ifdef CPUID_AWARE_COMPILER
 			; we must push/pop the registers <<CPUID>> writes to, as the
			; optimiser doesn't know about <<CPUID>>, and so doesn't expect
			; these registers to change.
			push eax
			push ebx
			push ecx
			push edx
#endif
			; <<CPUID>> 
			; eax = 0x80000007 --> edx: get processor power management
			mov eax,0x80000007
			CPUID_INSTRUCTION
			mov CPUPowerManagement, edx
			
#ifdef CPUID_AWARE_COMPILER
			pop edx
			pop ecx
			pop ebx
			pop eax
#endif
		}
	}


	__except (1) {
		return false;
	}

	// проверка работы
	Features.ExtendedFeatures.PowerManagement.HasTempSenseDiode =	((CPUPowerManagement & 0x00000001) != 0);
	Features.ExtendedFeatures.PowerManagement.HasFrequencyID =		((CPUPowerManagement & 0x00000002) != 0);
	Features.ExtendedFeatures.PowerManagement.HasVoltageID =		((CPUPowerManagement & 0x00000004) != 0);
	
	return true;
}

bool __cdecl CPUInfo::RetrieveExtendedCPUIdentity ()
{
	int ProcessorNameStartPos = 0;
	int CPUExtendedIdentity[12];

	if (!RetrieveCPUExtendedLevelSupport (0x80000002)) return false;
	if (!RetrieveCPUExtendedLevelSupport (0x80000003)) return false;
	if (!RetrieveCPUExtendedLevelSupport (0x80000004)) return false;

	__try {
		_asm {
#ifdef CPUID_AWARE_COMPILER
 			; we must push/pop the registers <<CPUID>> writes to, as the
			; optimiser doesn't know about <<CPUID>>, and so doesn't expect
			; these registers to change.
			push eax
			push ebx
			push ecx
			push edx
#endif
			; <<CPUID>> 
			; eax = 0x80000002 --> eax, ebx, ecx, edx: get processor name string (part 1)
			mov eax,0x80000002
			CPUID_INSTRUCTION
			mov CPUExtendedIdentity[0 * TYPE int], eax
			mov CPUExtendedIdentity[1 * TYPE int], ebx
			mov CPUExtendedIdentity[2 * TYPE int], ecx
			mov CPUExtendedIdentity[3 * TYPE int], edx

			; <<CPUID>> 
			; eax = 0x80000003 --> eax, ebx, ecx, edx: get processor name string (part 2)
			mov eax,0x80000003
			CPUID_INSTRUCTION
			mov CPUExtendedIdentity[4 * TYPE int], eax
			mov CPUExtendedIdentity[5 * TYPE int], ebx
			mov CPUExtendedIdentity[6 * TYPE int], ecx
			mov CPUExtendedIdentity[7 * TYPE int], edx

			; <<CPUID>> 
			; eax = 0x80000004 --> eax, ebx, ecx, edx: get processor name string (part 3)
			mov eax,0x80000004
			CPUID_INSTRUCTION
			mov CPUExtendedIdentity[8 * TYPE int], eax
			mov CPUExtendedIdentity[9 * TYPE int], ebx
			mov CPUExtendedIdentity[10 * TYPE int], ecx
			mov CPUExtendedIdentity[11 * TYPE int], edx

#ifdef CPUID_AWARE_COMPILER
			pop edx
			pop ecx
			pop ebx
			pop eax
#endif
		}
	}

	__except (1) {
		return false;
	}

	// возвращаемая инфа
	memcpy (ChipID.ProcessorName, &(CPUExtendedIdentity[0]), sizeof (int));
	memcpy (&(ChipID.ProcessorName[4]), &(CPUExtendedIdentity[1]), sizeof (int));
	memcpy (&(ChipID.ProcessorName[8]), &(CPUExtendedIdentity[2]), sizeof (int));
	memcpy (&(ChipID.ProcessorName[12]), &(CPUExtendedIdentity[3]), sizeof (int));
	memcpy (&(ChipID.ProcessorName[16]), &(CPUExtendedIdentity[4]), sizeof (int));
	memcpy (&(ChipID.ProcessorName[20]), &(CPUExtendedIdentity[5]), sizeof (int));
	memcpy (&(ChipID.ProcessorName[24]), &(CPUExtendedIdentity[6]), sizeof (int));
	memcpy (&(ChipID.ProcessorName[28]), &(CPUExtendedIdentity[7]), sizeof (int));
	memcpy (&(ChipID.ProcessorName[32]), &(CPUExtendedIdentity[8]), sizeof (int));
	memcpy (&(ChipID.ProcessorName[36]), &(CPUExtendedIdentity[9]), sizeof (int));
	memcpy (&(ChipID.ProcessorName[40]), &(CPUExtendedIdentity[10]), sizeof (int));
	memcpy (&(ChipID.ProcessorName[44]), &(CPUExtendedIdentity[11]), sizeof (int));
	ChipID.ProcessorName[48] = '\0';

	// т.к. мы всех производителей не знаем, то оставим белые места
	if (ChipManufacturer == Intel) {
		for (int nCounter = 0; nCounter < CHIPNAME_STRING_LENGTH; nCounter ++) {
			// или ноль и пробел
			if ((ChipID.ProcessorName[nCounter] != '\0') && (ChipID.ProcessorName[nCounter] != ' ')) {
				//имя найдено
				ProcessorNameStartPos = nCounter;
			
				break;
			}
		}

	
		if (ProcessorNameStartPos == 0) return true;

		
		memmove (ChipID.ProcessorName, &(ChipID.ProcessorName[ProcessorNameStartPos]), (CHIPNAME_STRING_LENGTH - ProcessorNameStartPos));
	}

	return true;
}

bool _cdecl CPUInfo::RetrieveClassicalCPUIdentity ()
{

	//выбираем производителя
	switch (ChipManufacturer) {
		case Intel:
			
			//Проверьте семьи / модель / версия для определения CPU ID.
			switch (ChipID.Family) {
				case 3:
					sprintf (ChipID.ProcessorName, "Newer i80386 family"); 
					break;
				case 4:
					switch (ChipID.Model) {
						case 0: STORE_CLASSICAL_NAME ("i80486DX-25/33"); break;
						case 1: STORE_CLASSICAL_NAME ("i80486DX-50"); break;
						case 2: STORE_CLASSICAL_NAME ("i80486SX"); break;
						case 3: STORE_CLASSICAL_NAME ("i80486DX2"); break;
						case 4: STORE_CLASSICAL_NAME ("i80486SL"); break;
						case 5: STORE_CLASSICAL_NAME ("i80486SX2"); break;
						case 7: STORE_CLASSICAL_NAME ("i80486DX2 WriteBack"); break;
						case 8: STORE_CLASSICAL_NAME ("i80486DX4"); break;
						case 9: STORE_CLASSICAL_NAME ("i80486DX4 WriteBack"); break;
						default: STORE_CLASSICAL_NAME ("Unknown 80486 family"); return false;
					}
					break;
				case 5:
					switch (ChipID.Model) {
						case 0: STORE_CLASSICAL_NAME ("P5 A-Step"); break;
						case 1: STORE_CLASSICAL_NAME ("P5"); break;
						case 2: STORE_CLASSICAL_NAME ("P54C"); break;
						case 3: STORE_CLASSICAL_NAME ("P24T OverDrive"); break;
						case 4: STORE_CLASSICAL_NAME ("P55C"); break;
						case 7: STORE_CLASSICAL_NAME ("P54C"); break;
						case 8: STORE_CLASSICAL_NAME ("P55C (0.25µm)"); break;
						default: STORE_CLASSICAL_NAME ("Unknown Pentium® family"); return false;
					}
					break;
				case 6:
					switch (ChipID.Model) {
						case 0: STORE_CLASSICAL_NAME ("P6 A-Step"); break;
						case 1: STORE_CLASSICAL_NAME ("P6"); break;
						case 3: STORE_CLASSICAL_NAME ("Pentium® II (0.28 µm)"); break;
						case 5: STORE_CLASSICAL_NAME ("Pentium® II (0.25 µm)"); break;
						case 6: STORE_CLASSICAL_NAME ("Pentium® II With On-Die L2 Cache"); break;
						case 7: STORE_CLASSICAL_NAME ("Pentium® III (0.25 µm)"); break;
						case 8: STORE_CLASSICAL_NAME ("Pentium® III (0.18 µm) With 256 KB On-Die L2 Cache "); break;
						case 0xa: STORE_CLASSICAL_NAME ("Pentium® III (0.18 µm) With 1 Or 2 MB On-Die L2 Cache "); break;
						case 0xb: STORE_CLASSICAL_NAME ("Pentium® III (0.13 µm) With 256 Or 512 KB On-Die L2 Cache "); break;
						default: STORE_CLASSICAL_NAME ("Unknown P6 family"); return false;
					}
					break;
				case 7:
					STORE_CLASSICAL_NAME ("Intel Merced (IA-64)");
					break;
				case 0xf:
				
					//Проверить расширенный бит семейства
					switch (ChipID.ExtendedFamily) {
						case 0:
							switch (ChipID.Model) {
								case 0: STORE_CLASSICAL_NAME ("Pentium® IV (0.18 µm)"); break;
								case 1: STORE_CLASSICAL_NAME ("Pentium® IV (0.18 µm)"); break;
								case 2: STORE_CLASSICAL_NAME ("Pentium® IV (0.13 µm)"); break;
								default: STORE_CLASSICAL_NAME ("Unknown Pentium 4 family"); return false;
							}
							break;
						case 1:
							STORE_CLASSICAL_NAME ("Intel McKinley (IA-64)");
							break;
					}
					break;
				default:
					STORE_CLASSICAL_NAME ("Unknown Intel family");
					return false;
			}
			break;

		case AMD:
			
			//Проверить семейство / модель / версия для определения CPU ID.
			switch (ChipID.Family) {
				case 4:
					switch (ChipID.Model) {
						case 3: STORE_CLASSICAL_NAME ("80486DX2"); break;
						case 7: STORE_CLASSICAL_NAME ("80486DX2 WriteBack"); break;
						case 8: STORE_CLASSICAL_NAME ("80486DX4"); break;
						case 9: STORE_CLASSICAL_NAME ("80486DX4 WriteBack"); break;
						case 0xe: STORE_CLASSICAL_NAME ("5x86"); break;
						case 0xf: STORE_CLASSICAL_NAME ("5x86WB"); break;
						default: STORE_CLASSICAL_NAME ("Unknown 80486 family"); return false;
					}
					break;
				case 5:
					switch (ChipID.Model) {
						case 0: STORE_CLASSICAL_NAME ("SSA5 (PR75, PR90, PR100)"); break;
						case 1: STORE_CLASSICAL_NAME ("5k86 (PR120, PR133)"); break;
						case 2: STORE_CLASSICAL_NAME ("5k86 (PR166)"); break;
						case 3: STORE_CLASSICAL_NAME ("5k86 (PR200)"); break;
						case 6: STORE_CLASSICAL_NAME ("K6 (0.30 µm)"); break;
						case 7: STORE_CLASSICAL_NAME ("K6 (0.25 µm)"); break;
						case 8: STORE_CLASSICAL_NAME ("K6-2"); break;
						case 9: STORE_CLASSICAL_NAME ("K6-III"); break;
						case 0xd: STORE_CLASSICAL_NAME ("K6-2+ or K6-III+ (0.18 µm)"); break;
						default: STORE_CLASSICAL_NAME ("Unknown 80586 family"); return false;
					}
					break;
				case 6:
					switch (ChipID.Model) {
						case 1: STORE_CLASSICAL_NAME ("Athlon™ (0.25 µm)"); break;
						case 2: STORE_CLASSICAL_NAME ("Athlon™ (0.18 µm)"); break;
						case 3: STORE_CLASSICAL_NAME ("Duron™ (SF core)"); break;
						case 4: STORE_CLASSICAL_NAME ("Athlon™ (Thunderbird core)"); break;
						case 6: STORE_CLASSICAL_NAME ("Athlon™ (Palomino core)"); break;
						case 7: STORE_CLASSICAL_NAME ("Duron™ (Morgan core)"); break;
						case 8: 
							if (Features.ExtendedFeatures.SupportsMP)
								STORE_CLASSICAL_NAME ("Athlon™ MP (Thoroughbred core)"); 
							else STORE_CLASSICAL_NAME ("Athlon™ XP (Thoroughbred core)");
							break;
						default: STORE_CLASSICAL_NAME ("Unknown K7 family"); return false;
					}
					break;
				default:
					STORE_CLASSICAL_NAME ("Unknown AMD family");
					return false;
			}
			break;

		case Transmeta:
			switch (ChipID.Family) {	
				case 5:
					switch (ChipID.Model) {
						case 4: STORE_CLASSICAL_NAME ("Crusoe TM3x00 and TM5x00"); break;
						default: STORE_CLASSICAL_NAME ("Unknown Crusoe family"); return false;
					}
					break;
				default:
					STORE_CLASSICAL_NAME ("Unknown Transmeta family");
					return false;
			}
			break;

		case Rise:
			switch (ChipID.Family) {	
				case 5:
					switch (ChipID.Model) {
						case 0: STORE_CLASSICAL_NAME ("mP6 (0.25 µm)"); break;
						case 2: STORE_CLASSICAL_NAME ("mP6 (0.18 µm)"); break;
						default: STORE_CLASSICAL_NAME ("Unknown Rise family"); return false;
					}
					break;
				default:
					STORE_CLASSICAL_NAME ("Unknown Rise family");
					return false;
			}
			break;

		case UMC:
			switch (ChipID.Family) {	
				case 4:
					switch (ChipID.Model) {
						case 1: STORE_CLASSICAL_NAME ("U5D"); break;
						case 2: STORE_CLASSICAL_NAME ("U5S"); break;
						default: STORE_CLASSICAL_NAME ("Unknown UMC family"); return false;
					}
					break;
				default:
					STORE_CLASSICAL_NAME ("Unknown UMC family");
					return false;
			}
			break;

		case IDT:
			switch (ChipID.Family) {	
				case 5:
					switch (ChipID.Model) {
						case 4: STORE_CLASSICAL_NAME ("C6"); break;
						case 8: STORE_CLASSICAL_NAME ("C2"); break;
						case 9: STORE_CLASSICAL_NAME ("C3"); break;
						default: STORE_CLASSICAL_NAME ("Unknown IDT\\Centaur family"); return false;
					}
					break;
				case 6:
					switch (ChipID.Model) {
						case 6: STORE_CLASSICAL_NAME ("VIA Cyrix III - Samuel"); break;
						default: STORE_CLASSICAL_NAME ("Unknown IDT\\Centaur family"); return false;
					}
					break;
				default:
					STORE_CLASSICAL_NAME ("Unknown IDT\\Centaur family");
					return false;
			}
			break;

		case Cyrix:
			switch (ChipID.Family) {	
				case 4:
					switch (ChipID.Model) {
						case 4: STORE_CLASSICAL_NAME ("MediaGX GX, GXm"); break;
						case 9: STORE_CLASSICAL_NAME ("5x86"); break;
						default: STORE_CLASSICAL_NAME ("Unknown Cx5x86 family"); return false;
					}
					break;
				case 5:
					switch (ChipID.Model) {
						case 2: STORE_CLASSICAL_NAME ("Cx6x86"); break;
						case 4: STORE_CLASSICAL_NAME ("MediaGX GXm"); break;
						default: STORE_CLASSICAL_NAME ("Unknown Cx6x86 family"); return false;
					}
					break;
				case 6:
					switch (ChipID.Model) {
						case 0: STORE_CLASSICAL_NAME ("6x86MX"); break;
						case 5: STORE_CLASSICAL_NAME ("Cyrix M2 Core"); break;
						case 6: STORE_CLASSICAL_NAME ("WinChip C5A Core"); break;
						case 7: STORE_CLASSICAL_NAME ("WinChip C5B\\C5C Core"); break;
						case 8: STORE_CLASSICAL_NAME ("WinChip C5C-T Core"); break;
						default: STORE_CLASSICAL_NAME ("Unknown 6x86MX\\Cyrix III family"); return false;
					}
					break;
				default:
					STORE_CLASSICAL_NAME ("Unknown Cyrix family");
					return false;
			}
			break;

		case NexGen:
			switch (ChipID.Family) {	
				case 5:
					switch (ChipID.Model) {
						case 0: STORE_CLASSICAL_NAME ("Nx586 or Nx586FPU"); break;
						default: STORE_CLASSICAL_NAME ("Unknown NexGen family"); return false;
					}
					break;
				default:
					STORE_CLASSICAL_NAME ("Unknown NexGen family");
					return false;
			}
			break;

		case NSC:
			STORE_CLASSICAL_NAME ("Cx486SLC \\ DLC \\ Cx486S A-Step");
			break;

		default:
			//Невозможно определить процессор
			STORE_CLASSICAL_NAME ("Unknown family");
			return false;
	}

	return true;
}



CPUSpeed::CPUSpeed ()
{
	unsigned int uiRepetitions = 1;
	unsigned int uiMSecPerRepetition = 50;
	__int64	i64Total = 0, i64Overhead = 0;

	for (unsigned int nCounter = 0; nCounter < uiRepetitions; nCounter ++) {
		i64Total += GetCyclesDifference (CPUSpeed::Delay, uiMSecPerRepetition);
		i64Overhead += GetCyclesDifference (CPUSpeed::DelayOverhead, uiMSecPerRepetition);
	}

	
	//высчитываем частоту
	i64Total -= i64Overhead;
	i64Total /= uiRepetitions;
	i64Total /= uiMSecPerRepetition;
	i64Total /= 1000;

	
	//сохраняем скорость проца
	CPUSpeedInMHz = (int) i64Total;
}

CPUSpeed::~CPUSpeed ()
{
}

__int64	__cdecl CPUSpeed::GetCyclesDifference (DELAY_FUNC DelayFunction, unsigned int uiParameter)
{
	unsigned int edx1, eax1;
	unsigned int edx2, eax2;
		

	//Рассчитать частоту процессора 
	__try {
		_asm {
			push uiParameter		; push parameter param
			mov ebx, DelayFunction	; store func in ebx

			RDTSC_INSTRUCTION

			mov esi, eax			; esi = eax
			mov edi, edx			; edi = edx

			call ebx				; call the delay functions

			RDTSC_INSTRUCTION

			pop ebx

			mov edx2, edx			; edx2 = edx
			mov eax2, eax			; eax2 = eax

			mov edx1, edi			; edx2 = edi
			mov eax1, esi			; eax2 = esi
		}
	}

	// 
	__except (1) {
		return -1;
	}

	return (CPUSPEED_I32TO64 (edx2, eax2) - CPUSPEED_I32TO64 (edx1, eax1));
}

void CPUSpeed::Delay (unsigned int uiMS)
{
	LARGE_INTEGER Frequency, StartCounter, EndCounter;
	__int64 x;


	//нужно получить значиние счетчик
	if (!QueryPerformanceFrequency (&Frequency)) return;
	x = Frequency.QuadPart / 1000 * uiMS;


	//получить начальное положение счетчика
	QueryPerformanceCounter (&StartCounter);

	do {
	
		//Получаем конечное положение счетчика.
		QueryPerformanceCounter (&EndCounter);
	} while (EndCounter.QuadPart - StartCounter.QuadPart < x);
}

void CPUSpeed::DelayOverhead (unsigned int uiMS)
{
	LARGE_INTEGER Frequency, StartCounter, EndCounter;
	__int64 x;


	//получить начальное положение счетчика
	if (!QueryPerformanceFrequency (&Frequency)) return;
	x = Frequency.QuadPart / 1000 * uiMS;


	QueryPerformanceCounter (&StartCounter);
	
	do {
		
		//Получаем конечное положение счетчика.
		QueryPerformanceCounter (&EndCounter);
	} while (EndCounter.QuadPart - StartCounter.QuadPart == x);
}
