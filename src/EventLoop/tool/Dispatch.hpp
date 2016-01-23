//
// Created by adugeek on 1/24/16.
//

#ifndef EVENTLOOP_TOOL_DISPATCH_H
#define EVENTLOOP_TOOL_DISPATCH_H

#include <functional>
#include <unordered_map>


template<typename KeyType, typename CallAble>
class Dispatch {
public:
    Dispatch() = default;

    Dispatch(const std::unordered_map<KeyType, std::function<void()> > &table) : table_(table) { };

    Dispatch(std::unordered_map<KeyType, std::function<void()> > &&table) : table_(std::move(table)) { };

    Dispatch(std::initializer_list<std::pair<const KeyType, CallAble >> &&arg) : table_(
            std::forward<decltype(arg)>(arg)) { };

    inline CallAble &operator[](const KeyType &__k) { return table_[__k]; }

    inline CallAble &operator[](KeyType &&__k) { return table_[std::move(__k)]; }

    inline void erase(const KeyType &__k) {
        table_.erase(__k);
    }

    inline size_t size() {
        return table_.size();
    }

private:
    std::unordered_map<KeyType, CallAble> table_;
};

#endif //EVENTLOOP_TOOL_DISPATCH_H
