// PushNotificationSystem.cpp
#include "PushNotificationSystem.h"
#include <iostream>
#include <iomanip>
#include <exception> // Для std::exception

PushNotificationSystem::PushNotificationSystem(int numSources, int bufferCapacity, int numChannels, int maxNotifs)
  : sources(), buffer(bufferCapacity), channels(), database(),
  dispatcher(&buffer, nullptr, &database), // Установим channels позже
  eventCalendar(), // Используем typedef
  currentTime(0.0), simulationComplete(false), totalNotifications(0), maxNotifications(maxNotifs),
  // --- Установка интервалов для снапшотов ---
  snapshotIntervalTime(5.0), // Например, снимок каждые 5 единиц времени
  snapshotIntervalCount(100) // Или каждые 100 обработанных событий
{

  // Инициализация источников (ИБ, ИЗ1)
  for (int i = 1; i <= numSources; i++) {
    sources.emplace_back(i, 0.5); // Lambda = 0.5 (средний интервал 2.0)
  }

  // Инициализация каналов (Д2П1, П32)
  channels = {
      Channel(1, 1, 2.0, 5.0), // Канал 1 - высший приоритет
      Channel(2, 2, 2.0, 5.0), // Канал 2
      Channel(3, 3, 2.0, 5.0)  // Канал 3 - низший приоритет
  };

  // Установка указателя на каналы в диспетчере
  std::vector<Channel*>* channelPtrs = new std::vector<Channel*>();
  for (auto& ch : channels) {
    channelPtrs->push_back(&ch);
  }
  dispatcher = PlacementDispatcher(&buffer, channelPtrs, &database);

  // Запланировать первые события генерации для каждого источника
  for (auto& source : sources) {
    double nextGenTime = source.getNextGenerationTime(currentTime);
    Notification firstNotif = source.generateNotification();
    // ЗАПИСАТЬ ГЕНЕРАЦИЮ
    database.recordGeneration(source.getId());
    eventCalendar.push(Event(nextGenTime, "GEN", source.getId(), firstNotif.getId()));
    totalNotifications++;
  }

  startTime = std::chrono::system_clock::now();
}

void PushNotificationSystem::runStepByStep() {
  std::cout << "Push Notification Delivery System Simulation (Variant 17)\n";
  std::cout << "Disciplines: ИБ|ИЗ1|ПЗ2|Д1ОЗ1|Д1ОО4|Д2П1|Д2Б3|П32|ОД2\n";
  std::cout << "--------------------------------------------------------\n";

  int snapshotCounter = 0; // Счётчик для интервала по количеству событий

  while (!simulationComplete && totalNotifications < maxNotifications) {
    displayState();

    char command;
    std::cout << "\nCommands:\n";
    std::cout << "S - Next Step (Process one event)\n";
    std::cout << "R - Run until next notification generation\n";
    std::cout << "T - Run until next channel free\n"; // Теперь будет работать корректно, если FREE_CHAN генерируется
    std::cout << "F - Finish simulation (run all events)\n";
    std::cout << "Q - Quit\n";
    std::cout << "Enter command: ";
    std::cin >> command;
    command = std::toupper(command);

    switch (command) {
    case 'S':
      if (!eventCalendar.empty()) {
        processNextEvent();
        snapshotCounter++;
        // Проверяем интервал снапшота по количеству событий
        if (snapshotCounter >= snapshotIntervalCount) {
          database.snapshotStatistics(currentTime);
          snapshotCounter = 0; // Сброс счётчика
        }
      }
      else {
        simulationComplete = true;
      }
      break;
    case 'R':
      runUntilEventType("GEN");
      break;
    case 'T': // --- КОМАНДА T ---
      // В текущей реализации СОБЫТИЯ ОСВОБОЖДЕНИЯ КАНАЛА (FREE_CHAN) НЕ ГЕНЕРИРУЮТСЯ!
      // Поэтому T будет работать как "Run until limit", если буфер полон.
      // Для корректной работы T, нужно генерировать события FREE_CHAN.
      // Это требует изменений: Channel должен *знать*, когда закончит обслуживание.
      // PushNotificationSystem должен планировать событие FREE_CHAN в календарь.
      // processNextEvent должен обрабатывать событие FREE_CHAN.
      // Пока что, T просто выполнит цикл до лимита или до освобождения канала (если оно как-то произойдёт).
      runUntilEventType("FREE_CHAN");
      break;
    case 'F':
      while (!eventCalendar.empty() && totalNotifications < maxNotifications) {
        processNextEvent();
        snapshotCounter++;
        if (snapshotCounter >= snapshotIntervalCount) {
          database.snapshotStatistics(currentTime);
          snapshotCounter = 0;
        }
      }
      simulationComplete = true;
      break;
    case 'Q':
      simulationComplete = true;
      break;
    default:
      std::cout << "Invalid command.\n";
    }
  }

  if (totalNotifications >= maxNotifications) {
    simulationComplete = true;
  }

  if (simulationComplete) {
    finalizeSimulation(); // Вызываем финализацию
  }
}

