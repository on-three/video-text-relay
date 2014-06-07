#include <queue>
#include <thread>
#include <mutex>
//#include <condition_variable>

//via: http://juanchopanzacpp.wordpress.com/2013/02/26/concurrent-queue-c11/

#ifndef __THREADSAFE_QUEUE_HPP__
#define __THREADSAFE_QUEUE_HPP__
 
template<class C>
class Queue{
 std::queue<C> m_queue;
 std::mutex m_mutex;
 
 public:
  Queue(){};
  ~Queue(){};
  void push(const C& c)  { std::lock_guard<std::mutex> l(m_mutex); m_queue.push(c); };
  void pop()             { std::lock_guard<std::mutex> l(m_mutex); m_queue.pop(); };
  bool empty()           { std::lock_guard<std::mutex> l(m_mutex); return m_queue.empty(); };

  C& front()             { std::lock_guard<std::mutex> l(m_mutex); return m_queue.front(); };
  const C& front() const { std::lock_guard<std::mutex> l(m_mutex); return m_queue.front(); };
};//Queue

#endif
