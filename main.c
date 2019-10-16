#if defined (_WIN32) || (_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

//#define COMMANDS ("")

/*Struct contains global flags*/
struct global {
	int DEBUG;
} g;

/*Check if compiled on windows and use windows specific functions to show messagebox. Check if compiled on windows was made before call to this function
so this shouldn't interfere with not existing when needed to not exist.*/
#if defined (_WIN32) || (_WIN64)
void messageBoxWin(char* command, char* errorMessage)
{
	/*Display MessageBox with errorMessage text, set window title to Auto, warning icon, yes no cancel buttons 
	(includes cancel so the window can be closed with the close window control in top right corner) and stores MessageBox return value, 
	and sets the no button (second button) to default.*/
	int msgboxId = MessageBox(HWND_DESKTOP, errorMessage, "Auto", MB_ICONWARNING |  MB_YESNOCANCEL | MB_DEFBUTTON2 );
	switch (msgboxId)
	{
		/*If yes was clicked, copy failed command to clipboard*/
		case IDYES:
			/*Open desktop clipboard.*/
			OpenClipboard(HWND_DESKTOP);
			EmptyClipboard();
			/*Allocate global memory for command string*/
			HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, strlen(command) + 1);
			/*If global allocation fails then closeclipboard.*/
			if (!hg) {
				CloseClipboard();
				break;
			}
			/*Copy failed command string to GlobalLock*/
			memcpy(GlobalLock(hg), command, strlen(command) + 1);
			/*Unlock it.*/
			GlobalUnlock(hg);
			/*Set clipboard data with copied command in plain text format.*/
			SetClipboardData(CF_TEXT, hg);
			/*Close clipboard*/
			CloseClipboard();
			GlobalFree(hg);
		break;
		/*Handles window close*/
		case WM_CLOSE:
			PostQuitMessage(0);
	}
}
#endif

int startproc(char* command)
{
	/*Check if compiled on windows and use windows specific functions to create process.*/
	#if defined (_WIN32) || (_WIN64)
	//put quotations around path if not done already
	PROCESS_INFORMATION pif;  //Gives info on the thread and..
							  //..process for the new process
	STARTUPINFO si; //Defines how to start the program

	ZeroMemory(&si, sizeof(si)); //Zero the STARTUPINFO struct
	si.cb = sizeof(si);         //Must set size of structures

	BOOL bRet = CreateProcess(
		NULL, //Path to executable file
		command,   //Command string
		NULL,   //Process handle not inherited
		NULL,   //Thread handle not inherited
		FALSE,  //No inheritance of handles
		0,      //No special flags
		NULL,   //Same environment block as this prog
		NULL,   //Current directory - no separate path
		&si,    //Pointer to STARTUPINFO
		&pif);   //Pointer to PROCESS_INFORMATION
	#endif

	/*Checks if create process fails.*/
	if (bRet == FALSE)
	{
		//Checks if DEBUG global varialbe is 1 to print if CreateProcess failed for token
		if (g.DEBUG)
			printf("%s: Failed to execute %s\n", __func__, command);
		/*Initialize and declare components of error message.*/
		char firstPart[] = "Unable to start:";
		char thirdPart[] = "\nCopy failed command to clipboard?";
		/*Calculate the size of the errorMessage text using firstPart, command, and thirdPart*/
		size_t errorMessageSize = (sizeof(firstPart) / sizeof(firstPart[0])) + (strlen(command) + 1) + (sizeof(thirdPart) / sizeof(thirdPart[0]));
		/*Allocate memory for errorMessage*/
		char* errorMessage = malloc(errorMessageSize);
		/*Use sprintf_s to combine firstPart, command, and thirdPart into errorMessage string using errorMessageSize*/
		sprintf_s(errorMessage, errorMessageSize, "%s %s%s", firstPart, command, thirdPart);
		/*Check if compiled on windows and use windows specific functions to show messagebox and pass command string and errorMessage. 
		The command string is passed so that it can easily be copied to the clipboard and not need to mess with string manipulation.*/
		#if defined (_WIN32) || (_WIN64)
			messageBoxWin(command, errorMessage);
		#endif
		/*Frees errorMessage*/
		free(errorMessage);
		/*Return 1 because not successful*/
		return 1;
	}

	/*
	Check if compiled on windows and use windows specific functions to close handles to process and thread. 
	Just in case it needs to be open for error checking.
	*/
	#if defined (_WIN32) || (_WIN64)
		CloseHandle(pif.hProcess);   //Close handle to process
		CloseHandle(pif.hThread);    //Close handle to thread
	#endif

	return 0;
}

void ShowConsole()
{
	/*Check if compiled on windows and use windows specific functions to hide console.*/
	#if defined (_WIN32) || (_WIN64)
		ShowWindow(GetConsoleWindow(), SW_SHOW);
	#endif
}
void HideConsole()
{
	/*Check if compiled on windows and use windows specific functions to hide console.*/
	#if defined (_WIN32) || (_WIN64)
		ShowWindow(GetConsoleWindow(), SW_HIDE);
	#endif
}

int main(int argc, char** argv)
{
	/*Hide and show console functions were put into individual functions due to the latter of the two call overriding the former regardless if it was in an if 
	statement and the condition disallowed it. I don't know why this occurs, but it stops when the functions are called from proxies (such as HideConsole/ShowConsole).*/
	HideConsole(); //If debug was before this, the console window tends to have more of a chance to show up especially on a slower computer.
	g.DEBUG = 0; //sets DEBUG in global struct to 0 to set debugging to false as default.
	if (argc > 1)
	{
		/*Compares first argument with -d to check if debugging enabled was passed.*/
		if (strcmp(argv[1], "-d"))
			g.DEBUG = 1;
	}
	if (g.DEBUG)
	{
		/*Show console if debugging is enabled.*/
		ShowConsole();
	}

	/*
	Eventually this may be replaced with a preprocessor definition which will be copied into the character array. 
	In the future I want there to be a separate tool that will use msbuild and flags and take requested input and compile requested executable.
	Put desired commands and arguments to them in this string. Separate them by \n and make sure the base path to executable is surrounded in quotes.
	If you want to run a bat file or series of commands, you need to run cmd.exe /c start <batfile>. If that doesn't work, try full path to cmd.exe.
	example declaration & initialization:
	char commandString[] = "\"C:\\Program Files (x86)\\Mozilla Firefox\\firefox.exe\" https://example.org\n\"C:\\Windows\\System32\\cmd.exe\" /c start C:\\example.bat";
	
	The aforementioned initilization and declaration will allow the string will be parsed and launch firefox (in a new window if firefox is already running) 
	with a single tab showing example.org (to clarify, if multiple web addresses are used, they will open serially in a single window 
	(again new window if firefox is already running)) and then executes cmd.exe /c which then executes start and a a bat file located at the root of the C drive.
	*/
	char commandString[] = "";
	const char* delim = "\n";
	char* context = NULL; 
	char* token = strtok_s(commandString, delim, &context); //begins separating string by \n
	int ret = 0;
	/*Loops through token and modifies until token is NULL. For each token, the loop passes to startproc to create a process*/
	while (token != NULL)
	{
		ret = startproc(token);
		/*If ret is not 0 it startproc failed*/
		if (ret != 0)
		{
			if (g.DEBUG)
				printf("startproc returned %d.", ret);
		}
		token = strtok_s(NULL, delim, &context);
	}
	
	return ret;
}