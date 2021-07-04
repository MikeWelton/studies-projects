#include <iostream>
#include <utility>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <regex>
#include <string>
#include <fcntl.h>
#include <uv.h>
#include "aux.h"

#define DEFAULT_PORT 8080
#define QUEUE_LENGTH 8
#define BUF_SIZE 1024
#define HTTP_VERSION "HTTP/1.1"

using namespace std;

using sockaddr_in_t = struct sockaddr_in;
using sockaddr_t = struct sockaddr;


class read_exception : public exception {};

class start_line {
public:
    string http_v;

    virtual string to_string() {
        return string();
    }
};


class request_line : public start_line {
public:
    string method;
    string request_target;

    request_line() = default;

    request_line(string method, string requestTarget, const string &http_v) :
            method(std::move(method)),
            request_target(std::move(requestTarget)) {
        this->http_v = http_v;
    }
};


class status_line : public start_line {
public:
    int status_code{};
    string reason_phrase;

    status_line() = default;

    status_line(int stat_code, string reason) {
        status_code = stat_code;
        reason_phrase = std::move(reason);
        http_v = string(HTTP_VERSION);
    }

    string to_string() override {
        if (reason_phrase.empty()) {
            return string();
        }
        return http_v + " " + std::to_string(status_code) + " " + reason_phrase + "\r\n";
    }
};


class header_field {
public:
    string field_name;
    string field_value;

    header_field() = default;

    header_field(string _field_name, string _field_value) :
            field_name(std::move(_field_name)),
            field_value(std::move(_field_value)) {}

    [[nodiscard]] string to_string() const {
        return field_name + ": " + field_value + "\r\n";
    }
};


template<typename T>
class http_message {
public:
    T startline{};
    vector<header_field> header_fields{};

    http_message() = default;

    explicit http_message(const T &_startline) {
        startline = _startline;
    }

    void add_start_line(const T &_startline) {
        startline = _startline;
    }

    void add_header_field(const header_field &header_field) {
        header_fields.push_back(header_field);
    }

    /* Adds another header field if it was absent. Returns true if adding field was successful
     * otherwise false. */
    bool add_header_field_if_absent(const header_field &header_field) {
        if (!header_exists(header_field.field_value)) {
            header_fields.push_back(header_field);
            return true;
        }
        return false;
    }

    /* Checks if header with given name was already added. */
    bool header_exists(const string &header) {
        return any_of(header_fields.begin(), header_fields.end(), [header](header_field &field) {
            return field.field_value == header;
        });
    }

    string to_string() {
        string ret = startline.to_string();
        for (header_field &field : header_fields) {
            ret += field.to_string();
        }
        ret += "\r\n";
        return ret;
    }
};


class server {
public:
    server() = default;

    server(string dir, string servers) : dir(std::move(dir)),
                                         servers_file(std::move(servers)),
                                         port(DEFAULT_PORT) {}

    server(string dir, string servers, int port) : dir(std::move(dir)),
                                                   servers_file(std::move(servers)),
                                                   port(port) {}


    void prepare_server() {
        sock = socket(PF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            exit(EXIT_FAILURE);
        }

        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = htonl(INADDR_ANY);
        server_address.sin_port = htons(port);

        if (bind(sock, (sockaddr_t *) &server_address, sizeof(server_address)) < 0) {
            exit(EXIT_FAILURE);
        }

        if (listen(sock, QUEUE_LENGTH) < 0) {
            exit(EXIT_FAILURE);
        }

        serv_file_desc = fopen(servers_file.c_str(), "r");
        if (serv_file_desc == nullptr) {
            exit(EXIT_FAILURE);
        }

        signal(SIGPIPE, SIG_IGN);
    }

    [[noreturn]] void run_server() {
        socklen_t client_address_len;
        for (;;) {
            connection_close = false;
            client_address_len = sizeof(client_address);
            msg_sock = accept(sock, (sockaddr_t *) &(client_address), &client_address_len);
            if (msg_sock < 0) {
                exit(EXIT_FAILURE);
            }

            read_messages();

            if (close(msg_sock) < 0) {
                exit(EXIT_FAILURE);
            }
        }
    }

private:
    string dir;
    string servers_file;
    FILE *serv_file_desc = nullptr;
    int port{};
    int sock{};
    int msg_sock{};
    sockaddr_in_t server_address{};
    sockaddr_in_t client_address{};
    char buffer_in[BUF_SIZE]{};
    char buffer_out[BUF_SIZE]{};
    http_message<request_line> message{};
    http_message<status_line> answer{};
    bool connection_close = false;

