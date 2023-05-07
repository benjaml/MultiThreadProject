#include "GUID.h"
#include <iostream>
#include <random>
#include <iostream>

std::unordered_set<GUID, std::hash<GUID>> GUID::usedGUIDs;

GUID GUID::GetNextGUID()
{
    std::cout << "Creating temp GUID" << std::endl;
    uint16_t a = (uint16_t)rand();
    uint16_t b = (uint16_t)rand();
    uint16_t c = (uint16_t)rand();
    uint16_t d = (uint16_t)rand();

    auto result = GUID::usedGUIDs.emplace(a, b, c, d);
    if (result.second)
    {
        return *result.first;
    }
    else
    {
        return GUID::GetNextGUID();
    }
}

std::string GUID::ToString()
{
    const int size = 4 * 16;
    char buffer[size];
    std::snprintf(buffer, size, "%X-%X-%X-%X", a, b, c, d);
    return std::string(buffer);
}

GUID::GUID()
{
    a = b = c = d = 0;
    std::cout << "Default GUID constructor" << ToString() << " at address : " << this << std::endl;
}

GUID::~GUID()
{
    std::cout << "Delete GUID : " << ToString() << " at address : " << this << std::endl;
}

bool GUID::operator==(const GUID& other) const
{
    return a == other.a && b == other.b && c == other.c && d == other.d;
}

bool GUID::operator<(const GUID& other) const
{
    return std::hash<GUID>{}(*this) < std::hash<GUID>{}(other);
}

GUID::GUID(uint16_t _a, uint16_t _b, uint16_t _c, uint16_t _d)
{
    a = _a;
    b = _b;
    c = _c;
    d = _d;
    std::cout << "Create GUID : " << ToString() << " at address : " << this << std::endl;
}

GUID::GUID(const GUID& ref)
{
    a = ref.a;
    b = ref.b;
    c = ref.c;
    d = ref.d;
    std::cout << "Create GUID  by copy : " << ToString() << " at address : " << this << std::endl;
}

inline size_t std::hash<GUID>::operator()(const GUID& guid) const
{
    size_t hash = 0;
    hash += (size_t)guid.a << 16 * 3;
    hash += (size_t)guid.b << 16 * 2;
    hash += (size_t)guid.c << 16;
    hash += guid.d;
    return hash;
}