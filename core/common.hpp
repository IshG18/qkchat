#pragma once
#include <memory>
#include <thread>
#include <mutex>
#include <deque>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <functional>
#include <vector>

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>