    /* Splits correct header line and using result creates header object. */
    static header_field split_header(string &header_line) {
        vector<string> tokens = split(header_line, ":");
        string header_val = split(tokens[1], " ")[0]; // remove unnecessary spaces
        return header_field(tokens[0], header_val);
    }

    /* Reads input into buffer then converts it into std::string and returns that string. */
    string read_input() {
        ssize_t read_len = read(msg_sock, buffer_in, BUF_SIZE);
        if (read_len < 0) {
            internal_error("Error when reading from socket");
            throw read_exception();
        }
        return string(buffer_in, read_len);
    }

    /* Writes http-message without body. Message before call should be in member answer.
     * Returns true write was successful otherwise false. */
    bool write_answer() {
        string ans_msg = answer.to_string();
        ssize_t write_len, msg_len = ans_msg.length();
        memset(buffer_out, 0, BUF_SIZE);
        sprintf(buffer_out, "%s", ans_msg.c_str());
        write_len = write(msg_sock, buffer_out, msg_len);
        if (write_len != msg_len) {
            return false;
        }
        return true;
    }

    void create_answer_with_close(int code, string err_msg) {
        status_line stat_line = status_line(code, std::move(err_msg));
        answer = http_message<status_line>(stat_line);
        answer.add_header_field(header_field("Connection", "close"));
        connection_close = true;
    }

    /* Sends to client info about internal error with code 500. */
    void internal_error(string err_msg) {
        create_answer_with_close(500, std::move(err_msg));
        connection_close = true;
    }

    void send_file(int file_desc) {
        int file_read_len, write_len;
        for (;;) {
            memset(buffer_out, 0, BUF_SIZE);
            file_read_len = read(file_desc, buffer_out, BUF_SIZE);
            if (file_read_len < 0) {
                internal_error("Problem with read from file");
                return;
            }
            else if (file_read_len == 0) {
                break;
            }

            write_len = write(msg_sock, buffer_out, file_read_len);
            if (write_len < 0) {
                connection_close = true;
                return;
            }
        }
    }

    void read_messages() {
        string prev_buffer;
        for (;;) {
            message = http_message<request_line>();
            answer = http_message<status_line>();
            try {
                parse_start_line(prev_buffer);
                if (answer.startline.status_code == 400) {
                    answer_message(-1);
                    break;
                }
                read_header_fields(prev_buffer);
                int file_desc = manage_request();
                answer_message(file_desc);
            }
            catch (read_exception&) {
                answer_message(-1);
            }

            if(answer.startline.status_code == 400 || connection_close) { // end connection in case of error
                break;
            }
        }
    }

    void parse_start_line(string &prev_buffer) {
        regex req_line("\\w+ \\/[a-zA-Z0-9.\\-/]* HTTP\\/1.1"),
                incorrect_path_chars("(GET|HEAD) /[^a-zA-Z0-9.\\-/ \r\n]+ HTTP\\/1.1"),
                get("GET \\/[a-zA-Z0-9.\\-/]* HTTP\\/1.1"),
                head("HEAD \\/[a-zA-Z0-9.\\-/]* HTTP\\/1.1");
        string input = prev_buffer;
        ulong pos;
        string sliced;

        while (true) {
            pos = input.find("\r\n");
            if (pos == string::npos) { // end of request line not found
                input.append(read_input());
            }
            else {
                sliced = string(input, 0, pos); // sliced start_line without CRLF
                break;
            }
        }

        if (regex_match(sliced, get) || regex_match(sliced, head)) {
            vector<string> tokens = split(sliced, " ");
            request_line r_line = request_line(tokens[0], tokens[1], tokens[2]);
            message.add_start_line(r_line);
        }
        else if (regex_match(sliced, incorrect_path_chars)) {
            status_line stat_line = status_line(404, "Incorrect chars in header");
            answer.add_start_line(stat_line);
        }
        else if (regex_match(sliced, req_line)) {
            status_line stat_line = status_line(501, "Wrong method - only GET and HEAD are accepted");
            answer.add_start_line(stat_line);
        }
        else {
            create_answer_with_close(400, "Incorrect request line");
        }
        prev_buffer = str_slice(input, pos + 2, input.length() - 1);
    }

