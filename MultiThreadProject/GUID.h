#pragma once
#include <string>
#include <map>
#include <unordered_set>

struct GUID 
{
public:
    uint16_t a, b, c, d;

    static GUID GetNextGUID();
    std::string ToString();
    GUID();
    GUID(uint16_t _a, uint16_t _b, uint16_t _c, uint16_t _d);
    GUID(const GUID& ref);
    ~GUID();

    bool operator==(const GUID& other) const;
    bool operator<(const GUID& other) const;

private:
    static std::unordered_set<GUID, std::hash<GUID>> usedGUIDs;
    static std::mutex lock;
};

template<>
struct std::hash<GUID>
{
    size_t operator()(const GUID& guid) const;
};
