// PlacementDispatcher.cpp
#include "PlacementDispatcher.h"
#include <limits> // Для numeric_limits

PlacementDispatcher::PlacementDispatcher(Buffer* buf, std::vector<Channel*>* chans, Database* db)
  : buffer(buf), channels(chans), database(db) {
}

void PlacementDispatcher::handleNewNotification(const Notification& notification) {
  // Передаем database в addNotification для фиксации вытеснения (Д1ОО4)
  bool success = buffer->addNotification(notification, database);
  if (!success) {
    // Это может произойти только если буфер был полон и дисциплина Д1ОО4 не сработала
    // В нашей реализации addNotification всегда добавляет (заменяет или помещает)
    // Но если бы были другие дисциплины отказа, это место было бы для них
  }
}

Channel* PlacementDispatcher::selectChannelByPriority() {
  Channel* selectedChannel = nullptr;
  int lowestPriority = std::numeric_limits<int>::max(); // Высший приоритет - наименьшее число

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

    if (notification.getId() > 0) { // Убедиться, что уведомление валидное
      double serviceTime = targetChannel->startProcessing(notification);

      // Записать статистику (в пошаговом режиме вызывается при выборе из буфера)
      // В автоматическом режиме это тоже сработает
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