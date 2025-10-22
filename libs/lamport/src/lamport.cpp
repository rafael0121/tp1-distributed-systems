#include <lamport.hpp>

// Constructor.
Lamport::Lamport(): m_curTimeStamp(0) {}

int64_t Lamport::updateTimeStamp() {
    m_curTimeStamp = m_curTimeStamp + 1;
    return m_curTimeStamp;
}

int64_t Lamport::updateTimeStamp(int64_t otherClock) {
    if(otherClock > m_curTimeStamp){
        m_curTimeStamp = otherClock + 1;
    }
    return m_curTimeStamp;
}


int64_t Lamport::curTimeStamp() const { return m_curTimeStamp; }
