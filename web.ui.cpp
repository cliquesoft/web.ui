// web.ui.cpp	the main source code file to the project.
//
// created	2015/07/24 by Dave Henderson (dhenderson@cliquesoft.org or support@cliquesoft.org)
// updated	2015/08/27 by Dave Henderson (dhenderson@cliquesoft.org or support@cliquesoft.org)
//
// Unless a valid Cliquesoft Private License (CPLv1) has been purchased for your
// device, this software is licensed under the Cliquesoft Public License (CPLv2)
// as found on the Cliquesoft website at www.cliquesoft.org.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the appropriate Cliquesoft License for details.




// #include Definitions

#include "webkitfltk/webkit.h"
//#include "webkit.h"
#include <iostream>				// for strings to work correctly			do I need this????
#include <FL/Fl_Window.H>
#include <FL/filename.H>        		// for 'fl_filename_name' call
#include <string.h>           			// strncmp, strlen, etc.

#include <FL/Fl_PNG_Image.H>
#include "icon.h"

// REMOVED 2017/01/18 - no longer require this since moving to 'paged'
#include <stdio.h>				// these are for forkProcess()
#include <unistd.h>

// REMOVED 2017/01/18 - we didn't use this approach to the value of the -s param
//#include <sys/types.h>			// these are for -s
//#include <sys/stat.h>
//#include <unistd.h>

#include <errno.h>				// this is used for makePath()

using namespace std;




// Variable Definitions

static webview *wkScreen;			// stores the webkit screen to interact with
static const char *sProgram;			// stores this programs name
static const char *sURI;			// stores the URI to load
static Fl_PNG_Image *sIcon;			// stores this programs application icon (for the titlebar)
static const char *sTitle = "web.ui";		// stores the desired title of the window
// REMOVED 2017/01/18 - no longer require this since moving to 'paged'
static const char *sExecute;			// stores the desired software to execute
static const char *sHome;			// stores the users home directory
static const char *sStorage = "/.tmp/web.ui";	// the suffix of the directory that will store various files like cookies.dat file (sHome will be pre-pended)

// REMOVED 2017/01/18 - no longer require this since moving to 'paged'
vector<char*> child_argv(10);			// create an array to store the passed parameters when using the '-e' parameter to this program

// Main class

class webwindow: public Fl_Window {
public:
	webwindow(int w, int h): Fl_Window(w, h, sTitle) {
		view = new webview(0, 0, w, h);
		resizable(view);

		if (sIcon)
			icon(sIcon);
	}

	webview *view;
};




// Functions

static void dlFile(const char *url, const char *file) {
// this function processes any downloaded files
	printf("Download %s to %s finished.\n", url, file);
}


bool makePath(std::string path) {			// http://stackoverflow.com/questions/675039/how-can-i-create-directory-tree-in-c-linux
// this function works like 'mkdir -p' in a Linux CLI environment
	bool bSuccess = false;
	int nRC = ::mkdir(path.c_str(), 0775);
	if (nRC == -1) {
		switch (errno) {
			case ENOENT:		// parent didn't exist, try to create it
				if (makePath(path.substr(0, path.find_last_of('/'))))		// Now, try to create again.
					bSuccess = 0 == ::mkdir( path.c_str(), 0775 );
				else
					bSuccess = false;
				break;
			case EEXIST:		// the directory already exists, or we are done creating the path
				bSuccess = true;
				break;
			default:
				bSuccess = false;
				break;
		}
	} else { bSuccess = true; }

	return bSuccess;
// LEFT OFF - add the ability to pass a user-defined chmod value
}


// REMOVED 2017/01/18 - no longer require this since moving to 'paged'
int forkProcess() {
// if this software was called with a '-e' parameter, then start that program and exit this program!
// http://timmurphy.org/2014/04/26/using-fork-in-cc-a-minimum-working-example/
// http://faq.cprogramming.com/cgi-bin/smartfaq.cgi?answer=1044654269&id=1043284392
	pid_t pid;

	switch ((pid = fork())) {
		case -1:			// the fork() has failed
			perror("fork");
			break;
		case 0:				// this is processed by the child
			execv(sExecute, child_argv.data());
			printf("ERROR: The execution of the desired program has failed.");
			exit(EXIT_FAILURE);
			break;
		default:			// this is processed by the parent which we just want to return from this function
			exit(0);
			break;
	}
}


