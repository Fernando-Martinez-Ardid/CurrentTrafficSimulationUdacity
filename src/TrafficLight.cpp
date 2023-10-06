#include <iostream>
#include <random>

#include "TrafficLight.h"
#include "TrafficObject.h"

/* Implementation of class "MessageQueue" */

template <class T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> uLock(_mutex);
    _condition.wait(uLock, [this] { 
        bool notEmpty = !_queue.empty(); 
        std::cout << "Condición de espera: " << (notEmpty ? "Se cumple" : "No se cumple") << std::endl;
        return notEmpty; 
        });
    T msg = std::move(_queue.front());
    _queue.pop_front();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lock(_mutex);
    std::cout << "Antes de agregar a la cola, tamaño: " << _queue.size() << std::endl;
    _queue.emplace_back(std::move(msg));
    _condition.notify_one();
    std::cout << "Despues de agregar a la cola, tamaño: " << _queue.size() << std::endl;
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
        
        TrafficLightPhase msg = _messageQueue.receive();
        std::lock_guard<std::mutex> lock(_mutex); 
		std::cout << "TrafficLight #" << getID() << " received message: " << (msg == green ? "green" : "other") << std::endl;  
      	if (msg == green)
            return; // Exit the loop when the traffic light is green
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
  	std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(4000, 6000);

    auto lastUpdate = std::chrono::system_clock::now();
    
    while(true)
    {
        
        int cycleDuration = distr(eng);
    
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
        
        // Check if it's time to toggle the traffic light phase
        if (elapsedTime >= cycleDuration)
        {
            // Toggle the current phase
            _currentPhase = (_currentPhase == red) ? green : red;
            std::cout << "TrafficLight #" << getID() << " changed to " << ((_currentPhase == red) ? "red" : "green") << std::endl;
            // Send an update to the message queue using move semantics
            _messageQueue.send(std::move(_currentPhase));

            // Reset the timer for the next cycle
            lastUpdate = std::chrono::system_clock::now();
            cycleDuration = distr(eng);
            
        }
    }
}


