#include <iostream>
#include <poll.h>
#include <set>
#include <stack>
#include <string>
#include <tuple>
#include <unistd.h>

void send(const std::string &line) {
  std::cout << line << std::endl;
  std::cout.flush();
}

std::string read_line_with_timeout(int timeout_ms = 2) {
  struct pollfd pfd;
  pfd.fd = STDIN_FILENO;
  pfd.events = POLLIN;

  int ret = poll(&pfd, 1, timeout_ms);
  if (ret > 0 && (pfd.revents & POLLIN)) {
    std::string line;
    if (std::getline(std::cin, line)) {
      if (!line.empty())
        return line;
    }
  }
  return "";
}

int main(int argc, char **argv) {
  if (argc != 2 || std::string(argv[1]).size() != 2) {
    std::cerr << "Usage: " << argv[0] << " <2-char prefix>" << std::endl;
    return 1;
  }

  const std::string prefix = argv[1];
  const std::string charset = "0123456789"
                              "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                              "abcdefghijklmnopqrstuvwxyz"
                              "-_";
  std::set<std::string> found;
  std::set<std::string> tried;

  using Pattern = std::tuple<std::string, std::string>; // (middle, suffix)
  std::stack<Pattern> to_try;

  to_try.emplace("", ""); // start with prefix only

  while (!to_try.empty()) {
    auto [middle, suffix] = to_try.top();
    to_try.pop();

    std::string pattern = prefix + middle + suffix;
    if (tried.count(pattern))
      continue;
    tried.insert(pattern);

    send(pattern);
    std::string r1 = read_line_with_timeout();
    if (r1.empty())
      continue;

    send(pattern);
    std::string r2 = read_line_with_timeout();
    if (r1.empty() || r2.empty())
      continue;

    if (r1 == r2) {
      if (!found.count(r1)) {
        std::cerr << "FOUND:     " << r1 << std::endl;
        found.insert(r1);
        send("SETADDR:" + r1);
        read_line_with_timeout(); // optional
      }
      if ((prefix + middle + suffix).size() < 19) {
        for (auto it = charset.rbegin(); it != charset.rend(); ++it) {
          std::string new_middle = *it + middle;
          to_try.emplace(new_middle, suffix);
        }
      }
    }

    // std::cerr << "COLLISION: " << pattern << std::endl;
    if (suffix.empty() && pattern.size() > prefix.size()) {
      suffix = pattern.substr(pattern.size() - 1);
      middle = middle.substr(0, middle.size() - 1);
    }

    if (suffix.empty() && pattern.size() > prefix.size()) {
      suffix = pattern.substr(pattern.size() - 1);
    }

    if ((prefix + middle + suffix).size() < 19) {
      for (auto it = charset.rbegin(); it != charset.rend(); ++it) {
        std::string new_middle = *it + middle;
        to_try.emplace(new_middle, suffix);
      }
    }
  }

  return 0;
}
