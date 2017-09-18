#ifndef __variable_table_h__
#define __variable_table_h__

#include "types.h"
#include <iostream>
//#include "hashes.h"
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#include <string>
#include <vector>
#include "utils.h"


using namespace std::tr1;
using namespace std;

class VariableTable {
private:
	unordered_map<string, bool> booleanTable;
	unordered_map<string, double> doubleTable;
	unordered_map<string, int> intTable;
	unordered_map<string, string> stringTable;

public:
	void analyzeLine(const string& line) {

		if (line.find('#') == 0)
		{
			return;
		}

		vector<string> output;

		string_utils::split(output, line, " ");

		if (output.size() != 3) {
			cerr << "Warning: cannot parse input command " << line << endl;

			return;
		}

		if (output[1] == "bool") {
			booleanTable[output[0]] = (output[2] == "false") ? false : true;
		}

		if (output[1] == "string") {
			stringTable[output[0]] = output[2];
		}

		if (output[1] == "int") {
			intTable[output[0]] = stoi(output[2].c_str());
		}

		if (output[1] == "double") {
			doubleTable[output[0]] = stod(output[2].c_str());
		}
	}

	string getStringValue(string var, string defaultValue,
			bool breakIfNotFound, string messageIfNotFound) {
		if (stringTable.find(var) == stringTable.end()) {
			if (breakIfNotFound) {
				cerr << "Variable " << var << " could not be found. Exiting."
						<< endl;

				exit(0);
			} else {
				return defaultValue;
			}
		}

		return stringTable[var];
	}

	bool getBooleanValue(string var, bool defaultValue, bool breakIfNotFound,
			string messageIfNotFound) {
		if (booleanTable.find(var) == booleanTable.end()) {
			if (breakIfNotFound) {
				cerr << "Variable " << var << " could not be found. Exiting."
						<< endl;

				exit(0);
			} else {
				return defaultValue;
			}
		}

		return booleanTable[var];
	}

	double getDoubleValue(string var, double defaultValue,
			bool breakIfNotFound, string messageIfNotFound) {
		if (doubleTable.find(var) == doubleTable.end()) {
			if (breakIfNotFound) {
				cerr << "Variable " << var << " could not be found. Exiting."
						<< endl;

				exit(0);
			} else {
				return defaultValue;
			}
		}

		return doubleTable[var];
	}

	int getIntValue(string var, int defaultValue, bool breakIfNotFound,
			string messageIfNotFound) {
		if (intTable.find(var) == intTable.end()) {
			if (breakIfNotFound) {
				cerr << "Variable " << var << " could not be found. Exiting."
						<< endl;

				exit(0);
			} else {
				return defaultValue;
			}
		}

		return intTable[var];
	}
};

#endif
