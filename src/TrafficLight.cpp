#include "TrafficLight.h"
#include <iostream>
#include <random>

/* Implementation of class "MessageQueue" */

template <typename T> T MessageQueue<T>::receive() {
  // FP.5a : The method receive should use std::unique_lock<std::mutex> and
  // _condition.wait() to wait for and receive new messages and pull them from
  // the queue using move semantics. The received object should then be returned
  // by the receive function.

  std::unique_lock<std::mutex> uniqueLock(_mutex);
  _condition.wait(uniqueLock, [this] { return !_queue.empty(); });

  T msg = std::move(_queue.back());
  _queue.pop_back();

  return msg;
}

template <typename T> void MessageQueue<T>::send(T &&msg) {
  // FP.4a : The method send should use the mechanisms
  // std::lock_guard<std::mutex> as well as _condition.notify_one() to add a new
  // message to the queue and afterwards send a notification.

  std::lock_guard<std::mutex> lockGuard(_mutex);

  std::cout << "  Message " << msg << " has been sent to the queue"
            << std::endl;
  _queue.push_back(std::move(msg));
  _condition.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight() { _currentPhase = TrafficLightPhase::red; }

void TrafficLight::waitForGreen() {
  // FP.5b : add the implementation of the method waitForGreen, in which an
  // infinite while-loop runs and repeatedly calls the receive function on the
  // message queue. Once it receives TrafficLightPhase::green, the method
  // returns.

  while (true) {
    // sleep for 1ms on every iteration to reduce CPU usage
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    TrafficLightPhase tlp = _messageQueue.receive();

    if (tlp == green)
      return;
  }
}

TrafficLightPhase TrafficLight::getCurrentPhase() { return _currentPhase; }

void TrafficLight::simulate() {
  // FP.2b : Finally, the private method „cycleThroughPhases“ should be started
  // in a thread when the public method „simulate“ is called. To do this, use
  // the thread queue in the base class.
  threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases() {
  // FP.2a : Implement the function with an infinite loop that measures the time
  // between two loop cycles and toggles the current phase of the traffic light
  // between red and green and sends an update method to the message queue using
  // move semantics. The cycle duration should be a random value between 4 and 6
  // seconds. Also, the while-loop should use std::this_thread::sleep_for to
  // wait 1ms between two cycles.

  // generate a random number
  std::random_device rd;
  std::mt19937 eng(rd());
  std::uniform_real_distribution<> urdist(4, 6);

  // set the duration or each cycle
  double cycleDuration = urdist(eng);
  std::chrono::time_point<std::chrono::system_clock> lastUpdate;

  // set the last update
  lastUpdate = std::chrono::system_clock::now();

  while (true) {
    // sleep to reduce the streess on the CPU
    std::this_thread::sleep_for(std::chrono::microseconds(1));

    // check the time since the last update
    long timeSinceLastUpdate =
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now() - lastUpdate)
            .count();

    // check if we need to change the ligth on this cycle
    if (timeSinceLastUpdate >= cycleDuration) {
      if (getCurrentPhase() == green)
        _currentPhase = red;
      else
        _currentPhase = green;

      // send message to queue
      _messageQueue.send(std::move(getCurrentPhase()));

      // reset last update time
      lastUpdate = std::chrono::system_clock::now();
    }
  }
}
