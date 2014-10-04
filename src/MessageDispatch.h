/**
 * \file   MessageDispatch.h
 * \author Jan Milik <milikjan@fit.cvut.cz>
 * \date   2014-03-17
 *
 * \brief  Header file for the MessageDispatch class.
 */

#ifndef MESSAGEQUEUE_D3GJUHJZ
#define MESSAGEQUEUE_D3GJUHJZ


#include <cppapp/cppapp.h>
using namespace cppapp;


/**
 * \brief MessageListener listens to messages sent to a \ref MessageListener.
 */
template<class T>
class MessageListener : public Object {
public:
	virtual void sendMessage(const T &msg) = 0;
};


template<class T>
class FunctionMessageListener : public MessageListener<T> {
public:
	typedef void (*Function)(const T &msg);

private:
	Function fn_;

public:
	FunctionMessageListener(Function fn) : fn_(fn) {}
	
	virtual void sendMessage(const T &msg)
	{
		fn_(msg);
	}
};


template<class T>
class MessageQueueListener : public MessageListener<T> {
private:
	Channel<T> queue_;

public:
	virtual void sendMessage(const T &msg) {
		queue_.send(msg);
	}
	
	Channel<T>& getQueue() { return queue_; }
};


/**
 * \brief Class dispatching messages to a collection of message listeners.
 *
 * \see MessageListener
 */
template<class T>
class MessageDispatch {
public:
	typedef T MessageType;

private:
	MessageDispatch(const MessageDispatch<T>& other);
	
	vector<Ref<MessageListener<T> > > listeners_;
	
public:
	/**
	 * Constructor.
	 */
	MessageDispatch() {}
	/**
	 * Destructor.
	 */
	virtual ~MessageDispatch() {}
	
	/**
	 * \brief Sends a message \c msg to all registered listeners.
	 *
	 * Listeners are registered by the \ref MessageDispatch::addListener method.
	 */
	void sendMessage(const T &msg)
	{
		FOR_EACH(listeners_, it) {
			(*it)->sendMessage(msg);
		}
	}
	
	void addListener(Ref<MessageListener<T> > listener)
	{
		listeners_.push_back(listener);
	}
	
	void addListener(typename FunctionMessageListener<T>::Function fn)
	{
		addListener(new FunctionMessageListener<T>(fn));
	}
	
	static MessageDispatch<T>& getInstance()
	{
		static MessageDispatch<T> instance;
		return instance;
	}
};


template<class T>
struct Message {
	virtual string toString() {
		return "Message()";
	};
	
	void send() {
		MessageDispatch<T>::getInstance().sendMessage(*this);
	}
};


#endif /* end of include guard: MESSAGEQUEUE_D3GJUHJZ */

