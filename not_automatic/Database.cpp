// Database.cpp
#include "Database.h"
#include <iostream>
#include <iomanip>
#include <cmath> // Для sqrt, если понадобится stddev

Database::Database() : deliveredCount(0), rejectedCount(0) {}

void Database::recordDelivery(const Notification& notification, double serviceTime, int channelId) {
  deliveredCount++;
  sourceDelivered[notification.getSourceId()]++; // Учёт доставленных (начало обслуживания)

  // --- Учёт времени ожидания (T_ож) ---
  // Время от входа в буфер до *покидания* буфера (начало обслуживания)
  double waitTime = notification.getWaitTime();
  if (waitTime >= 0) { // Засчитываем даже 0, если уведомление не ждало
    sourceTotalWaitTime[notification.getSourceId()] += waitTime;
    sourceTotalWaitTimeSquared[notification.getSourceId()] += waitTime * waitTime;
    sourceWaitedCount[notification.getSourceId()]++; // Увеличиваем счётчик для усреднения и дисперсии T_ож
  }
  // --------------------------------------

  // --- Учёт времени обслуживания (T_об) ---
  // Используем serviceTime, переданный из Channel (время, проведённое в канале)
  sourceTotalServiceTime[notification.getSourceId()] += serviceTime;
  sourceTotalServiceTimeSquared[notification.getSourceId()] += serviceTime * serviceTime;
  sourceServicedCount[notification.getSourceId()]++; // Увеличиваем счётчик для усреднения и дисперсии T_об
  // ------------------------------------------

  // --- Учёт времени пребывания (T_преб) ---
  // Время от генерации до *входа в канал* (конец обслуживания)
  double systemTime = notification.getSystemTime();
  if (systemTime >= 0) { // Убедимся, что время корректно
    sourceTotalSystemTime[notification.getSourceId()] += systemTime;
    sourceTotalSystemTimeSquared[notification.getSourceId()] += systemTime * systemTime;
    sourceProcessedCount[notification.getSourceId()]++; // Увеличиваем счётчик для усреднения и дисперсии T_преб
  }
  // ------------------------------------------

  // --- Учёт канала ---
  channelUsage[channelId]++;
  channelTotalServiceTime[channelId] += serviceTime;
  channelTotalServiceTimeSquared[channelId] += serviceTime * serviceTime;
  // -----------------
}

void Database::recordRejection(const Notification& notification) {
  rejectedCount++;
  sourceRejected[notification.getSourceId()]++; // Учёт вытесненных
}

void Database::recordGeneration(int sourceId) {
  sourceGeneratedCount[sourceId]++; // Учёт сгенерированных
}

void Database::reset() {
  deliveredCount = 0;
  rejectedCount = 0;
  sourceGeneratedCount.clear(); // Сброс n_gen
  sourceDelivered.clear();
  sourceRejected.clear();
  sourceTotalWaitTime.clear();
  sourceTotalWaitTimeSquared.clear();
  sourceWaitedCount.clear();
  sourceTotalServiceTime.clear();
  sourceTotalServiceTimeSquared.clear();
  sourceServicedCount.clear();
  sourceTotalSystemTime.clear();
  sourceTotalSystemTimeSquared.clear();
  sourceProcessedCount.clear();
  channelUsage.clear();
  channelTotalServiceTime.clear();
  channelTotalServiceTimeSquared.clear();
  // Сброс графиков
  timePointsForGraphs.clear();
  rejectionRatesOverTime.clear();
  avgWaitTimesOverTime.clear();
  avgServiceTimesOverTime.clear();
  avgSystemTimesOverTime.clear();
  channelLoadsOverTime.clear();
}

// --- Методы получения (const-friendly с использованием find/at) ---
int Database::getSourceGeneratedCount(int sourceId) const {
  auto it = sourceGeneratedCount.find(sourceId);
  return (it != sourceGeneratedCount.end()) ? it->second : 0; // n_gen
}

int Database::getSourceDeliveredCount(int sourceId) const {
  auto it = sourceDelivered.find(sourceId);
  return (it != sourceDelivered.end()) ? it->second : 0; // n_delivered
}

int Database::getSourceRejectedCount(int sourceId) const {
  auto it = sourceRejected.find(sourceId);
  return (it != sourceRejected.end()) ? it->second : 0; // m_rejected
}

double Database::getSourceRejectionRate(int sourceId) const {
  int generated = getSourceGeneratedCount(sourceId); // n_gen
  int rejected = getSourceRejectedCount(sourceId);   // m_rej
  // ИСПРАВЛЕНО: Проверка на 0 при делении
  return (generated > 0) ? static_cast<double>(rejected) / generated : 0.0; // p_отк = m/n
}

