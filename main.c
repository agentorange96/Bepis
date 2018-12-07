/*	Bepis
	Copyright 2018 Peter A. Miller

	This file is part of Bepis.

	Bepis is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	Bepis is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	You should have received a copy of the GNU General Public License
	along with Bepis.  If not, see <http://www.gnu.org/licenses/>.
*/

//This is the main Bepis source file
//This program relies on InpOut32.DLL
//InpOut32.DLL can be found at:
//	http://www.highrez.co.uk/downloads/inpout32/default.htm


#pragma comment(lib, "User32.lib")
#include "windows.h"
#include "stdio.h"

//Interact with driver
typedef void	(__stdcall *lpOut32)(short, short);
typedef short	(__stdcall *lpInp32)(short);
typedef BOOL	(__stdcall *lpIsInpOutDriverOpen)(void);
typedef BOOL	(__stdcall *lpIsXP64Bit)(void);
lpOut32 gfpOut32;
lpInp32 gfpInp32;
lpIsInpOutDriverOpen gfpIsInpOutDriverOpen;
lpIsXP64Bit gfpIsXP64Bit;

//Generate a sound
void Sound(unsigned int freq)
{
	gfpOut32(0x43, 0xB6);
	gfpOut32(0x42, (freq & 0xFF));
	gfpOut32(0x42, (freq >> 9));
	Sleep(10);
	gfpOut32(0x61, gfpInp32(0x61) | 0x03);
}

//Stop the speaker
void StopSound()
{
	gfpOut32(0x61, (gfpInp32(0x61) & 0xFC));
}

//Main function
int main(int argc, char* argv[])
{
	//Load DLL
	HINSTANCE hInpOutDll = LoadLibrary ( "InpOut32.DLL" ) ;
	if ( hInpOutDll != NULL ){
		//Connect to driver
		gfpOut32 = (lpOut32)GetProcAddress(hInpOutDll, "Out32");
		gfpInp32 = (lpInp32)GetProcAddress(hInpOutDll, "Inp32");
		gfpIsInpOutDriverOpen = (lpIsInpOutDriverOpen)GetProcAddress(hInpOutDll, "IsInpOutDriverOpen");
		gfpIsXP64Bit = (lpIsXP64Bit)GetProcAddress(hInpOutDll, "IsXP64Bit");
		if (!gfpIsInpOutDriverOpen()){
			printf("Cannot load driver\n");
			exit(1);
		}
	}
	else{
		printf("Connot load DLL\nPlease install InpOut32 from:\nhttp://www.highrez.co.uk/downloads/inpout32/default.htm\n");
		FreeLibrary ( hInpOutDll );
		exit(1);
	}

	//Tuning Mode
	if(argc > 1 && argv[1][0] == 'T'){
		int freq = 2711;
		int run = 1;
		int inc;
		int mul;
		printf("Bepis Tuning Mode"
				"\nThis can be used with an instrument tuner"
				"\nto determine frequency numbers associated with each pitch."
				"\n\nControls:"
				"\nX\tIncrease Pitch by 1"
				"\nZ\tDecrease Pitch by 1"
				"\nD\tMultiply step by 10   (Hold while pressing X or Z)"
				"\nH\tMultiply step by 100  (Hold while pressing X or Z)"
				"\nK\tMultiply step by 1000 (Hold while pressing X or Z)"
				"\nP\tPrint the current frequency number"
				"\nQ\tQuit the program\n");
		while(run){
			mul = 1;	//This is the increment multiplier
			inc = 0;	//This is the increment of change
			//Multiply by 10
			if(GetKeyState('D') & 0x8000)
			{
				mul = 10;
			}
			//Multiply by 100
			else if(GetKeyState('H') & 0x8000)
			{
				mul = 100;
			}
			//Multiply by 1000
			if(GetKeyState('K') & 0x8000)
			{
				mul = mul*1000;
			}
			//Decrease frequency
			if(GetKeyState('Z') & 0x8000)
			{
				inc = 1;
			}
			//Increase frequency
			else if(GetKeyState('X') & 0x8000)
			{
				inc = -1;
			}
			//Quit
			if(GetKeyState('Q') & 0x8000)
			{
				StopSound();
				run = 0;
			}
			//Set sound
			if(run){
				freq += inc * mul;
				if(freq < 1){
					freq = 1;
				}
				if(freq > 65535){
					freq = 65535;
				}
				Sound(freq);
			}
			//Print the current frequency number
			if(GetKeyState('P') & 0x8000)
			{
				printf("Beep Value: %u\n",freq);
			}
			Sleep(100);
		}
		exit(0);
	}
	//Kill speaker if running
	else if(argc > 1 && argv[1][0] == 'K'){
		StopSound();
	}
	//Play a tone for a length of time
	else if(argc > 3){
		int freq;	//What is the tone number?
		int time;	//How long?
		sscanf(argv[1], "%u", &freq);
		sscanf(argv[2], "%u", &time);
		Sound(freq);
		Sleep(time);
		StopSound();
	}
	//Help menu
	else{
		printf("Welcome to Bepis!"
				"\nUsage:"
				"\nBepis FREQ TIME"
				"\n\tPlays the the frequency FREQ for TIME seconds"
				"\n\tFREQ is not in Hertz. It is a value ~2386360/Hertz"
				"\nBepis K"
				"\n\tKills whatever note is playing"
				"\n\tThe PC speaker must be explicitly turned off."
				"\n\tIf you killed Bepis with ^C, this will stop the pain."
				"\nBepis T"
				"\n\tEnters tuning mode"
				"\n\tThis is useful for determining what pitch goes with what FREQ value"
				"\nBepis FILE.MIDI"
				"\n\tHopefully this will play a single MIDI track out your PC speaker"
				"\n\tThis is a planned future addition to BEPIS"
				"\nBeepis H"
				"\n\tThis or anything else not listed will list this documentation");
	}
	//Unload library
	FreeLibrary ( hInpOutDll ) ;
	return 0;
}
