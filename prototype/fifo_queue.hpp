#include "config.h"
#include <vector>
#include <mutex>
// #include <queue>
#include <stack>
#include <iostream>
template <typename T> class fifo_queue {
  //    std::queue<T> queue;

  std::stack<T> lifo;
  uint64_t last_blk_id = 0; // the last block id that was processed
  std::mutex mtx;

public:
  fifo_queue() = default;

  ~fifo_queue() {
    if (lifo.size() > 0) {
      std::cout
          << "fifo_queue: destructor called, but queue is not empty, size="
          << lifo.size() << "\n";
    }
  }

  void push(T msg) {
    std::lock_guard<std::mutex> lock(mtx);
    lifo.push(msg);
  }

  bool empty() {
    std::lock_guard<std::mutex> lock(mtx);
    return lifo.empty();
  }

  T pop() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!lifo.empty()) {
      T msg = lifo.top();
      lifo.pop();
      last_blk_id = (last_blk_id > msg->blk_id)
                        ? last_blk_id
                        : msg->blk_id; // update the last processed block id
      return msg;
    }
    return nullptr;
  }

  bool has_elems_to_be_processed() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!lifo.empty()) {
      T msg = lifo.top();
      if (msg->blk_id > last_blk_id) {
        last_blk_id = msg->blk_id; // update the last processed block id
        std::cout << __PRETTY_FUNCTION__ << ": last_blk_id=" << last_blk_id
                  << std::endl;
        return true;
      }
    }
    // std::cout << __PRETTY_FUNCTION__ << ": no elements to process" <<
    // std::endl;
    return false;
  }

  std::vector<T> pop_until_blk_id(uint64_t blk_id) {
    std::vector<T> ret_vec;
    std::lock_guard<std::mutex> lock(mtx);
    while (!lifo.empty()) {
      T msg = lifo.top();
      if (msg->blk_id <= blk_id) {
        ret_vec.emplace_back(msg);
        lifo.pop();
      } else {
        return ret_vec;
      }
    }
    return ret_vec;
  }
};