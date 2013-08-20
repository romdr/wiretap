#include "WiretapViewer.h"

#ifdef WIN32
#include <Windows.h>
#endif

#ifdef WIN32
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main(int arc, char** arv)
#endif
{
	Wiretap::ProfileViewer profileViewer(1024, 576, "Wiretap", 13001);

	profileViewer.Start();

	while (profileViewer.IsOpen())
	{
		profileViewer.Update();
	}

	profileViewer.Stop();

	return 0;
}
