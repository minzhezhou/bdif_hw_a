#pragma once
#include "myutil.h"

std::string get_time() {
    std::time_t result = std::time(NULL);
    std::string ret = std::string(std::asctime(std::localtime(&result)));
    return ret.substr(0,ret.size()-1);
}

class timer {
    using string = std::string;
public:    
    static const int months[13];// = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    static const long long ys = 365*24*3600;
    long long get_year_secs(int y, int m, int d) {
        long long res = 0;
        for (int i = 2001; i < y; i++) {
            res += ys;
            if (i % 4 == 0) {
                res += 24*3600;
            }
        }
        int dcnt = 0;
        for (int i = 1; i < m; i++) {
            dcnt += months[i];
        }
        dcnt += (d-1);
        if (y % 4 == 0 && m > 2) {
            dcnt++;
        }
        res += dcnt * 24 * 3600;
        return res;
    }
    long long get_time_stamp(string const& s) {
        int pdel = 0;
        try{
        while (s[pdel]!=':') {
            pdel++;
        }
	    if (pdel != 8 && s.length() < 19)
		    return 0;
	    string ds = s.substr(0,8);
	    string ts = s.substr(9,8);
        string ms = "0." + s.substr(18);
        //std::cout << ds << " " << ts << " " << ms << "\n";
	    long long dsecs = get_d_secs(ds);
        if (dsecs <= 0) {
            return 0;
        }
        long long tsecs = get_t_secs(ts);
        if (tsecs < 0) {
            return 0;
        }
        return (dsecs + tsecs) * 1000000 + (long long)(atof(ms.c_str()) * 1000000);
        } catch (std::exception& e) {
            return 0;
        }
    }
    
    long long get_t_secs(string const& time_str) {
        auto cols = split_string(time_str, ':');
        if (cols.size() < 3) {
            return -1;
        }
        int hh = atoi(cols[0].c_str());
        if (hh <0 || hh > 23) {
            return -1;
        }
        int mm = atoi(cols[1].c_str());
        if (mm <0 || mm > 59) {
            return -1;
        }
        int ss = atoi(cols[2].c_str());
        if (ss <0 || ss > 59) {
            return -1;
        }
        return ((hh*60 + mm) * 60) + ss;
    }

    long long get_d_secs(string const& date_str) {
        if (date_str.length() != 8) {
            return 0;
        }
        std::string ystr = date_str.substr(0,4);
        std::string mstr = date_str.substr(4,2);
        std::string dstr = date_str.substr(6);
        int yy = atol(ystr.c_str());
        if (yy < 2001 || yy > 2015) {
            return 0;
        }
        int mm = atol(mstr.c_str());
        if (mm < 1 || mm > 12) {
            return 0;
        }
        int dd = atol(dstr.c_str());
        if (dd < 1 || dd > 31) {
            return 0;
        }
        return get_year_secs(yy, mm ,dd);
    }
};
const int timer::months[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
