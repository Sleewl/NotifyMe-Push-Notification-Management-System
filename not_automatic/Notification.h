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
  std::chrono::system_clock::time_point enterChannelTime;

public:
  Notification();

  Notification(int id, int sourceId);

  int getId() const;
  int getSourceId() const;
  std::chrono::system_clock::time_point getCreationTime() const;
  NotificationStatus getStatus() const;
  void setStatus(NotificationStatus newStatus);


  void setEnterBufferTime();
  void setEnterChannelTime();

  double getWaitTime() const;
  double getSystemTime() const; 

  std::string getStatusString() const;
};

#endif 
