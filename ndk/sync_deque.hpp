#pragma once
#include <mutex>
#include <deque>
#include <memory>
template <class T>
class sync_deque final {
public:
    sync_deque() = default;
	~sync_deque() = default;
	inline void set_max_size(int sz) {
		max_size_ = sz;
	}
	bool push_front(const T &val) {
		std::unique_lock<std::mutex>lk(lock_);
		if (size_ >= max_size_) {
			return false;
		}
		deque_.push_front(val);
		return true;
	}
	bool push_front(T &&val) {
		std::unique_lock<std::mutex>lk(lock_);
		if (size_ >= max_size_) {
			return false;
		}
		deque_.push_front(val);
		return true;
	}
    bool pop_front(T &val) {
        std::unique_lock<std::mutex>lk(lock_);
		if (true == deque_.empty()) {
			return false;
		}
		val = deque_.front();
		deque_.pop_front();
		--size_;
		return true;
	}
	bool pop_back(T &val) {
        std::unique_lock<std::mutex>lk(lock_);
		if (true == deque_.empty()) {
			return false;
		}
		val = deque_.back();
		deque_.pop_back();
		--size_;
		return true;
	}
	inline bool empty() {
		std::unique_lock<std::mutex>lk(lock_);
		return true == deque_.empty();
	}
    inline size_t get_size() {
        std::unique_lock<std::mutex>l(lock_);
		return deque_.size();
    }
private:
	std::deque<T>deque_;
	std::mutex lock_;
	int size_ = 0;
	int max_size_ = 1024 * 10;
};