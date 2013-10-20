#include <stdio.h>
#include "Main.h"
#include "Resource.h"

#define ID_MAIN_EDIT_CONTROL 10001

HWND hMainEditControl;

const char* windowClassName = "MainWindowClass";

BOOL CALLBACK SaveDialogProc(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK OpenDialogProc(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam);
BOOL CALLBACK DeveloperDialogProc(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK MainWindowProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	HWND hWnd;
	WNDCLASSEX wc;

	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.hCursor = LoadCursor(NULL,IDC_ARROW);
	wc.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_ERIC));
	wc.hIconSm = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_ERIC));
	wc.hInstance = hInstance;
	wc.lpfnWndProc = MainWindowProc;
	wc.lpszClassName = windowClassName;
	wc.lpszMenuName = (LPCSTR)IDR_MAIN_MENU;
	wc.style = CS_HREDRAW|CS_VREDRAW;

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL,"Error: unable to register the window class","Error",MB_OK|MB_ICONERROR);
		return 0;
	}

	hWnd = CreateWindowEx(0,
		                  windowClassName,
						  "Eric's Notepad",
						  WS_OVERLAPPEDWINDOW,
						  CW_USEDEFAULT,
						  CW_USEDEFAULT,
						  640,480,
						  NULL,
						  NULL,
						  hInstance,
						  NULL);

	if (!hWnd)
	{
		MessageBox(NULL,"Error: unable to create the window","Error",MB_OK|MB_ICONERROR);
		return 0;
	}

	UpdateWindow(hWnd);
	ShowWindow(hWnd,SW_SHOW);

	MSG msg;

	PostMessage(hWnd,WM_ACTIVATE,MAKEWPARAM(5,3),0);

	while(GetMessage(&msg,hWnd,0,0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK MainWindowProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{	
		case WM_CREATE:
			{
				/*
				* Create the Main Menu
				*/

				HMENU hMenu;
				HMENU hPopupMenu;

				hMenu = CreateMenu();

				hPopupMenu = CreatePopupMenu();
				AppendMenu(hPopupMenu,MF_STRING,CMD_FILE_OPEN,"Open");
				AppendMenu(hPopupMenu,MF_STRING,CMD_FILE_SAVE,"Save");
				AppendMenu(hPopupMenu,MF_STRING,CMD_FILE_EXIT,"Exit");
				AppendMenu(hMenu,MF_STRING|MF_POPUP,(UINT_PTR)hPopupMenu,"File");

				hPopupMenu = CreatePopupMenu();
				AppendMenu(hPopupMenu,MF_STRING,CMD_EDIT_UNDO,"Undo");
				AppendMenu(hPopupMenu,MF_STRING,CMD_EDIT_REDO,"Redo");
				AppendMenu(hMenu,MF_STRING|MF_POPUP,(UINT_PTR)hPopupMenu,"Edit");

				hPopupMenu = CreatePopupMenu();
				AppendMenu(hPopupMenu,MF_STRING,CMD_ABOUT_DEVELOPER,"Developer");
				AppendMenu(hMenu,MF_STRING|MF_POPUP,(UINT_PTR)hPopupMenu,"About");

				SetMenu(hWnd,hMenu);

				/*
				* Create the Main Edit Control
				*/

				hMainEditControl = CreateWindow("EDIT",
					                            0,
												WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_AUTOHSCROLL|ES_AUTOVSCROLL|WS_VSCROLL|WS_HSCROLL,
												0,0,
												620,425,
												hWnd,
												(HMENU)ID_MAIN_EDIT_CONTROL,
												GetModuleHandle(NULL),
												NULL);

				break;
			}

		case WM_COMMAND:

			switch(LOWORD(wParam))
			{
				case CMD_FILE_OPEN:
					DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_OPEN_DIALOG),hWnd,OpenDialogProc);
					break;

				case CMD_FILE_SAVE:
					DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_SAVE_DIALOG),hWnd,SaveDialogProc);
					break;

				case CMD_FILE_EXIT:
					DestroyWindow(hWnd);
					break;

				case CMD_EDIT_UNDO:
					SendMessage(hMainEditControl,EM_UNDO,0,0);
					break;

				case CMD_ABOUT_DEVELOPER:
					DialogBox(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_DEVELOPER_DIALOG),hWnd,DeveloperDialogProc);
					break;
			}

			break;

		case WM_DESTROY:
			DestroyWindow(hWnd);
			break;

		case WM_QUIT:
			PostQuitMessage(0);
			break;

		default:	
			return DefWindowProc(hWnd,msg,wParam,lParam);
	}

	return 0;
}

