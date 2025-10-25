#ifndef LAMPORT_H
#define LAMPORT_H

#include <vector>
#include <cstdint>

class Lamport
{
public:
    Lamport();

    int64_t updateTimestamp();
    int64_t updateTimestamp(int64_t);

    int64_t curTimestamp() const;

private:
    int m_curTimestamp;
};

#endif // LAMPORT_H
