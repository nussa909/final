#ifndef COMMON_H
#define COMMON_H

#include <atomic>
#include <vector>
#include <memory>

using tNumType = unsigned long long;
constexpr tNumType cULong_MaxLimit = 0xffffffffffffffff;
constexpr tNumType cValue_MaxLimit= (cULong_MaxLimit-1) / 3;
struct tNode
{
    std::atomic<tNumType> value{0};
    std::atomic<tNumType> chainLen{0};
    std::atomic<bool> isFailed{false};

    tNode()
        : value(0)
        , chainLen(0)
        , isFailed(false)
    {}

    tNode(tNumType& v, tNumType& c, bool b)
        : value(v)
        , chainLen(c)
        , isFailed(b)
    {}

    tNode(const tNode& rhs)
    {
        value.store(rhs.value.load());
        chainLen.store(rhs.chainLen.load());
    }

    tNode& operator= (const tNode& rhs)
    {
        if(&rhs == this) return *this;

        value.store(rhs.value);
        chainLen.store(rhs.chainLen);
        return *this;
    }

    bool operator<(const tNode& n) const
    {
        return chainLen.load() < n.chainLen.load();
    }
};

struct tResult
{
    tResult(tNumType& size)
        : data(size)
        , max(nullptr)
        , timeDuration(0){}

    std::vector<tNode> data;
    tNode* max{nullptr};
    int timeDuration; // ms
};

#endif // COMMON_H