    void parse_field(string &input) {
        regex correct_header("[a-zA-Z\\-]+:[ ]*[a-zA-Z0-9.\\-/]+[ ]*"),
                content_type("Content-type:[ ]*[a-zA-Z]+[ ]*", regex_constants::icase),
                content_length("Content-length:[ ]*\\d+[ ]*", regex_constants::icase),
                conn("connection", regex_constants::icase);
        if (regex_match(input, content_length)) {
            header_field field = split_header(input);
            if (!(message.add_header_field_if_absent(field))) {
                create_answer_with_close(400, "Duplicate header name");
                return;
            }
            if (regex_match(input, content_length) && field.field_value != "0") { // multiple 0's are incorrect
                create_answer_with_close(400, "Message cannot have body");
            }
        }
        else if (!regex_match(input, correct_header)) { // incorrect header
            create_answer_with_close(400, "Incorrect header");
        }
        else {
            header_field field = split_header(input);
            if (!regex_match(field.field_name, conn)) {
                return;
            }
            if (field.field_value != "close") {
                create_answer_with_close(400, "Connection can only have value close");
                return;
            }
            if (!(message.add_header_field_if_absent(field))) {
                create_answer_with_close(400, "Duplicate header name");
                return;
            }
            connection_close = true;
            answer.add_header_field(header_field("Connection", "close"));
        }
        // other header-like strings are ignored
    }

    void read_header_fields(string &prev_buffer) {
        string input = prev_buffer;
        ulong pos;

        while (true) {
            pos = input.find("\r\n");
            if (pos == string::npos) { // end of header field not found
                input.append(read_input());
            }
            else if (pos <= input.length() - 4 && input[pos + 2] == '\r' && input[pos + 3] == '\n') { // end of message found
                input = str_slice(input, 0, pos - 1);
                prev_buffer = str_slice(input, pos + 4, input.length() - 1);
                parse_field(input);
                break;
            }
            else if (pos == 0) { // end of message found
                prev_buffer = str_slice(input, 2, input.length() - 1);
                break; // nothing to parse, there are just two chars "\r\n"
            }
            else { // end of header field found
                string field = str_slice(input, 0, pos - 1);
                parse_field(field);
                input = str_slice(input, pos + 2, input.length() - 1);
            }
        }
    }

    int manage_request() {
        if (!(answer.startline.to_string().empty())) {
            return -1; // nothing to do, error already occurred
        }

        string filename = message.startline.request_target;
        string filepath = path((dir + filename).c_str());
        if (!file_dir_exist(filepath)) { // file doesn't exist
            string address_port = search_in_file(filename, serv_file_desc);
            rewind(serv_file_desc); // reset to the start of file
            if (address_port.empty()) { // file also not found on any server
                answer.startline = status_line(404, "File not found");
            }
            else { // file moved
                answer.startline = status_line(302, "Resource moved to other server");
                answer.add_header_field(header_field("Location", "http://" + address_port + filename));
            }
            return -1;
        }
        else { // file found
            if (!file_in_dir(filepath, dir)) { // but outside of given directory
                status_line stat_line = status_line(404, "Requested file is outside of directory");
                answer = http_message<status_line>(stat_line);
                return -1;
            }

            int fd = open(filepath.c_str(), O_RDONLY);
            struct stat stats{};
            if (fd < 0 || fstat(fd, &stats) < 0) {
                internal_error("Error with opening requested file");
                return -1;
            }
            answer.startline = status_line(200, "OK"); // so far so good
            answer.add_header_field(header_field("Content-Type", "application/octet-stream"));
            answer.add_header_field(header_field("Content-Length", to_string(stats.st_size)));
            return fd;
        }
    }

    void answer_message(int file_desc) {
        if (!write_answer() && file_desc != -1) {
            close(file_desc);
            connection_close = true;
            return;
        }

        if (file_desc != -1) {
            if (message.startline.method != "HEAD") { // have to send file
                send_file(file_desc);
            }
            if (close(file_desc) < 0) {
                internal_error("Problem with closing file descriptor");
            }
        }
    }
};

int main(int argc, char **argv) {
    server serv;
    string dir, serv_file;
    if (argc == 3) {
        serv = server(dir = check_path(argv[1]), serv_file = check_path(argv[2]));
    }
    else if (argc == 4) {
        serv = server(dir = check_path(argv[1]), serv_file = check_path(argv[2]),
                      number(argv[3]));
    }
    else {
        cout << "serwer <nazwa-katalogu-z-plikami>"
                " <plik-z-serwerami-skorelowanymi> [<numer-portu-serwera>]\n";
        exit(EXIT_FAILURE);
    }
    if (!(file_dir_exist(dir) && file_dir_exist(serv_file))) { // exit if given file or dir doesn't exist
        exit(EXIT_FAILURE);
    }

    serv.prepare_server();
    serv.run_server();
    return 0;
}
