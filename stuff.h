#pragma once
#include <iostream>
#include <chrono>
#include <thread>
#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <vector>
#include <string>
#include <fstream>
using std::cout;
using std::thread;
using std::string;
using std::vector;
using namespace std::literals;
using namespace std::chrono;

////Variables////
int xpos, lastxpos, vlastxpos;

bool recording = false;
bool playing = true;

vector<bool> actions;

////Functions/////
DWORD GetModuleBaseAddress(TCHAR* modname, DWORD pid)
{
	DWORD modBaseAddr = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
	MODULEENTRY32 ModuleEntry32 = { 0 };
	ModuleEntry32.dwSize = sizeof(MODULEENTRY32);

	if (Module32First(hSnapshot, &ModuleEntry32)) {
		do {
			if (_tcscmp(ModuleEntry32.szModule, modname) == 0) {
				modBaseAddr = (DWORD)ModuleEntry32.modBaseAddr;
				break;
			}
		} while (Module32Next(hSnapshot, &ModuleEntry32));
	}
	CloseHandle(hSnapshot);
	return modBaseAddr;
}

bool FileExists(string name)
{
	std::fstream file;
	file.open(name);
	if (file.fail()) {
		return false;
	}
	else {
		return true;
	}
}

DWORD moduleBaseAddress, baseOffset = 0x003222D0;
DWORD baseAddress;
DWORD xPositionAddress, lastxposaddr, pid;
vector<DWORD> xPosOffsets = { 0x164, 0x224, 0x67C };

_TCHAR moduleName[] = _T("GeometryDash.exe");

bool GetAddresses()
{
	HWND hWnd;
	if (hWnd = FindWindowA(0, "Geometry Dash")) {
		GetWindowThreadProcessId(hWnd, &pid);
		HANDLE wHandle = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
		xPositionAddress = 0;

		while (xPositionAddress < 0x100000) {
			std::this_thread::sleep_for(5ms);
			//Get module base address
			moduleBaseAddress = GetModuleBaseAddress(moduleName, pid);
			ReadProcessMemory(wHandle, (LPVOID)(moduleBaseAddress + baseOffset), &baseAddress, sizeof(baseAddress), NULL);

			//Get xpos
			xPositionAddress = baseAddress;
			for (int i = 0; i < xPosOffsets.size() - 1; i++) {
				ReadProcessMemory(wHandle, (LPVOID)(xPositionAddress += xPosOffsets[i]), &xPositionAddress, sizeof(xPositionAddress), NULL);
			}
			xPositionAddress += xPosOffsets[xPosOffsets.size() - 1];
		}

		cout << "xpos addr: " << std::hex << xPositionAddress << std::endl
			<< std::oct;
		return true;
	}
	else {
		cout << "Could not find window!";
		return false;
	}
}

string FormatNum(string value)
{
	string s = value;
	int n = s.length() - 3;
	while (n > 0) {
		s.insert(n, ",");
		n -= 3;
	}
	return s;
}

bool FileExist(string name) {
	std::fstream file;
	file.open(name);
	if (file.fail()) {
		return false;
	}
	else {
		return true;
	}
}

void Start()
{
	cout << "Controls:\nUse right click instead of left!\n-'p' plays/stops macro\n-'r' enables/disables recording\n-'g' reget xpos address for level\n-'left' to save and quit\n-'right' to quit without saving\n\n";
	GetAddresses();

	if (FileExist("macro.txt")) {
		string input;
		cout << "Do you want to load current macro? (y/n): ";
		std::cin >> input;

		if (input == "y") {
			cout << "Loading... ";
			//Load macro
			std::ifstream macro("macro.txt");
			if (macro.is_open()) {
				string line;
				int i = 0;
				while (getline(macro, line)) {
					actions.resize(i + 1);
					actions[i] = stof(line);
					i++;
				}
			}
			cout << "Locked and loaded!\n";
		}
	}
	cout << "Not Recording!\nNot Playing!\n";
}
SHORT key = VK_SPACE;
UINT mappedKey = MapVirtualKey(LOBYTE(key), 0);
INPUT input = { 0 };

void Input(int in) {
	input.type = INPUT_KEYBOARD;
	input.ki.wScan = mappedKey;
	input.ki.dwFlags = KEYEVENTF_SCANCODE;

	if (in == 1) {
		input.ki.dwFlags = 0;
		SendInput(1, &input, sizeof(input));
	}
	else {
		input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
		SendInput(1, &input, sizeof(input));
	}
}

void Other() {
	while (true) {
		if ((GetAsyncKeyState(VkKeyScan('p')) & 0x0001) != 0) {
			if (playing) {
				playing = false;
				cout << "Not playing!" << std::endl;
			}
			else {
				playing = true;
				recording = false;
				cout << "Playing!" << std::endl;
			}
		}
		if ((GetAsyncKeyState(VkKeyScan('r')) & 0x0001) != 0) {
			if (recording) {
				recording = false;
				cout << "Not recording!" << std::endl;
			}
			else {
				playing = false;
				recording = true;
				cout << "Recording!" << std::endl;
			}
		}
		if ((GetAsyncKeyState(VkKeyScan('g')) & 0x0001) != 0) {
			cout << "Regetting address...";
			lastxposaddr = xPositionAddress;
			GetAddresses();
			cout << "Done!\n";
		}
		std::this_thread::sleep_for(2ms);
	}
}