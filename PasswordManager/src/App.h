#pragma once


void ExitApplication(HWND hwnd);

bool OpenFile(HWND hwndParent, const WCHAR * szFilepath);

void UpdateDisplay(HWND hwnd);

void LaunchBrowser(HWND hwnd);

//void LaunchBrowserClipboard(HWND hwnd);

void LaunchAgnosticBrowserClipboard(HWND hwnd);

void LaunchClipboardMonitor(HWND hwnd, bool fIncludeURL);

void LaunchClipboardMonitorWithAccount(HWND hwnd);

void UpdateMenuItems(HWND hwnd);

void EditEntry(HWND hwnd);

void AddEntry(HWND hwnd);

void RemoveEntry(HWND hwnd);

void Close(HWND hwnd);

void NewFile(HWND hwndParent);

void SaveFile(HWND hwndParent);

void SaveFileAs(HWND hwndParent);

void SaveFileEncrypted(HWND hwndParent);

void CreateRecentFilesMenu(HWND hwnd);

void ResetRecentFilesMenu(HWND hwnd);

void OpenRecentFile(HWND hwnd, UINT id);

void ClearRecentFileMenu(HWND hwnd);

void ProcessCommandLine(HWND hwnd, const WCHAR * szCmdLine);

void UpdateEditorAccount(HWND hwnd, long direction, const Account ** ppaccount);

void ShowBrowserCommandDialog(HWND hwnd);

void ShowHelpDialog(HWND hwndParent);

void ShowToolsDialog(HWND hwndParent);

void ShowSearchDialog(HWND hwndParent);

void PopulateToolsMenu(HWND hwnd);

void LaunchTool(HWND hwndParent, int ID);

void ProcessClipboardMonitorClosing(HWND hwndParent);

void ExportXML(HWND hwndParent);

void ImportXML(HWND hwndParent);

void SwapAccount(HWND hwndParent, int delta);

#define MOVE_ACCOUNT_BEFORE	-1
#define MOVE_ACCOUNT_AFTER	1

void MoveAccount(HWND hwndParent, int type);

HMENU CreateContextMenu(HWND hwnd);

void ShowContextMenu(HWND hwnd, int clientX, int clientY);

void CloneEntry(HWND hwnd);

void QueryOpenFile(const wchar_t* szPath, bool* fRet);