double Database::getSourceAvgWaitTime(int sourceId) const {
  auto countIt = sourceWaitedCount.find(sourceId);
  auto timeIt = sourceTotalWaitTime.find(sourceId);

  if (countIt != sourceWaitedCount.end() && countIt->second > 0 &&
    timeIt != sourceTotalWaitTime.end()) {
    return timeIt->second / countIt->second; // T_ож
  }
  return 0.0;
}

double Database::getSourceAvgServiceTime(int sourceId) const {
  auto countIt = sourceServicedCount.find(sourceId);
  auto timeIt = sourceTotalServiceTime.find(sourceId);

  if (countIt != sourceServicedCount.end() && countIt->second > 0 &&
    timeIt != sourceTotalServiceTime.end()) {
    return timeIt->second / countIt->second; // T_об (приблизительно из serviceTime)
  }
  return 0.0;
}

double Database::getSourceAvgSystemTime(int sourceId) const {
  auto countIt = sourceProcessedCount.find(sourceId);
  auto timeIt = sourceTotalSystemTime.find(sourceId);

  if (countIt != sourceProcessedCount.end() && countIt->second > 0 &&
    timeIt != sourceTotalSystemTime.end()) {
    return timeIt->second / countIt->second; // T_преб
  }
  return 0.0;
}

double Database::getSourceVarianceWaitTime(int sourceId) const {
  auto countIt = sourceWaitedCount.find(sourceId);
  auto timeIt = sourceTotalWaitTime.find(sourceId);
  auto timeSqIt = sourceTotalWaitTimeSquared.find(sourceId);

  if (countIt != sourceWaitedCount.end() && countIt->second > 1 && // Дисперсия требует > 1 наблюдения
    timeIt != sourceTotalWaitTime.end() && timeSqIt != sourceTotalWaitTimeSquared.end()) {
    double mean = timeIt->second / countIt->second;
    double meanSq = timeSqIt->second / countIt->second;
    return meanSq - (mean * mean); // D_ож
  }
  return 0.0; // Возвращаем 0, если недостаточно данных для расчёта дисперсии
}

double Database::getSourceVarianceServiceTime(int sourceId) const {
  auto countIt = sourceServicedCount.find(sourceId);
  auto timeIt = sourceTotalServiceTime.find(sourceId);
  auto timeSqIt = sourceTotalServiceTimeSquared.find(sourceId);

  if (countIt != sourceServicedCount.end() && countIt->second > 1 && // Дисперсия требует > 1 наблюдения
    timeIt != sourceTotalServiceTime.end() && timeSqIt != sourceTotalServiceTimeSquared.end()) {
    double mean = timeIt->second / countIt->second;
    double meanSq = timeSqIt->second / countIt->second;
    return meanSq - (mean * mean); // D_об (из serviceTime)
  }
  return 0.0; // Возвращаем 0, если недостаточно данных для расчёта дисперсии
}

int Database::getDeliveredCount() const { return deliveredCount; }
int Database::getRejectedCount() const { return rejectedCount; }
int Database::getTotalProcessed() const { return deliveredCount + rejectedCount; }

double Database::getRejectionRate() const {
  int total = getTotalProcessed();
  return (total > 0) ? static_cast<double>(rejectedCount) / total : 0.0;
}

int Database::getChannelUsage(int channelId) const {
  auto it = channelUsage.find(channelId);
  return (it != channelUsage.end()) ? it->second : 0;
}

// --- ИСПРАВЛЕНО: Добавлен totalTime ---
double Database::getChannelUtilization(int channelId, double totalTime) const {
  auto usageIt = channelUsage.find(channelId);
  auto timeIt = channelTotalServiceTime.find(channelId);

  if (usageIt != channelUsage.end() && timeIt != channelTotalServiceTime.end()) {
    // Утилизация = (суммарное время обслуживания) / (общее время моделирования)
    return totalTime > 0 ? timeIt->second / totalTime : 0.0;
  }
  return 0.0;
}
// ---------------------------------------

double Database::getChannelVarianceServiceTime(int channelId, double totalTime) const {
  auto usageIt = channelUsage.find(channelId);
  auto timeIt = channelTotalServiceTime.find(channelId);
  auto timeSqIt = channelTotalServiceTimeSquared.find(channelId);

  if (usageIt != channelUsage.end() && usageIt->second > 1 && // Дисперсия требует > 1 наблюдения
    timeIt != channelTotalServiceTime.end() && timeSqIt != channelTotalServiceTimeSquared.end()) {
    double mean = timeIt->second / usageIt->second;
    double meanSq = timeSqIt->second / usageIt->second;
    return meanSq - (mean * mean);
  }
  return 0.0; // Возвращаем 0, если недостаточно данных для расчёта дисперсии
}

