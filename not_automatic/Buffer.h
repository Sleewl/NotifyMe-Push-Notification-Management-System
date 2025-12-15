// Buffer.h
#ifndef BUFFER_H
#define BUFFER_H

#include "Notification.h" // Для хранения Notification
#include <vector>

// Предварительное объявление Database (forward declaration)
class Database;

// Класс Буфера (с кольцевой организацией - Д1ОЗ1)
class Buffer {
private:
  int capacity;
  int pointer; // Указатель на следующую позицию для *поиска* свободного места при *добавлении* (Д1ОЗ1)
  std::vector<Notification> notifications;
  std::vector<bool> occupied; // Для отслеживания занятых ячеек

public:
  Buffer(int capacity);

  bool isFull() const;
  bool isEmpty() const;

  // Добавление уведомления (Д1ОЗ1 - кольцо)
  // Теперь принимает Database для фиксации статистики вытеснения (Д1ОО4)
  bool addNotification(Notification notification, Database* db = nullptr);

  // Выбор уведомления (Д2Б3 - кольцо)
  Notification getNextNotification();

  int getPointer() const;
  int getCapacity() const;
  int getUsedSlots() const;

  const std::vector<Notification>& getNotifications() const;
  const std::vector<bool>& getOccupied() const;
};

#endif // BUFFER_H