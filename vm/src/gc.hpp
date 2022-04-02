#ifndef _GC_HPP_
#define _GC_HPP_
#include <vector>
#include <memory>

class Vm;
class Object;
class Value;
class Table;

class Gc
{
public:
    explicit Gc(Vm *);
    void collectGarbage();
    bool shouldCollect();
    void addToBytesAllocated(std::size_t);

private:
    void markRoots();
    void markValue(Value &);
    void markObject(std::shared_ptr<Object>);
    void markTable(Table &);
    void traceReferences();
    void sweep();
    void blackenObject(std::shared_ptr<Object>);
    void markValues(std::vector<Value> &);
    void tableRemoveWhite(Table &);
    Vm *vm;
    std::vector<std::shared_ptr<Object>> grayStack;
    std::size_t bytesAllocated = 0;
    int nextGC = 1024 * 1024;
};
#endif