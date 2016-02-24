#include <cstdio>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <cinttypes>
#include <chrono>
#include <iomanip>
#include "myutil.h"
#include "mpi.h"
#include "record_parser.h"
#include "ad_tester.h"
#include "shell.h"
int rank, nodes;
long long set_wbuf(char* &p, std::vector<string>& vec) {
    long long res = 0;
    for (auto& s: vec) {
        res += s.length() + 1;
    }
    p = new char[res];
    char* ptr = p;
    for (auto& s : vec) {
        strcpy(ptr, s.c_str());
        ptr += s.length();
        *ptr = '\n';
        ptr++;
    }
    return res;
}

int mpi_group_read(char const* fname, char* &buf, int rank, int nodes) {
    MPI_File fh;
    MPI_Status status;
	MPI_Offset file_sz;
    int ret = MPI_File_open(MPI_COMM_WORLD, fname, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
    if (ret) {
        std::cout << "file open error, mpi_fopen ret:" << ret << std::endl;
        return ret;
    }
    ret = MPI_File_get_size(fh, &file_sz);
    std::cout << "mpi_file_sz ret:" << ret << " file size:" << file_sz << std::endl;
    long long tmp_sz = file_sz / nodes;
    long long read_cnt = rank==nodes-1 ? file_sz - (nodes-1)*tmp_sz : tmp_sz;
    std::cout << "rank is:" << rank << " nodes number is:" << nodes << " read from " << rank * tmp_sz 
        << " to " << rank * tmp_sz + read_cnt << std::endl;

    buf = new char[read_cnt + 1];
    ret = MPI_File_read_at(fh, rank*tmp_sz, buf, read_cnt, MPI_BYTE, &status);
    MPI_File_close(&fh);
    if (ret) {
        std::cout << "mpi read error code:" << ret;
        delete[] buf;    
        return ret;
    }
    buf[read_cnt] = 0;
    return 0;
}

int mpi_group_write(char const* fname,char* wbuf, long long wsz, int rank, int nodes) {
    int ret;
    MPI_File fh;
    MPI_Status status;

    long long *allsz = (long long*)malloc(sizeof(long long) * nodes);
    MPI_Allgather(&wsz, 1, MPI_LONG, allsz, 1, MPI_LONG, MPI_COMM_WORLD);
    for (int i = 1; i < nodes; i++) {
        allsz[i] += allsz[i-1];
    }

    ret = MPI_File_open(MPI_COMM_WORLD, fname, MPI_MODE_WRONLY|MPI_MODE_CREATE, MPI_INFO_NULL, &fh);
    ret = MPI_File_set_size(fh, 0);
    if (ret) {
        std::cout << "file open error, mpi_fopen ret:" << ret << std::endl;
        return 0;
    }
    ret = MPI_File_write_at_all(fh, allsz[rank] - wsz, (void*)wbuf, wsz, MPI_BYTE, &status);
    if (ret) {
        std::cout << "file write error, mpi_fwrite ret:" << ret << std::endl;
        return 0;
    }
    MPI_File_close(&fh);
}

int main(int argc, char **argv){
    using clock = std::chrono::system_clock;
    using duration = std::chrono::duration<double, std::milli>;
    std::stringstream ss;
    char* fname = argv[1];
    MPI_File fh;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nodes);

    char* buf;
    // read data into memory
    ss << get_time() << "Rank:" << rank << " start reading...\n";
    auto start_read = clock::now();
    int ret = mpi_group_read(fname, buf, rank, nodes);
    auto finish_read = clock::now();
    if (ret) {
        return ret;
    }
    ss << get_time() << "Rank:" << rank << " finished reading...\n";
    auto diff = finish_read-start_read;
    ss << get_time() << "Rank:" << rank << " reading cost time:" << diff.count() << " milli secs\n";
    
    // scrub data into 2 vector
    rec_parser blk;
    char* pbuf = buf;
    std::vector<string> valid_res;
    std::vector<string> invalid_res;
    std::vector<double> ret_res;
    int cntsum = 0;
    ss << get_time() << " Rank:" << rank << " start scrubing...\n";
    auto start_scrub = clock::now();
    cntsum = blk.parse(pbuf, valid_res, invalid_res, ret_res);
    auto end_scrub = clock::now();
    ss << get_time() << " Rank:" << rank << " finished scrubing...\n";
    diff = end_scrub - start_scrub;
    ss << get_time() << " Rank:" << rank << " Scrubing cost time " << diff.count() << " milli secs\n";
    double ret_stat[3] = {blk.ret_sum, blk.ret_sq, double(ret_res.size())};

    std::cout << "rank:" << rank << " sum:" << ret_stat[0] << " sq:" << ret_stat[1] << " cnt:" << ret_stat[2] << "\n";
    delete[] buf;

    // write noise to disk and clear the memory
    char* wbuf;
    ss << get_time() << " Rank:" << rank << " start writing noise data...\n";
    auto start_write_noise = clock::now();
    long long wsz = set_wbuf(wbuf, invalid_res);
    char fname2[] = "noise.txt";
    ret = mpi_group_write(fname2, wbuf, wsz, rank, nodes);
    auto end_write_noise = clock::now();
    ss << get_time() << " Rank:" << rank << " finished writing noise data...\n";
    diff = end_write_noise - start_write_noise;
    ss << get_time() << " Rank:" << rank << " writing noise data cost time:" << diff.count() << " milli secs\n";
    if (wbuf) {
        delete[] wbuf;
    }
    invalid_res.clear();
    
    // write signal to disk and clear the memory
    ss << get_time() << " Rank:" << rank << " start writing signal data...\n";
    auto start_write_signal = clock::now();
    wsz = set_wbuf(wbuf, valid_res);
    char fname3[] = "signal.txt";
    ret = mpi_group_write(fname3, wbuf, wsz, rank, nodes);
    auto end_write_signal = clock::now();
    ss << get_time() << " Rank:" << rank << " finished writing signal data...\n";
    diff = end_write_signal - start_write_signal;
    ss << get_time() << " Rank:" << rank << " writing signal data cost time:" << diff.count() << "\n";
    if (wbuf) {
        delete[] wbuf;
    }

    // write log to disk
    char logname1[] = "scrub_log.txt";
    std::vector<string> logvec(1, ss.str());
    wsz = set_wbuf(wbuf, logvec);
    ret = mpi_group_write(logname1, wbuf, wsz, rank, nodes);
    if (wbuf) {
        delete[] wbuf;
    }
    
    //diff.count();
    /*************************************************/
    /**** Part II normality test of asset return *****/
    /*************************************************/
    std::stringstream ss2;
    ss.str("");
    ss.clear();
    ss << get_time() << " Rank:" << rank << " start testing normality by aderson darling test...\n";
    double* all_ret_stat = new double[nodes * 3];
    MPI_Allgather(ret_stat, 3, MPI_DOUBLE, all_ret_stat, 3, MPI_DOUBLE, MPI_COMM_WORLD);
    double ret_mean = 0, ret_sd = 1, ret_cnt = 0;
    for (int i = 0; i < nodes; i++) {
        ret_mean += all_ret_stat[3*i];
        ret_sd += all_ret_stat[3*i+1];
        ret_cnt += all_ret_stat[3*i+2];
    }
    ret_mean /= ret_cnt;
    ret_sd = sqrt(ret_sd/ret_cnt - ret_mean * ret_mean);
    AD_tester norm_tester(ret_mean, ret_sd, ret_cnt);
    ss << get_time() << " Rank:" << rank << " global sample mean and variance calculated\n";
    if (rank == 0) {
        ss2 << "Mean:" << ret_mean << "\n" << "Stderr:" << ret_sd << "\n" << "Sample size:" << (long long)(ret_cnt) << "\n";
    }
    std::vector<long long> stat_loc(nodes, 0);
    for (int i = 0; i < nodes -1; i++) {
        stat_loc[i+1] += stat_loc[i] + (long long)(all_ret_stat[3*i+2]);
    }
    double ad_stat = 0;
    for (long long i = 0; i < ret_res.size(); i++) {
        norm_tester.update_ad_stat(stat_loc[rank] + i, ret_res[i]);
    }
    ss << get_time() << " Rank:" << rank << " local AD statistic calculated\n";
    double local_ad_sum = norm_tester.get_sum();
    double* pall_ad_sum = new double[nodes];
    MPI_Allgather(&local_ad_sum, 1, MPI_DOUBLE, pall_ad_sum, 1, MPI_DOUBLE, MPI_COMM_WORLD);
    double all_ad_sum = 0;
    for (int i = 0; i < nodes; i++) {
        all_ad_sum += pall_ad_sum[i];
    }
    ss << get_time() << " Rank:" << rank << " global AD statistic calculated\n";
    double p_value = norm_tester.get_p_value(norm_tester.get_stat(all_ad_sum));
    ss << get_time() << " Rank:" << rank << " normality p value calculated\n";
    if (rank == 0) {
        ss2 << "AD statistic = " << norm_tester.get_stat(all_ad_sum) << "\n";
        ss2 << "P value = " << p_value << "\n";
        ss2 << "Normality can be rejected at significance level " << p_value;
    }
    //std::cout << " Rank:" << rank << " sum:" << all_ad_sum << " stat:" << norm_tester.get_stat(all_ad_sum) << " p:" << p_value << "\n"; 

    if (all_ret_stat) {
        delete[] all_ret_stat;
    }
    if (pall_ad_sum) {
        delete[] pall_ad_sum;
    }
    
    char logname2[] = "test_normality_log.txt";
    char logname3[] = "test_normality_result.txt";
    logvec.clear();
    logvec.push_back(ss.str());
    wsz = set_wbuf(wbuf, logvec);
    ret = mpi_group_write(logname2, wbuf, wsz, rank, nodes);
    if (wbuf) {
        delete[] wbuf;
    }
    logvec.clear();
    logvec.push_back(ss2.str());
    wsz = set_wbuf(wbuf, logvec);
    ret = mpi_group_write(logname3, wbuf, wsz, rank, nodes);
    if (wbuf) {
        delete[] wbuf;
    }

    std::cout << shell_get_command_output("free") << "\n";
    MPI_Finalize();
    return 0;
}
