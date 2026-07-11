// =============================================================
// DESUBTICK - Minimal CreateMove Hook
// Forces flSubtick = 0.0f for ALL inputs
// =============================================================

#include "hook.h"
#include <cstdio>
#include <cstring>
#include "MinHook.h"

// ----- Static members -----

bool Desubtick::m_initialized = false;
uintptr_t Desubtick::m_createMoveAddress = 0;
void* Desubtick::m_originalCreateMove = nullptr;

// ----- Pattern comparison -----

bool Desubtick::ComparePattern(const BYTE* pData, const BYTE* pPattern, const char* pMask, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (pMask[i] == 'x' && pData[i] != pPattern[i]) {
            return false;
        }
    }
    return true;
}

// ----- Find CreateMove -----

uintptr_t Desubtick::FindCreateMove() {
    HMODULE client = GetModuleHandleW(L"client.dll");
    if (!client) return 0;

    // Primary pattern (CS2 2025)
    const BYTE pattern1[] = {
        0x48, 0x89, 0x5C, 0x24, 0x08, // mov [rsp+8], rbx
        0x57,                          // push rdi
        0x48, 0x83, 0xEC, 0x20,        // sub rsp, 20h
        0x8B, 0x81, 0x00, 0x00, 0x00, 0x00, // mov eax, [rcx+???]
        0x4C, 0x8B, 0x89, 0x00, 0x00, 0x00, 0x00, // mov r9, [rcx+???]
        0x48, 0x89, 0x5C, 0x24, 0x38, // mov [rsp+38h], rbx
        0x48, 0x89, 0x74, 0x24, 0x40  // mov [rsp+40h], rsi
    };
    const char mask1[] = "xxxxxxxxxxx????xxx????xxxxxxxxxx";

    // Fallback pattern
    const BYTE pattern2[] = {
        0x8B, 0x81, 0x00, 0x00, 0x00, 0x00, // mov eax, [rcx+???]
        0x4C, 0x8B, 0x89, 0x00, 0x00, 0x00, 0x00 // mov r9, [rcx+???]
    };
    const char mask2[] = "xx????xxx????";

    // Get module info
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)client;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return 0;

    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)((BYTE*)client + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) return 0;

    BYTE* start = (BYTE*)client;
    BYTE* end = start + nt->OptionalHeader.SizeOfImage;

    // Try primary pattern
    for (BYTE* p = start; p < end - sizeof(pattern1); p++) {
        if (ComparePattern(p, pattern1, mask1, sizeof(pattern1))) {
            return (uintptr_t)p;
        }
    }

    // Try fallback
    for (BYTE* p = start; p < end - sizeof(pattern2); p++) {
        if (ComparePattern(p, pattern2, mask2, sizeof(pattern2))) {
            return (uintptr_t)p;
        }
    }

    return 0;
}

// =============================================================
// HOOKED CREATEMOVE 
// =============================================================

typedef bool(__thiscall* CreateMoveFn)(PVOID64, float, CUserCmd*);

bool __thiscall Desubtick::HookedCreateMove(PVOID64 pInput, float flInputSampleTime, CUserCmd* pCmd) {
    // Call original first
    CreateMoveFn original = (CreateMoveFn)m_originalCreateMove;
    bool ret = original(pInput, flInputSampleTime, pCmd);

    if (!pCmd) return ret;

    // =============================================================
    // DESUBTICK: Force flSubtick to 0.0 (tick boundary)
    // This applies to ALL inputs in this command (aim, shoot, movement)
    // =============================================================
    pCmd->flSubtick = 0.0f;

    // =============================================================
    // SHOOTING: If mouse1 is held, IN_ATTACK is already set by the game
    // We don't need to add anything - it's already there.
    // The subtick zeroing above will also apply to the shot.
    // =============================================================

    // That's it. Every command gets subtick = 0.0, including attacks.
    // No toggles, no hotkeys, no auto-shoot. Just works.

    return ret;
}

// =============================================================
// HOOK SETUP
// =============================================================

bool Desubtick::HookCreateMove() {
    if (MH_Initialize() != MH_OK) return false;

    if (MH_CreateHook(
        (LPVOID)m_createMoveAddress,
        (LPVOID)&HookedCreateMove,
        (LPVOID*)&m_originalCreateMove
    ) != MH_OK) {
        return false;
    }

    if (MH_EnableHook((LPVOID)m_createMoveAddress) != MH_OK) {
        return false;
    }

    return true;
}

void Desubtick::UnhookCreateMove() {
    if (m_createMoveAddress) {
        MH_DisableHook((LPVOID)m_createMoveAddress);
        MH_RemoveHook((LPVOID)m_createMoveAddress);
    }
    MH_Uninitialize();
}

// ----- Public API -----

bool Desubtick::Initialize() {
    if (m_initialized) return true;

    m_createMoveAddress = FindCreateMove();
    if (!m_createMoveAddress) return false;

    if (!HookCreateMove()) return false;

    m_initialized = true;
    return true;
}

void Desubtick::Shutdown() {
    if (!m_initialized) return;
    UnhookCreateMove();
    m_initialized = false;
}