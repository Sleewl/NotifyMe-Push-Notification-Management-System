// PushNotificationSystem.h
#ifndef PUSH_NOTIFICATION_SYSTEM_H
#define PUSH_NOTIFICATION_SYSTEM_H

#include "Source.h"
#include "Buffer.h"
#include "Channel.h"
#include "Database.h"
#include "PlacementDispatcher.h"
#include "Event.h"
#include <vector>
#include <queue>
#include <chrono>


class PushNotificationSystem {
private:
  std::vector<Source> sources;
  Buffer buffer;
  std::vector<Channel> channels;
  Database database;
  PlacementDispatcher dispatcher;

  std::priority_queue<Event, std::vector<Event>, EventComparator> eventCalendar;

  double currentTime;
  bool simulationComplete;
  int totalNotifications;
  int maxNotifications;

  std::chrono::system_clock::time_point startTime;

public:
  PushNotificationSystem(int numSources, int bufferCapacity, int numChannels, int maxNotifs = 100);


  void runStepByStep();

private:
  void processNextEvent();
  void runUntilNextGen();
  void runUntilNextFreeChan();
  void displayState();
};

#endif // PUSH_NOTIFICATION_SYSTEM_H