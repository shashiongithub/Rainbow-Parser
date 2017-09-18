/*
 * ChartProjectionCKY.h
 *
 *  Created on: Sep 17, 2015
 *      Author: scohen
 */

#ifndef CHARTPROJECTIONCKY_H_
#define CHARTPROJECTIONCKY_H_

#include "grammar.h"
#include <time.h>     
#include <cmath>

class ChartProjectionCKY {
public:

	// TODO
	// what does the chart need to look like?
	// meaning, what is the data structure for it?

	// leftInsideChart[a][i][j] is a pointer to a vector that gives the inside probabilities for all left end points
	unordered_map<inpre_symbol, double***> leftInsideChart;
	unordered_map<inpre_symbol, double***> leftInsideChart2;

	// rightInsideChart[a][i][j] is a pointer to a vector that gives the outside probabilities for all right end points
	unordered_map<inpre_symbol, double***> rightInsideChart;
	unordered_map<inpre_symbol, double***> rightInsideChart2;

	int sentLength;

	cnf_vocab_reducer vocab;

	bool noU;

	ChartProjectionCKY() {
		pGrammar = NULL;
		pAlphabet = NULL;

		realSR = new real_semiring();
	}

	virtual ~ChartProjectionCKY() {
		// delete all tables

		delete realSR;
		delete pGrammar;
	}

	cnf_grammar<inpre_symbol, vocab_symbol, my_double> *pGrammar;

	cnf_grammar_alphabet* pAlphabet;

	tensor_semiring tensor_sr;

	real_semiring* realSR;

	array_2d<double> U;

	array_2d<double>** V;

	double delta, eps;

	array_2d<double> getV(int i, int j)
	{
		return V[i][j];
	}

	void parseFile(penn_treebank* ptb, string grammarFile, string vocabFile,
			int threshold, int k, double es, double dt, int avgCount, int m2, int largest) {


		cerr<<"Sampling Gaussians..."<<endl;

		V = new array_2d<double>*[largest];

		for (int i=0; i < largest; i++)
		{
			V[i] = new array_2d<double>[largest];

			cerr<<i<<"/"<<largest<<endl;

			for (int j=i; j < largest; j++)
			{
				V[i][j] = memutils::create_2d_array(largest, m2, 0.0);

                    		for (int k=0; k < largest; k++)
                        	{
                               		for (int k2=0; k2 < m2; k2++) {
                                        	V[i][j][k][k2] = gaussian() / sqrt(m2);
                                	}
                        	}
			}
		}
		cerr<<"Done sampling Gaussians"<<endl;

		pAlphabet = new cnf_grammar_alphabet();

		delta = dt;
		eps = es;

		tensor_semiring tensor_sr;

		cerr << "epsilon = " << eps << " delta = " << dt << endl;

		latent_variable_cnf_grammar grammar(&tensor_sr, pAlphabet);

		grammar.parse_grammar_file(grammarFile);

		pGrammar = grammar.create_double_grammar(realSR);

		vocab.load(vocabFile);

		vocab.set_threshold(threshold);

		ptb->reset_iterator();

		int sentNum = 0;

		while (ptb->next_tree()) {
			sentNum++;

			auto tree = ptb->current_penn_tree();

			vocab.modify_tree(tree);

			vector<string> pre = tree->get_terminal_yield();

			cerr << "Sentence " << sentNum << ":";
			for (int i = 0; i < pre.size(); i++) {
				cerr << " " << pre[i];
			}
			cerr << endl;

			double d = 0.0;

			clock_t t1;

			t1 = clock();

			int totalSoFar = 1;

			if (avgCount > 0) {
				for (int i = 0; i < avgCount; i++) {
					double d_ = parse(pre, k, m2);

					if (abs((double)(d_ - d / totalSoFar)) < 10.0) {
						d += d_;
						totalSoFar++;
					}
				}
			} else {
				d = 1.0;
				totalSoFar = 2.0;
			}

			totalSoFar--;

			t1 = clock() - t1 + 1;

			double t1_ = 0.0;
			if (totalSoFar > 0) {
				t1_ = t1 / totalSoFar;

				d = d / totalSoFar;
			}

			double d2;

			clock_t t2;

			t2 = clock();

			d2 = parse(pre, 0, 0);

			if (d2 == 0.0) {
				d2 = 0.0001;
			}

			t2 = clock() - t2 + 1;

			cerr << "Len: " << pre.size() << "  numRepetitions: " << totalSoFar
					<< "  average: " << d << "  true: " << d2 << " avgTime: "
					<< t1_ << "  totalTime: " << t1 << "  totalTimeTrue: "
					<< t2 << "  eps/true: " << sqrt((d - d2) * (d - d2)) / d2
					<< "  speed-up(true/approx):" << ((0.0 + t2) / t1) << endl;
		}
	}

