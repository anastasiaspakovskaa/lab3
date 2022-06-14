#include <windows.h>
#include <iostream>
#include<vector>
#include<string>

using namespace std;

CRITICAL_SECTION cs;
HANDLE* hOutEvent;
HANDLE* hAddEvent;
int stopInd = 0;

struct fun {
	int i = 0;
	int size = 0;
	int* arr = NULL;
};

DWORD WINAPI marker(LPVOID aa)
{
	fun* a = (fun*)aa;

	vector<int> positions;
	srand(a->i);
start: 
	while (true)
	{
		int k = rand() % a->size;
		if (a->arr[k] == 0)
		{
			Sleep(5);
			a->arr[k] = a->i;
			Sleep(5);
			if (find(positions.begin(), positions.end(), k) == positions.end())
				positions.push_back(k);
		}
		else
		{
			EnterCriticalSection(&cs);
			printf("\nThread index: %d%s", a->i, "\n");
			printf("Number of marked elements: %d%s", positions.size(), "\n");
			printf("Cannot mark: %d%s", k, "\n");
			LeaveCriticalSection(&cs);

			break;
		}
	}
	SetEvent(hOutEvent[a->i]);
	WaitForSingleObject(hAddEvent[a->i], INFINITE);

	if (a->i == stopInd) {
		for (int i = 0; i < positions.size(); i++)
		{
			a->arr[positions[i]] = 0;
		}
	}
	else {
		goto start;
	}
	return 0;
}

int main() {

	InitializeCriticalSection(&cs);

	int size;
	printf("Enter size: ");
	scanf_s("%d", &size);

	int* arr = new int[size];
	for (int i = 0; i < size; i++) {
		arr[i] = 0;
	}
	int number;
	printf("Enter number of threads marker: ");
	scanf_s("%d", &number);

	int* flags = new int[number];
	for (int i = 0; i < number; i++) {
		flags[i] = 0;
	}

	hAddEvent = new HANDLE[number];
	hOutEvent = new HANDLE[number];
	HANDLE* hThread = new HANDLE[number];
	for (int i = 0; i < number; i++) {
		struct fun* a = new fun;
		a->i = i;
		a->size = size;
		a->arr = arr;
		DWORD IDThread_i;
		hThread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)marker, (LPVOID)a, 0, &IDThread_i);
		if (hThread[i] == NULL)
			return GetLastError();

		hOutEvent[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (hOutEvent[i] == NULL)
			return GetLastError();

		hAddEvent[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (hAddEvent[i] == NULL)
			return GetLastError();
	}

	int counter = number;

	while (counter != 0) {
		for (int i = 0; i < number; i++) {
			if (flags[i] != 1)
				WaitForSingleObject(hOutEvent[i], INFINITE);
		}

		for (int i = 0; i < size; i++) {
			printf("%d%s", arr[i], " ");
		}

		printf("\n\n\nPress number of thread to stop: ");
		scanf_s("%d", &stopInd);
		flags[stopInd] = 1;
		SetEvent(hAddEvent[stopInd]);
		WaitForSingleObject(hThread[stopInd], INFINITE);
		SetEvent(hOutEvent[stopInd]);

		for (int i = 0; i < size; i++) {
			printf("%d%s", arr[i], " ");
		}

		for (int i = 0; i < number; i++) {
			if (flags[i] != 1)
				SetEvent(hAddEvent[i]);
		}

		counter--;
	}
	
	for (int i = 0; i < number; i++) {
		CloseHandle(hThread[i]);
		CloseHandle(hOutEvent[i]);
		CloseHandle(hAddEvent[i]);
	}

	delete[] arr;

	return 0;
}
