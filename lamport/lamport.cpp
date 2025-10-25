#include <lamport.hpp>

// Constructor.
Lamport::Lamport(): m_curTimestamp(1) {}

int64_t Lamport::updateTimestamp() {
    m_curTimestamp = m_curTimestamp + 1;
    return m_curTimestamp;
}

int64_t Lamport::updateTimestamp(int64_t otherClock) {
    if(otherClock > m_curTimestamp){
        m_curTimestamp = otherClock + 1;
    } else {
        m_curTimestamp = m_curTimestamp + 1;
    }
    return m_curTimestamp;
}


int64_t Lamport::curTimestamp() const { return m_curTimestamp; }
