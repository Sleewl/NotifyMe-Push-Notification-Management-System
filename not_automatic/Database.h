// Database.h
#ifndef DATABASE_H
#define DATABASE_H

#include "Notification.h"
#include <map>
#include <string>
#include <vector>

// Предварительное объявление (forward declaration)
class Database;

// Класс Базы Данных для статистики
class Database {
private:
  int deliveredCount;
  int rejectedCount;
  // --- Для p_отк = m/n_gen ---
  std::map<int, int> sourceGeneratedCount; // Общее количество сгенерированных (n_gen) - нужно отдельно от delivered/rejected
  // -----------------------------
  std::map<int, int> sourceDelivered; // Количество доставленных (начало обслуживания)
  std::map<int, int> sourceRejected;  // Количество вытесненных (m_rej)
  // --- Для T_ожидания (T_БП) ---
  std::map<int, double> sourceTotalWaitTime; // Сумма T_ожидания
  std::map<int, double> sourceTotalWaitTimeSquared; // Сумма квадратов T_ожидания (для дисперсии)
  std::map<int, int> sourceWaitedCount; // Количество заявок, которые ждали (для усреднения и дисперсии T_ож)
  // ------------------------------
  // --- Для T_обслуживания (T_об) ---
  std::map<int, double> sourceTotalServiceTime; // Сумма T_обслуживания (переданное из канала)
  std::map<int, double> sourceTotalServiceTimeSquared; // Сумма квадратов T_обслуживания (для дисперсии)
  std::map<int, int> sourceServicedCount; // Количество заявок, которые обслуживались (для усреднения и дисперсии T_об)
  // -----------------------------------
  // --- Для T_пребывания (T_преб) ---
  std::map<int, double> sourceTotalSystemTime; // Сумма T_пребывания (для проверки T_ож + T_об)
  std::map<int, double> sourceTotalSystemTimeSquared; // Сумма квадратов T_пребывания (для дисперсии)
  std::map<int, int> sourceProcessedCount; // Количество заявок, завершивших обслуживание (для усреднения и дисперсии T_преб)
  // ----------------------------------
  std::map<int, int> channelUsage; // Количество раз, когда канал начал обслуживание
  std::map<int, double> channelTotalServiceTime; // Суммарное время обслуживания каналом
  std::map<int, double> channelTotalServiceTimeSquared; // Сумма квадратов времени обслуживания каналом (для дисперсии)

  // --- Данные для графиков ---
  std::vector<double> timePointsForGraphs;
  std::vector<std::map<int, double>> rejectionRatesOverTime; // p_отк = rejected / generated
  std::vector<std::map<int, double>> avgWaitTimesOverTime; // avg T_ож
  std::vector<std::map<int, double>> avgServiceTimesOverTime; // avg T_об
  std::vector<std::map<int, double>> avgSystemTimesOverTime; // avg T_преб
  std::vector<std::map<int, double>> channelLoadsOverTime; // load
  // ---------------------------

public:
  Database();

  // --- Методы записи ---
  // serviceTime передаётся из канала
  void recordDelivery(const Notification& notification, double serviceTime, int channelId);
  void recordRejection(const Notification& notification);
  void recordGeneration(int sourceId); // Новое: для учёта n_gen
  // -------------------
  void reset();

  // --- Методы получения ---
  // Эти методы теперь const-friendly, используя find/at
  int getSourceGeneratedCount(int sourceId) const; // n_gen
  int getSourceDeliveredCount(int sourceId) const; // n_delivered
  int getSourceRejectedCount(int sourceId) const; // m_rejected
  double getSourceRejectionRate(int sourceId) const; // p_отк = rejected / generated_for_source
  double getSourceAvgWaitTime(int sourceId) const; // T_ож
  double getSourceAvgServiceTime(int sourceId) const; // T_об (приблизительно из serviceTime)
  double getSourceAvgSystemTime(int sourceId) const; // T_преб
  double getSourceVarianceWaitTime(int sourceId) const; // D_ож
  double getSourceVarianceServiceTime(int sourceId) const; // D_об (из serviceTime)
  // -------------------------
  int getDeliveredCount() const;
  int getRejectedCount() const;
  int getTotalProcessed() const; // delivered + rejected

  double getRejectionRate() const; // (delivered + rejected) > 0 ? rejected / (delivered + rejected) : 0

  int getChannelUsage(int channelId) const;
  // --- ИСПРАВЛЕНО: Добавлен totalTime ---
  double getChannelUtilization(int channelId, double totalTime) const; // (sum_service_time) / totalTime
  // ---------------------------------------
  double getChannelVarianceServiceTime(int channelId, double totalTime) const; // D_канала (аппроксимация)

  // --- Метод вывода ---
  // ИСПРАВЛЕНО: Принимает totalTime извне
  void printStatistics(double totalTime) const;
  // -------------------

  // --- Методы для графиков ---
  // ИСПРАВЛЕНО: Принимает currentTime извне
  void snapshotStatistics(double currentTime);
  void printGraphData() const;
  // ---------------------------
};

#endif // DATABASE_H