#include <iostream>
#include <random>
#include "TrafficLight.h"
#include <queue>
#include <future>

/* Implementation of class "MessageQueue" */


// The method "receive" uses std::unique_lock<std::mutex> and _condition.wait() 
// to wait for and receive new messages and pull them from the queue using move semantics. 
// The received object is then  returned 
// Queue modification is performed under the lock 
template <typename T>
T MessageQueue<T>::receive()
{

    std::unique_lock<std::mutex> uLock(_mutex);
   _cond.wait(uLock, [this] { return !_queue.empty(); }); // pass unique lock to condition variable
    // remove last vector element from queue and put it inside msg
    T msg = std::move(_queue.back());
    _queue.pop_back();

    return msg; // will not be copied due to return value optimization (RVO) in C++
}

// The method "send"  uses the mechanisms std::lock_guard<std::mutex> 
// as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
template <typename T>
void MessageQueue<T>::send(T &&msg)
{

    // simulate some work

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // perform vector modification under the lock
    std::lock_guard<std::mutex> uLock(_mutex);
    // add vector to queue
    std::cout << "Message " << msg << " has been sent to the queue" << std::endl;
    _queue.push_back(std::move(msg));
    _cond.notify_one(); // notify client after pushing new Vehicle into vector
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
     _currentPhase = TrafficLightPhase::red;
     trafficLightMessageQueue=std::make_shared<MessageQueue<TrafficLightPhase>>();
}


// Repeatedly calling the "receive" function on the Message Queue. 
// Once it receives TrafficLightPhase::green, the method returns.

void TrafficLight::waitForGreen()
{
   while (true)
	{

		/* Check until Green Light Phase signal is received from the Message Queue */
		TrafficLightPhase current_phase = trafficLightMessageQueue->receive();
		if (current_phase == green)
		{
			return;
		}

        /* Sleep between the iterations*/
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    //Starting the private method „cycleThroughPhases“ in a thread, using the thread queue from the base class.(TrafficObject)
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // This function have an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration is a random value between 4 and 6 seconds. 

       std::default_random_engine defEngine(time(0));
       std::uniform_int_distribution<int> randomValue(4, 6);

       //Lock the Standart Output to print out the Message
	   std::unique_lock<std::mutex> lck(_mutex);
	   std::cout <<  "Thread ID :" <<std::this_thread::get_id() <<" | Cycling through the Phases"<< std::endl;
	   lck.unlock();

      /* Get Random Cycle Duration */
      int cycleDuration = randomValue(defEngine);

      //Make initial TimeStamp
      auto lastTimeStamp =std::chrono::system_clock::now();
      
      while(true)
      {
         long timePassedSinceUpdate= std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - lastTimeStamp).count();


		if (timePassedSinceUpdate >= cycleDuration)
		{
          //Toggle the Traffic Light Phase
          _currentPhase=(_currentPhase==TrafficLightPhase::green) ? TrafficLightPhase::red :TrafficLightPhase::green;
          
          //Capture new TrafficLight condition,following the previous "Toggle" action
          TrafficLightPhase newTrafficLightPhase = _currentPhase;
          
          /* Send the update message with new TrafficLightPhase status to the Message Queue */
          std::future<void> sentMessage = std::async(std::launch::async, &MessageQueue<TrafficLightPhase>::send, trafficLightMessageQueue, std::move(newTrafficLightPhase));
		  sentMessage.wait(); //wait for task to complete

          
         	/* Reset Stop Watch before next Cycle */
			lastTimeStamp = std::chrono::system_clock::now();

			/* Get Random Cycle Duration for the Next Cycle*/
			cycleDuration = randomValue(defEngine);
            
            /*Sleep a bit before the next Cycle */
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

        }
      }

       
    }


