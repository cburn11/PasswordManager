#include "checkhaveibeenpwned.h"
#include "checkhaveibeenpwned_mainwindow.h"

int WINAPI wWinMain(HINSTANCE, HINSTANCE, WCHAR * szCmdLine, int) {

	CHIBP_MainWindow main_window{};

	main_window.ShowDialogBox(NULL);

	std::wstring cmdline{ szCmdLine };
	if( cmdline.length() > 0 ) {
		main_window.SetAccount(cmdline);
		main_window.CheckAccount();
	}

	main_window.run();

	return 0;
}