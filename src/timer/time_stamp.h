#ifndef TIME_STAMP_H
#define TIME_STAMP_H

#include<iostream>


class TimeStamp{
private:
    size_t spent_time_;
public:
    TimeStamp(size_t spent_time=0);
    size_t get_spent_time() const;
    bool operator<(const TimeStamp& that) const;
    bool operator>(const TimeStamp& that) const;
    bool operator==(const TimeStamp& that) const;
    static std::string to_string(const TimeStamp& time_stamp);
    static TimeStamp now();
};


#endif
