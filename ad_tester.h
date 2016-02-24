#pragma once
#include "myutil.h"

class AD_tester {
private:
    std::vector<float> const cris = {0, 0.576, 0.656,  0.787,  0.918,  1.092};    
    std::vector<float> const sigs = {1, 0.15, 0.1, 0.05, 0.025, 0.01, 1e-5};
    double mean, sigma, cnt, sum;
    double phi(double);
public:

    AD_tester(double m, double s, double n) : mean(m), sigma(s), cnt(n), sum(0) {}
    inline double get_p_value(double x) {
        int i = 0;
        while (cris[i] < x && i < 6) {
            i++;
        }
        return sigs[i];
    }

    void update_ad_stat(long long offset, double x) {
        x = (x-mean) / sigma;
        double phix = phi(x);
        double v = (2*offset - 1) * log(phix) + 
        (2*(cnt - offset) + 1) * log(1 - phix);
        sum += v / cnt;
        //std::cout << " m:" << mean << " sigma:" << sigma << " x:" << x << " phi:" << phix << " v:" << v << " sum:" << sum << "\n";
    }
    double get_sum() {return sum; }
    double get_stat(double s) {
        double ad_stat = -1 * (s + cnt) * (1 + 4/cnt - 25/cnt/cnt);
        return ad_stat;
    }
};
 double AD_tester::phi(double x)
{
  static const double RT2PI = sqrt(4.0*acos(0.0));
  static const double SPLIT = 7.07106781186547;
  static const double N0 = 220.206867912376;
  static const double N1 = 221.213596169931;
  static const double N2 = 112.079291497871;
  static const double N3 = 33.912866078383;
  static const double N4 = 6.37396220353165;
  static const double N5 = 0.700383064443688;
  static const double N6 = 3.52624965998911e-02;
  static const double M0 = 440.413735824752;
  static const double M1 = 793.826512519948;
  static const double M2 = 637.333633378831;
  static const double M3 = 296.564248779674;
  static const double M4 = 86.7807322029461;
  static const double M5 = 16.064177579207;
  static const double M6 = 1.75566716318264;
  static const double M7 = 8.83883476483184e-02;

  const double z = fabs(x);
  double c = 0;//1e-30;

  if(z<=37.0)
  {
    const double e = exp(-z*z/2.0);
    if(z<SPLIT)
    {
      const double n = (((((N6*z + N5)*z + N4)*z + N3)*z + N2)*z + N1)*z + N0;
      const double d = ((((((M7*z + M6)*z + M5)*z + M4)*z + M3)*z + M2)*z + M1)*z + M0;
      c = e*n/d;
    }
    else
    {
      const double f = z + 1.0/(z + 2.0/(z + 3.0/(z + 4.0/(z + 13.0/20.0))));
      c = e/(RT2PI*f);
    }
  }
  //std::cout << "x:" << x << " cdf:" << (x<=0.0 ? c : 1-c) << "\n";
  return x<=0.0 ? c : 1-c;
}

