#include <stdio.h>
#include "mailbox.h"

int MAILBOX::count = 0;

DWORD GetControlSumForFile(LPCTSTR fName) {
	HANDLE h = CreateFile(fName, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

	if (h == INVALID_HANDLE_VALUE) return MAILBOX_FILE;

	DWORD dwSize = GetFileSize(h, 0);
	PBYTE pMem = new BYTE[dwSize];
	DWORD dwCount, dwCS;
	BOOL b = ReadFile(h, pMem, dwSize, &dwCount, 0);
	if (b) {
		dwCS = GetControlSum(pMem, dwSize);
		b = WriteFile(h, &dwCS, sizeof(dwCS), &dwCount, 0);
	}
	delete[]pMem;
	CloseHandle(h);
	return b ? MAILBOX_OK : MAILBOX_FILE;
}


DWORD GetControlSum(PBYTE pMem, DWORD dwCount) {
	// pMem - ����������, �������� ���������� ����� ��������� �����
	// dwCount - ������ ����� ��������� �����

	DWORD dwCS = 0;
	DWORD dwFull = dwCount / 4;
	PDWORD p32Mem = (PDWORD)pMem;
	for (DWORD i = 0; i < dwFull; ++i)
		dwCS += p32Mem[i]%499979;

	DWORD dwNotFull = dwCount % 4;
	if (dwNotFull) {
		pMem += dwFull * 4;
		DWORD dwNumber = 0;
		memcpy(&dwNumber, pMem, dwNotFull);
		dwCS += dwNumber % 499979;
	}
	return dwCS;
}

MAILBOX::MAILBOX(LPCTSTR fName, size_t MaxSize) {
	BOOL b;
	DWORD dwCount;
	_tcscpy_s(this->fName, fName);
	HANDLE h = CreateFile(fName, GENERIC_READ,
		FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (h == INVALID_HANDLE_VALUE) {
		if (MaxSize) {
			// ���� �� ���������� ����� � ������ �� ����� 0
			// ������� ����� ����
			Title.MaxSize = MaxSize;
			Title.MessageCounts = 0;
			Title.MessageSize = 0;
			h = CreateFile(fName, GENERIC_WRITE,
				FILE_SHARE_READ, 0, CREATE_NEW, 0, 0);

			if (h == INVALID_HANDLE_VALUE) {
				dwError = MAILBOX_FILE;
				return;
			}

			dwCS = GetControlSum((PBYTE)&Title, sizeof(Title));

			// ���������� � ������ ���� ����� ������ ���� Title
			b = WriteFile(h, &Title, sizeof(Title), &dwCount, 0);
			// ���� �������� �������, �������� ����������� ����� � �����
			if (b) b = WriteFile(h, &dwCS, sizeof(dwCS), &dwCount, 0);
			CloseHandle(h);
			// �������� ����� ������ �����
			dwCurSize = sizeof(Title) + sizeof(dwCS);
			if (!b) {
				dwError = MAILBOX_FILE;
				return;
			}
			dwError = MAILBOX_OK;
		}
		else {
			dwError = MAILBOX_CREATE;
			return;
		}
	}
	else {
		dwCurSize = GetFileSize(h, 0); // �������� ������ �����
		PBYTE pMem = new BYTE[dwCurSize];
		// ��������� � pMem ���������� �����
		b = ReadFile(h, pMem, dwCurSize, &dwCount, 0);
		if (b) {
			dwCS = *(DWORD*)(pMem + dwCurSize - 4);
			DWORD CalcCS = GetControlSum(pMem, dwCurSize - 4);
			dwError = (CalcCS == dwCS) ? MAILBOX_OK : MAILBOX_CS;
			PMAILBOX_TITLE pTitle = (PMAILBOX_TITLE)pMem;
			Title = *pTitle;
		}
		else {
			dwError = MAILBOX_FILE;
		}
		delete[]pMem;
		CloseHandle(h);
	}
}

MAILBOX& MAILBOX::Add(TCHAR * Msg) {
	// ���������� ������ ������ � ����
	// ������ ���������
	DWORD dwLen = _tcslen(Msg) * sizeof(TCHAR);
	dwError = MAILBOX_CREATE;

	if (Title.MaxSize >= dwCurSize + dwLen + sizeof(DWORD)) {
		Title.MessageCounts += 1;
		Title.MessageSize += dwLen;
		dwError = MAILBOX_FILE;
		HANDLE h = CreateFile(this->fName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		BOOL b;
		DWORD dwCount;

		if (h != INVALID_HANDLE_VALUE) {
			b = WriteFile(h, &Title, sizeof(Title), &dwCount, 0);
			if (b) {
				LONG dwHigeOffs = -1;
				SetFilePointer(h, -4, &dwHigeOffs, FILE_END);
				b = WriteFile(h, &dwLen, sizeof(dwLen), &dwCount, 0);
				if (b) b = WriteFile(h, Msg, dwLen, &dwCount, 0);
				CloseHandle(h); h = INVALID_HANDLE_VALUE;
				if (b) {
					dwError = GetControlSumForFile(fName);
				}
			}
			if (h != INVALID_HANDLE_VALUE) CloseHandle(h);
		}
	}

	return *this;
}

bool MAILBOX::CheckCRC() {
	BOOL b = TRUE;
	DWORD dwCS;
	DWORD CalcCS;
	HANDLE h = CreateFile(fName, GENERIC_READ,
		FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

	if (h != INVALID_HANDLE_VALUE) {
		dwCurSize = GetFileSize(h, 0); // �������� ������ �����
		PBYTE pMem = new BYTE[dwCurSize];
		// ��������� � pMem ���������� �����
		b = ReadFile(h, pMem, dwCurSize, 0, 0);
		if (b) {
			dwCS = *(DWORD*)(pMem + dwCurSize - 4);
			CalcCS = GetControlSum(pMem, dwCurSize - 4);
		}

		delete[] pMem;
		CloseHandle(h);
	}

	return (CalcCS == dwCS) ? true : false;
}

DWORD MAILBOX::ReadMessage(TCHAR *msg, DWORD i, bool removing) {
	// msg - ������������ ���������
	// ������������ �������� - ������ ���������

	BOOL b = TRUE;
	DWORD dwLen, dwCount;
	dwError = MAILBOX_NUMBER;

	if (i < Title.MessageCounts) {
		dwError = MAILBOX_FILE;
		HANDLE h = CreateFile(this->fName, GENERIC_READ,
			FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if (h != INVALID_HANDLE_VALUE) {
			// ������������� ��������� ����� ��������� ������ ��������� �����
			SetFilePointer(h, sizeof(Title), NULL, FILE_BEGIN);

			// ������� ��������� �� �������
			for (DWORD j = 0; j < i; ++j) {
				// �������� ������ ���������
				b = ReadFile(h, &dwLen, sizeof(dwLen), &dwCount, 0);
				if (!b) break;
				// ���������� �� ������ ���������
				SetFilePointer(h, dwLen, NULL, FILE_CURRENT);
			}

			// �������� ������ i-���� ���������
			if (b) b = ReadFile(h, &dwLen, sizeof(dwLen), &dwCount, 0);
			if (b && msg) {
				b = ReadFile(h, msg, dwLen, &dwCount, 0);
				msg[dwLen / sizeof(TCHAR)] = 0;
			}
			dwError = b ? MAILBOX_OK : MAILBOX_FILE;
			CloseHandle(h);
		}
	}

	if (removing) {
		this->Remove(i);
	}

	return dwLen;
}

MAILBOX& MAILBOX::Remove(DWORD i) {
	// �������� ��������� �� ����� �� ������� (������� � 0)

	dwError = MAILBOX_NUMBER;
	if (i < Title.MessageCounts) {
		Title.MessageCounts -= 1;
		dwError = MAILBOX_FILE;
		HANDLE h = CreateFile(this->fName, GENERIC_READ |
			GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		dwCurSize = GetFileSize(h, 0);
		BOOL b;
		DWORD dwCount, dwLen = 0;
		if (h != INVALID_HANDLE_VALUE) {
			b = WriteFile(h, &Title, sizeof(Title), &dwCount, 0);
			DWORD dwOld = sizeof(Title); // ��������� �� i-�� ���������
			DWORD dwNew; // ��������� �� ��������� ����� ����������
			DWORD dwCnt; // ������ ���������� ���������
			if (b) {
				// ������� �� i-��� ���������
				for (DWORD j = 0; j < i; ++j) {
					b = ReadFile(h, &dwLen, sizeof(dwLen), &dwCount, 0);
					if (!b) break;
					dwOld = SetFilePointer(h, dwLen, 0, FILE_CURRENT);
				}

				// �������� ������ �������� ���������
				b = ReadFile(h, &dwLen, sizeof(dwLen), &dwCount, 0);
				Title.MessageSize -= dwLen;
				// ���������� �� ������� ���������
				dwNew = SetFilePointer(h, dwLen, 0, FILE_CURRENT);
				// ������� ������ ���������� ���������
				dwCnt = dwCurSize - dwNew - 4;
				PBYTE pMem = new BYTE[dwCnt];
				SetFilePointer(h, dwNew, 0, FILE_BEGIN);
				// �������� � pMem ���������� ���������
				b = ReadFile(h, pMem, dwCnt, &dwCount, 0);
				if (b) {
					// ��������� ��������� �� ������ ���������� ���������
					SetFilePointer(h, dwOld, 0, FILE_BEGIN);
					// �������� ��������� ��������� �������� ����� ����
					b = WriteFile(h, pMem, dwCnt, &dwCount, 0);
					SetEndOfFile(h);
				}
				delete[] pMem;
				dwError = b ? MAILBOX_OK : MAILBOX_FILE;
			}

			SetFilePointer(h, 0, 0, FILE_BEGIN);
			b = WriteFile(h, &Title, sizeof(Title), &dwCount, 0);

			if (h != INVALID_HANDLE_VALUE)
				CloseHandle(h);
			if (b) dwError = GetControlSumForFile(this->fName);
		}
	}
	return *this;
}

void MAILBOX::Clear() {
	DWORD dwCount;
	BOOL b;

	Title.MessageCounts = 0;
	Title.MessageSize = 0;
	HANDLE h = CreateFile(this->fName, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (h != INVALID_HANDLE_VALUE) {
		b = WriteFile(h, &Title, sizeof(Title), &dwCount, 0);

		if (b) {
			// ������������� ��������� ����� ��������� ������ �����
			SetFilePointer(h, 12, NULL, FILE_BEGIN);
			SetEndOfFile(h);
			CloseHandle(h);
		}

		dwError = b ? MAILBOX_OK : MAILBOX_FILE;
	}

	dwError = GetControlSumForFile(this->fName);
}

int GetCountOfMailBox() {
	WIN32_FIND_DATA FindFileData;
	int count = 0;
	HANDLE hf;
	hf = FindFirstFile(_T(".\\mails\\*.dat"), &FindFileData);

	if (hf != INVALID_HANDLE_VALUE) {
		do {
			count++;
			wprintf(_T("\t%s\n"), FindFileData.cFileName);
		} while (FindNextFile(hf, &FindFileData) != 0);
		FindClose(hf);
	}

	return count;
}
