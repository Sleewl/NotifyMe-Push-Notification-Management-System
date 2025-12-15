// Buffer.cpp
#include "Buffer.h"
#include "Database.h" // Для recordRejection
#include <algorithm> // Для std::all_of, std::none_of, std::count

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

// Добавление уведомления (Д1ОЗ1 - кольцо)
// Теперь принимает Database для фиксации статистики вытеснения (Д1ОО4)
bool Buffer::addNotification(Notification notification, Database* db) {
  if (isFull()) {
    // Дисциплина отказа (Д1ОО4 - замена последнего)
    // Найти последнюю поступившую (с наибольшим указателем до инкремента)
    // В Д1ОЗ1 "последняя поступившая" - это уведомление в позиции `(pointer - 1) % capacity`
    int lastPos = (pointer == 0) ? capacity - 1 : pointer - 1;

    // Вытеснить последнее уведомление
    Notification displaced = notifications[lastPos];
    displaced.setStatus(NotificationStatus::REJECTED);

    // ЗАПИСАТЬ СТАТИСТИКУ О ВЫТСНЕНИИ
    if (db) {
      db->recordRejection(displaced);
    }

    // Поместить новое уведомление на освободившееся место
    notifications[lastPos] = notification;
    occupied[lastPos] = true;
    notification.setEnterBufferTime(); // Устанавливаем время входа в буфер
    notification.setStatus(NotificationStatus::BUFFERED);

    return true; // Уведомление добавлено, предыдущее вытеснено
  }

  // Найти свободное место, начиная с указателя (Д1ОЗ1)
  int startPos = pointer;
  do {
    if (!occupied[pointer]) {
      notifications[pointer] = notification;
      occupied[pointer] = true;
      notification.setEnterBufferTime(); // Устанавливаем время входа в буфер
      notification.setStatus(NotificationStatus::BUFFERED);

      // Переместить указатель на следующую позицию
      pointer = (pointer + 1) % capacity;
      return true;
    }
    pointer = (pointer + 1) % capacity;
  } while (pointer != startPos);

  return false; // Не должно сработать, если isFull() корректен
}

// Выбор уведомления (Д2Б3 - кольцо)
Notification Buffer::getNextNotification() {
  if (isEmpty()) {
    return Notification(0, 0); // Пустое уведомление
  }

  // Найти следующее уведомление, начиная с указателя (Д2Б3)
  int startPos = pointer;
  do {
    if (occupied[pointer]) {
      Notification notification = notifications[pointer];
      // Освободить ячейку
      occupied[pointer] = false;
      notifications[pointer] = Notification(0, 0);

      // Переместить указатель на следующую позицию (это ключевое для Д2Б3!)
      pointer = (pointer + 1) % capacity;

      // Установить время *покидания* буфера (начала обслуживания)
      notification.setLeaveBufferTime();

      return notification;
    }
    pointer = (pointer + 1) % capacity;
  } while (pointer != startPos);

  return Notification(0, 0); // Не должно сработать, если isEmpty() корректен
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