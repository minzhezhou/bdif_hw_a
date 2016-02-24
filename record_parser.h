#pragma once
#include "myutil.h"
#include "timer.h"
using string = std::string;

double Max_Price = 0, Max_Volume = 0;
static timer gtimer;

struct rec {
    string str;
    int status;
    int idate;
    long long ts;
    double fprice, fvol;
    rec() : status(-1) {}
    rec(string& ln) {
        status = -1;
        fprice = -1;
        fvol = -1;
        ts = 0;
        idate = 0;
	    auto cols = split_string(ln, ',');
		if (cols.size() > 2) {
			ts = gtimer.get_time_stamp(cols[0]);
            if (ts == 0) {
                return;
            }
			idate = atoi(cols[0].substr(0,8).c_str());
            try {
			    fprice = atof(cols[1].c_str());
			    //fprice = boost::lexical_cast<double>(cols[1]);
			    //fvol = boost::lexical_cast<double>(cols[2]);
			    fvol = atof(cols[2].c_str());
            } catch (std::exception &e) {
                //std::cout << ln << std::endl;
                return;
            }
            if (fprice > 0 && fvol > 0 && fprice < 20000 && fvol < 10000000) {
                status = 0;
            } else {
                status = 1;
            }
            str = std::move(ln);
        }
    }
};

struct rec_parser {
    double w, price_avg, price_sq, vol_avg, vol_sq, ret_sum, ret_sq;
    long long cnt;
    rec_parser() : cnt(0),ret_sum(0), ret_sq(0) {}
    
    int calc_ret(std::vector<rec>& vrec, double& ret, int ia, int ib) {
        if (ia == 0) {
            return 1;
        }
        long long ta = vrec[ia].ts;
        long long tb = vrec[ib].ts;
        double delta = abs(ta-tb);
        double pa = vrec[ia].fprice;
        double pb = vrec[ib].fprice;
        if (delta < 50000 || delta > 1000000) {
            return 1;
        }
        if (ta > tb) {
            ret = (pa - pb) / pb / sqrt(delta);
        } else {
            ret = (pb - pa) / pa / sqrt(delta);
        }
        //std::cout << "pa:" << pa << " pb:" << pb << " ret:" << ret << "\n";
        return 0;
    }

    int double_check(int idx, std::vector<rec>& vrec) {
        float vn = 0, wn = 0, tn = 0;
        int n = idx + 10;
        if (n <= vrec.size()) {
            // forward
            for (int i = idx + 1; i < n; i++) {
                vn += 1;
                if (abs(vrec[i].ts - vrec[idx].ts) < 2000000) {
                    wn += 1;
                }
            }
            if (wn/vn >= 0.7) {
                return 1;
            }
        } else {
            //backward
            n = idx - 10;
            for (int i = n; i < idx; i++) {
                vn += 1;
                if (abs(vrec[i].ts - vrec[idx].ts) < 2000000) {
                    wn += 1;
                }
            }
            if (wn/vn >= 0.7) {
                return 1;
            }
        }
        return 0;
    }
    int parse(char* &pbuf, std::vector<string>& valid_res,
            std::vector<string>& invalid_res,
            std::vector<double>& ret_res) {
        price_avg = price_sq = vol_avg = vol_sq = 0;
        std::deque<rec> recq;
        if (*pbuf == 0) {
            return 0;
        }
        auto vstr = split_string(pbuf, '\n', 10000);
        std::vector<rec> vrec;
        long long last_ts = 0;
        while (vstr.size()) {
            int last_date = 0, idx = 0, last_idx = 0;
            vrec.clear();
            for (auto& s: vstr) {
                rec r(s);
                if (r.status < 0) {
                    //std::cout << "line parse error:" << std::endl;
                    continue;
                }
                vrec.push_back(r);
            }
            for (int idx = 0; idx < vrec.size(); idx++) {
                rec const& r = vrec[idx];
                int vflag = 1;
                if (check_price_volume(r)) {
                    vflag = 0;
                }
                if (abs(r.ts - last_ts) > 2000000) {
                    vflag = double_check(idx, vrec);
                }
                if (vflag) {
                    double ret;
                    int ret_flag = calc_ret(vrec, ret, last_idx, idx);
                    if (ret_flag == 0) {
                        ret_res.push_back(ret);
                        ret_sum += ret;
                        ret_sq += ret*ret;
                    }
                    last_ts = r.ts;
                    last_idx = idx;
                    valid_res.push_back(r.str);
                } else {
                    invalid_res.push_back(r.str);
                }
                cnt++;
            }
            vstr = split_string(pbuf, '\n', 10000);
        }
        return 0;
    }

        /*
            double w = double(cnt) / double(cnt + 1);
            double price = r.fprice;
            double volume = r.fvol;
            price_avg = price_avg * w + price * (1 - w);
			price_sq = price_sq * w + price * price * (1 - w);
			vol_avg = vol_avg * w + volume * (1 - w);
			vol_sq = vol_sq * w + volume * volume * (1 - w);
            dcnt[r.idate].update(cnt);
            vrec.push_back(r);
			cnt++;
	    Max_Price = price_avg + 4 * sqrt((-price_avg * price_avg + price_sq) / float(cnt));
	    Max_Volume = vol_avg + 10 * sqrt((-vol_avg * vol_avg + vol_sq) / float(cnt));
        */
    int check_price_volume(rec const &r) {
        if (r.status == 1) {
            return 1;
        } else {
            if (0) {
                return 1;
            }
        }
        return 0;
    }
};
