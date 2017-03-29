#pragma once
#ifdef OS_LAB_2_EXPORTS
#define RSAAPI extern "C" _declspec (dllexport)
#else
#define RSAAPI extern "C" _declspec (dllimport)
#endif
RSAAPI unsigned E_D(unsigned *, unsigned *);
RSAAPI bool Stealth(unsigned, unsigned, unsigned, unsigned *);
RSAAPI bool Flare(unsigned, unsigned, unsigned, unsigned *);

