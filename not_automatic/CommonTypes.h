// CommonTypes.h
#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <chrono>
#include <string>
#include <functional> // Для std::function

// Статусы уведомлений (Заявок)
enum class NotificationStatus {
  CREATED,
  BUFFERED,
  PROCESSING,
  PROCESSED,
  REJECTED
};

// Структура События
struct Event {
  double time;
  std::string type; // "GEN", "FREE_CHAN"
  int sourceId;     // Для событий генерации
  int notificationId; // Для событий генерации
  int channelId;    // Для событий освобождения канала

  Event(double t, const std::string& tp, int src = -1, int nid = -1, int cid = -1)
    : time(t), type(tp), sourceId(src), notificationId(nid), channelId(cid) {
  }
};

// Компаратор для std::priority_queue (min-heap по времени)
struct EventComparator {
  bool operator()(const Event& a, const Event& b) const {
    return a.time > b.time; // min-heap по времени
  }
};

// Тип для календаря событий
using EventQueue = std::priority_queue<Event, std::vector<Event>, EventComparator>;

#endif // COMMON_TYPES_H