BOOL CALLBACK OpenDialogProc(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
			{
				// limit the length of the text to MAX_PATH
				HWND hEdit = GetDlgItem(hDlg,ID_EDIT_CONTROL);
				SendMessage(hEdit,EM_SETLIMITTEXT,MAX_PATH,0);

				break;
			}

		case WM_COMMAND:

			switch(LOWORD(wParam))
			{
				case ID_OPEN:
					{
						HANDLE hFile;
						HWND hEditControl = GetDlgItem(hDlg,ID_EDIT_CONTROL); // get the edit control handle

						char path[MAX_PATH];

						SendMessage(hEditControl,WM_GETTEXT,MAX_PATH,(LPARAM)path); // get the path entered by the user

						hFile = CreateFile(path,
										   GENERIC_READ,
										   FILE_SHARE_READ,
										   NULL,
										   OPEN_EXISTING, // only open if it exists
										   FILE_ATTRIBUTE_NORMAL,
										   NULL);

						// was file found
						if (GetLastError() == ERROR_FILE_NOT_FOUND)
						{
							MessageBox(NULL,"Error: unable to find that file","Error",MB_ICONERROR|MB_OK);
						}
						else if (hFile != INVALID_HANDLE_VALUE)
						{
							int fileLen = GetFileSize(hFile,NULL); // get the size of the file in bytes (1 byte = 1 character)
							char* buffer = new char[fileLen+1];

							DWORD bytesRead;

							ReadFile(hFile,buffer,fileLen,&bytesRead,NULL); // read the whole file

							buffer[fileLen] = '\0'; // null terminate the string

							// did we read the whole file
							if (bytesRead != fileLen)
							{
								MessageBox(NULL,"Warning: not all the data was read from the file","Warning",MB_ICONWARNING|MB_OK);
							}

							SetWindowText(hMainEditControl,buffer); // put the newly filled buffer in the main edit control

							delete[] buffer;
						}

						CloseHandle(hFile);

						EndDialog(hDlg,0);

						break;
					}

				case ID_CANCEL:
					EndDialog(hDlg,0);
					break;
			}

			break;

		case WM_CLOSE:
			EndDialog(hDlg,0);
			break;

		default:
			return FALSE;
	}

	return TRUE;
}

BOOL CALLBACK SaveDialogProc(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
			{
				// limit the length of the text to MAX_PATH
				HWND hEdit = GetDlgItem(hDlg,ID_EDIT_CONTROL);
				SendMessage(hEdit,EM_SETLIMITTEXT,MAX_PATH,0);

				break;
			}

		case WM_COMMAND:

			switch(LOWORD(wParam))
			{
				case ID_SAVE:
					{
						HANDLE hFile;
					    HWND hEditControl = GetDlgItem(hDlg,ID_EDIT_CONTROL); // get edit box handle
						
						char path[MAX_PATH+1] = {0};

						SendMessage(hEditControl,WM_GETTEXT,MAX_PATH,(LPARAM)path); // get the path the user entered

						// open a file for writing
						hFile = CreateFile(path,
										   GENERIC_WRITE,
										   FILE_SHARE_READ,
										   NULL,
										   CREATE_NEW, // fails if file exists
										   FILE_ATTRIBUTE_NORMAL,
										   NULL);

						if (GetLastError() == ERROR_FILE_EXISTS)
						{
							// overwrite?
							int messageBox = MessageBox(NULL,"This file already exisits! Would you like to overwrite it?","Warning",MB_YESNO|MB_ICONWARNING);

							// user clicked yes
							if (messageBox == IDYES)
							{
								hFile = CreateFile(path,
										   GENERIC_WRITE,
										   FILE_SHARE_READ,
										   NULL,
										   CREATE_ALWAYS, // overwrite
										   FILE_ATTRIBUTE_NORMAL,
										   NULL);
							}
						}

						// if file is valid
						if (hFile != INVALID_HANDLE_VALUE)
						{
							int textLen = GetWindowTextLength(hMainEditControl); // length of text in main edit

							textLen += 1; // add one to accomidate the null terminator

							char* buffer = new char[textLen];

							GetWindowText(hMainEditControl,buffer,textLen); // fill up the text buffer to the top

							DWORD bytesWritten;

							WriteFile(hFile,buffer,textLen-1,&bytesWritten,NULL); // write text buffer to file except for null terminator

							// was all all data was coppied
							if (bytesWritten != textLen-1)
							{
								MessageBox(NULL,"Warning: not all the txtx data was saved","Warning",MB_OK|MB_ICONWARNING);
							}

							delete[] buffer;
						}

						CloseHandle(hFile);

						EndDialog(hDlg,0);
						break;
					}

				case ID_CANCEL:
					EndDialog(hDlg,0);
			}

			break;

		case WM_CLOSE:
			EndDialog(hDlg,0);
			break;

		default:
			return FALSE;
	}

	return TRUE;
}

BOOL CALLBACK DeveloperDialogProc(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_COMMAND:

			switch(LOWORD(wParam))
			{
				case ID_OK:
					EndDialog(hDlg,0);
					break;
			}

		case WM_CLOSE:
			EndDialog(hDlg,0);
			break;

		default:
			return FALSE;
	}

	return TRUE;
}