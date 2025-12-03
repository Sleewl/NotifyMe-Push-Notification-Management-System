// Database.h
#ifndef DATABASE_H
#define DATABASE_H

#include <map>
#include <string>

#include "Notification.h" 

class Database {
private:
  int deliveredCount;
  int rejectedCount;
  std::map<int, int> sourceDelivered;
  std::map<int, int> sourceRejected;
  std::map<int, double> sourceTotalWaitTime;
  std::map<int, double> sourceTotalSystemTime;
  std::map<int, int> sourceProcessedCount; 
  std::map<int, int> channelUsage;
  std::map<int, double> channelTotalServiceTime;

public:
  Database();

  void recordDelivery(const Notification& notification, double serviceTime, int channelId);
  void recordRejection(const Notification& notification);
  void reset();

  int getDeliveredCount() const;
  int getRejectedCount() const;
  int getTotalProcessed() const;

  double getRejectionRate() const;

  double getSourceAvgWaitTime(int sourceId) const;
  double getSourceAvgSystemTime(int sourceId) const;

  int getSourceDelivered(int sourceId) const;
  int getSourceRejected(int sourceId) const;

  int getChannelUsage(int channelId) const;
  double getChannelUtilization(int channelId, double totalTime) const;

  void printStatistics(double totalTime) const;
};

#endif // DATABASE_H