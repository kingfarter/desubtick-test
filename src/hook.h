#pragma once

#include <Windows.h>
#include <cstdint>

// ----- Forward declarations for Source 2 structs -----

typedef void* PVOID64;
typedef float QAngle[3];

// CUserCmd struct (Source 2)
struct CUserCmd {
    PVOID64 vtable;           // 0x00
    int     tick_count;       // 0x08
    QAngle  viewangles;       // 0x0C
    float   forwardmove;      // 0x18
    float   sidemove;         // 0x1C
    float   upmove;           // 0x20
    int     buttons;          // 0x24  <-- IN_ATTACK goes here
    int     impulse;          // 0x28
    int     weapon_select;    // 0x2C
    int     weapon_subtype;   // 0x30
    int     random_seed;      // 0x34
    short   mousedx;          // 0x38
    short   mousedy;          // 0x3A
    int     command_number;   // 0x3C
    float   flSubtick;        // 0x40  <-- THE SUBTICK TIMESTAMP
    float   flInputSampleTime;// 0x44
};

// ----- Constants -----

#define IN_ATTACK  (1 << 0)   // Mouse1 / +attack

// =============================================================
// Desubtick Class 
// =============================================================

class Desubtick {
public:
    static bool Initialize();
    static void Shutdown();

private:
    static bool m_initialized;
    static uintptr_t m_createMoveAddress;
    static void*     m_originalCreateMove;

    static uintptr_t FindCreateMove();
    static bool      HookCreateMove();
    static void      UnhookCreateMove();

    // Hooked function
    static bool __thiscall HookedCreateMove(PVOID64 pInput, float flInputSampleTime, CUserCmd* pCmd);

    // Pattern scanning helpers
    static bool ComparePattern(const BYTE* pData, const BYTE* pPattern, const char* pMask, size_t len);
};