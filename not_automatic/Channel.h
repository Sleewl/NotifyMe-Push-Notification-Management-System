// Channel.h
#ifndef CHANNEL_H
#define CHANNEL_H

#include "Notification.h"
#include <random>

class Channel {
private:
  int id;
  int priority; // Приоритет канала (Д2П1 - выбор по приоритету)
  bool isBusy;
  Notification currentNotification;
  double serviceTimeMin;
  double serviceTimeMax;
  std::mt19937 rng;
  std::uniform_real_distribution<double> uniformDist; // Для времени обслуживания (П32)

public:
  Channel(int id, int priority, double minTime, double maxTime);

  int getId() const;
  int getPriority() const;
  bool isChannelBusy() const;

  // Начать обработку уведомления, возвращает время обслуживания
  double startProcessing(Notification notification);

  void freeChannel();

  Notification getCurrentNotification() const;

  int getCurrentNotificationId() const;

  int getCurrentNotificationSourceId() const;
};

#endif // CHANNEL_H