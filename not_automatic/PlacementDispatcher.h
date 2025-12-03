// PlacementDispatcher.h
#ifndef PLACEMENT_DISPATCHER_H
#define PLACEMENT_DISPATCHER_H

#include "Buffer.h"
#include "Channel.h" 
#include "Database.h"
#include "Notification.h" 
#include <vector>


class PlacementDispatcher {
private:
  Buffer* buffer;
  std::vector<Channel*>* channels;
  Database* database;

public:
  PlacementDispatcher(Buffer* buf, std::vector<Channel*>* chans, Database* db);

  void handleNewNotification(const Notification& notification);

  Channel* selectChannelByPriority();

  void tryProcessFromBuffer();

  int getBufferPointer() const;

  const std::vector<Notification>& getBufferNotifications() const;

  const std::vector<bool>& getBufferOccupied() const;
};

#endif // PLACEMENT_DISPATCHER_H