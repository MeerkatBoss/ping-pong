/**
 * @file Command.hpp
 * @author MeerkatBoss (solodovnikov.ia@phystech.su)
 *
 * @brief
 *
 * @version 0.0.1
 * @date 2024-11-03
 *
 * @copyright Copyright MeerkatBoss (c) 2024
 */
#ifndef __CLIENT_COMMAND_HPP
#define __CLIENT_COMMAND_HPP

#include <istream>

enum class Command {
  Unknown, Ping, Scream, Stop
};

std::istream& operator>>(std::istream& is, Command& command);


#endif /* Command.hpp */
