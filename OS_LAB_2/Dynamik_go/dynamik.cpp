#include <windows.h>
#include <iostream>
#include <tchar.h>
#include <stdio.h> 
using namespace std;

typedef unsigned (*TE_D)(unsigned *, unsigned *);
typedef bool (*TSTEALTH)(unsigned, unsigned, unsigned, unsigned *);
typedef bool(*TFLARE)(unsigned, unsigned, unsigned, unsigned *);

TE_D e_d;
TSTEALTH st;
TFLARE fl;
HMODULE hlib;

int main(){
	hlib = LoadLibrary(_T("RSA.dll"));
	if (hlib == NULL) {
		FreeLibrary(hlib);
		cout << "RSA.dll dont load";
		return -1;
	}
	e_d = (TE_D)GetProcAddress(hlib, "E_D");
	if (e_d == NULL) {
		cout << "E_D dont load";
		FreeLibrary(hlib);
		return -1;
	}
	st = (TSTEALTH)GetProcAddress(hlib, "Stealth");
	if (st == NULL) {
		cout << "Stealth dont load";
		FreeLibrary(hlib);
		return -1;
	}
	fl = (TFLARE)GetProcAddress(hlib, "Flare");
	if (fl == NULL) {
		cout << "Flare dont load";
		FreeLibrary(hlib);
		return -1;
	}
	unsigned E, D, n, M, M_after,M_stealth;
	n = e_d(&E, &D);
	wchar_t str[1000];
	wchar_t after_str[1000];
	cout << "Enter string" << endl;
	wcin >> str;
	int k = 0;
	for (int i = 0; str[i] != '\0'; ++i) {
		wchar_t symbol = str[i];
		M = (unsigned)symbol;
		st(M, E, n, &M_stealth);
		fl(M_stealth, D, n, &M_after);
			
			cout << M_stealth << ' ';
			cout << M_after<< ' ';
			after_str[k] = ((wchar_t)(M_after));
			k++;
		

	}
	cout << endl;
	after_str[k] = '\0';
	for (int i = 0; after_str[i] != '\0'; ++i) {
		printf("%wc", after_str[i]);
	}
	FreeLibrary(hlib);
	return 0;
}