// REMOVED 2017/01/18 - we didn't use this approach to the value of the -s param
//int typeTarget(const char *path, int link) {
//// returns the type of target being passed: 0=absent, 1=directory, 2=file, 3=symlink, 4=block device, 5=character device, 6=named pipe, 7=socket
//// path 	the target path to test
//// link 	whether to include testing for symlinks or not
//	struct stat stat_info;
//
//	if (stat(path, &stat_info) == -1) { return 0; }						// if the path doesn't exist					WARNING: do this first to make sure that something does in fact exist (since this follows (sym)link chains)
//	if (link && lstat(path, &stat_info) == 0 && S_ISLNK(stat_info.st_mode)) { return 3; }	// if we need to test for symlink -AND- the path is a symlink	WARNING: this MUST come 2nd in the processing order!!!
//
//	stat(path, &stat_info);									// re-populate &stat_info
//	if (S_ISDIR(stat_info.st_mode)) { return 1; }						// if the path is a directory
//	if (S_ISREG(stat_info.st_mode)) { return 2; }						// if the path is a file
//	if (S_ISBLK(stat_info.st_mode)) { return 4; }						// if the path is a block device file
//	if (S_ISCHR(stat_info.st_mode)) { return 5; }						// if the path is a character device file
//	if (S_ISFIFO(stat_info.st_mode)) { return 6; }						// if the path is a named pipe
//	if (S_ISSOCK(stat_info.st_mode)) { return 7; }						// if the path is a socket file
//}


int procSwitches(int argc, char **argv, int &i) {
// process each argument given on the command line (via argv) and returns number of processed parameters, 0 on error
//	http://www.cplusplus.com/forum/articles/13355/
	const char *Param = argv[i];				// set a pointer to the passed argument value (argv[i]) to the 's' variable (as defined by the inclusion of the asterisk)

	if (Param[0] != '-') {					// if the first character of the passed argument (argv[i]) is not a dash, then...
		const char* prefix = strstr(Param, "http");
		if (Param == prefix) {				//   check if the 'Param' contains the 'http' prefix and if so, then...
			sURI = Param;				//   set the global variable to the passed URI
			i += 1;					//   increment the 'i' counter
			return 1;				//   return the number of processed parameters
		}
		return 0;					// otherwise, we have encountered an error (as any associated value is processed below)
	}
	Param++;						// if we've made it here, we need to process the argument (e.g. '-m'), not its associated value (e.g. '800x600') which is done further below

	// process single parameters:
	if (!strcmp(Param,"h")) {				// if the user passed the 'help' request, then...
		return 0;					//    exit the function (which will later exit the app)
	} else if (!strcmp(Param,"d")) {			// if the user passed the 'download' switch, then...
		wk_set_download_func(dlFile);			//   set the function to call when downloading
		return 1;					//   return the number of arguments processed
	}

	// process parameters with a value:
	const char *Value = argv[i+1];
	if (i >= argc-1 || !Value) { return 0; }		// all the rest need an argument, and if missing, indicate we have an error!

	if (*Param == 'i') {					// if we've encountered the 'icon' parameter, then...
		sIcon = new Fl_PNG_Image(Value);		//   assign its value to the appropriate variable
// REMOVED 2017/01/18 - no longer require this since moving to 'paged'
	} else if (*Param == 'e') {				// if we've encountered the 'execute' parameter, then...
		sExecute = Value;
// LEFT OFF - check that a path is part of the script/binary to execute (otherwise the fork will fail) - simply check for a preceeding . (e.g. ./child.exe) or / (e.g. /some/path/child.exe)
//		copy(argv + 2, argv + argc, begin(child_argv));	// copy the argv array without the first 2 values (so we only have the name and parameters of the program to execute)
//		child_argv.push_back(nullptr);			// terminate the array with a NULL pointer
		i=argc;						// these next two lines prevent any more cli parameters from being processed since everything after the '-e' is considered a parameter to that referenced program
		return argc;
//	} else if (*Param == 's') {				// if we've encountered the 'storage' parameter, then...
//		sStorage = Value;
	} else if (*Param == 't') {				// if we've encountered the 'title' parameter, then...
		sTitle = Value;
	} else {						// otherwise the switch is unrecognized, so indicate that!
		return 0;
	}

	i += 2;							// return the fact that we consumed 2 switches:
	return 2;
}

