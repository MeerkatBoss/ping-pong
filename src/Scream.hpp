/**
 * @file Scream.hpp
 * @author MeerkatBoss (solodovnikov.ia@phystech.su)
 *
 * @brief
 *
 * @version 0.0.1
 * @date 2024-11-03
 *
 * @copyright Copyright MeerkatBoss (c) 2024
 */
#ifndef __SCREAM_HPP
#define __SCREAM_HPP

#include <cstddef>
#include <string_view>

static constexpr size_t TcpScreamLength = 20 * 1024;

static constexpr std::string_view UdpScream = "I Have No TCP And I Must Scream";

#endif /* Scream.hpp */
