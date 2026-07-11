// =============================================================
// DESUBTICK 
// =============================================================

#include <Windows.h>
#include <thread>
#include <atomic>
#include "hook.h"

std::atomic<bool> g_running{true};

// ----- Main thread -----
DWORD WINAPI MainThread(LPVOID lpParam) {
    // Wait for game to fully load
    while (!GetModuleHandleW(L"client.dll")) {
        Sleep(100);
        if (!g_running) return 0;
    }

    // Give the game a moment to initialize
    Sleep(1000);

    // Initialize the hook
    if (Desubtick::Initialize()) {
        // Silent success - no message boxes
        // You can add a debug print if needed:
        // OutputDebugStringA("[Desubtick] Loaded successfully!\n");
    }

    // Keep thread alive
    while (g_running) {
        Sleep(1000);
    }

    Desubtick::Shutdown();
    return 0;
}

// ----- DLL Entry -----
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        HANDLE hThread = CreateThread(NULL, 0, MainThread, NULL, 0, NULL);
        if (hThread) CloseHandle(hThread);
    } else if (fdwReason == DLL_PROCESS_DETACH) {
        g_running = false;
    }
    return TRUE;
}