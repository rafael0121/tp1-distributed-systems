#include <list>
#include <cstdint>
#include <mutex>
#include <algorithm>

class NeighborsReleased{
private:
    // Count access garanted.
    std::list<int32_t> _neighbors_released;
    int _total_clients;

    // Mutex to avoid concurrency.
    std::mutex _mt_neighbors_released;

    void lock(){
        _mt_neighbors_released.lock();
    }

    void unlock(){
        _mt_neighbors_released.unlock();
    }

public:
    NeighborsReleased(int total_clients){
        lock();

        _total_clients = total_clients;

        unlock();
    }

    void push(int32_t client_id){
        lock();

        // Check if client already released access.
        if(std::find(_neighbors_released.begin(), _neighbors_released.end(), client_id) == _neighbors_released.end()){
            _neighbors_released.push_back(client_id);
        }

        unlock();
    }

    int count(){
        int ret;

        lock();

        ret = _neighbors_released.size();

        unlock();

        return ret;
    }

    bool tryAccess() {
        bool ret;

        lock();

        if(_neighbors_released.size() == _total_clients){
            // Clean neighbors_released.
            _neighbors_released.clear();
            ret = true;
        } else {
            ret = false;
        }

        unlock();

        return ret;
    }

};
