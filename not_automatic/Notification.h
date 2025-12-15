// Notification.h
#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include "CommonTypes.h"
#include <chrono>
#include <string>

class Notification {
private:
  int id;
  int sourceId;
  std::chrono::system_clock::time_point creationTime;
  NotificationStatus status;
  std::chrono::system_clock::time_point enterBufferTime;
  std::chrono::system_clock::time_point leaveBufferTime; // Время выхода из буфера (вход в канал)
  std::chrono::system_clock::time_point enterChannelTime; // Время входа в канал (конец обслуживания)

public:
  Notification(); // Для инициализации буфера
  Notification(int id, int sourceId);

  int getId() const;
  int getSourceId() const;
  NotificationStatus getStatus() const;
  void setStatus(NotificationStatus newStatus);

  void setEnterBufferTime();
  void setLeaveBufferTime(); // Новое
  void setEnterChannelTime();

  // Время в буфере (T_ожидания)
  double getWaitTime() const;
  // Время в канале (T_обслуживания) - приблизительно, рассчитывается при завершении
  // double getServiceTime() const; // Не хранится, передаётся из Channel
  // Время в системе (T_пребывания)
  double getSystemTime() const;

  std::string getStatusString() const;
};

#endif // NOTIFICATION_H