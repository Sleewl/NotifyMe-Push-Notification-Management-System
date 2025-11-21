// main.cpp
#include "PushNotificationSystem.h"
#include <iostream>
#include <exception>

int main() {
  setlocale(LC_ALL, "Ru");
  try {

    PushNotificationSystem system(3, 5, 3, 20);

    system.runStepByStep();
  }
  catch (const std::exception& e) {
    std::cerr << "Error in main: " << e.what() << std::endl;
    return 1;
  }
  catch (...) {
    std::cerr << "Unknown error occurred in main." << std::endl;
    return 1;
  }

  return 0;
}