// PlacementDispatcher.h
#ifndef PLACEMENT_DISPATCHER_H
#define PLACEMENT_DISPATCHER_H

#include "Buffer.h" // Для хранения указателя
#include "Channel.h" // Для хранения указателя
#include "Database.h" // Для хранения указателя
#include "Notification.h" // Для обработки Notification
#include <vector>

// Класс Диспетчера постановки
class PlacementDispatcher {
private:
  Buffer* buffer;
  std::vector<Channel*>* channels;
  Database* database;

public:
  PlacementDispatcher(Buffer* buf, std::vector<Channel*>* chans, Database* db);

  // Обработать новое уведомление от источника
  void handleNewNotification(const Notification& notification);

  // Выбрать канал по приоритету (Д2П1)
  Channel* selectChannelByPriority();

  // Попытаться обработать уведомление из буфера
  void tryProcessFromBuffer();

  // Получить указатель буфера для отображения
  int getBufferPointer() const;

  // Получить состояние буфера для отображения
  const std::vector<Notification>& getBufferNotifications() const;

  const std::vector<bool>& getBufferOccupied() const;
};

#endif // PLACEMENT_DISPATCHER_H