#include"time_stamp.h"


TimeStamp::TimeStamp(size_t spent_time) : spent_time_(spent_time) {}


size_t TimeStamp::get_spent_time() const {
    return spent_time_;
}

bool TimeStamp::operator<(const TimeStamp &that) const {
    return spent_time_ < that.spent_time_;
}

bool TimeStamp::operator>(const TimeStamp &that) const {
    return spent_time_ > that.spent_time_;
}

bool TimeStamp::operator==(const TimeStamp &that) const {
    return spent_time_ == that.spent_time_
}

std::string TimeStamp::to_string(const TimeStamp &time_stamp) {

}

TimeStamp TimeStamp::now() {
    return TimeStamp();
}
