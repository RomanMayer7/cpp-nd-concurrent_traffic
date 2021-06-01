#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include "TrafficObject.h"

enum TrafficLightPhase { red, green };

// forward declarations to avoid include cycle
class Vehicle;


// Definition of „MessageQueue“ class(which is the member of "TrafficLight" class defined below),it has the public methods send and receive. 
// "send" takes an rvalue reference of type TrafficLightPhase whereas "receive" should return this type. 
// class also defines an std::dequeue called _queue, which stores objects of type TrafficLightPhase. 
// Also, there is an std::condition_variable as well as an std::mutex as private members. 

template <class T>
class MessageQueue
{
public:
 T receive();
 void send(T &&msg);

private:
    std::mutex _mutex;
	std::condition_variable _cond;
	std::deque<T> _queue;
};

//----------------------------------------------------------------------------------------------
class TrafficLight:public TrafficObject
{
public:
    // constructor / desctructor
    TrafficLight();
    // getters / setters
    TrafficLightPhase getCurrentPhase();

    // typical behaviour methods
    void waitForGreen();
    void simulate();

private:
   
    //-----typical behaviour methods----------
    void cycleThroughPhases();

    //-------------members--------------------
    //a private member of type "MessageQueue" for messages of type TrafficLightPhase 
    // each new TrafficLightPhase is pushed into it within the infinite loop 
    // via call to "MessageQueue<T>::send" method (in conjunction with move semantics.)
    std::shared_ptr<MessageQueue<TrafficLightPhase>> trafficLightMessageQueue;

    TrafficLightPhase _currentPhase;
    std::condition_variable _condition;
    std::mutex _mutex;
};

#endif