void PushNotificationSystem::processNextEvent() {
  if (eventCalendar.empty()) {
    simulationComplete = true;
    return;
  }

  Event event = eventCalendar.top();
  eventCalendar.pop();
  currentTime = event.time;

  std::cout << "\n--- PROCESSING EVENT ---\n";
  std::cout << "Time: " << std::fixed << std::setprecision(3) << currentTime
    << ", Type: " << event.type;
  if (event.type == "GEN") {
    std::cout << ", Source: " << event.sourceId << ", Notification: " << event.notificationId;
  }
  else if (event.type == "PROC_END" || event.type == "FREE_CHAN") {
    std::cout << ", Channel: " << event.channelId;
  }
  std::cout << "\n";

  try {
    if (event.type == "GEN") {
      // Обработка события генерации
      Notification newNotification = Notification(event.notificationId, event.sourceId);

      // ЗАПИСАТЬ ГЕНЕРАЦИЮ
      database.recordGeneration(event.sourceId);

      // Обработать уведомление диспетчером постановки
      // Сначала пробуем поставить в канал
      Channel* targetChannel = dispatcher.selectChannelByPriority();
      if (targetChannel) {
        double serviceTime = targetChannel->startProcessing(newNotification);
        // Записать статистику при постановке в канал (в пошаговом режиме)
        // serviceTime передаётся из канала
        database.recordDelivery(newNotification, serviceTime, targetChannel->getId());
        std::cout << "Notification " << event.notificationId << " from Source " << event.sourceId << " sent to Channel " << targetChannel->getId() << ".\n";
      }
      else {
        // Если каналов нет, поставить в буфер
        dispatcher.handleNewNotification(newNotification);
        std::cout << "Notification " << event.notificationId << " from Source " << event.sourceId << " sent to Buffer.\n";
      }

      // Запланировать следующее событие генерации от этого источника
      Source& source = sources[event.sourceId - 1]; // Индексация с 0
      double nextGenTime = source.getNextGenerationTime(currentTime);
      Notification nextNotif = source.generateNotification();
      // ЗАПИСАТЬ ГЕНЕРАЦИЮ СЛЕДУЮЩЕЙ
      database.recordGeneration(source.getId());
      eventCalendar.push(Event(nextGenTime, "GEN", source.getId(), nextNotif.getId()));
      totalNotifications++;

      // Попытаться обработать уведомление из буфера, если есть свободный канал
      dispatcher.tryProcessFromBuffer();

    }
    else if (event.type == "FREE_CHAN") { // Обработка события освобождения канала (не генерируется в этой версии)
      int channelId = event.channelId;
      Channel& channel = channels[channelId - 1]; // Индексация с 0

      if (channel.isChannelBusy()) {
        Notification finishedNotification = channel.getCurrentNotification();

        // Записать статистику *только при завершении обслуживания*
        // double serviceTime = finishedNotification.getSystemTime() - finishedNotification.getWaitTime(); // Приблизительно
        // database.recordDelivery(finishedNotification, serviceTime, channelId); // Уже записано при выборе из буфера

        // Освободить канал
        channel.freeChannel();

        std::cout << "Channel " << channelId << " finished processing Notification " << finishedNotification.getId() << " and became free.\n";

        // Попытаться обработать уведомление из буфера, так как канал освободился
        dispatcher.tryProcessFromBuffer();
      }
      else {
        std::cout << "Channel " << channelId << " was already free. No action taken.\n";
      }
    }
  }
  catch (const std::exception& e) {
    std::cerr << "Exception during processing event: " << e.what() << std::endl;
    // Логика обработки исключения (например, пропустить событие)
  }
}

void PushNotificationSystem::runUntilEventType(const std::string& eventType) {
  bool found = false;
  while (!eventCalendar.empty() && !found && totalNotifications < maxNotifications) {
    Event topEvent = eventCalendar.top();
    if (topEvent.type == eventType) {
      found = true;
    }
    processNextEvent(); // Обрабатываем текущее событие
    // Если это было искомое событие, цикл завершится после обработки
  }
}

