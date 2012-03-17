//#include "hexioctrl.h"
#include "NewHeaders.h"
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include "NTPORT.H"


int main(int argc, char* argv[])
{	
	
	int i, bus, devc, func;
	unsigned char rev_id, cl_st, cl_sr, cl_ml;
	unsigned long int addr, res;
	unsigned short int ven_id, dev_id, subv_id;	
	for (bus = 0; bus < 256; bus++)
	{
		for (devc = 0; devc < 32; devc++)
		{
			for (func = 0; func < 8; func++)
			{
				addr = 0x80000000 + (bus << 16) + (devc << 11) + (func << 8);
				//res = asm_proc(addr);
				Outpd(0xCF8,addr);
				res=Inpd(0xCFC);

				ven_id = (unsigned short int) res; // ост 2 байта младш
				if (ven_id == 0xFFFF) continue; // 
				dev_id = (unsigned short int) (res >> 16);  // 2 старш байта вправо
				
				addr += 32*2;
				Outpd(0xCF8,addr);
				res=Inpd(0xCFC);
				cl_st = (unsigned char) (res >> 24);
				cl_sr = (unsigned char) (res >> 16);
				cl_ml = (unsigned char) (res >> 8);
				rev_id = (unsigned char) res;
				
				addr += 32*4;
				Outpd(0xCF8,addr);
				res=Inpd(0xCFC);
				subv_id = (unsigned short int) res;
					printf("\nPCI %d:%d:%d\n",bus,devc,func);
					printf("Vendor ID: %X ",ven_id);
					for(i = 0; i < sizeof(PciVenTable)/sizeof(PCI_VENTABLE); i++)
					{	
						if(PciVenTable[i].VenId == ven_id) 
						{	
							printf("%s",PciVenTable[i].VenFull);
						}				
					}
					printf("\nSubsystem Vendor ID: %X\n",subv_id);
					printf("Device ID: %X ",dev_id);
					for(i = 0; i < sizeof(PciDevTable)/sizeof(PCI_DEVTABLE); i++)
					{	
						if(PciDevTable[i].DevId == dev_id)
						{
							printf("%s",PciDevTable[i].ChipDesc);
						}
					}
					printf("\nRevision ID: %X\n",rev_id);
					printf("Class ID: %X:%X:%X ",cl_st,cl_sr,cl_ml);
					for(int i = 0; i < sizeof(PciClassCodeTable)/sizeof(PCI_CLASSCODETABLE); i++)
					{	
						if(PciClassCodeTable[i].BaseClass == cl_st)
						{
							printf("%s",PciClassCodeTable[i].BaseDesc);
							break;
						}
					}
					printf("\n----------------------------");
				//}

			}
		}
	}
	getch();
	return 0;
}

