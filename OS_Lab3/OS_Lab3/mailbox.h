#pragma once
#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <windows.h>
#include <string.h>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>


enum {
	MAILBOX_OK,
	MAILBOX_CREATE,
	MAILBOX_FILE,
	MAILBOX_NUMBER,
	MAILBOX_CS
};

typedef struct _MAILBOX_TITLE {
	DWORD MaxSize, MessageSize, MessageCounts; // размер €щика (4 байта), количество сообщений (4 байта)
}MAILBOX_TITLE, *PMAILBOX_TITLE;

class MAILBOX {
	TCHAR fName[MAX_PATH]; // путь к файлу (2 * 260 байта)
	MAILBOX_TITLE Title;
	DWORD dwError; // ’ранение последней ошибки
	static int count; // количество сообщений
	DWORD dwCurSize; // текущий размер почтового €щика в байтах
	DWORD dwCS; //  онтрольна сума (4 байта)

public:
	MAILBOX(LPCTSTR fName, size_t MaxSize = 0);
	MAILBOX& Add(TCHAR * Msg);
	MAILBOX& Remove(DWORD Number);
	DWORD ReadCounts() {
		return Title.MessageCounts;
	}
	DWORD ReadMessage(TCHAR *res, DWORD i, bool removing = false);
	void Clear();
	DWORD GetLastError() { return dwError; }
	bool CheckCRC();
	~MAILBOX() { count--; }
};

DWORD GetControlSum(PBYTE pMem, DWORD dwCount);

int GetCountOfMailBox();
