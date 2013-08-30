#include "WiretapViewer.h"

#ifdef WIN32
#include <Windows.h>
#include "../resource.h"
#endif

#ifdef WIN32
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main(int arc, char** arv)
#endif
{
	Wiretap::ProfileViewer profileViewer(1024, 576, "Wiretap", 13001);

	profileViewer.Start();

	// Set the window icon (also used for alt-tab). TODO: Implement for linux/unix
#ifdef WIN32
	HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	if(hIcon)
    {
		SendMessage(profileViewer.GetWindowHandle(), WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	}
#endif

	while (profileViewer.IsOpen())
	{
		profileViewer.Update();
	}

	profileViewer.Stop();

	return 0;
}
