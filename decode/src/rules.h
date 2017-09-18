#ifndef RULES_H_
#define RULES_H_

#include <tr1/unordered_map>
#include <functional>

#include "utils.h"

class cfg_rule {

};

template <class T> class arbitrary_cfg_rule : cfg_rule {
private:
	array_1d<T> _rhs;
	T _lhs;

public:
	arbitrary_cfg_rule<T>(T _lhs, array_1d<T> _rhs) {
		this->_rhs = _rhs;
		this->_lhs = _lhs;
	}

	int rhs_length() const {
		return _rhs.length();
	}

	T rhs(int i) const {
		return _rhs[i];
	}

	T lhs() const {
		return _lhs;
	}

	bool operator==(const arbitrary_cfg_rule<T>& rule) const {

		if (!(rule.lhs() == this->lhs())) {
			return false;
		}
		if (!(rule.rhs_length() == this->rhs_length())) {
			return false;
		}

		for (my_uint i=0; i < this->rhs_length(); i++) {
			if (!(this->rhs(i) == rule.rhs(i))) {
				return false;
			}
		}

		return true;
	}

	bool operator!=(const arbitrary_cfg_rule<T>& rule) const {
		return (!(*this == rule));
	}

};

template <class T> class cnf_rule : public cfg_rule {

};

template <class T> class cnf_binary_rule : public cnf_rule<T> {
private:
	T _a, _b, _c;

public:
	cnf_binary_rule(T _a, T _b, T _c) {
		this->_a = _a;
		this->_b = _b;
		this->_c = _c;
	}

	T a() const {
		return _a;
	}

	T b() const {
		return _b;
	}

	T c() const {
		return _c;
	}

	bool operator==(const cnf_binary_rule<T>& rule) const {
		return ((a() == rule.a()) && (b() == rule.b()) && (c() == rule.c()));
	}
};

template <class T, class E> class cnf_term_rule : public cnf_rule<T> {
private:
	T _a;
	E _x;

public:
	cnf_term_rule(T __a, E __x) :
		_a(__a), _x(__x) {
	}

	T a() const {
		return _a;
	}

	E x() const {
		return _x;
	}

	bool operator==(const cnf_term_rule<T, E>& r) const {
		return ((r.x() == x()) && (r.a() == a()));
	}
};

namespace std {
namespace tr1 {
template<class T, class E> struct hash<cnf_term_rule<T, E> > {
public:
	size_t operator()(const cnf_term_rule<T, E>& k) const {
		return ((hash<T>()(k.a()))^ (hash<E>()(k.x()) << 1) >> 1);
	}
};

template<class T> struct hash<cnf_binary_rule<T> > {
public:
	size_t operator()(const cnf_binary_rule<T>& k) const {
		return ( (hash<T>()(k.c())) ^ (( ((hash<T>()(k.a()))^ (hash<T>()(k.b())
				<< 1) >> 1) << 1) >> 1) );
	}
};

template<class T> struct hash<arbitrary_cfg_rule<T> > {
public:
	size_t operator()(const arbitrary_cfg_rule<T>& k) const {

		size_t my_hash = 0;

		for (my_uint i=0; i < k.rhs_length(); i++) {
			my_hash = (my_hash ^ ((hash<T>()(k.rhs(i))) << 1) >> 1);
		}

		my_hash = ((my_hash ^ ((hash<T>()(k.lhs())) << 1)) >> 1);

		return my_hash;
	}
};
}

}

#endif /*RULES_H_*/
