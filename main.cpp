#include <Windows.h>
#include <iostream>
#include <vector>

DWORD pid = 0;
HANDLE hProcess = 0;
uintptr_t gameBase = 0;

// Чтение памяти
template<typename T> T ReadMem(uintptr_t address) {
    T buffer;
    ReadProcessMemory(hProcess, (LPCVOID)address, &buffer, sizeof(T), 0);
    return buffer;
}

// Запись памяти
template<typename T> void WriteMem(uintptr_t address, T value) {
    WriteProcessMemory(hProcess, (LPVOID)address, &value, sizeof(T), 0);
}

// Поиск процесса
bool FindProcess() {
    HWND hwnd = FindWindowA("UnityWndClass", "Blockpost");
    if (!hwnd) return false;
    
    GetWindowThreadProcessId(hwnd, &pid);
    if (!pid) return false;
    
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    return hProcess != NULL;
}

// Поиск модуля игры
uintptr_t GetGameBase() {
    HMODULE hMods[1024];
    DWORD cbNeeded;
    
    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        for (int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
            char modName[MAX_PATH];
            if (GetModuleFileNameExA(hProcess, hMods[i], modName, sizeof(modName))) {
                if (strstr(modName, "blockpost_module.dll")) {
                    return (uintptr_t)hMods[i];
                }
            }
        }
    }
    return 0;
}

// Аимбот
void Aimbot() {
    uintptr_t localPlayer = ReadMem<uintptr_t>(gameBase + 0x234560);
    uintptr_t entityList = ReadMem<uintptr_t>(gameBase + 0x234568);
    
    if (!localPlayer || !entityList) return;
    
    // Поиск ближайшего врага
    float closestDist = 9999.0f;
    uintptr_t target = 0;
    
    for (int i = 0; i < 12; i++) {
        uintptr_t entity = ReadMem<uintptr_t>(entityList + i * 0x14);
        if (!entity || entity == localPlayer) continue;
        
        int health = ReadMem<int>(entity + 0xFC);
        if (health <= 0) continue;
        
        // Расчет дистанции
        Vector3 localPos = ReadMem<Vector3>(localPlayer + 0x34);
        Vector3 enemyPos = ReadMem<Vector3>(entity + 0x34);
        
        float dist = sqrt(pow(enemyPos.x - localPos.x, 2) + pow(enemyPos.y - localPos.y, 2));
        
        if (dist < closestDist) {
            closestDist = dist;
            target = entity;
        }
    }
    
    // Наведение
    if (target && closestDist < 50.0f) {
        Vector3 localPos = ReadMem<Vector3>(localPlayer + 0x34);
        Vector3 targetPos = ReadMem<Vector3>(target + 0x34);
        
        // Расчет угла
        float deltaX = targetPos.x - localPos.x;
        float deltaY = targetPos.y - localPos.y;
        float angle = atan2(deltaY, deltaX) * 180 / 3.14159;
        
        // Запись угла
        WriteMem<float>(localPlayer + 0x40, angle);
    }
}

// ВХ (стены)
void Wallhack() {
    // Включаем видимость через память
    WriteMem<bool>(gameBase + 0x1F8C2, true);
}

// Нет отдачи
void NoRecoil() {
    uintptr_t localPlayer = ReadMem<uintptr_t>(gameBase + 0x234560);
    if (!localPlayer) return;
    
    // Обнуляем отдачу
    WriteMem<float>(localPlayer + 0x58, 0.0f); // recoilX
    WriteMem<float>(localPlayer + 0x5C, 0.0f); // recoilY
}

// Бесконечные патроны
void UnlimitedAmmo() {
    uintptr_t localPlayer = ReadMem<uintptr_t>(gameBase + 0x234560);
    if (!localPlayer) return;
    
    int currentWeapon = ReadMem<int>(localPlayer + 0x128);
    if (currentWeapon > 0) {
        WriteMem<int>(currentWeapon + 0x30, 30); // ammo
    }
}

// Меню в консоли
void ShowMenu() {
    system("cls");
    std::cout << "=== Blockpost External Cheat ===" << std::endl;
    std::cout << "1. Включить Аимбот (ПКМ)" << std::endl;
    std::cout << "2. Включить ВХ" << std::endl;
    std::cout << "3. Включить Нет отдачи" << std::endl;
    std::cout << "4. Включить Беск. патроны" << std::endl;
    std::cout << "5. Включить все" << std::endl;
    std::cout << "0. Выход" << std::endl;
    std::cout << "Выберите опцию: ";
}

int main() {
    SetConsoleTitleA("Blockpost External Cheat");
    
    std::cout << "Поиск процесса Blockpost..." << std::endl;
    
    if (!FindProcess()) {
        std::cout << "Процесс не найден! Запустите игру сначала." << std::endl;
        system("pause");
        return 0;
    }
    
    gameBase = GetGameBase();
    if (!gameBase) {
        std::cout << "Модуль игры не найден!" << std::endl;
        system("pause");
        return 0;
    }
    
    std::cout << "Игра найдена! PID: " << pid << std::endl;
    
    bool aimbot = false, wallhack = false, noRecoil = false, unlimitedAmmo = false;
    
    while (true) {
        ShowMenu();
        
        int choice;
        std::cin >> choice;
        
        switch (choice) {
            case 1: aimbot = !aimbot; break;
            case 2: wallhack = !wallhack; break;
            case 3: noRecoil = !noRecoil; break;
            case 4: unlimitedAmmo = !unlimitedAmmo; break;
            case 5: 
                aimbot = wallhack = noRecoil = unlimitedAmmo = true;
                break;
            case 0: return 0;
        }
        
        // Применение читов
        while (true) {
            if (GetAsyncKeyState(VK_END) & 1) break;
            
            if (aimbot) Aimbot();
            if (wallhack) Wallhack();
            if (noRecoil) NoRecoil();
            if (unlimitedAmmo) UnlimitedAmmo();
            
            Sleep(10);
        }
    }
    
    CloseHandle(hProcess);
    return 0;
}
