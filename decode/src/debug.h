#ifndef DEBUG_H_
#define DEBUG_H_

#include <string>
#include <iostream>
#include <sstream>

using namespace std;

class debug {
private:
	static time_t tics[1000];
	static int current_tic;
public:
	static void msg(string m) {
		cerr<<m<<endl;
	}
	
	static void report_parameter(string s, int v)
	{
		cerr<<s<<" = "<<v<<endl;
	}

	static void report_parameter(string s, string v)
	{
		cerr<<s<<" = "<<v<<endl;
	}

	static void report_parameter(string s, double v)
	{
		cerr<<s<<" = "<<v<<endl;
	}

	static void report_parameter(string s, bool v)
	{
		cerr<<s<<" = "<<(v ? "TRUE" : "FALSE")<<endl;
	}

	static int tic() {
		current_tic++;

		if ((current_tic < 0) || (current_tic > 999)) {
			current_tic = 0;
		}

		tics[current_tic] = time(0);

		ostringstream s;

		s<<"Timer "<<current_tic<<" started";

		debug::msg(s.str());
		
		return current_tic;
	}

	static void toc(int t) {
		time_t finish = time(0);

		ostringstream s;

		s<<"Timer "<<t<<" finished in "<<(finish-tics[t])<<" seconds";

		debug::msg(s.str());
	}
};

#endif /*DEBUG_H_*/
