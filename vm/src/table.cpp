#include <iostream>
#include "object.hpp"
#include "table.hpp"

#define TABLE_MAX_LOAD 0.75

using std::make_shared;
using std::move;
using std::nullopt;
using std::optional;
using std::shared_ptr;
using std::uint8_t;
using std::vector;

inline int growCapacity(int capacity)
{
    return capacity <= 0 ? 8 : 2 * capacity;
}

Table::Table()
{
    entries.resize(0);
}

bool Table::set(shared_ptr<StringObject> key, Value value)
{
    checkAndAdjustCapacity();
    auto index = findEntryIndex(key);
    auto &entry = entries[index];
    auto isNew = !entry.key && isNil(entry.value);
    if (isNew)
        count++;

    entry.key = move(key);
    entry.value = move(value);
    return isNew;
}

optional<Value> Table::get(shared_ptr<StringObject> &key) const
{
    if (!count)
        return nullopt;

    const auto &entry = entries[findEntryIndex(key)];
    if (!entry.key)
        return nullopt;

    return entry.value;
}

bool Table::deleteKey(shared_ptr<StringObject> &key)
{
    if (!count)
        return false;

    auto index = findEntryIndex(key);
    auto &entry = entries[index];
    if (!entry.key)
        return false;

    entries[index].key = shared_ptr<StringObject>();
    entries[index].value = FalseVal;
    return true;
}

optional<shared_ptr<StringObject>> Table::findKey(const std::string &alias, uint32_t hash)
{
    if (!count)
        return nullopt;

    uint8_t index = hash & (entries.size() - 1);
    for (;;)
    {
        const auto &entry = entries[index];
        if (!entry.key)
        {
            if (isNil(entry.value))
            {
                return nullopt;
            }
        }
        else
        {
            if (entry.key->hash == hash && entry.key->str == alias)
                return entry.key;
        }

        index = (index + 1) & (entries.size() - 1);
    }
}

int Table::findEntryIndex(shared_ptr<StringObject> &key) const
{
    auto tombstoneIndex = -1;
    uint8_t index = key->hash & (entries.size() - 1);
    for (;;)
    {
        const auto &entry = entries[index];
        if (!entry.key)
        {
            if (isNil(entry.value))
            {
                return tombstoneIndex < 0 ? index : tombstoneIndex;
            }
            else
            {
                if (tombstoneIndex < 0)
                    tombstoneIndex = index;
            }
        }
        else if (entry.key == key)
            return index;

        index = (index + 1) & (entries.size() - 1);
    }
}

void Table::checkAndAdjustCapacity()
{
    if (count + 1 > entries.size() * TABLE_MAX_LOAD)
    {
        int newCapacity = growCapacity(entries.size());
        adjustCapacity(newCapacity);
    }
}

void Table::adjustCapacity(int newCapacity)
{
    auto oldSize = entries.size();
    entries.resize(newCapacity);

    // Re-index old entries.
    for (int i = 0; i < oldSize; i++)
    {
        if (entries[i].key)
        {
            auto entry = entries[i];
            entries[i] = Entry{};
            entries[findEntryIndex(entry.key)] = move(entry);
        }
    }
}

int Table::size() const
{
    return count;
}

void Table::addAll(const Table &other)
{
    for (const auto &entry : other.entries)
    {
        if (entry.key)
            set(entry.key, entry.value);
    }
}