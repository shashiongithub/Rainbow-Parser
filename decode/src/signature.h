#ifndef SIGNATURE_H_
#define SIGNATURE_H_

#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <fstream>

#include "table_variable.h"

class signature {
public:
	static VariableTable vars;

	static void state_signature(int argc, char** argv) {

		time_t rawtime;
		struct tm * timeinfo;

		time(&rawtime);
		timeinfo = localtime(&rawtime);

		char hostname[1024];
		gethostname(hostname, 1024);

		cerr << "============================" << endl;

		cerr << "Loading " << argv[0] << " on " << asctime(timeinfo) << endl;

		cerr << "Machine name: " << hostname << endl;

		cerr << "Process ID: " << getpid() << endl;

		cerr << "Command line:";

		for (int i = 1; i < argc; i++) {
			cerr << " " << argv[i];
		}

		cerr << endl;

		cerr << "Text variables: " << endl;

		ifstream myfile(argv[1]);

		if (myfile.is_open()) {
			for (std::string line; std::getline(myfile, line);) {
				cerr << line << endl;
				vars.analyzeLine(line);
			}
		} else {
			for (std::string line; std::getline(std::cin, line);) {
				cerr << line << endl;
				vars.analyzeLine(line);
			}
		}

		cerr << "============================" << endl << endl;
	}
};

#endif /*SIGNATURE_H_*/
