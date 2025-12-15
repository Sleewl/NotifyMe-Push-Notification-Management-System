// PushNotificationSystem.h
#ifndef PUSH_NOTIFICATION_SYSTEM_H
#define PUSH_NOTIFICATION_SYSTEM_H

#include "Source.h" // Для хранения вектора
#include "Buffer.h" // Для хранения объекта
#include "Channel.h" // Для хранения вектора
#include "Database.h" // Для хранения объекта
#include "PlacementDispatcher.h" // Для хранения объекта
#include "CommonTypes.h" // Для Event, EventComparator
#include <vector>
#include <queue>
#include <chrono>

// Основной класс Системы
class PushNotificationSystem {
private:
  std::vector<Source> sources;
  Buffer buffer;
  std::vector<Channel> channels;
  Database database;
  PlacementDispatcher dispatcher;

  EventQueue eventCalendar; // Теперь используем typedef из CommonTypes.h

  double currentTime;
  bool simulationComplete;
  int totalNotifications;
  int maxNotifications;

  // Параметры для автоматического режима
  double snapshotIntervalTime; // Интервал времени для снятия снапшотов
  int snapshotIntervalCount;   // Интервал количества событий для снятия снапшотов

  // Для отслеживания времени работы системы
  std::chrono::system_clock::time_point startTime;

public:
  PushNotificationSystem(int numSources, int bufferCapacity, int numChannels, int maxNotifs = 100);

  // Запуск пошаговой симуляции
  void runStepByStep();

private:
  void processNextEvent();
  void runUntilEventType(const std::string& eventType);
  void displayState();
  void finalizeSimulation(); // Новый метод для финализации
};

#endif // PUSH_NOTIFICATION_SYSTEM_H