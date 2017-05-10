#include "windows.h"
#include "mailBox.h"
#include <stdio.h>
#include <iostream>
using namespace std;

void _tmain() {
	_tsetlocale(LC_ALL, _T("Russian"));

	TCHAR value[50];
	TCHAR commandName[10];
	TCHAR Msg[80];

	CreateDirectory(_T(".\\mails\\"), NULL);

	while (true) {
		wcout << _T("¬ведите одну из предложанных команд:") << endl;
		wcout << _T("emails (вывести все почтовые €щики)") << endl;
		wcout << _T("open (открыть или создать почтовый €щик)")<<endl;
		wcout << _T("exit (выйти из программы)")<<endl;
		wcin >> commandName;

		if (!wcscmp(commandName, L"emails")) {
			wcout << _T("\t оличество почтовых €щиков: ") << GetCountOfMailBox() << endl;
			continue;
		}
		else if (!wcscmp(commandName, L"exit")) {
			break;
		}
		else if (wcscmp(commandName, L"open")) {
			continue;
		}

		while (true) {
			TCHAR filename[100];
			TCHAR fullname[100]{ _T(".\\mails\\") };

			wcout << _T("¬ведите название почтового €щика дл€ работы (exit дл€ выхода): ");
			wcin >> filename;
			if (!wcscmp(filename, L"exit")) break;

			wcscat(fullname, filename);
			MAILBOX MailBox(fullname, 1000);

			wcout << _T("\n оманды дл€ работы с почтовым €щиком:") << endl;
			wcout << _T("\tcount: вывести текущее количество сообщений почтового €щика") << endl;
			wcout << _T("\tadd: добавить сообщени€ S") << endl;
			wcout << _T("\tremove i: удалить сообщени€ с индексом i (отсчет с нул€)") << endl;
			wcout << _T("\tclear: очистить почтовый €щик") << endl;
			wcout << _T("\treadAll: вывести все сообщени€ почтового €щика") << endl;
			wcout << _T("\tread: вывести сообщени€ с индексом i (отсчет с нул€)") << endl;
			wcout << _T("\treadAndDel: вывести и удалить сообщени€ с индексом i (отсчет с нул€)") << endl;
			wcout << _T("\tcheck: дл€ проверки целостности файла почтового €щика") << endl;
			wcout << _T("\texit: перейти на предыдущий этап или выйти из программы") << endl;

			while (true) {
				wcout << _T("¬ведите команду: ");
				wcin >> commandName;

				if (!wcscmp(commandName, L"count")) {
					wcout << _T("“екущее количество сообщений в €щике: ") << MailBox.ReadCounts() << endl;
				}
				else if (!wcscmp(commandName, L"add")) {
					TCHAR temp;
					wscanf(_T("%c"), &temp);
					while (true) {


						wcout << _T("¬ведите сообщение: ");
						int k = 0;
						wscanf(_T("%c"), &temp);
						while (temp!= '\n') {
							value[k++] = temp;
							wscanf(_T("%c"), &temp);
						}
						value[k] = _T('\0');
						
						if (!wcscmp(value, L"exit")) break;
						MailBox.Add(value);
					}
				}
				else if (!wcscmp(commandName, L"remove")) {
					int index;
					while (true) {
						wcout << _T("¬ведите номер сообщени€: ");
						wcin >> value;
						index = _wtoi(value);

						if (!wcscmp(value, L"exit")) break;

						if (index < MailBox.ReadCounts()) {
							MailBox.Remove(index);
						}
						else {
							wcout << _T("— таким индексом собщение не существует") << endl;
						}

					}
				}
				else if (!wcscmp(commandName, L"clear")) {
					MailBox.Clear();
					wcout << _T("ѕочтовый €щик очищен!") << endl;
				}
				else if (!wcscmp(commandName, L"readAll")) {
					for (int i = 0; i < MailBox.ReadCounts(); ++i) {
						MailBox.ReadMessage(Msg, i);
						_tprintf(_T("%s\n"), Msg);
					}

					continue;
				}
				else if (!wcscmp(commandName, L"read")) {
					int index;
					while (true) {
						wcout << _T("¬ведите номер сообщени€: ");
						wcin >> value;

						

						index = _wtoi(value);

						if (!wcscmp(value, L"exit")) break;

						if (index < MailBox.ReadCounts()) {
							MailBox.ReadMessage(Msg, index);
							_tprintf(_T("%s\n"), Msg);
						}
						else {
							wcout << _T("— таким индексом собщение не существует") << endl;
						}

					}
				}
				else if (!wcscmp(commandName, L"readAndDel")) {
					int index;
					while (true) {
						wcout << _T("¬ведите номер сообщени€: ");
						wcin >> value;
						index = _wtoi(value);

						if (!wcscmp(value, L"exit")) break;
						if (index < MailBox.ReadCounts()) {
							MailBox.ReadMessage(Msg, index, true);
							_tprintf(_T("%s\n"), Msg);
						}
						else {
							wcout << _T("— таким индексом собщение не существует") << endl;
						}
					}
				}
				else if (!wcscmp(commandName, L"check")) {
					if (MailBox.CheckCRC()) {
						wcout << _T("Check: OK") << endl;
					}
					else {
						wcout << _T("Check: Bad File")<<endl;
					}
				}

				if (!wcscmp(commandName, L"exit")) break;
			}

		}
	}
}