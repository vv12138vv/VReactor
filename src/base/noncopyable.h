#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H


class noncopyable {
public:
    noncopyable& operator=(const noncopyable& that) = delete;
    noncopyable(const noncopyable& that) = delete;

protected:
    noncopyable()=default;
    ~noncopyable()=default;
};


#endif