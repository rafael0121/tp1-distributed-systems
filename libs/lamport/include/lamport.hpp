#ifndef LAMPORT_H
#define LAMPORT_H

#include <vector>
#include <cstdint>

class Lamport
{
public:
    Lamport();

    int64_t updateTimeStamp();
    int64_t updateTimeStamp(int64_t);

    int64_t curTimeStamp() const;

private:
    int m_curTimeStamp;
};

#endif // LAMPORT_H
