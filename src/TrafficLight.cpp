#include <iostream>
#include <random>
#include <queue>
#include <future>

#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

/* 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
}
*/

/* Implementation of class "TrafficLight" */

/* 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
}

*/

template <typename T>
T MessageQueue<T>::receive()
{
    // Eric.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> uni_lock(this->_mutex);
    this->_condition.wait(uni_lock, [this](){ return !this->_queue.empty(); })
    T msg = std::move(this->_queue.back());
    this->_condition.notify_one();
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // Eric.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> uni_lock(this -> _mutex);
    this->_queue.push_back(std::move(msg));
    this->_condition.notify_one();
}

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // Eric.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true) 
    {
        auto phrase = this->_msgQueue.receive();
        if (phrase == green) { return; }
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

// Eric 2b
void TrafficLight::simulate()
{
    this->threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// Eric 2a
void TrafficLight::cycleThroughPhases()
{
    std::random_device rd;
    std::mt19937 engine(rd());
    std::uniform_int_distribution<> distribution(4, 6);
    int cycle_duration = distribution(engine);    
    auto last_update = std::chrono::system_clock::now();

    while (true) 
    {
        int time_elapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - last_update).count();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (time_elapsed < cycle_duration) { continue; }

        if ( _currentPhase == red ) { _currentPhase = green; } 
        if ( _currentPhase == green ) { _currentPhase = red; } 

        TrafficLightPhase signal = _currentPhase;
        auto thread = std::async(std::launch::async, &MessageQueue<TrafficLightPhase>::send, _msgQueue, std::move(signal));
    }

}