// Buffer.cpp
#include "Buffer.h"
#include "Database.h" 
#include <algorithm>

Buffer::Buffer(int capacity) : capacity(capacity), pointer(0) {
  notifications.resize(capacity);
  occupied.resize(capacity, false);
}

bool Buffer::isFull() const {
  return std::all_of(occupied.begin(), occupied.end(), [](bool o) { return o; });
}

bool Buffer::isEmpty() const {
  return std::none_of(occupied.begin(), occupied.end(), [](bool o) { return o; });
}


bool Buffer::addNotification(Notification notification, Database* db) {
  if (isFull()) {

    int lastPos = (pointer == 0) ? capacity - 1 : pointer - 1;


    Notification displaced = notifications[lastPos];
    displaced.setStatus(NotificationStatus::REJECTED);

    if (db) {
      db->recordRejection(displaced);
    }

    notifications[lastPos] = notification;
    occupied[lastPos] = true;
    notification.setEnterBufferTime(); 
    notification.setStatus(NotificationStatus::BUFFERED);

    return true; 
  }

  int startPos = pointer;
  do {
    if (!occupied[pointer]) {
      notifications[pointer] = notification;
      occupied[pointer] = true;
      notification.setEnterBufferTime();
      notification.setStatus(NotificationStatus::BUFFERED);

      this->pointer = (pointer + 1) % capacity;
      return true;
    }
    pointer = (pointer + 1) % capacity;
  } while (pointer != startPos); 

  return false;
}

Notification Buffer::getNextNotification() {
  if (isEmpty()) {
    return Notification(0, 0); 
  }

  int startPos = pointer;
  do {
    if (occupied[pointer]) {
      Notification notification = notifications[pointer];
      occupied[pointer] = false;
      notifications[pointer] = Notification(0, 0);

      int nextPos = (pointer + 1) % capacity;
      this->pointer = nextPos;

      return notification;
    }
    pointer = (pointer + 1) % capacity;
  } while (pointer != startPos);


  return Notification(0, 0);
}

int Buffer::getPointer() const { return pointer; }
int Buffer::getCapacity() const { return capacity; }
int Buffer::getUsedSlots() const {
  return std::count(occupied.begin(), occupied.end(), true);
}

const std::vector<Notification>& Buffer::getNotifications() const {
  return notifications;
}

const std::vector<bool>& Buffer::getOccupied() const {
  return occupied;
}