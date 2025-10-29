#include <lamport.hpp>

// Constructor.
Lamport::Lamport(): _curTimestamp(1) {}

/**
 * Update timestamp plus one.
 */
int64_t Lamport::updateTimestamp() {
    int32_t return_timestamp;

    mt_curTimestamp.lock();

    _curTimestamp = _curTimestamp + 1;

    return_timestamp = _curTimestamp;

    mt_curTimestamp.unlock();

    return _curTimestamp;
}

/**
 * Update timestamp with the higher value between
 * the current processor timestamp and the clock
 * of the other processor.
 */
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

/**
 * Return the current timestamp.
 */
int64_t Lamport::curTimestamp() { 
    int32_t return_timestamp;

    mt_curTimestamp.lock();

    return_timestamp = _curTimestamp;

    mt_curTimestamp.unlock();

    return return_timestamp; 
}
