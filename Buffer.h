// Buffer.h
#ifndef BUFFER_H
#define BUFFER_H

#include "Notification.h" 
#include <vector>


class Database;

class Buffer {
private:
  int capacity;
  int pointer; 
  std::vector<Notification> notifications;
  std::vector<bool> occupied; 

public:
  Buffer(int capacity);

  bool isFull() const;
  bool isEmpty() const;


  bool addNotification(Notification notification, Database* db = nullptr);


  Notification getNextNotification();

  int getPointer() const;
  int getCapacity() const;
  int getUsedSlots() const;

  const std::vector<Notification>& getNotifications() const;
  const std::vector<bool>& getOccupied() const;
};

#endif // BUFFER_H