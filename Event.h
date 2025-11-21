// Event.h
#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <functional>


struct Event {
  double time;
  std::string type; 
  int sourceId;    
  int notificationId; 
  int channelId;    

  Event(double t, const std::string& tp, int src = -1, int nid = -1, int cid = -1);
};


struct EventComparator {
  bool operator()(const Event& a, const Event& b) const {
    return a.time > b.time;
  }
};

#endif // EVENT_H