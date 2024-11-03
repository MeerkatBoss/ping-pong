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

static constexpr size_t TcpScreamLength = 100 * 1024;

static inline constexpr char UdpScream[] = "I Have No TCP And I Must Scream";

static constexpr size_t UdpScreamLength = sizeof(UdpScream) - 1;

#endif /* Scream.hpp */
