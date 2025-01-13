#pragma once

#include "daisy_seed.h"
#include "ring_buffer.h"

namespace stmlib {

enum ControlType {
    CONTROL_POT = 0,
    CONTROL_ENCODER = 1,
    CONTROL_ENCODER_CLICK = 2,
    CONTROL_ENCODER_LONG_CLICK = 3,
    CONTROL_SWITCH = 4,
    CONTROL_SWITCH_HOLD = 5,
    CONTROL_REFRESH = 0xff
};

struct Event {
    ControlType control_type;
    uint16_t control_id;
    int32_t data;
};

template<uint16_t size = 32>
class EventQueue {
public:
    EventQueue() { }
    
    void Init() {
        events_.Init();
        last_event_time_ = daisy::System::GetNow();
    }
    
    void Flush() {
        events_.Flush();
    }
    
    void AddEvent(ControlType control_type, uint16_t id, int32_t data) {
        Event e;
        e.control_type = control_type;
        e.control_id = id;
        e.data = data;
        events_.Overwrite(e);
        Touch();
    }
    
    void Touch() {
        last_event_time_ = daisy::System::GetNow();
    }
    
    size_t available() {
        return events_.readable();
    }
    
    uint32_t idle_time() {
        uint32_t now = daisy::System::GetNow();
        return now - last_event_time_;
    }
    
    Event PullEvent() {
        return events_.ImmediateRead();
    }
    
private:
    uint32_t last_event_time_;
    RingBuffer<Event, size> events_;
};

}  // namespace stmlib
