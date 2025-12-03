// CommonTypes.h
#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <chrono>

using namespace std::chrono;

enum class NotificationStatus {
  CREATED,
  BUFFERED,
  PROCESSING,
  PROCESSED,
  REJECTED
};

#endif // COMMON_TYPES_H
