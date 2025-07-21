#include "config.h"
#include <vector>
#include <mutex>
#include <queue>

template <typename T>
class fifo_queue {
    std::queue<T> queue;
    std::mutex mtx;
    public:
        fifo_queue() = default;

        ~fifo_queue() {
            if (queue.size() > 0) {
                printf("fifo_queue: destructor called, but queue is not empty, size=%lu\n",\
                     queue.size());
            }
        } 
        
        void push(T msg) {
            std::lock_guard<std::mutex> lock(mtx);
            queue.push(msg);
        }

        bool empty() {
            std::lock_guard<std::mutex> lock(mtx);
            return queue.empty();
        }

        T pop() {
            std::lock_guard<std::mutex> lock(mtx);
            if (!queue.empty()) {
                T msg = queue.front();
                queue.pop();
                return msg;
            }
            return nullptr;
        }

        std::vector<T> pop_until_blk_id(uint64_t blk_id) {
            std::vector<T> ret_vec;
            std::lock_guard<std::mutex> lock(mtx);
            while (!queue.empty()) {
                T msg = queue.front();
                if (msg->blk_id <= blk_id) {
                    ret_vec.emplace_back(msg);
                    queue.pop();
                }
                else {
                    return ret_vec;
                }
            }
            return ret_vec;
        }
};