static webview *popup(const char *url) {
// create a new child browser window
	webwindow *win = new webwindow(800, 600);
	win->end();

	win->view->load(url);
	win->show();

	return win->view;
}



// Start the Program!

int main(int argc, char **argv) {

	// Load the default icon
	sIcon = new Fl_PNG_Image("default.png", icon_png, sizeof(icon_png));

	// process any passed parameters
	sProgram = fl_filename_name(argv[0]);			// store the name of the program in the 'sProgram' variable
	int i;							// used for processing each passed CLI switch within procSwitches()
	if (Fl::args(argc, argv, i, procSwitches) < argc) {	// process each of the passed arguments from the cli and show the following message upon any error	NOTE: the 'procSwitches()' function is the callback to 'Fl::args'	http://www.fltk.org/doc-1.1/Fl.html#Fl.args
		Fl::error(
			"\nThe syntax for this application is:\n"
			"   web.ui [OPTION] [URI]\n\n"
			"   web.ui is an extremely small program designed for working with webpage based\n"
			"   user interfaces similar to the Mozilla project known as 'Prism'. There are a\n"
			"   few command line switches that can be used to alter behavior.\n"
			"\nOperational switches:\n"
			"  -h[elp]\t\tDisplays this menu\n\n"
			"  -d[ownload]\t\tEnables web.ui to download files\n"
// REMOVED 2017/01/18 - no longer require this since moving to 'paged'
//			"  -e[xecute] FILE\tExecute a program (via the GUI)\n"			// this enables the GUI to start programs when they are clicked. so instead of doing something like "gnome-terminal -e '...'", web.ui can be used in a similar fashion "web.ui -e '...'"
// LEFT OFF - implement 'maxmized' and 'fullscreen' as a valid value to the -g parameter
			"  -g[eometry] WxH+X+Y\tWindow startup size and position\n"		// built-in: Sets the initial window position and size according the the standard X geometry string.
			"  -i[con] FILE\t\tSets the app icon contained in the titlebar\n"
			"  -s[torage] DIR\tSets the storage directory for system files;\n"	// for files like 'cookies.dat', browsing cache, etc
			"  -t[itle] STRING\tSets the title of the window\n"
		);
		exit(1);					// if we're in here, we've encountered an error, so exit this program
	}

// REMOVED 2017/01/18 - no longer require this since moving to 'paged'
	// if we need to execute a program and close...
//	if (sExecute != NULL) {					// if the '-e' parameter was passed, then...
//		forkProcess();
//		exit(0);					// this is here just as a safety net!				WARNING: this line should NEVER be reached, but it here just in case
//	}

	// check the environment before starting the software
	//std::string sStorageDir = string(sStorage);
	//if (string(sStorage) == "/.tmp/web.ui") {		// if the user didn't pass the -s parameter, then...
		if ((sHome = getenv("HOME")) == NULL) {		//   check that the $HOME environment variable is set		http://stackoverflow.com/questions/2910377/get-home-directory-in-linux-c
			printf("ERROR: The home directory of the user could not be determined!\n\nPlease set the HOME global variable before using web.ui.\n");
			exit(2);
		}
//		sStorage = sHome + sStorage;			//   update the sStorage value to now be a complete path
	//	sStorageDir = string(sHome) + string(sStorage);
	//}
// UPDATED 2017/01/18 - to use the variable value
	if (makePath(string(sHome) + "/.tmp/web.ui") == false) {		// set the path where cookies are stored
//	if (makePath(string(sStorage)) == false) {		// set the path where cookies and other data are stored
	//if (makePath(sStorageDir) == false) {		// set the path where cookies and other data are stored
		printf("ERROR: The storage directory tree could not be created.\n");
		exit(3);
	}

	// create the necessary variables for use later in the program		http://stackoverflow.com/questions/1995053/const-char-concatenation
// UPDATED 2017/01/18 - updated the approach
	int iSize = strlen(sHome) + strlen(sStorage) + 1;	// calculate the required buffer size (also accounting for the null terminator)
	char* sCookies = new char[iSize];			// create a new variable (allocating enough memory for the concatenated string)
	strcpy(sCookies, sHome);				// now copy the sHome 'const char' string into the sCookies variable
	strcat(sCookies, sStorage);				// now append the sStorage 'const char' string into the sCookies variable as well for a full path
	//int iSize = strlen(sStorage) + 1;			// calculate the required buffer size (also accounting for the null terminator)
	//char* sCookies = new char[iSize];			// create a new variable (allocating enough memory for the concatenated string)
	//strcpy(sCookies, sStorage);				// now copy the sStorage 'const char' string into the sCookies variable

	// initialize the webview since we need to start the UI!		NOTE: this was moved down here so if the '-e' parameter was passed we don't have to clean up any of this
	webkitInit();						// initialize webkit

	// make any adjustments to the software settings, before creating views...
	//wk_set_download_func(dlFile);				   set the function to call when downloading			NOTE: this was set in procSwitches() above
	wk_set_cookie_path(sCookies);				// set the directory where the cookie.dat file is stored
	wk_set_popup_func(popup);				// LK: popups
// LEFT OFF - set up the cache to work in ~/.tmp/web.ui/cache

	// Create the main window
	webwindow *winApp = new webwindow(800, 600);		// create a new window to contain web.ui with the default dimensions of 800x600
	wkScreen = winApp->view;				// save the main webkit object (wkScreen)
	winApp->end();
	winApp->show(argc, argv);

	if (sURI != NULL)					// if a URI was passed, then...
		wkScreen->load(sURI);
	else							// otherwise, show a blank screen
		wkScreen->load("about:blank");

	// run the web.ui application
	Fl::run();

	// Cleanup the app upon closing
	delete winApp;								// delete the web.ui window
	wk_drop_caches();							// webkit cache cleanup

	return 0;
}




