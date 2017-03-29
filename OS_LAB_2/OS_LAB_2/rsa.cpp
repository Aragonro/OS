#include <stdlib.h>
#include "ad_rsa.h"
#include <iostream>
#include "rsa.h"


RSAAPI unsigned E_D(unsigned *E, unsigned *D) {
	unsigned p, q;
	p = Simple();
	do {
		q = Simple();
	} while (p == q);
	unsigned n = p * q;
	unsigned fi = (p - 1) * (q - 1);
	*E = 3;
	for (*E = 3; Are_Simples(*E, fi) != 1; *E += 2);
	*D = 3;
	while ((*E)*(*D) % fi != 1) {
		*D+=1;
	}

	return n;
}

RSAAPI bool Stealth(unsigned M, unsigned E, unsigned n, unsigned *Res) {
	if (M > n) {
		return false;
	}
	*Res = Q_Pow(M, E, n);
	return true;
}

RSAAPI bool Flare(unsigned M, unsigned D, unsigned n, unsigned *Res) {
	if (M > n) {
		return false;
	}
	*Res = Q_Pow(M, D, n);
	return true;
}

