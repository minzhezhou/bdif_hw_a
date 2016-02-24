#pragma once
#include <string> 
#include <cstring> 
#include <vector>
//#include "boost/lexical_cast.hpp"
//#include "boost/noncopyable.hpp"
#include <mutex>
#include <thread>
#include <cstdio>
#include <cmath>
#include <memory>
#include <cstdlib>
#include <exception>
#include <vector>
#include <list>
#include <deque>
#include <memory>
#include <fstream>
#include <iostream>
#include <map>
#include <unordered_map>
#include <ctime>
#include <functional>

template<class T = std::string>
std::vector<T> split_string(const std::string& str, char delim) {
	size_t num = 1;
	const char* p;
	for (p = str.c_str(); *p != 0; p++) {
		if (*p == delim) {
			num++;
		}
	}
	std::vector<T> list(num);
	const char* last = str.c_str();
	num = 0;
	for (p = str.c_str(); *p != 0; p++) {
		if (*p == delim) {
			list[num++] = T(last, p - last);
			//list[num++] = boost::lexical_cast<T>(last, p - last);
			last = p + 1;
		}
	}
	list[num++] = T(last, p - last);
	//list[num++] = boost::lexical_cast<T>(last, p - last);
	return list;
}

template<class T = std::string>
std::vector<T> split_string(char* &str, char delim, int cnt) {
	size_t num = 1;
	char* p;
    char* begin = str;
    if (*str == 0) {
        return std::vector<T>();
    }
	for (p = str; *p != 0 && num < cnt; p++) {
		if (*p == delim) {
			num++;
		}
	}
    while (*p != 0 && *p != delim) {
        p++;
    }
    if (*p == delim) {
        *p = 0;
        str = p + 1;
    } else {
        str = p;
    }

	std::vector<T> list(num);
	char* last = begin;
	num = 0;
	for (p = begin; *p != 0; p++) {
		if (*p == delim) {
			list[num++] = T(last, p - last);
			//list[num++] = boost::lexical_cast<T>(last, p - last);
			last = p + 1;
		}
	}
	list[num++] = T(last, p - last);
	//list[num++] = boost::lexical_cast<T>(last, p - last);
	return list;
}

// split string by spaces. Leading and tailing spaces are ignored. Consecutive spaces are treated as one delim.
template<class T = std::string>
std::vector<T> split_string(const std::string& str) {
	size_t num = 0;
	const char* p;
	for (p = str.c_str(); *p != 0; ) {
		if (!isspace(*p)) {
			num++;
			p++;
			while (*p != 0 && !isspace(*p)) {
				p++;
			}
		}
		else {
			p++;
		}
	}
	std::vector<T> list(num);
	num = 0;
	for (p = str.c_str(); *p != 0; ) {
		if (!isspace(*p)) {
			const char* last = p;
			p++;
			while (*p != 0 && !isspace(*p)) {
				p++;
			}
			//list[num++] = boost::lexical_cast<T>(last, p - last);
			list[num++] = T(last, p - last);
		}
		else {
			p++;
		}
	}
	return std::move(list);
}
/*
template<class T>
std::string join_strings(const std::vector<T>& strs, char delim) {
	std::string str;
	for (size_t i = 0; i < strs.size(); i++) {
		if (i > 0) {
			str += delim;
		}
		//str += boost::lexical_cast<std::string>(strs[i]);
	    str += strs[i];
    }
	return std::move(str);
}
*/
template <typename T>
std::ostream& operator<<(std::ostream& ostr, const std::vector<T>& list)
{
	for (auto &i : list) {
		ostr << i << ",";
	}
	return ostr;
}

template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& ostr, const std::pair<T1, T2>& p) {
	ostr << "[" << p.first << "," << p.second << "]";
	return ostr;
}

template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& ostr, const std::map<T1, T2>& m) {
	for (auto &p : m) {
	    ostr << "[" << p.first << "->" << p.second << "], ";
    }
    return ostr;
}

template <typename T>
std::ostream& operator<<(std::ostream& ostr, const std::list<T>& llist)
{
	for (auto &i : llist) {
		ostr << " " << i;
	}
	return ostr;
}

template <typename T>
std::ostream& operator<<(std::ostream& ostr, const std::deque<T>& dq)
{
	for (auto &i : dq) {
		ostr << " " << i;
	}
	return ostr;
}

void randomfill(std::vector<int> &v, int n, int random = 1) {
	long long seed = 0;
	if (random)
		seed = std::time(0);
	std::srand(seed); // use current time as seed for random generator
	for (auto& e:v) {
		e = std::rand() % n;
	}
}
