// Source.h
#ifndef SOURCE_H
#define SOURCE_H

#include "Notification.h" // Для возврата Notification
#include <random>

// Класс Источника (ИБ - Бесконечный)
class Source {
private:
  int id;
  double lambda; // Параметр для генерации интервалов (1/среднее время между заявками)
  int notificationCount;
  std::mt19937 rng;
  std::exponential_distribution<double> expDist; // Для интервалов между заявками (ИЗ1 - Пуассон)

public:
  Source(int id, double lambda);

  // Метод для получения времени до следующей генерации (используется в календаре событий)
  double getNextGenerationTime(double currentTime);

  // Генерация уведомления
  Notification generateNotification();

  int getId() const;
  int getGeneratedCount() const;
};

#endif // SOURCE_H