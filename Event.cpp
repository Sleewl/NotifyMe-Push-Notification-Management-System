// Event.cpp
#include "Event.h"

Event::Event(double t, const std::string& tp, int src, int nid, int cid)
  : time(t), type(tp), sourceId(src), notificationId(nid), channelId(cid) {
}