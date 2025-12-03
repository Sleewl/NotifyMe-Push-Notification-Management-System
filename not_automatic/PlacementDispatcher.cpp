// PlacementDispatcher.cpp
#include "PlacementDispatcher.h"
#include <limits> 

PlacementDispatcher::PlacementDispatcher(Buffer* buf, std::vector<Channel*>* chans, Database* db)
  : buffer(buf), channels(chans), database(db) {
}

void PlacementDispatcher::handleNewNotification(const Notification& notification) {
  bool success = buffer->addNotification(notification, database);
  if (!success) {
  }
}

Channel* PlacementDispatcher::selectChannelByPriority() {
  Channel* selectedChannel = nullptr;
  int lowestPriority = std::numeric_limits<int>::max();

  for (Channel* channel : *channels) {
    if (!channel->isChannelBusy() && channel->getPriority() < lowestPriority) {
      selectedChannel = channel;
      lowestPriority = channel->getPriority();
    }
  }

  return selectedChannel;
}

void PlacementDispatcher::tryProcessFromBuffer() {
  Channel* targetChannel = selectChannelByPriority();

  if (targetChannel && !buffer->isEmpty()) {
    Notification notification = buffer->getNextNotification();

    if (notification.getId() > 0) { 
      double serviceTime = targetChannel->startProcessing(notification);
      database->recordDelivery(notification, serviceTime, targetChannel->getId());
    }
  }
}

int PlacementDispatcher::getBufferPointer() const {
  return buffer->getPointer();
}

const std::vector<Notification>& PlacementDispatcher::getBufferNotifications() const {
  return buffer->getNotifications();
}

const std::vector<bool>& PlacementDispatcher::getBufferOccupied() const {
  return buffer->getOccupied();
}