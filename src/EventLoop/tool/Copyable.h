#ifndef EVENTLOOP_TOOL_COPYABLE_H
#define EVENTLOOP_TOOL_COPYABLE_H

class NonCopyable {

public:
    NonCopyable(const NonCopyable &) = delete;

    NonCopyable(NonCopyable &&) = delete;

protected:
    constexpr NonCopyable() = default;

    NonCopyable &operator=(const NonCopyable &) = delete;

    NonCopyable &operator=(NonCopyable &&) = delete;

};

class Copyable {
protected:
    constexpr Copyable() = default;
};

#endif //EVENTLOOP_TOOL_COPYABLE_H
