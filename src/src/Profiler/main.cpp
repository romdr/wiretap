#include <Windows.h>
#include "WiretapProfiler.h"

void Update();
void DoTask1();
void DoTask2();
void DoTask2A();
void DoTask2B();
void DoTask3();

int main(int argc, char* argv)
{
	while (true)
	{
		Update();

		Wiretap::Profiler::DumpEvents();
		Wiretap::Profiler::SendData();

		// Dbg
		Sleep(10 + rand() % 15);
	}

	return 0;
}

void Update()
{
	PROFILE_SCOPE(Update);

	DoTask1();
	DoTask2();
	DoTask3();
	Sleep(rand() % 5);
}

void DoTask1()
{
	PROFILE_SCOPE(DoTask1);
	Sleep(1 + rand() % 4);
}

void DoTask2()
{
	PROFILE_SCOPE(DoTask2);

	DoTask2A();
	DoTask2B();
	Sleep(rand() % 20);
}

void DoTask2A()
{
	PROFILE_SCOPE(DoTask2A);
	Sleep(5 + rand() % 3);
}

void DoTask2B()
{
	PROFILE_SCOPE(DoTask2B);
	Sleep(4 + rand() % 2);
}

void DoTask3()
{
	PROFILE_SCOPE(DoTask3);
	Sleep(3 + rand() % 2);
}