#ifndef TICTOC_H_
#define TICTOC_H_

#include <time.h>

class tictoc {
private:
	string message, short_message;
	time_t start;
	int counter, step, step2, n;

public:
	tictoc(string msg, string short_msg) {
		message = msg;
		short_message = short_msg;
		
		start = 0;
	}

	void tic(int n = 0) {
		
		cerr<<"Starting tictoc: ["<<message<<" "<<short_message<<"]"<<endl;
		this->counter = 0;
		this->n = n;

		start = time(NULL);

		step = (n / 1000) + 1;

		step2 = (n / 10) + 1;
	}

	void update() {
		if (step > 1) {
			counter++;

			if ((counter % step) == 0) {
				cerr<<".";
			}

			if ((counter % step2) == 0) {
				if (((double) counter / n) >= 0.1) {
					cerr<<"["<<((counter * 100 / n))<< "%, " << ((time(NULL)
							- start)) << "s "<<short_message+"]";
				}
			}
		} else {
			cerr<<".";
		}
	}

	void toc() {

		int seconds = (int)((time(NULL) - start));
		
		cerr<<"Stopping clock ["<<message<<"]: "<<seconds<<" seconds"<<endl;
	}
};

#endif /*TICTOC_H_*/