	double gaussian() {

		double u = ((double) rand() / (RAND_MAX)) * 2 - 1;
		double v = ((double) rand() / (RAND_MAX)) * 2 - 1;
		double r = u * u + v * v;
		if (r == 0 || r > 1)
			return gaussian();
		double c = sqrt(-2 * log(r) / r);
		return u * c;
	}

	int rejuvenateU(int k, int n) {
		int m;

		U.destroy();

		if (k == 0) {
			m = n + 1;
			noU = true;
		} else {
			if (k == 1) {
				m = (int) (1 + 4 * (2 * log((n / sqrt(2)) + log(4 / delta)))
						/ (eps * eps));
			} else {
				m = k;
			}

			noU = false;

			U = memutils::create_2d_array(m, n + 1, 0.0);

			for (int i = 0; i < U.rows(); i++) {
				for (int j = 0; j < U.cols(); j++) {
					U[i][j] = gaussian() / sqrt(m);
				}
			}

			//cerr<<"For n="<<n<<" chose m="<<m<<endl;
		}

		return m;
	}

	double parse(vector<string> tokens, int k, int m2) {

		int n = tokens.size();

		int m = rejuvenateU(k, n);

		unordered_map<inpre_symbol, double> alpha;

		auto& interminals = pGrammar->alphabet()->nonterminals();

		for (auto a_iter = interminals.begin(); a_iter != interminals.end(); a_iter++) {
			auto a = *a_iter;

			alpha[a] = 0.0;

			leftInsideChart[a] = new double**[n + 1];
			rightInsideChart[a] = new double**[n + 1];

			for (int i = 0; i <= n; i++) {
				leftInsideChart[a][i] = new double*[n + 1];
				rightInsideChart[a][i] = new double*[n + 1];

				for (int j = 0; j <= n; j++) {
					leftInsideChart[a][i][j] = new double[m];
					rightInsideChart[a][i][j] = new double[m];

					for (int k = 0; k < m; k++) {
						leftInsideChart[a][i][j][k] = 0.0;
						rightInsideChart[a][i][j][k] = 0.0;
					}
				}
			}
		}

		auto& preterminals = pGrammar->alphabet()->preterminals();

		for (auto a_iter = preterminals.begin(); a_iter != preterminals.end(); a_iter++) {
			auto a = *a_iter;

			alpha[a] = 0.0;

			leftInsideChart[a] = new double**[n + 1];
			rightInsideChart[a] = new double**[n + 1];

			for (int i = 0; i <= n; i++) {
				leftInsideChart[a][i] = new double*[n + 1];
				rightInsideChart[a][i] = new double*[n + 1];

				for (int j = 0; j <= n; j++) {
					leftInsideChart[a][i][j] = new double[m];
					rightInsideChart[a][i][j] = new double[m];

					for (int k = 0; k < m; k++) {
						leftInsideChart[a][i][j][k] = 0.0;
						rightInsideChart[a][i][j][k] = 0.0;
					}
				}
			}
		}

		// second stage initialization
		//
		//

		if (m2 > 0) {

		for (auto a_iter = interminals.begin(); a_iter != interminals.end(); a_iter++) {
			auto a = *a_iter;

			alpha[a] = 0.0;

			leftInsideChart2[a] = new double**[n + 1];
			rightInsideChart2[a] = new double**[n + 1];

			for (int i = 0; i <= n; i++) {
				leftInsideChart2[a][i] = new double*[n + 1];
				rightInsideChart2[a][i] = new double*[n + 1];

				for (int j = 0; j <= n; j++) {
					leftInsideChart2[a][i][j] = new double[m2];
					rightInsideChart2[a][i][j] = new double[m2];

					for (int k = 0; k < m2; k++) {
						leftInsideChart2[a][i][j][k] = 0.0;
						rightInsideChart2[a][i][j][k] = 0.0;
					}
				}
			}
		}

		for (auto a_iter = preterminals.begin(); a_iter != preterminals.end(); a_iter++) {
			auto a = *a_iter;

			alpha[a] = 0.0;

			leftInsideChart2[a] = new double**[n + 1];
			rightInsideChart2[a] = new double**[n + 1];

			for (int i = 0; i <= n; i++) {
				leftInsideChart2[a][i] = new double*[n + 1];
				rightInsideChart2[a][i] = new double*[n + 1];

				for (int j = 0; j <= n; j++) {
					leftInsideChart2[a][i][j] = new double[m2];
					rightInsideChart2[a][i][j] = new double[m2];

					for (int k = 0; k < m2; k++) {
						leftInsideChart2[a][i][j][k] = 0.0;
						rightInsideChart2[a][i][j][k] = 0.0;
					}
				}
			}
		}

		}


		for (int len = 1; len <= n; len++) {

			/*
			 cerr<<"len: "<<(len-1)<<endl;
			 for (auto a_i = leftInsideChart.begin(); a_i
			 != leftInsideChart.end(); a_i++) {
			 auto a = (*a_i).first;
			 for (int i = 0; i <= n; i++) {
			 for (int j = 0; j <= n; j++) {
			 for (int k = 0; k < m; k++) {
			 if ((leftInsideChart[a][i][j][k] > 0)
			 || (rightInsideChart[a][i][j][k] > 0)) {
			 cerr << "a:" << a.sym() << " " << "i:" << i << " "
			 << "j:" << j << " " << "k:" << k << " ";
			 cerr << "left:" << leftInsideChart[a][i][j][k]
			 << " ";
			 cerr << "right:"
			 << rightInsideChart[a][i][j][k] << endl;
			 }
			 }
			 }
			 }
			 }*/

			if (len == 1) {
				for (int i = 0; i < n; i++) {
					string word = tokens[i];

					int j = i + len;

					auto preterms = pGrammar->preterminal_rules_with_term(
							pGrammar->alphabet()->vocab_lookup_string(word));

					for (auto iter = preterms.begin(); iter != preterms.end(); iter++) {
						auto a = (*iter).a();

						for (int k = 0; k < m; k++) {
							leftInsideChart[a][i][i + 1][k] += getU(i, k)
									* pGrammar->weight(*iter);
							rightInsideChart[a][i][i + 1][k] += getU(i + 1, k)
									* pGrammar->weight(*iter);
						}
					}

					if (j < n) {
						// update the rest of the tables
						for (int k = 0; k < m; k++) {
							double u = getU(j, k);

							for (auto a_iter = rightInsideChart.begin(); a_iter
									!= rightInsideChart.end(); a_iter++) {
								auto a = (*a_iter).first;
								rightInsideChart[a][i][j + 1][k]
										= rightInsideChart[a][i][j][k];
							}
						}
					}

					if (i > 0) {
						for (int k = 0; k < m; k++) {
							double u = getU(i, k);

							for (auto a_iter = leftInsideChart.begin(); a_iter
									!= leftInsideChart.end(); a_iter++) {
								auto a = (*a_iter).first;
								leftInsideChart[a][i - 1][j][k]
										= leftInsideChart[a][i][j][k];
							}
						}
					}


					if ((m2 > 0) && (j - i + 1 > m2)) {
						auto V = getV(i,j);

						/*memutils::create_2d_array(m, m2, 0.0);

                    				for (int k=0; k < V.rows(); k++)
                        			{
                               				 for (int k2=0; k2 < V.cols(); k2++) {
                                        			V[k][k2] = gaussian() / sqrt(m2);
                                			}
                        			}
						*/
					for (auto a_iter = leftInsideChart.begin(); a_iter != leftInsideChart.end(); a_iter++) {

						auto a = (*a_iter).first;

						// sample V
						//

						for (int k=0; k < m; k++) {
							for (int k2 = 0; k2 < m2; k2++) {
								leftInsideChart2[a][i][j][k2] += leftInsideChart[a][i][j][k] * V[k][k2];
								rightInsideChart2[a][i][j][k2] += rightInsideChart[a][i][j][k] * V[k][k2];
							}
						}

					}
						//V.destroy();
					}
				}
			} else {
				for (int i = 0; i < n - len + 1; i++) {
					int j = i + len;

					if ((m2 > 0) && (j - i + 1 > m2)) {
						//auto V = memutils::create_2d_array(m, m2, 0.0);
						auto V = getV(i,j);

                    				/*for (int k=0; k < V.rows(); k++)
                        			{
                               				 for (int k2=0; k2 < V.cols(); k2++) {
                                        			V[k][k2] = gaussian() / sqrt(m2);
                                			}
                        			}*/
					for (auto a_iter = leftInsideChart.begin(); a_iter != leftInsideChart.end(); a_iter++) {
						// sample V
						//

						auto a = (*a_iter).first;


						for (int k=0; k < m; k++) {
							for (int k2 = 0; k2 < m2; k2++) {
								leftInsideChart2[a][i][j][k2] += leftInsideChart[a][i][j][k] * V[k][k2];
								rightInsideChart2[a][i][j][k2] += rightInsideChart[a][i][j][k] * V[k][k2];
							}
						}

					}
						//V.destroy();
					}

					// alpha is alpha_a_ij
					for (auto a_iter = alpha.begin(); a_iter != alpha.end(); a_iter++) {
						(*a_iter).second = 0.0;
					} // for alpha


					for (auto b_iter = leftInsideChart.begin(); b_iter
							!= leftInsideChart.end(); b_iter++) {
						auto b = (*b_iter).first;

						auto leftVec = ((m2 > 0) && (j - i + 1 > m2)) ? leftInsideChart2[b][i][j] : leftInsideChart[b][i][j];

						if (leftVec != NULL) {
							auto& ruleSet = pGrammar->binary_rules_with_b(b);

							for (auto rule_iter = ruleSet.begin(); rule_iter
									!= ruleSet.end(); rule_iter++) {

								auto rule = (*rule_iter);

								auto c = rule.c();
								auto a = rule.a();

								auto rightVec = ((m2 > 0) && (j - i + 1 > m2)) ? rightInsideChart2[c][i][j] : rightInsideChart[c][i][j];

								if (rightVec != NULL) {
									for (int k = 0; k < ((m2 > 0) && (j - i + 1 > m2)) ? m2 : m; k++) {
										if ((leftVec[k] > 0) && (rightVec[k] > 0)) {
											alpha[a] += leftVec[k] * rightVec[k] * pGrammar->weight(rule);
										}
									/* OLD STUFF for (int k = 0; k < m; k++) {
										alpha[a] += leftVec[k] * rightVec[k]
												* pGrammar->weight(rule);
									*/
									} // for dot product
								} // rightVec != null
							} // for c
						} // leftVec != null
					} // for b

					if (j < n) {
						// update the rest of the tables
						for (int k = 0; k < m; k++) {
							double u = getU(j, k);

							for (auto a_iter = rightInsideChart.begin(); a_iter
									!= rightInsideChart.end(); a_iter++) {
								auto a = (*a_iter).first;

								leftInsideChart[a][i][j + 1][k]
										= leftInsideChart[a][i][j][k] + u
												* alpha[a];
							}
						}
					}

					if (i > 0) {
						for (int k = 0; k < m; k++) {
							double u = getU(i, k);

							for (auto a_iter = leftInsideChart.begin(); a_iter
									!= leftInsideChart.end(); a_iter++) {
								auto a = (*a_iter).first;

								rightInsideChart[a][i - 1][j][k]
										= rightInsideChart[a][i][j][k] + u
												* alpha[a];
							}
						}
					}

				} // for i
			} // else
		} // for len

		double d = 0.0;

		for (auto a_iter = rightInsideChart.begin(); a_iter
				!= rightInsideChart.end(); a_iter++) {
			auto a = (*a_iter).first;

			double* l = leftInsideChart[a][0][n];
			double* r = rightInsideChart[a][0][n];

			double s = 0.0;

			if ((l != NULL) && (r != NULL)) {
				for (int k = 0; k < m; k++) {
					s += l[k] * r[k] * pGrammar->weight(a);
					//cerr<<"multiplying "<<l[k]<<" by "<<r[k]<<" by "<<pGrammar->weight(a)<<endl;
				}
			}

			//cerr<<"a:"<<pGrammar->alphabet()->nonterminal_to_string(a)<<" sum="<<s<<endl;

			d += s;
		}

		U.destroy();

		for (auto a_iter = rightInsideChart.begin(); a_iter
				!= rightInsideChart.end(); a_iter++) {

			auto a = (*a_iter).first;

			double*** l = rightInsideChart[a];

			for (int i = 0; i < n + 1; i++) {
				for (int j = 0; j < n + 1; j++) {
					delete l[i][j];
				}

				delete l[i];
			}

			delete l;
		}

		for (auto a_iter = leftInsideChart.begin(); a_iter
				!= leftInsideChart.end(); a_iter++) {

			auto a = (*a_iter).first;

			double*** l = leftInsideChart[a];

			for (int i = 0; i < n + 1; i++) {
				for (int j = 0; j < n + 1; j++) {
					delete l[i][j];
				}

				delete l[i];
			}

			delete l;
		}

		return d;
	}

	double getU(int i, int k) {
		if (noU) {
			if (i == k) {
				return 1.0;
			} else {
				return 0.0;
			}
		} else {
			return U[k][i];
		}
	}

private:
};

#endif /* CHARTPROJECTIONCKY_H_ */