// --- Метод вывода (ИСПРАВЛЕНО: Принимает totalTime извне) ---
void Database::printStatistics(double totalTime) const {
  std::cout << "\n===== СВОДНАЯ ТАБЛИЦА (ОР1) =====\n";

  // --- Таблица 1: Характеристики источников ---
  std::cout << "\n--- Таблица 1: Характеристики источников ВС ---\n";
  // Печатаем заголовок в соответствии с требованиями
  std::cout << "№ источника | Количество заявок (n_gen) | p_отк | T_преб | T_БП (T_ож) | T_обсл | D_БП (D_ож) | D_обсл\n";
  std::cout << "------------|---------------------------|-------|--------|-------------|--------|-------------|-------\n";

  for (int i = 1; i <= 3; i++) { // Предполагаем 3 источника
    // ИСПОЛЬЗУЕМ const-friendly методы (find/at)
    int n_gen = getSourceGeneratedCount(i);
    int n_deliv = getSourceDeliveredCount(i);
    int m_rej = getSourceRejectedCount(i);
    double p_otk = (n_gen > 0) ? static_cast<double>(m_rej) / n_gen : 0.0; // p_отк = m/n_gen
    double avg_system_time = getSourceAvgSystemTime(i); // T_преб
    double avg_wait_time = getSourceAvgWaitTime(i);     // T_ож (T_БП)
    double avg_service_time = getSourceAvgServiceTime(i); // T_об (приблизительно)
    double var_wait_time = getSourceVarianceWaitTime(i);  // D_ож (D_БП)
    double var_service_time = getSourceVarianceServiceTime(i); // D_об (приблизительно из serviceTime)

    std::cout << std::setw(11) << i << " | "
      << std::setw(25) << n_gen << " | " // Используем n_gen
      << std::fixed << std::setprecision(4)
      << std::setw(5) << p_otk << " | "
      << std::setw(6) << avg_system_time << " | "
      << std::setw(11) << avg_wait_time << " | "
      << std::setw(6) << avg_service_time << " | "
      << std::setw(11) << var_wait_time << " | "
      << std::setw(6) << var_service_time << "\n";

    // Проверка T_преб = T_ож + T_об (приблизительно)
    double sum_components = avg_wait_time + avg_service_time;
    if (std::abs(avg_system_time - sum_components) > 1e-3) { // Простая проверка с погрешностью
      std::cout << "      (Проверка: T_преб (" << avg_system_time << ") ~= T_ож (" << avg_wait_time << ") + T_об (" << avg_service_time << ") = " << sum_components << ")\n";
    }
  }
  // ------------------------------------------

  // --- Таблица 2: Характеристики приборов ---
  std::cout << "\n--- Таблица 2: Характеристики приборов ВС ---\n";
  std::cout << "Канал | Коэффициент использования\n";
  std::cout << "-------|---------------------------\n";

  for (int i = 1; i <= 3; i++) { // Предполагаем 3 канала
    // ИСПРАВЛЕНО: передаём totalTime
    double utilization = getChannelUtilization(i, totalTime);
    std::cout << std::setw(5) << i << " | "
      << std::fixed << std::setprecision(5)
      << std::setw(27) << utilization << "\n";
  }
  // ------------------------------------------
}
// ---------------------------

