#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include "aux.h"


using namespace std;

#define BUF_SIZE 1024


string check_path(const char *str) {
    string p = path(str);
    if (p.empty()) {
        exit(EXIT_FAILURE);
    }
    return p;
}

string path(const char *str) {
    char *path = realpath(str, nullptr);
    if (path == nullptr) {
        return string();
    }
    string ret = string(path);
    free(path);
    return ret;
}

int number(char *str) {
    return stoi(str, nullptr, 10);
}

bool file_dir_exist(string &file) {
    return access(file.c_str(), R_OK) != -1;
}

bool is_file(string &file) {
    struct stat path_stat{};
    stat(file.c_str(), &path_stat);
    return !S_ISDIR(path_stat.st_mode);
}

bool file_in_dir(string &file, string &dir) {
    return is_file(file) && file.compare(0, dir.length(), dir) == 0;
}

vector<string> split(const string &str, const string &delimiter) {
    vector<string> tokens;
    size_t end = 0, start = str.find_first_not_of(delimiter, end);

    while (start != string::npos) {
        end = str.find_first_of(delimiter, start);
        tokens.push_back(str.substr(start, end - start));
        start = str.find_first_not_of(delimiter, end);
    }
    return tokens;
}

string str_slice(string &str, size_t start, size_t end) {
    return str.length() - 1 >= start ? str.substr(start, end - start + 1) : string();
}

string search_in_file(string &str, FILE *file) {
    const char *str_c = str.c_str();
    char buf[BUF_SIZE];
    while(fgets(buf, BUF_SIZE, file) != nullptr) {
        if((strstr(buf, str_c)) != nullptr) {
            string line = string(buf, strlen(buf));
            line = str_slice(line, str.length(), strlen(buf) - 2);
            vector<string> tokens = split(line, "\t");
            return tokens[0] + ":" + tokens[1];
        }
    }
    return string();
}
