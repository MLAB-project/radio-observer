/**
 * \file   PyRef.h
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-08-21
 *
 * \brief  Header file for the PyRef class.
 */

#ifndef PYREF_QOKADTFD
#define PYREF_QOKADTFD


#include <cstdlib>
#include <vector>
using namespace std;

#include <Python.h>


class PyRef;


class PyRefBase {
public:
	typedef PyObject* Ptr;

protected:
	Ptr ptr_;

	PyRefBase() : ptr_(NULL) {}
	PyRefBase(Ptr ptr) : ptr_(ptr) {}
	PyRefBase(const PyRefBase& other) : ptr_(other.ptr_) {}

public:
	inline operator Ptr () const { return ptr_; }
	
	PyRef getAttr(const char *name);
	
	bool isCallable() const
	{
		return (ptr_ != NULL) && (PyCallable_Check(ptr_));
	}
	
	bool tryCallArgs(Ptr arguments, Ptr *returnValue)
	{
		Ptr returned = PyObject_CallObject(ptr_, arguments);
		
		if (returned == NULL)
			return false;
		
		*returnValue = returned;
		return true;
	}
};


/**
 * \todo Write documentation for class PyRef.
 */
class PyRef : public PyRefBase {
private:
	void setPtr(Ptr value)
	{
		if (ptr_ != NULL)
			Py_DECREF(ptr_);
		ptr_ = value;
		if (ptr_ != NULL)
			Py_INCREF(ptr_);
	}
	
public:
	/**
	 * Constructor.
	 */
	PyRef() : PyRefBase(NULL) {}
	
	PyRef(Ptr ptr) : PyRefBase(ptr) {}
	
	/**
	 * Copy constructor.
	 */
	PyRef(const PyRef& other) :
		PyRefBase(NULL)
	{
		setPtr(other.ptr_);
	}
	
	/**
	 * Destructor.
	 */
	~PyRef()
	{
		setPtr(NULL);
	}
	
	PyRef& operator=(Ptr ptr)
	{
		setPtr(ptr);
		return *this;
	}
	
	PyRef& operator=(const PyRef& other)
	{
		if (this == &other) return *this;
		
		setPtr(other.ptr_);
		return *this;
	}
	
	
	static PyRef newTuple(int size)
	{
		return PyRef(PyTuple_New(size));
	}
	
	static PyRef newString(const char *value)
	{
		return PyString_FromString(value);
	}
};


class PyInit {
public:
	PyInit()
	{
		Py_Initialize();
	}

	~PyInit()
	{
		Py_Finalize();
	}
};


class PyExtModule {
private:
	static const PyMethodDef SENTINEL;
	
	bool                 initialized_;
	
	const char          *name_;
	//PyRef                module_;
	PyObject            *module_;
	vector<PyMethodDef>  methods_;

protected:
	void addMethod(const char *name, PyCFunction fun, int flags, char *doc)
	{
		methods_.back() = {
			name,  // ml_name
			fun,   // ml_meth
			flags, // ml_flags
			doc    // ml_doc
		};
		
		methods_.push_back(SENTINEL);
	}

public:
	PyExtModule(const char *name) :
		initialized_(false), name_(name)
	{
		methods_.push_back(SENTINEL);
	}
	
	virtual const char* getName() { return name_; }
	virtual void        initMethods() = 0;
	
	void init()
	{
		if (initialized_) return;
		initialized_ = true;
		
		//methods_.push_back(SENTINEL);
		
		module_ = Py_InitModule(getName(), &(methods_[0]));
	}
};


//class Python {
//private:
//	Python();
//	~Python();
//
//public:
//	static int simpleFile(const char *fileName)
//	{
//		FILE *fp = fopen(fileName, "r");
//		return PyRun_SimpleFile(fp, fileName, true);
//	}
//};


#endif /* end of include guard: PYREF_QOKADTFD */

