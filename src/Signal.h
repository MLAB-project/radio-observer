/**
 * \file   Signal.h
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-08-20
 *
 * \brief  Header file for the Signal class.
 */

#ifndef SIGNAL_FO6CU5KO
#define SIGNAL_FO6CU5KO


#include <csignal>
#include <cerrno>
#include <cstring>
#include <vector>
using namespace std;

#include <cppapp/Logger.h>
#include <cppapp/utils.h>
using namespace cppapp;


class SignalCallback {
public:
	virtual ~SignalCallback() {}
	
	virtual void call(int signNum) = 0;
};


template<class T>
class MethodSignalCallback : public SignalCallback {
public:
	typedef void (T::*Method)(int sigNum);
private:
	T      *instance_;
	Method  method_;
public:
	MethodSignalCallback(T *inst, Method method) :
		instance_(inst), method_(method)
	{}
	
	virtual void call(int sigNum)
	{
		(*instance_.*method_)(sigNum);
	}
};


template<int N>
class Signal_ {
public:
	typedef void (*Handler)(int);

private:
	const char *name_;
	Handler     original_;
	
	static vector<SignalCallback*> callbacks_;
	
	static void handler(int sigNum)
	{
		LOG_INFO("Received signal " << sigNum <<
			    ", running " << callbacks_.size() <<
			    " callbacks.");
		
		for (typename vector<SignalCallback*>::reverse_iterator it = callbacks_.rbegin();
			it != callbacks_.rend(); it++) {
			(*it)->call(sigNum);
		}
	}

public:
	Signal_(const char *name) :
		name_(name), original_(NULL)
	{ }
	
	void install()
	{
		VAR(result, signal(N, &handler));

		if (result == SIG_ERR) {
			int error = errno;
			LOG_ERROR("Setting signal handler for " << name_ <<
					" failed: " << strerror(error) <<
					" (code: " << error << ")");
		} else {
			original_ = result;
		}
	}
	
	void uninstall() { signal(N, original_); }
	
	template<class T>
	void pushMethod(T *inst, typename MethodSignalCallback<T>::Method method)
	{
		VAR(ptr, new MethodSignalCallback<T>(inst, method));
		callbacks_.push_back(ptr);
	}

	void pop()
	{
		SignalCallback *callback = callbacks_.back();
		callbacks_.pop_back();
		
		delete callback;
	}
};

template<int N>
vector<SignalCallback*> Signal_<N> ::callbacks_;


class Signal {
private:
	Signal();
	Signal(const Signal &other);
	~Signal();
	
public:
	#define DEF_SIG(name) static Signal_<SIG##name> name;
	DEF_SIG(INT);
	DEF_SIG(TERM);
	#undef DEF_SIG
};


///**
// * \todo Write documentation for class Signal.
// */
//class Signal {
//public:
//	typedef void (*Handler)(int);
//	
//	class Callback {
//	public:
//		virtual void call(int signNum) = 0;
//	};
//	
//	template<class T>
//	class MethodCallback {
//	public:
//		typedef void (T::*Method)(int sigNum);
//	private:
//		T      *instance_;
//		Method  method_;
//	public:
//		MethodCallback(T *inst, Method method) :
//			instance_(inst), method_(method)
//		{}
//		
//		virtual void call(int sigNum)
//		{
//			(*instance_.*method_)(sigNum);
//		}
//	};
//	
//private:
//	int         number_;
//	const char *name_;
//
//	/**
//	 * Constructor.
//	 */
//	Signal(int number, const char *name) :
//		number_(number), name_(name)
//	{ }
//	
//	/**
//	 * Copy constructor.
//	 */
//	Signal(const Signal& other);
//	
//	static Signal interrupt_;
//	static Signal terminate_;
//
//public:
//	/**
//	 * Destructor.
//	 */
//	~Signal() {}
//	
//	void setHandler(Handler handler);
//	void setIgnore()  { setHandler(SIG_IGN); }
//	void setDefault() { setHandler(SIG_DFL); }
//	
//	static Signal& interrupt() { return interrupt_; }
//	static Signal& terminate() { return terminate_; }
//};


#endif /* end of include guard: SIGNAL_FO6CU5KO */

