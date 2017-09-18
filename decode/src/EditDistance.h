#ifndef __editdistance_h__
#define __editdistance_h__

#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <string.h>
#include <fstream>
#include <unistd.h>
#include <malloc.h>

using namespace std;

class EditDistanceCalculator {

public:

	int ** v;
	int ** left_ptr;
	int ** right_ptr;
	int ** action;

	int mCapacity, nCapacity;

	bool transposition;

	EditDistanceCalculator()
	{
		init(1000, 1000);
	}

	void init(int m, int n) {

		mCapacity = m;
		nCapacity = n;

		v = new int*[m+1];
		left_ptr = new int*[m+1];
		right_ptr = new int*[m+1];
		action = new int*[m+1];

 	       for (int i = 0; i <= m; i++) {
			v[i] = new int[n+1];
			left_ptr[i] = new int[n+1];
			right_ptr[i] = new int[n+1];
			action[i] = new int[n+1];
        	}
	}

	virtual ~EditDistanceCalculator()
	{
		destroy();
	}

	void destroy()
	{
		int m = mCapacity;
		int n = nCapacity;

 	       for (int i = 0; i <= m; i++) {
			delete [] v[i];
			delete [] left_ptr[i];
			delete [] right_ptr[i];
			delete [] action[i];
        	}

		delete [] v;
		delete [] left_ptr;
		delete [] right_ptr;
		delete [] action;
	}

int count(string s1, string s2, string& ret_s1, string& ret_s2)
{
        int m = s1.length();
        int n = s2.length();

	bool changed = false;
	if (m >= mCapacity) {
		m = mCapacity + 100;
		changed = true;
	}

	if (n >= nCapacity) {
		n = nCapacity + 100;
		changed = true;
	}

	if (changed) { cerr<<"Re-initializing buffers."<<endl; destroy(); init(m, n); }

        for (int i = 0; i <= m; i++) {
                v[i][0] = i;
		// TODO
		// should be i == 0 ? 'F' : 'i'
		action[i][0] = 'F';
        }

        for (int j = 0; j <= n; j++) {
                v[0][j] = j;
		// TODO
		// should be j == 0 ? 'F' : 'd'
		action[0][j] = 'F';
        }
 
        for (int i = 1; i <= m; i++) {
                for (int j = 1; j <= n; j++) {

                        if (s1[i-1] == s2[j-1]) {
				v[i][j] = v[i-1][j-1];

				left_ptr[i][j] = i-1;
				right_ptr[i][j] = j-1;
				action[i][j] = 'm';
			} else {
				v[i][j] = 1;

				int a = v[i][j-1];
				int b = v[i-1][j];
				int c = v[i-1][j-1];

				if (a < b) {
					if (a < c) {
						v[i][j] += a;
						left_ptr[i][j] = i;
						right_ptr[i][j] = j-1;
						action[i][j] = 'd';
					} else {
						v[i][j] += c;
						left_ptr[i][j] = i-1;
						right_ptr[i][j] = j-1;
						action[i][j] = 's';
					}
				} else {
					if (b < c) {
						v[i][j] += b;
						left_ptr[i][j] = i-1;
						right_ptr[i][j] = j;
						action[i][j] = 'i';
					} else {
						v[i][j] += c;
						left_ptr[i][j] = i-1;
						right_ptr[i][j] = j-1;
						action[i][j] = 's';
					}
				}

				if ((i >= 2) && (j >= 2)) {
					if ((transposition) && (s1[i-1] == s2[j-2]) && (s1[i-2] == s2[j-1])) {
						if (v[i-2][j-2] + 1 < v[i][j]) {
							v[i][j] = 1 + v[i-2][j-2];
							left_ptr[i][j] = i-2;
							right_ptr[i][j] = j-2;
							action[i][j] = 't';
						}
					}
				}
			}
                }
        }

	int i = m;
	int j = n;

	vector<char> input;
	vector<char> output;

	char a = action[i][j];
	vector<char> actions;

	while (a != 'F') 
	{
		a = action[i][j];

		if (a != 'F') {
		actions.push_back(a);
		}

		if (a == 'm') {
			input.push_back(s1[i-1]);
			output.push_back(s2[j-1]);
		}

		if (a == 'd') {
			input.push_back('-');
			output.push_back(s2[j-1]);
		}

		if (a == 's') {
			input.push_back(s1[i-1]);
			output.push_back(s2[j-1]);
		}

		if (a == 'i') {
			input.push_back(s1[i-1]);
			output.push_back('-');
		}

		if (a == 't') {
			input.push_back(s1[i-1]);
			input.push_back(s1[i-2]);
			output.push_back(s2[j-1]);
			output.push_back(s2[j-2]);
		}

		/*if (a == 'F') {
			input.push_back(s1[i-1]);
			output.push_back(s2[j-1]);
		}*/

		int old_i = i;
		int old_j = j;
		i = left_ptr[old_i][old_j];
		j = right_ptr[old_i][old_j];

		/*if (i == old_i) { input.push_back('-'); }
		if (j == old_j) { output.push_back('-'); }*/
	}

	ret_s1 = "";
	for (int i=input.size()-1; i>=0; i--)
	{
		if (i < input.size()-1) { ret_s1 += ' '; }
		ret_s1 = ret_s1 + input[i];
	}

	ret_s2 = "";
	for (int i=output.size()-1; i>=0; i--)
	{
		if (i < output.size()-1) { ret_s2 += ' '; }
		ret_s2 = ret_s2 + output[i];
	}

        return v[m][n];
}

	void doEditDistance(string filename, string dictFile, bool useDict, bool outputBacktrace, bool doTransposition)
	{
		vector<string> all_dict;

		transposition = doTransposition;

		if (useDict) {
		      ifstream myfile(dictFile.c_str());

		      if (myfile.is_open()) {
			string line;
		        while (getline(myfile, line)) {
				all_dict.push_back(line);
            		}

      	    		myfile.close();
      		      } else {
				cerr<<"Could not open "<<dictFile<<endl;
				return;
			}

			cerr<<"Read dictionary file "<<dictFile<<endl;
		}


		cerr<<"Opening "<<filename<<endl;
	      ifstream myfile(filename.c_str());

		double total_count = 0;
		double total_edit = 0;

		if (myfile.is_open()) {
			string line;
			int lineNumber = 0;
			while (getline(myfile, line)) {
					lineNumber++;

					cerr<<"Reading line "<<lineNumber<<": "<<line<<endl;

					int m = 10000;
					char* str = strdup(line.c_str());
					char* str1 = strdup(strtok(str, "\t"));

					string s1;
					string s2;

					string last_string = "";

					int c = 0;

					if (useDict) {
						for (int i=0; i < all_dict.size(); i++) {
							c = count(str1, all_dict[i], s1, s2);

							if (c < m) {
								last_string = all_dict[i];
								m = c;
							}
						}

						cout<<last_string<<endl;
						cerr<<"Chosen string: "<<last_string<<endl;
					} else {
						char* str2 = strdup(strtok(NULL, "\t"));
						c = count(str1, str2, s1, s2);
						if (outputBacktrace)
						{
							cout<<s1<<endl<<s2<<endl;
						}

						cerr<<s1<<endl<<s2<<endl;
						if (str2 != NULL) { free(str2); }
					}

					cerr<<"Number of edit operations: "<<c<<endl;
					total_edit += c;
					total_count++;

					free(str1);
					free(str);
	    		}


			cerr<<"Average distance: "<<(total_edit / total_count)<<endl;
      	    		myfile.close();
      		}
	}

};

#endif
