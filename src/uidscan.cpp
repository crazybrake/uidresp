/**
 * @file uidscan.cpp
 * @brief uid scanning utility that probes devices using partial pattern
 *        matching
 *
 * sends string uid patterns with a prefix and reads back responses. the
 * search goes deeper in case of collisions, trying to find all unique uids
 * on the line
 *
 * usage:
 * - sends string (with prefix) to stdout
 * - reads lines from stdin
 * - detects uid collisions and recursively refines pattern
 * - confirms uid by repeating pattern
 * - mutes confirmed uids using setaddr command
 *
 * expected to be used with a compatible responder (see `uidresp.cpp`)
 */

#include <getopt.h>
#include <poll.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <vector>

constexpr int MAXLEN = 17; // uid length w/o prefix
const std::string CHARSET = "0123456789"
                            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                            "abcdefghijklmnopqrstuvwxyz"
                            "-_";
constexpr int POLL_TIMEOUT = 200;


int gl_timeout = POLL_TIMEOUT;

/**
 * @brief return reversed string
 *
 * we use reverse strings in algorithm because it's much more easier to
 * append symbols to string than insert them to beginning. this
 * `reverse_string` used before sending
 */
std::string reverse_string(const std::string &line)
{
    return std::string(line.rbegin(), line.rend());
}

/**
 * @brief read data, detect timeout and, partly, a collision (empty
 * line with "\n" is the collision, too)
 *
 * @return 1. empty string if timeout
 *         2. "!" if collision because "!" is not a valid symbol in
 *            any response
 *         3. response string
 */
std::string read_line(int timeout_ms = POLL_TIMEOUT)
{
    struct pollfd pfd;
    pfd.fd = STDIN_FILENO;
    pfd.events = POLLIN;

    int ret = poll(&pfd, 1, timeout_ms);

    if (ret > 0 && (pfd.revents & POLLIN)) {

        std::string line;
        if (std::getline(std::cin, line)) {

            if (!line.empty())
                return line; // normal response

            return "!"; // collision
        }
    }
    return ""; // timeout or error
}


/**
 *  self-explaining function name ;)
 */
void send(const std::string &line)
{
    std::cout << line << std::endl;
    std::cout.flush();
}

/**
 *  self-explaining function name ;)
 */
std::string send_and_recv(const std::string &line,
    int timeout = POLL_TIMEOUT)
{
    send(line);
    return read_line(timeout);
}

/**
 *  @brief "assign" address: prevent uid from responding
 */
void mute(const std::string &uid)
{
    // see `src/uidresp.cpp` for additional commands
    send("SETADDR:" + uid);
}

/**
 * @brief send command to enable all uids to respond
 */
void reset_all()
{
    send("RESETALL");
}

/**
 * @brief detect collisions by "collision symbol" ("!"), length and by
 *        comparing with additional response
 */
bool collision(const std::string &s)
{
    if ((s == "!") || (s.size() != MAXLEN+2))
        return true;
    mute(s);
    std::string resp = read_line();
    // uid length should be MAXLEN + 2 (prefix length)
    if ((resp.size() != MAXLEN + 2) || (resp != s)) {
        return true;
    }

    return false;
}

/**
 * the main magic is here: generate pattern, send it, check, recursively go
 * deeper in the case of collision
 *
 * this is a recurring funcion, i.e. it could call itself
 *
 * @returns 1. CHARSET.size() if nothing found
 *          2. the charset index where uid was found
 */
