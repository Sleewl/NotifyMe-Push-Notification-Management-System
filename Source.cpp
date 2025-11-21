// Source.cpp
#include "Source.h"

Source::Source(int id, double lambda)
  : id(id), lambda(lambda), notificationCount(0), rng(std::random_device()()),
  expDist(lambda) {
}

double Source::getNextGenerationTime(double currentTime) {
  return currentTime + expDist(rng);
}

Notification Source::generateNotification() {
  notificationCount++;
  return Notification(notificationCount, id);
}

int Source::getId() const { return id; }
int Source::getGeneratedCount() const { return notificationCount; }