// Notification.cpp
#include "Notification.h"

Notification::Notification()
  : id(0), sourceId(0), creationTime(std::chrono::system_clock::now()),
  status(NotificationStatus::REJECTED),
  enterBufferTime(std::chrono::system_clock::time_point::min()),
  enterChannelTime(std::chrono::system_clock::time_point::min()) {
}

Notification::Notification(int id, int sourceId)
  : id(id), sourceId(sourceId), creationTime(std::chrono::system_clock::now()),
  status(NotificationStatus::CREATED),
  enterBufferTime(std::chrono::system_clock::time_point::min()),
  enterChannelTime(std::chrono::system_clock::time_point::min()) {
}

int Notification::getId() const { return id; }
int Notification::getSourceId() const { return sourceId; }
std::chrono::system_clock::time_point Notification::getCreationTime() const { return creationTime; }
NotificationStatus Notification::getStatus() const { return status; }
void Notification::setStatus(NotificationStatus newStatus) { status = newStatus; }

void Notification::setEnterBufferTime() { enterBufferTime = std::chrono::system_clock::now(); }
void Notification::setEnterChannelTime() { enterChannelTime = std::chrono::system_clock::now(); }

double Notification::getWaitTime() const {
  if (enterBufferTime.time_since_epoch().count() != 0 && enterChannelTime.time_since_epoch().count() != 0) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(enterChannelTime - enterBufferTime).count() / 1000.0;
  }
  return 0.0;
}

double Notification::getSystemTime() const {
  if (creationTime.time_since_epoch().count() != 0 && enterChannelTime.time_since_epoch().count() != 0) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(enterChannelTime - creationTime).count() / 1000.0;
  }
  return 0.0;
}

std::string Notification::getStatusString() const {
  switch (status) {
  case NotificationStatus::CREATED: return "CREATED";
  case NotificationStatus::BUFFERED: return "BUFFERED";
  case NotificationStatus::PROCESSING: return "PROCESSING";
  case NotificationStatus::PROCESSED: return "PROCESSED";
  case NotificationStatus::REJECTED: return "REJECTED";
  default: return "UNKNOWN";
  }
}