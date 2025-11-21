// Channel.cpp
#include "Channel.h"
#include <stdexcept>

Channel::Channel(int id, int priority, double minTime, double maxTime)
  : id(id), priority(priority), isBusy(false), serviceTimeMin(minTime), serviceTimeMax(maxTime),
  rng(std::random_device()()), uniformDist(minTime, maxTime) {
}

int Channel::getId() const { return id; }
int Channel::getPriority() const { return priority; }
bool Channel::isChannelBusy() const { return isBusy; }

double Channel::startProcessing(Notification notification) {
  if (isBusy) {
    throw std::runtime_error("Channel is already busy");
  }

  isBusy = true;
  currentNotification = notification;
  currentNotification.setEnterChannelTime();
  currentNotification.setStatus(NotificationStatus::PROCESSING);


  double serviceTime = uniformDist(rng);
  return serviceTime;
}

void Channel::freeChannel() {
  isBusy = false;
}

Notification Channel::getCurrentNotification() const {
  return currentNotification;
}

int Channel::getCurrentNotificationId() const {
  return currentNotification.getId();
}

int Channel::getCurrentNotificationSourceId() const {
  return currentNotification.getSourceId();
}