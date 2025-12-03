// Channel.h
#ifndef CHANNEL_H
#define CHANNEL_H

#include "Notification.h" 
#include <random>


class Channel {
private:
  int id;
  int priority;
  bool isBusy;
  Notification currentNotification;
  double serviceTimeMin;
  double serviceTimeMax;
  std::mt19937 rng;
  std::uniform_real_distribution<double> uniformDist;

public:
  Channel(int id, int priority, double minTime, double maxTime);

  int getId() const;
  int getPriority() const;
  bool isChannelBusy() const;

  double startProcessing(Notification notification);

  void freeChannel();

  Notification getCurrentNotification() const;

  int getCurrentNotificationId() const;

  int getCurrentNotificationSourceId() const;
};

#endif // CHANNEL_H