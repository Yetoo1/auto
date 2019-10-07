#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

//make everything dynamic
//eventually make a struct to hold this information and make it so that sites defined here are default
//if in the event that the values cannot be loaded from external means
int main()
{
	//make this dynamic array as well so that we can eventually move onto dynamic array
	const char eclipsePath[] = "C:\\Apps\\eclipseJava\\eclipse.exe.lnk";
	const char firefoxPath[] = "\"C:\\Program Files (x86)\\Mozilla Firefox\\firefox.exe\"";
	//make firefoxSites dynamic
	const char firefoxSites[2][256] = {"https://canvas.cccd.edu", "https://google.com"};
	//const char firefoxSites[1][256]; 
	char* firefoxCommandFull = 0; 
	size_t firefoxSiteSize = 0;
	for (int i = 0; i < sizeof(firefoxSites) / sizeof(firefoxSites[0]); ++i)
	{
		firefoxSiteSize += strlen(firefoxSites[i] + 1); 
	}
	firefoxSiteSize += 1; //for terminating null 	
	
	//printf("%zu\n", sizeof(firefoxPath) + (firefoxSiteSize) + (sizeof(firefoxCommandFull)));
	
	firefoxCommandFull =  malloc(sizeof(firefoxPath) + firefoxSiteSize + (sizeof(firefoxCommandFull)));
	//printf("after firefoxCommandFull allocation %zu\n", _msize(firefoxCommandFull));
	 	
	 //This is better
	sprintf_s(firefoxCommandFull, _msize(firefoxCommandFull), "%s", firefoxPath);
	for (int i = 0; i < sizeof(firefoxSites) / sizeof(firefoxSites[0]); ++i)
	{
		sprintf_s(firefoxCommandFull, _msize(firefoxCommandFull), "%s %s", firefoxCommandFull, firefoxSites[i]);
	}
	//printf("%s\n", firefoxCommandFull);
	
	system(firefoxCommandFull);
	system(eclipsePath);
	free(firefoxCommandFull);
	return 0;
}