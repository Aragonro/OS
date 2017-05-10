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
	// pMem - переменная, хранящая содержимое файла почтового ящика
	// dwCount - размер файла почтового ящика

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
			// если не существует ящика и размер не равен 0
			// создаем новый ящик
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

			// записываем в пустой файл ящика данные поля Title
			b = WriteFile(h, &Title, sizeof(Title), &dwCount, 0);
			// если операция успешна, записать контрольную сумму в конце
			if (b) b = WriteFile(h, &dwCS, sizeof(dwCS), &dwCount, 0);
			CloseHandle(h);
			// обновить общий размер ящика
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
		dwCurSize = GetFileSize(h, 0); // получаем размер ящика
		PBYTE pMem = new BYTE[dwCurSize];
		// выгружаем в pMem содержимое ящика
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
	// добавление нового письма в ящик
	// размер сообщения
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
		dwCurSize = GetFileSize(h, 0); // получаем размер ящика
		PBYTE pMem = new BYTE[dwCurSize];
		// выгружаем в pMem содержимое ящика
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
	// msg - возвращаемое сообщение
	// возвращаемое значение - размер сообщения

	BOOL b = TRUE;
	DWORD dwLen, dwCount;
	dwError = MAILBOX_NUMBER;

	if (i < Title.MessageCounts) {
		dwError = MAILBOX_FILE;
		HANDLE h = CreateFile(this->fName, GENERIC_READ,
			FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if (h != INVALID_HANDLE_VALUE) {
			// устанавливаем указатель после системных данных почтового ящика
			SetFilePointer(h, sizeof(Title), NULL, FILE_BEGIN);

			// листаем сообщения до нужного
			for (DWORD j = 0; j < i; ++j) {
				// получаем размер сообщения
				b = ReadFile(h, &dwLen, sizeof(dwLen), &dwCount, 0);
				if (!b) break;
				// сдвигаемся на размер сообщения
				SetFilePointer(h, dwLen, NULL, FILE_CURRENT);
			}

			// получаем размер i-того сообщения
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
	// удаление сообщения из ящика по индексу (начиная с 0)

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
			DWORD dwOld = sizeof(Title); // указатель на i-ое сообщение
			DWORD dwNew; // указатель на сообщение после удаляемого
			DWORD dwCnt; // размер оставшихся сообщений
			if (b) {
				// доходим до i-ого сообщения
				for (DWORD j = 0; j < i; ++j) {
					b = ReadFile(h, &dwLen, sizeof(dwLen), &dwCount, 0);
					if (!b) break;
					dwOld = SetFilePointer(h, dwLen, 0, FILE_CURRENT);
				}

				// получаем размер текущего сообщения
				b = ReadFile(h, &dwLen, sizeof(dwLen), &dwCount, 0);
				Title.MessageSize -= dwLen;
				// сдвигаемся на текущее сообщение
				dwNew = SetFilePointer(h, dwLen, 0, FILE_CURRENT);
				// считаем размер оставшихся сообщений
				dwCnt = dwCurSize - dwNew - 4;
				PBYTE pMem = new BYTE[dwCnt];
				SetFilePointer(h, dwNew, 0, FILE_BEGIN);
				// копируем в pMem оставшиеся сообщения
				b = ReadFile(h, pMem, dwCnt, &dwCount, 0);
				if (b) {
					// переводим указатель на начало удаляемого сообщения
					SetFilePointer(h, dwOld, 0, FILE_BEGIN);
					// затираем удаляемое сообщение стоящими после него
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
			// устанавливаем указатель после системных данных ящика
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
