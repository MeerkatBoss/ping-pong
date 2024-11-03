#include "Command.hpp"
#include <algorithm>
#include <cctype>
#include <ios>
#include <string>

namespace client {

std::istream& operator>>(std::istream& is, Command& command) {
  std::string string;
  is >> string;
  std::transform(string.begin(), string.end(), string.begin(),
    [](char ch) { return char(tolower(ch)); }
  );

  if (string == "ping") {
    command = Command::Ping;
  } else if (string == "scream") {
    command = Command::Scream;
  } else if (string == "stop") {
    command = Command::Stop;
  } else {
    is.setstate(std::ios_base::failbit);
  }

  return is;
}

} // namespace client
