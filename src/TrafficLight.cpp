#include <iostream>
#include <random>
#include "TrafficLight.h"

constexpr double durationMin   = 4.0;
constexpr double durationRange = 2.0;

std::chrono::duration<double> GenerateRandomDuration()
{
	return std::chrono::duration<double>(durationMin + (static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX) * durationRange)); 
}

/* Implementation of class "MessageQueue" */
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
  	std::unique_lock<std::mutex> mssgLock(_mutex);
  	_condition.wait(mssgLock, [this] { return !_queue.empty(); });
  	T mssg = std::move(_queue.back());
  	_queue.pop_back();
  	return mssg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
  	std::lock_guard<std::mutex> mssgLock(_mutex);
  	_queue.push_back(std::move(msg));
  	_condition.notify_one();
}

/* Implementation of class "TrafficLight" */
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
  	while (true)
    {
      	if (_phasesQueue.receive() == TrafficLightPhase::green) return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
  	threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    
  	// std::chrono::duration<float> randomDuration = durationMin + (<float>std::rand() / <float>RAND_MAX) * durationRange); 
  	std::chrono::duration<double> randomDuration = GenerateRandomDuration();
  	std::chrono::high_resolution_clock::time_point lastUpdate = std::chrono::high_resolution_clock::now();
  
	while (true)
    {
		// random wait generated for 4-6 seconds
    	if (std::chrono::high_resolution_clock::now() - lastUpdate > randomDuration)
        {
        	_currentPhase = _currentPhase == TrafficLightPhase::red ? TrafficLightPhase::green : TrafficLightPhase::red; // toggles light color
        	_phasesQueue.send(std::move(_currentPhase));                                                                 // sends new phase to queue
          
        	lastUpdate = std::chrono::high_resolution_clock::now();                                                      // resets update clock
        	randomDuration = GenerateRandomDuration();                                                                   // generates new duration between 4.0 and 6.0
        }
    	else
        {
        	std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    
}