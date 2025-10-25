#include <lamport.hpp>

// Constructor.
Lamport::Lamport(): _curTimestamp(1) {}

int64_t Lamport::updateTimestamp() {
    int32_t return_timestamp;

    mt_curTimestamp.lock();

    _curTimestamp = _curTimestamp + 1;

    return_timestamp = _curTimestamp;

    mt_curTimestamp.unlock();

    return _curTimestamp;
}

int64_t Lamport::updateTimestamp(int64_t otherClock) {
    int32_t return_timestamp;

    mt_curTimestamp.lock();

    if(otherClock > _curTimestamp){

        _curTimestamp = otherClock + 1;


    } else {
        _curTimestamp = _curTimestamp + 1;
    }
    return_timestamp = _curTimestamp;

    mt_curTimestamp.unlock();

    return return_timestamp;
}


int64_t Lamport::curTimestamp() { 
    int32_t return_timestamp;

    mt_curTimestamp.lock();

    return_timestamp = _curTimestamp;

    mt_curTimestamp.unlock();

    return return_timestamp; 
}
