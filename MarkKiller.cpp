#include <stdio.h>
#include <iostream>
#include <xmarker/xmarker.h>

#define DLIB_NO_GUI_SUPPORT

#include <dlib/cmd_line_parser.h>

using namespace std;

int main(int argc, char ** argv)
{
    try
    {
#ifdef WIN32
		freopen("NUL","w",stderr);
#else
		freopen("/dev/null","w",stderr);
#endif

        CXMarker xMarker;

        cout << endl << endl << "MarkKiller v0.2" << endl << endl;


        if(argc != 2)
        {
            cout << endl << "usage: MarkKiller [config-file-path]" << endl << endl;

            return -1;
        }

        cout << "--> Reading configuration.." << endl;

        if(!xMarker.ReadConfiguration(argv[1]))
        {
            cout << "--> Failed!" << endl << endl;
            return -1;
        }

        cout << "--> Initializing source stream.." << endl;

        if(!xMarker.OpenSource())
        {
            cout << "--> Failed!" << endl << endl;
            return -1;
        }

        cout << "--> Initializing target stream.." << endl;

        if(!xMarker.OpenTarget())
        {
            cout << "--> Failed!" << endl << endl;
            return -1;
        }

		cout << "--> Processing..." << endl;

		if (!xMarker.Perform())
		{
			cout << "--> Failed!" << endl << endl;
			return -1;
		}

    }
    catch(exception& e)
    {
        cout << "--> Failure:" << e.what();
        return -1;
    }

    return 0;
}