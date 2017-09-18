#ifndef HASHES_H_
#define HASHES_H_

namespace std {
namespace tr1 {

template<> struct hash<tuple<inpre_symbol, inpre_symbol, inpre_symbol>> {
public:
	size_t operator()(const tuple<inpre_symbol, inpre_symbol, inpre_symbol>& h) const {

		return ( (hash<inpre_symbol>()(get<0>(h)) << 1) ^ (hash<inpre_symbol>()(get<1>(h)) << 1) ^ (hash<inpre_symbol>()(get<2>(h)) << 1));
	}
};

template<> struct hash<tuple<my_uint, my_uint, my_uint, inpre_symbol>> {
public:
	size_t operator()(const tuple<my_uint, my_uint, my_uint, inpre_symbol>& h) const {
		return (((get<0> (h) ^ (get<1> (h) << 1)) ^ (get<2> (h) << 1)) ^ (hash<
				inpre_symbol> ()(get<3> (h)) << 1));
	}
};

template<> struct hash<tuple<my_uint, my_uint, vocab_symbol>> {
public:
	size_t operator()(const tuple<my_uint, my_uint, vocab_symbol>& h) const {
		return ((get<0> (h) ^ (get<1> (h) << 1)) ^ (hash<vocab_symbol> ()(
				get<2> (h)) << 1));
	}
};

template<> struct hash<tuple<my_uint, my_uint, inpre_symbol>> {
public:
	size_t operator()(const tuple<my_uint, my_uint, inpre_symbol>& h) const {
		return ((get<0> (h) ^ (get<1> (h) << 1)) ^ (hash<inpre_symbol> ()(
				get<2> (h)) << 1));
	}
};

template<> struct hash<pair<my_uint, my_uint>> {
public:
	size_t operator()(const pair<my_uint, my_uint>& h) const {
		return (h.first ^ (h.second << 1));
	}
};



}
}

#endif /*HASHES_H_*/