size_t scan(const std::string& s, const std::string& pfx,
    std::set<std::string>& found_uids, size_t index)
{
    static int level = 0; // recursion level
    std::cerr << "ENTER SCAN: level=" << level
        << " index=" << index << std::endl;

    if (s.size() >= MAXLEN) {
        std::cerr << "ERROR: length limit reached!" << level
            << " index=" << index << std::endl;
        level--;
        return CHARSET.size();
    }

    size_t pos = index;
    size_t inner_index = 0;

    for (; pos < CHARSET.size(); pos++) {
        std::string next;
        if (level == 0) { // scan by prefix only on level 0
            next = s;
            pos = CHARSET.size(); // for return if no collisions
        } else {
            next = s + CHARSET[pos];
        }
        // see comments for reverse_string() above
        std::string resp = send_and_recv(pfx + reverse_string(next),
            gl_timeout);

        // timeout, i.e. no answer
        if (resp.empty()) {
            inner_index = 0;
            continue;
        }

        if (collision(resp)) {

            char c = (inner_index < CHARSET.size())
                ? CHARSET[inner_index] : '.';
            char p = (pos < CHARSET.size())
                ? CHARSET[pos] : '.';
            std::cerr << "COLLISION: " << pfx + reverse_string(next)
                << " inner_index=" << inner_index
                << " (\"" << c << "\")"
                << " level=" << level << " pos=" << pos
                << " (\"" << p << "\")" << std::endl;

            if (inner_index < CHARSET.size()) {
                // go deeper into recursion
                level++;
                inner_index = scan(next, pfx, found_uids, inner_index);
                level--;

                char c1 = (inner_index < CHARSET.size())
                    ? CHARSET[inner_index] : '.';
                std::cerr << "RETURN FROM SCAN: inner_index="
                    << inner_index
                    << " (\"" << c1 << "\")"
                    << " level=" << level << " pos=" << pos
                    << " (\"" << p << "\")" << std::endl;

                /*
                 * need to check the same collision point: if only one uid
                 * remains it will be found immediately
                 */
                pos--;
            } else {
                inner_index = 0;
            }
            continue;
        }

        if (found_uids.insert(resp).second) {
            std::cerr << "FOUND: " << resp << std::endl;
            if (level > 1) {
                return pos;
            }
            else {
                inner_index = 0;
            }
        }
    }
    return pos;
}

/**
 *  check timeout parameter for valid value
 */
bool parse_timeout(const char* arg, int& value_out) {
    char* endptr = nullptr;
    errno = 0;
    long val = std::strtol(arg, &endptr, 10);

    if (errno != 0 || endptr == arg || *endptr != '\0' || val < 0) {
        return false;
    }

    value_out = static_cast<int>(val);
    return true;
}

/**
 * just for changing global variable
 */
void set_timeout(int timeout)
{
    gl_timeout = timeout;
}

/*
 * help message
 */
void usage(char *progname)
{
    std::cerr << "Usage: " << progname
        << " [--timeout|-t <msec>] <prefix> [prefix ...]\n";
}

/**
 * @brief the program accepts one optional parameter `-t <timeout in ms>`
 *        and at least one required parameter: vendor id (two characters)
 *
 * @example
 *
 *    uidscan -t 500 CB HS ZL
 */
int main(int argc, char **argv)
{
    set_timeout(POLL_TIMEOUT);

// ---- options parsing ----

    const struct option long_opts[] = {
        {"timeout", required_argument, nullptr, 't'},
        {nullptr, 0, nullptr, 0}
    };

    int opt;
    int timeout = 0;
    while ((opt = getopt_long(argc, argv, "t:",
        long_opts, nullptr)) != -1) {
        switch (opt) {
        case 't':
            if (!parse_timeout(optarg, timeout)) {
                std::cerr << "Invalid timeout value: " << optarg
                    << std::endl;
                return 1;
            }
            set_timeout(timeout);
            break;
        default:
            usage(argv[0]);
            return 1;
        }
    }
    if (optind >= argc) {
        usage(argv[0]);
        return 1;
    }

// ---- scan logic starts here ----

    reset_all(); // move all devices to "no address" state

    // iterate over prefixes
    std::set<std::string> found_uids;
    for (int i = optind; i < argc; ++i) {
        std::string pfx = argv[i];
        scan("", pfx, found_uids, 0);
    }
    reset_all();
    std::cerr << std::endl;
    std::cerr << "== search complete ==" << std::endl;
    std::cerr << "total uids found: " << found_uids.size() << std::endl;
    std::cerr << std::endl;
    for (auto uid: found_uids) {
        std::cerr << uid << std::endl;
    }
    std::cerr << std::endl;

    return 0;
}