// LEFT OFF - checking these two sites for relevent info
// https://pseudoscripter.wordpress.com/2014/04/02/fltk-hello-world/
// http://forum.tinycorelinux.net/index.php?topic=18541.0




// ADDITIONS:
//	[ ] prevent drag-n-drop files on the screen to load (html) files
//	[ ] -g[eometry] WxH+X+Y	specifies the size and offset of the application; 'maximized' loads maximized, 'fullscreen' loads fullscreen.
//	[ ] center any alerts (and remove the source from the return [e.g. http://127.0.0.1:12345/default.sh?blah...])
//	[ ] set the font correctly (or is this an issue with TC not having the proper fonts)
//	[ ] remove the right-click context menu; only allow shift-F5 to refresh (to prevent accidental refreshes by user)
//	[ ] disable ALL hotkey commands (alt-tab seems to mess up the display)
// REMOVED:
//	[ ] -d[ownload disable]	prevents downloading											THIS IS THE DEFAULT WAY, NO NEED FOR A SWITCH
//	[ ] prevent highlighting of text, images, etc											CONTROL THIS IN CSS
//	[ ] -b[rowser] loads the most basic broswer for emergency situations, contains: < > [URI		] B S			NOTE: the trailing [B]ookmarks and [S]ettings buttons are disabled; overrides the -d parameter
//		the entire thing is rendered in a webview.  the above bar is a <div> and the 'window' will be an iframe.  Cheap browser!  Useful for emergency situations
//		move where the cookies, cache, etc are stored so as to not infringe on web.ui apps


// CAN THIS COMPILE ON WINDOWS?  This can be useful with clients and other of our software offerings.

// can we allow project css files to manipulate the alert(), confirm(), and prompt() modals?  e.g. #cdbAlert { for overall <div> }, #cdbAlert p { for message area }, #cdbAlert #buttons { for buttons wrapper div }, #cdbAlert .button { for button styling }, ...
// this way each individual project can tailor the modals to look like their own projects instead of having a global appearence
