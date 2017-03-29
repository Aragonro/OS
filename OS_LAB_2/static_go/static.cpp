#include "..\OS_LAB_2\rsa.h"
#include <iostream>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

RSAAPI unsigned E_D(unsigned *, unsigned *);
RSAAPI bool Stealth(unsigned, unsigned, unsigned, unsigned *);
RSAAPI bool Flare(unsigned, unsigned, unsigned, unsigned *);

int main() {
	setlocale(LC_ALL, "English");
	unsigned E, D, n, M, M_after, M_stealth;
	n = E_D(&E, &D);
	wchar_t str[1000];
	wchar_t after_str[1000];
	cout << "Enter string" << endl;
	wcin >> str;
	int k = 0;
	for (int i = 0; str[i] != '\0'; ++i) {
		wchar_t symbol = str[i];
		M = (unsigned)symbol;
		Stealth(M, E, n, &M_stealth);
		Flare(M_stealth, D, n, &M_after);

		cout << M_stealth << ' ';
		after_str[k] = ((wchar_t)(M_after));
		k++;


	}
	cout << endl;
	after_str[k] = '\0';
	for (int i = 0; after_str[i] != '\0'; ++i) {
		printf("%wc", after_str[i]);
	}
	return 0;
}

