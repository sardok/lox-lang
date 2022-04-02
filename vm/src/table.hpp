#ifndef _TABLE_HPP_
#define _TABLE_HPP_
#include <vector>
#include <memory>
#include <optional>
#include "value.hpp"

class StringObject;
class Gc;

struct Entry
{
    explicit Entry() = default;
    explicit Entry(std::shared_ptr<StringObject> key, Value value)
        : key{std::move(key)}, value{std::move(value)} {}
    std::shared_ptr<StringObject> key{};
    Value value{NilVal};
};

class Table
{
public:
    explicit Table();
    bool set(std::shared_ptr<StringObject>, Value);
    std::optional<Value> get(std::shared_ptr<StringObject> &) const;
    bool deleteKey(std::shared_ptr<StringObject> &);
    std::optional<std::shared_ptr<StringObject>> findKey(const std::string &, std::uint32_t);
    int size() const;
    void addAll(const Table &);

private:
    friend Gc;
    int findEntryIndex(std::shared_ptr<StringObject> &) const;
    void checkAndAdjustCapacity();
    void adjustCapacity(int);
    int count = 0;
    std::vector<Entry> entries;
};
#endif