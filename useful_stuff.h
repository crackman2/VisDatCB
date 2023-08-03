#pragma once

#include <fstream>

namespace useful {
	int schift = 0;
	bool dbg = false;

	void toClipboard(const std::string &s) {
		OpenClipboard(0);
		EmptyClipboard();
		HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, s.size());
		if (!hg) {
			CloseClipboard();
			return;
		}
		memcpy(GlobalLock(hg), s.c_str(), s.size());
		GlobalUnlock(hg);
		SetClipboardData(CF_TEXT, hg);
		CloseClipboard();
		GlobalFree(hg);
	}

	unsigned long rgb16_to_rgb32(unsigned short a) {
		bool dbg = false;
		/* 1. Extract the red, green and blue values */
		if (dbg)
			std::cout << "Value read: 0x" << std::hex << a << std::endl;
		/* from rrrr rggg gggb bbbb */
		unsigned long r = (a & 0xF800) >> 11;
		unsigned long g = (a & 0x07E0) >> 5;
		unsigned long b = (a & 0x001F);

		/* 2. Convert them to 0-255 range:
		There is more than one way. You can just shift them left:
		to 00000000 rrrrr000 gggggg00 bbbbb000
		r <<= 3;
		g <<= 2;
		b <<= 3;
		But that means your image will be slightly dark and
		off-colour as white 0xFFFF will convert to F8,FC,F8
		So instead you can scale by multiply and divide: */

		r = r * 255 / 31;
		g = g * 255 / 63;
		b = b * 255 / 31;


		/* This ensures 31/31 converts to 255/255 */

		/* 3. Construct your 32-bit format (this is 0RGB): */
		//return (r << 16) | (g << 8) | b;
		DWORD res = ((r << (24 + schift)) | (g << (16 + schift)) | b << (8 + schift) | (0xFF) >> (0 + schift));
		if (dbg) {

			std::cout << "R: 0x" << std::hex << r << " G: 0x" << g << " B: 0x" << b << std::endl;
			std::cout << "Result: 0x" << std::hex << res << std::endl;
			system("pause");
		}
		return res;

		// Or for BGR0:
		//return (r << 24) | (g << 16) | (b << 0);

	}


	unsigned long rgb8_to_rgb32(BYTE a) {

		/* 1. Extract the red, green and blue values */
		if (dbg)
			std::cout << "Value read: 0x" << std::hex << a << std::endl;
		/* from rrrr rggg gggb bbbb */
		//unsigned long r = (a >> 5) * 255 / 7;
		//unsigned long g = ((a >> 2) & 0x07) * 255 / 7;
		//unsigned long b = (a & 0x03) * 255 / 3;

		unsigned long r = (a >> 5) * 32;
		unsigned long g = ((a & 28) >> 2) * 32;
		unsigned long b = (a & 0x03) * 64;

		/* 2. Convert them to 0-255 range:
		There is more than one way. You can just shift them left:
		to 00000000 rrrrr000 gggggg00 bbbbb000
		r <<= 3;
		g <<= 2;
		b <<= 3;
		But that means your image will be slightly dark and
		off-colour as white 0xFFFF will convert to F8,FC,F8
		So instead you can scale by multiply and divide: */

		//r = r * 255 / 31;
		//g = g * 255 / 63;
		//b = b * 255 / 31;


		/* This ensures 31/31 converts to 255/255 */

		/* 3. Construct your 32-bit format (this is 0RGB): */
		//return (r << 16) | (g << 8) | b;
		DWORD res = ((r << (24 + schift)) | (g << (16 + schift)) | b << (8 + schift) | (0xFF) >> (0 + schift));
		if (dbg) {

			std::cout << "R: 0x" << std::hex << r << " G: 0x" << g << " B: 0x" << b << std::endl;
			std::cout << "Result: 0x" << std::hex << res << std::endl;
			system("pause");
		}
		return res;

		// Or for BGR0:
		//return (r << 24) | (g << 16) | (b << 0);

	}

	std::string ExePath() {
		char buffer[MAX_PATH];
		GetModuleFileName(NULL, buffer, MAX_PATH);
		std::string::size_type pos = std::string(buffer).find_last_of("\\/");
		return std::string(buffer).substr(0, pos);
	}


	/* stolen from cocomelonc https://cocomelonc.github.io/pentest/2021/09/29/findmyprocess.html*/
	int findMyProc(const char *procname) {

      HANDLE hSnapshot;
      PROCESSENTRY32 pe;
      int pid = 0;
      BOOL hResult;

      // snapshot of all processes in the system
      hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
      if (INVALID_HANDLE_VALUE == hSnapshot) return 0;

      // initializing size: needed for using Process32First
      pe.dwSize = sizeof(PROCESSENTRY32);

      // info about first process encountered in a system snapshot
      hResult = Process32First(hSnapshot, &pe);

      // retrieve information about the processes
      // and exit if unsuccessful
      while (hResult) {
        // if we find the process: return process ID
        if (strcmp(procname, pe.szExeFile) == 0) {
          pid = pe.th32ProcessID;
          break;
        }
        hResult = Process32Next(hSnapshot, &pe);
      }

      // closes an open handle (CreateToolhelp32Snapshot)
      CloseHandle(hSnapshot);
      return pid;
    }

    void appendCommentsToIni(){
        /* SimpleIni.h doesn't really handle this well, so let's cheat */
        std::ofstream outFile(std::string(useful::ExePath() + std::string("/VisDat_Config.ini")).c_str(), std::ios::app);

        if (outFile.is_open()) {
            // Append new lines to the file
            outFile << "\n";
            outFile << "; - VisDatCB only works with 32bit application and works best for \n";
            outFile << ";   software rendered games. You can directly draw on textures loaded in memory \n\n";
            outFile << "; - 'title' can be either the window Title or the executable name like 'hl.exe' \n";
            outFile << "; - 'winx' and 'winy' are the internal resolution of the window. the actual window size \n";
            outFile << ";             will be width = winx * scale  and height = winy * scale. you can also just \n";
            outFile << ";             resize the window by dragging but the cursor will not be correctly positioned anymore \n";
            outFile << "; - 'bitdeph' determines the format how the information is displayed. 8, 16 and 32 are valid \n";
            outFile << ";             8 bit will show weird colors because it requires a palette to be known. still useful \n";
            outFile << "; - 'start'   determines the starting point from where to go. 0x400000 is default. enter address as hex number\n";
            outFile << "; - 'bFullPageReading' reads entire screen at once as opposed to pixel by pixel.\n";
            outFile << ";             massive performance boost, but screen freezes when access to memory is denied.\n";
            outFile << ";             leaving it on 0 is more reliable\n\n";
            outFile << "; Github: https://github.com/crackman2/VisDatCB \n";


            // Close the file
            outFile.close();

            std::cout << "Lines added successfully." << std::endl;
        } else {
            std::cerr << "Failed to open file for appending." << std::endl;
        }
    }

    void firstTimeINISetup(CSimpleIniA * config){
        std::cout << "VisDat_Config.ini not found! Creating file" << std::endl;
        config->SetValue("settings", "title", "CHANGE_ME_TO_WindowTitle_or_EXE_name");
        config->SetValue("settings", "winx", "256");
        config->SetValue("settings", "winy", "256");
        config->SetValue("settings", "scale", "3");
        config->SetValue("settings", "bitdepth", "32");
        config->SetValue("settings", "start", "400000");
        config->SetValue("settings", "textsize", "14");
        config->SetValue("settings", "bFullPageReading", "0");
        if (config->SaveFile(std::string(useful::ExePath() + std::string("/VisDat_Config.ini")).c_str()) < 0) {
            std::cout << "Could not create config" << std::endl;
        } else {
            std::cout << "Config saved! Restart VisDat" << std::endl;
        }
        Sleep(1000);
        appendCommentsToIni();
        system("pause");
        exit(0);
    }
}
