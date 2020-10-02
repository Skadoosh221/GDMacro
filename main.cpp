#include "stuff.h"

float value;
void SetValues() {
    //auto t1 = high_resolution_clock::now();

    HANDLE wHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (ReadProcessMemory(wHandle, (LPCVOID)xPositionAddress, &value, sizeof(value), NULL) != 0) {
        xpos = value / 4;
    }
    CloseHandle(wHandle);

    //auto t2 = high_resolution_clock::now();
    //system("cls");
    //cout << "Duration: " << duration_cast<microseconds>(t2 - t1).count() << "us\n";
}

void Mac() {
    if (xpos != vlastxpos && xpos >= 0) {
        if (recording) {
            if ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0) {
                actions[xpos] = 1;
                Input(1);
            }
            else {
                actions[xpos] = 0;
                Input(0);
            }
        }

        if (playing) {
            if (actions[xpos] == 1) {
                Input(1);
            }
            else {
                Input(0);
            }
        }
    }
}

int main() {
    Start();
    thread other(Other);
    other.detach();

    while (true) {
        //Save and exit
        if ((GetAsyncKeyState(VK_LEFT) & 0x0001) != 0) {
            remove("macro.txt");

            cout << "Saving..pls wait bro :(\n";
            std::ofstream macro("macro.txt");
            if (macro.is_open()) {
                for (int i = 0; i < actions.size(); i++) {
                    macro << actions[i] << std::endl;
                }
            }
            cout << "Saved!\n";
            return 0;
        }
        if ((GetAsyncKeyState(VK_RIGHT) & 0x0001) != 0) {
            cout << "Exit!\n";
            return 0;
        }

        SetValues();
        while (xpos >= actions.size()) {
            actions.resize(xpos + 1);
        }
        Mac();

        vlastxpos = lastxpos;
        lastxpos = xpos;
    }
}