// --- Методы для графиков (ИСПРАВЛЕНО: Принимают currentTime, используют totalTime) ---
void Database::snapshotStatistics(double currentTime) {
  timePointsForGraphs.push_back(currentTime);

  // Снимок p_отк для каждого источника
  std::map<int, double> currentRejRates;
  for (int i = 1; i <= 3; i++) { // Предполагаем 3 источника
    // ИСПОЛЬЗУЕМ const-friendly логику (find/at)
    int gen = getSourceGeneratedCount(i); // n_gen
    int rej = getSourceRejectedCount(i);  // m_rej
    currentRejRates[i] = (gen > 0) ? static_cast<double>(rej) / gen : 0.0;
  }
  rejectionRatesOverTime.push_back(currentRejRates);

  // Снимок avg T_ож для каждого источника
  std::map<int, double> currentAvgWaitTimes;
  for (int i = 1; i <= 3; i++) {
    // ИСПОЛЬЗУЕМ const-friendly логику (find/at)
    auto countIt = sourceWaitedCount.find(i);
    auto timeIt = sourceTotalWaitTime.find(i);
    if (countIt != sourceWaitedCount.end() && countIt->second > 0 && timeIt != sourceTotalWaitTime.end()) {
      currentAvgWaitTimes[i] = timeIt->second / countIt->second;
    }
    else {
      currentAvgWaitTimes[i] = 0.0;
    }
  }
  avgWaitTimesOverTime.push_back(currentAvgWaitTimes);

  // Снимок avg T_об для каждого источника
  std::map<int, double> currentAvgServiceTimes;
  for (int i = 1; i <= 3; i++) {
    // ИСПОЛЬЗУЕМ const-friendly логику (find/at)
    auto countIt = sourceServicedCount.find(i);
    auto timeIt = sourceTotalServiceTime.find(i);
    if (countIt != sourceServicedCount.end() && countIt->second > 0 && timeIt != sourceTotalServiceTime.end()) {
      currentAvgServiceTimes[i] = timeIt->second / countIt->second;
    }
    else {
      currentAvgServiceTimes[i] = 0.0;
    }
  }
  avgServiceTimesOverTime.push_back(currentAvgServiceTimes);

  // Снимок avg T_преб для каждого источника
  std::map<int, double> currentAvgSystemTimes;
  for (int i = 1; i <= 3; i++) {
    // ИСПОЛЬЗУЕМ const-friendly логику (find/at)
    auto countIt = sourceProcessedCount.find(i);
    auto timeIt = sourceTotalSystemTime.find(i);
    if (countIt != sourceProcessedCount.end() && countIt->second > 0 && timeIt != sourceTotalSystemTime.end()) {
      currentAvgSystemTimes[i] = timeIt->second / countIt->second;
    }
    else {
      currentAvgSystemTimes[i] = 0.0;
    }
  }
  avgSystemTimesOverTime.push_back(currentAvgSystemTimes);

  // Снимок загрузки каналов
  std::map<int, double> currentChannelLoads;
  for (int i = 1; i <= 3; i++) {
    // ИСПРАВЛЕНО: передаём totalTime
    currentChannelLoads[i] = getChannelUtilization(i, totalTime > 0 ? totalTime : 1.0); // Используем totalTime извне или текущее
  }
  channelLoadsOverTime.push_back(currentChannelLoads);
}

void Database::printGraphData() const {
  std::cout << "\n===== ДАННЫЕ ДЛЯ ГРАФИКОВ (ОР2) =====\n";

  size_t numSnapshots = timePointsForGraphs.size();
  if (numSnapshots == 0) {
    std::cout << "Нет данных для графиков (snapshotStatistics не вызывался).\n";
    return;
  }

  std::cout << "\n--- График: Вероятность отказа по источникам ---\n";
  std::cout << "Time\tSource\tP_Otk\n";
  for (size_t i = 0; i < numSnapshots; ++i) {
    for (int j = 1; j <= 3; j++) {
      // Используем at() для безопасности
      std::cout << timePointsForGraphs[i] << "\t" << j << "\t" << rejectionRatesOverTime[i].at(j) << "\n";
    }
  }

  std::cout << "\n--- График: Среднее время ожидания по источникам ---\n";
  std::cout << "Time\tSource\tT_Wait\n";
  for (size_t i = 0; i < numSnapshots; ++i) {
    for (int j = 1; j <= 3; j++) {
      std::cout << timePointsForGraphs[i] << "\t" << j << "\t" << avgWaitTimesOverTime[i].at(j) << "\n";
    }
  }

  std::cout << "\n--- График: Среднее время обслуживания по источникам ---\n";
  std::cout << "Time\tSource\tT_Service\n";
  for (size_t i = 0; i < numSnapshots; ++i) {
    for (int j = 1; j <= 3; j++) {
      std::cout << timePointsForGraphs[i] << "\t" << j << "\t" << avgServiceTimesOverTime[i].at(j) << "\n";
    }
  }

  std::cout << "\n--- График: Среднее время в системе по источникам ---\n";
  std::cout << "Time\tSource\tT_System\n";
  for (size_t i = 0; i < numSnapshots; ++i) {
    for (int j = 1; j <= 3; j++) {
      std::cout << timePointsForGraphs[i] << "\t" << j << "\t" << avgSystemTimesOverTime[i].at(j) << "\n";
    }
  }

  std::cout << "\n--- График: Загрузка каналов ---\n";
  std::cout << "Time\tChannel\tLoad\n";
  for (size_t i = 0; i < numSnapshots; ++i) {
    for (int j = 1; j <= 3; j++) {
      std::cout << timePointsForGraphs[i] << "\t" << j << "\t" << channelLoadsOverTime[i].at(j) << "\n";
    }
  }
}
// ---------------------------