void PushNotificationSystem::displayState() {
  std::cout << "\n===== ФОРМАЛИЗОВАННАЯ СХЕМА МОДЕЛИ (ОД2) =====\n";

  // Календарь событий (первые 5)
  std::cout << "\n--- КАЛЕНДАРЬ СОБЫТИЙ ---\n";
  std::cout << "Time | Type     | Details\n";
  std::cout << "-----|----------|--------\n";

  std::vector<Event> tempEvents;
  int count = 0;
  while (!eventCalendar.empty() && count < 5) {
    Event e = eventCalendar.top(); eventCalendar.pop();
    tempEvents.push_back(e);
    std::cout << std::fixed << std::setprecision(3)
      << std::setw(5) << e.time << " | "
      << std::setw(8) << e.type << " | ";
    if (e.type == "GEN") {
      std::cout << "Src: " << e.sourceId << ", Notif: " << e.notificationId;
    }
    else if (e.type == "FREE_CHAN") {
      std::cout << "Chan: " << e.channelId;
    }
    std::cout << "\n";
    count++;
  }
  // Восстановить события в календарь
  for (auto it = tempEvents.rbegin(); it != tempEvents.rend(); ++it) {
    eventCalendar.push(*it);
  }

  // Состояние буфера
  std::cout << "\n--- СОСТОЯНИЕ БУФЕРА (Capacity: " << buffer.getCapacity()
    << ", Used: " << buffer.getUsedSlots() << ", Pointer: " << dispatcher.getBufferPointer() << ") ---\n";
  std::cout << "Pos | Occupied | Notification ID | Source | Status\n";
  std::cout << "----|----------|-----------------|--------|-------\n";

  const auto& notifs = dispatcher.getBufferNotifications();
  const auto& occupied = dispatcher.getBufferOccupied();

  for (int i = 0; i < notifs.size(); i++) {
    std::cout << std::setw(3) << i << " | "
      << std::setw(8) << (occupied[i] ? "YES" : "NO") << " | "
      << std::setw(15) << (occupied[i] ? std::to_string(notifs[i].getId()) : "Empty") << " | "
      << std::setw(6) << (occupied[i] ? std::to_string(notifs[i].getSourceId()) : "-") << " | "
      << (occupied[i] ? notifs[i].getStatusString() : "EMPTY") << "\n";
  }

  // Состояние каналов
  std::cout << "\n--- СОСТОЯНИЕ КАНАЛОВ ---\n";
  std::cout << "Chan | Priority | Busy | Current Notification (Source)\n";
  std::cout << "-----|----------|------|-----------------------------\n";
  for (const auto& channel : channels) {
    std::string busyStatus = channel.isChannelBusy() ? "YES" : "NO";
    std::string currentNotif = "None";
    if (channel.isChannelBusy()) {
      currentNotif = std::to_string(channel.getCurrentNotificationId()) +
        " (" + std::to_string(channel.getCurrentNotificationSourceId()) + ")";
    }

    std::cout << std::setw(4) << channel.getId() << " | "
      << std::setw(8) << channel.getPriority() << " | "
      << std::setw(4) << busyStatus << " | "
      << currentNotif << "\n";
  }

  // Промежуточная статистика
  std::cout << "\n--- ПРОМЕЖУТОЧНАЯ СТАТИСТИКА ---\n";
  std::cout << "Time: " << std::fixed << std::setprecision(3) << currentTime << "\n";
  std::cout << "Total Notifications Processed: " << totalNotifications << "\n";
  std::cout << "Delivered: " << database.getDeliveredCount() << ", Rejected: " << database.getRejectedCount() << "\n";
  std::cout << "Rejection Rate: " << std::fixed << std::setprecision(3) << database.getRejectionRate() * 100 << "%\n";
}

// --- Новый метод finalizeSimulation ---
void PushNotificationSystem::finalizeSimulation() {
  auto endTime = std::chrono::system_clock::now();
  double totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() / 1000.0;

  std::cout << "\n===== ЗАВЕРШЕНИЕ СИМУЛЯЦИИ =====\n";
  std::cout << "Общее модельное время: " << totalTime << " секунд\n";

  // Вывод ОР1 (Сводной таблицы) - передаём totalTime
  database.printStatistics(totalTime);

  // Вывод ОР2 (Данных для графиков) - не требует totalTime
  database.printGraphData();

  std::cout << "\nСимуляция завершена. Результаты выведены.\n";
}
// --------------------------------------