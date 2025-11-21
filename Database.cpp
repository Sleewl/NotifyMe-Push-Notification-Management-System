// Database.cpp
#include "Database.h" 
#include <iostream>
#include <iomanip>

Database::Database() : deliveredCount(0), rejectedCount(0) {}


void Database::recordDelivery(const Notification& notification, double serviceTime, int channelId) {
  deliveredCount++;
  sourceDelivered[notification.getSourceId()]++;
  sourceTotalWaitTime[notification.getSourceId()] += notification.getWaitTime();
  sourceTotalSystemTime[notification.getSourceId()] += notification.getSystemTime();
  sourceProcessedCount[notification.getSourceId()]++;

  channelUsage[channelId]++;
  channelTotalServiceTime[channelId] += serviceTime;
}

void Database::recordRejection(const Notification& notification) {
  rejectedCount++;
  sourceRejected[notification.getSourceId()]++;
}


void Database::reset() {
  deliveredCount = 0;
  rejectedCount = 0;
  sourceDelivered.clear();
  sourceRejected.clear();
  sourceTotalWaitTime.clear();
  sourceTotalSystemTime.clear();
  sourceProcessedCount.clear();
  channelUsage.clear();
  channelTotalServiceTime.clear();
}

int Database::getDeliveredCount() const { return deliveredCount; }
int Database::getRejectedCount() const { return rejectedCount; }
int Database::getTotalProcessed() const { return deliveredCount + rejectedCount; }

double Database::getRejectionRate() const {
  int total = getTotalProcessed();
  return (total > 0) ? static_cast<double>(rejectedCount) / total : 0.0;
}

double Database::getSourceAvgWaitTime(int sourceId) const {
  auto countIt = sourceProcessedCount.find(sourceId);
  auto timeIt = sourceTotalWaitTime.find(sourceId);

  if (countIt != sourceProcessedCount.end() && countIt->second > 0 &&
    timeIt != sourceTotalWaitTime.end()) {
    return timeIt->second / countIt->second;
  }
  return 0.0;
}

double Database::getSourceAvgSystemTime(int sourceId) const {
  auto countIt = sourceProcessedCount.find(sourceId);
  auto timeIt = sourceTotalSystemTime.find(sourceId);

  if (countIt != sourceProcessedCount.end() && countIt->second > 0 &&
    timeIt != sourceTotalSystemTime.end()) {
    return timeIt->second / countIt->second;
  }
  return 0.0;
}

int Database::getSourceDelivered(int sourceId) const {
  auto it = sourceDelivered.find(sourceId);
  return (it != sourceDelivered.end()) ? it->second : 0;
}

int Database::getSourceRejected(int sourceId) const {
  auto it = sourceRejected.find(sourceId);
  return (it != sourceRejected.end()) ? it->second : 0;
}

int Database::getChannelUsage(int channelId) const {
  auto it = channelUsage.find(channelId);
  return (it != channelUsage.end()) ? it->second : 0;
}

double Database::getChannelUtilization(int channelId, double totalTime) const {
  auto usageIt = channelUsage.find(channelId);
  auto timeIt = channelTotalServiceTime.find(channelId);

  if (usageIt != channelUsage.end() && timeIt != channelTotalServiceTime.end()) {
    return totalTime > 0 ? timeIt->second / totalTime : 0.0;
  }
  return 0.0;
}

void Database::printStatistics(double totalTime) const {
  std::cout << "\n===== СВОДНАЯ ТАБЛИЦА (ОР1) =====\n";
  std::cout << "Всего обработано: " << getTotalProcessed() << "\n";
  std::cout << "Доставлено: " << deliveredCount << "\n";
  std::cout << "Вытеснено: " << rejectedCount << "\n";

  double totalSystemTimeAll = 0.0;
  int totalProcessedForAvg = 0;
  for (int i = 1; i <= 3; i++) {
    auto timeIt = sourceTotalSystemTime.find(i);
    auto countIt = sourceProcessedCount.find(i);
    if (timeIt != sourceTotalSystemTime.end() && countIt != sourceProcessedCount.end()) {
      totalSystemTimeAll += timeIt->second;
      totalProcessedForAvg += countIt->second;
    }
  }
  std::cout << "Среднее время доставки (все): " << std::fixed << std::setprecision(3)
    << (totalProcessedForAvg > 0 ? totalSystemTimeAll / totalProcessedForAvg : 0.0) << "\n";

  for (int i = 1; i <= 3; i++) {
    std::cout << "\nИсточник " << i << ":\n";
    std::cout << "  Доставлено: " << getSourceDelivered(i) << "\n";
    std::cout << "  Вытеснено: " << getSourceRejected(i) << "\n";
    std::cout << "  Среднее время ожидания: " << std::fixed << std::setprecision(3)
      << getSourceAvgWaitTime(i) << "\n";
    std::cout << "  Среднее время в системе: " << std::fixed << std::setprecision(3)
      << getSourceAvgSystemTime(i) << "\n";
  }

  for (int i = 1; i <= 3; i++) {
    std::cout << "\nКанал " << i << ":\n";
    std::cout << "  Загрузка: " << std::fixed << std::setprecision(3)
      << getChannelUtilization(i, totalTime) * 100 << "%\n";
  }
}