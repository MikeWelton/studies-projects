#ifndef ZADANIE1_AUX_H
#define ZADANIE1_AUX_H

#include <vector>
#include <string>


using namespace std;

#define BUF_SIZE 1024

/* Checks if given path is correct. If path is correct then function returns this
 * path parsed to absolute path otherwise calls exit(error). */
string check_path(const char *str);

/* Parses given path to absolute path. Returns string with path in case of success else empty string. */
string path(const char *str);

int number(char *str);

/* Returns true if file (or directory) exist else false. */
bool file_dir_exist(string &file);

/* Checks if file is a file. */
bool is_file(string &file);

/* Checks if file is a file. Then checks if file is located in given dir.
 * If both conditions are met then function returns true otherwise false. */
bool file_in_dir(string &file, string &dir);

/* Split string str into tokens using given delimiter. */
vector<string> split(const string &str, const string &delimiter);

/* Returns string slice from position start to end.
 * If start is out of bound returns empty string. */
string str_slice(string &str, size_t start, size_t end);

/* Searches line with given string in given file. If such line is found then
* "server:port" string is returned. Otherwise function returns empty string. */
string search_in_file(string &str, FILE *file);

#endif //ZADANIE1_AUX_H
