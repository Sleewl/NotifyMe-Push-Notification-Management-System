// PushNotificationSystem.cpp
#include "PushNotificationSystem.h"
#include <iostream>
#include <iomanip>
#include <exception>

PushNotificationSystem::PushNotificationSystem(int numSources, int bufferCapacity, int numChannels, int maxNotifs)
  : sources(), buffer(bufferCapacity), channels(), database(),
  dispatcher(&buffer, nullptr, &database),
  currentTime(0.0), simulationComplete(false), totalNotifications(0), maxNotifications(maxNotifs) {

  for (int i = 1; i <= numSources; i++) {
    sources.emplace_back(i, 0.5); 
  }

  channels = {
      Channel(1, 1, 2.0, 5.0), 
      Channel(2, 2, 2.0, 5.0),
      Channel(3, 3, 2.0, 5.0)
  };

  std::vector<Channel*>* channelPtrs = new std::vector<Channel*>();
  for (auto& ch : channels) {
    channelPtrs->push_back(&ch);
  }
  dispatcher = PlacementDispatcher(&buffer, channelPtrs, &database);

  for (auto& source : sources) {
    double nextGenTime = source.getNextGenerationTime(currentTime);
    Notification firstNotif = source.generateNotification();
    eventCalendar.push(Event(nextGenTime, "GEN", source.getId(), firstNotif.getId()));
    totalNotifications++;
  }

  startTime = std::chrono::system_clock::now();
}

void PushNotificationSystem::runStepByStep() {
  std::cout << "Push Notification Delivery System Simulation (Variant 17)\n";
  std::cout << "Disciplines: ÈÁ|ÈÇ1|ÏÇ2|Ä1ÎÇ1|Ä1ÎÎ4|Ä2Ï1|Ä2Á3|Ï32|ÎÄ2\n";
  std::cout << "--------------------------------------------------------\n";

  while (!simulationComplete && totalNotifications < maxNotifications) {
    displayState();

    char command;
    std::cout << "\nCommands:\n";
    std::cout << "S - Next Step (Process one event)\n";
    std::cout << "R - Run until next notification generation\n";
    std::cout << "T - Run until next channel free\n";
    std::cout << "F - Finish simulation (run all events)\n";
    std::cout << "Q - Quit\n";
    std::cout << "Enter command: ";
    std::cin >> command;
    command = std::toupper(command);

    switch (command) {
    case 'S':
      if (!eventCalendar.empty()) {
        processNextEvent();
      }
      else {
        simulationComplete = true;
      }
      break;
    case 'R':
      runUntilNextGen();
      break;
    case 'T':
      runUntilNextFreeChan();
      break;
    case 'F':
      while (!eventCalendar.empty() && totalNotifications < maxNotifications) {
        processNextEvent();
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
    auto endTime = std::chrono::system_clock::now();
    double totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() / 1000.0;
    database.printStatistics(totalTime);
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
      Notification newNotification = Notification(event.notificationId, event.sourceId);

      Channel* targetChannel = dispatcher.selectChannelByPriority();
      if (targetChannel) {
        double serviceTime = targetChannel->startProcessing(newNotification);
        database.recordDelivery(newNotification, serviceTime, targetChannel->getId());
        std::cout << "Notification " << event.notificationId << " from Source " << event.sourceId << " sent to Channel " << targetChannel->getId() << ".\n";
      }
      else {
        dispatcher.handleNewNotification(newNotification);
        std::cout << "Notification " << event.notificationId << " from Source " << event.sourceId << " sent to Buffer.\n";
      }

      Source& source = sources[event.sourceId - 1];
      double nextGenTime = source.getNextGenerationTime(currentTime);
      Notification nextNotif = source.generateNotification();
      eventCalendar.push(Event(nextGenTime, "GEN", source.getId(), nextNotif.getId()));
      totalNotifications++;

      dispatcher.tryProcessFromBuffer();

    }
    else if (event.type == "FREE_CHAN") {
      int channelId = event.channelId;
      Channel& channel = channels[channelId - 1];

      if (channel.isChannelBusy()) {
        Notification finishedNotification = channel.getCurrentNotification();

        channel.freeChannel();

        std::cout << "Channel " << channelId << " finished processing Notification " << finishedNotification.getId() << " and became free.\n";

        dispatcher.tryProcessFromBuffer();
      }
      else {
        std::cout << "Channel " << channelId << " was already free. No action taken.\n";
      }
    }
  }
  catch (const std::exception& e) {
    std::cerr << "Exception during processing event: " << e.what() << std::endl;
  }
}

void PushNotificationSystem::runUntilNextGen() {
  bool foundGen = false;
  while (!eventCalendar.empty() && !foundGen && totalNotifications < maxNotifications) {
    Event topEvent = eventCalendar.top();
    if (topEvent.type == "GEN") {
      foundGen = true;
    }
    processNextEvent();
  }
}

void PushNotificationSystem::runUntilNextFreeChan() {
  bool foundFreeChan = false;
  while (!eventCalendar.empty() && !foundFreeChan && totalNotifications < maxNotifications) {
    Event topEvent = eventCalendar.top();
    if (topEvent.type == "FREE_CHAN") {
      foundFreeChan = true;
    }
    processNextEvent();

  }
}

void PushNotificationSystem::displayState() {
  std::cout << "\n===== ÔÎÐÌÀËÈÇÎÂÀÍÍÀß ÑÕÅÌÀ ÌÎÄÅËÈ (ÎÄ2) =====\n";

  std::cout << "\n--- ÊÀËÅÍÄÀÐÜ ÑÎÁÛÒÈÉ ---\n";
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
  for (auto it = tempEvents.rbegin(); it != tempEvents.rend(); ++it) {
    eventCalendar.push(*it);
  }

  std::cout << "\n--- ÑÎÑÒÎßÍÈÅ ÁÓÔÅÐÀ (Capacity: " << buffer.getCapacity()
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

  std::cout << "\n--- ÑÎÑÒÎßÍÈÅ ÊÀÍÀËÎÂ ---\n";
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

  std::cout << "\n--- ÏÐÎÌÅÆÓÒÎ×ÍÀß ÑÒÀÒÈÑÒÈÊÀ ---\n";
  std::cout << "Time: " << std::fixed << std::setprecision(3) << currentTime << "\n";
  std::cout << "Total Notifications Processed: " << totalNotifications << "\n";
  std::cout << "Delivered: " << database.getDeliveredCount() << ", Rejected: " << database.getRejectedCount() << "\n";
  std::cout << "Rejection Rate: " << std::fixed << std::setprecision(3) << database.getRejectionRate() * 100 << "%\n";
}