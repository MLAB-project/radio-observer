/**
 * \file   RingBufferTest.h
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-06-06
 *
 * \brief  Header file for the RingBufferTest class.
 */

#ifndef RINGBUFFERTEST_TEKSJAT1
#define RINGBUFFERTEST_TEKSJAT1

#include <cppapp/cppapp.h>
using namespace cppapp;

#include "../src/RingBuffer.h"


/**
 * \todo Write documentation for class RingBufferTest.
 */
class RingBufferTest : public TestCase {
public:
	/**
	 * Constructor.
	 */
	RingBufferTest()
	{
		TEST_ADD(RingBufferTest, testConstructor);
		TEST_ADD(RingBufferTest, testCopyConstructor);
		TEST_ADD(RingBufferTest, testCopyConstructorOverlap);
		TEST_ADD(RingBufferTest, testPush);
	}
	
	void testConstructor()
	{
		RingBuffer<int> buffer(100);
		
		TEST_EQUALS(100, buffer.getCapacity(), "created buffer has wrong capacity");
		TEST_EQUALS(0, buffer.getSize(), "created buffer should have size 0");
		TEST_ASSERT(buffer.isEmpty(), "created buffer should be empty");
		TEST_ASSERT(!buffer.isFull(), "created buffer should not be full");
	}
	
	void testCopyConstructor()
	{
		RingBuffer<int> a(100);
		
		for (int i = 0; i < 10; i++)
			a.push(i);
		
		TEST_EQUALS(10, a.getSize(), "just checking");
		
		RingBuffer<int> b(a);
		
		TEST_EQUALS(10, a.getSize(),
				  "copied buffer's size should not change");
		TEST_EQUALS(a.getSize(), b.getSize(),
				  "copy's size should be the same as original");
		
		//for (int i = 0; i < a.getSize(); i++) {
		//	int valueA;
		//	int valueB;
		//	//TEST_ASSERT(b.tryPop(&valueB),
		//	//		  "there should still be elements in the buffer");
		//	//TEST_ASSERT(a.tryPop(&valueA),
		//	//		  "something is wrong with tryPop()");
		//	//TEST_EQUALS(valueA, valueB,
		//	//		  "the elements poped from the copy should be equal "
		//	//		  "to the original");
		//}
	}
	
	void testCopyConstructorOverlap()
	{
		int cap = 100;
		RingBuffer<int> a(cap);
		
		TEST_EQUALS(cap, a.getCapacity(),
				  "buffer should have the specfified capacity");
		
		for (int i = 0; i < (cap * 3); i++)
			a.push(i);
		
		TEST_ASSERT(a.getSize() <= cap, "just checking");
		
		RingBuffer<int> b(a);
		
		TEST_EQUALS(cap, a.getCapacity(),
				  "copied buffer's size should not change");
		TEST_EQUALS(a.getSize(), b.getSize(),
				  "copy's size should be the same as original");
		
		//for (int i = 0; i < a.getSize(); i++) {
		//	int valueA;
		//	int valueB;
		//	TEST_ASSERT(b.tryPop(&valueB),
		//			  "there should still be elements in the buffer");
		//	TEST_ASSERT(a.tryPop(&valueA),
		//			  "something is wrong with tryPop()");
		//	TEST_EQUALS(valueA, valueB,
		//			  "the elements poped from the copy should be equal "
		//			  "to the original");
		//}

	}
	
	void testPush()
	{
		int size = 100;
		RingBuffer<int> buffer(size);
		
		for (int i = 0; i < (size * 5); i++) {
			buffer.push(i);
			
			TEST_ASSERT(!buffer.isEmpty(),
					  "after append, buffer should not be empty");
			
			TEST_EQUALS(buffer.isFull(), (buffer.getSize() == buffer.getCapacity()),
					  "buffer should be full when size equals capacity");
			
			if (buffer.isFull()) {
				TEST_EQUALS(size, buffer.getSize(),
						  "buffer size should no longer increase");
			} else {
				TEST_EQUALS(i + 1, buffer.getSize(),
						  "buffer size shoold increase after push");
			}
			
			//if ((i + 1) >= (size - 1)) {
			//	TEST_EQUALS(size - 1, buffer.getSize(),
			//			  "buffer size should increase with each append until "
			//			  "it is full");
			//	TEST_ASSERT(buffer.isFull(),
			//			  "after append certain number of elements, the buffer "
			//			  "should be full");
			//} else {
			//	TEST_EQUALS(i + 1, buffer.getSize(),
			//			  "buffer size should increase with each append until "
			//			  "it is full");
			//	TEST_ASSERT(!buffer.isFull(),
			//			  "buffer should not be full until its capacity is filled");
			//}
		}
	}
};

RUN_SUITE(RingBufferTest);


class RingBuffer2DTest : public TestCase {
public:
	RingBuffer2DTest()
	{
		TEST_ADD(RingBuffer2DTest, testConstructor1);
		TEST_ADD(RingBuffer2DTest, testConstructor2);
		TEST_ADD(RingBuffer2DTest, testPush);
		TEST_ADD(RingBuffer2DTest, testMark);
		TEST_ADD(RingBuffer2DTest, testReservations);
	}
	
	template<class T>
	T* fillRow(RingBuffer2D<T> *buffer, T value)
	{
		T* row = buffer->push();
		for (int i = 0; i < buffer->getWidth(); i++) {
			row[i] = value;
		}
		return row;
	}
	
	void testConstructor(int width, int chunkSize)
	{
		RingBuffer2D<int> buffer(width, chunkSize);
		TEST_EQUALS(0, buffer.getCapacity(),
				  "created buffer has wrong capacity");
		TEST_EQUALS(0, buffer.getSize(), "created buffer should have 0 size");
		TEST_ASSERT(buffer.isEmpty(), "created buffer should be empty");
		TEST_ASSERT(!buffer.isFull(), "created buffer should not be full");
	}
	
	void testConstructor(int width, int chunkSize, int capacity)
	{
		RingBuffer2D<int> buffer(width, chunkSize, capacity);
		TEST_ASSERT(buffer.getCapacity() >= capacity,
				  "created buffer has wrong capacity");
		TEST_EQUALS(0, buffer.getSize(), "created buffer should have 0 size");
		TEST_ASSERT(buffer.isEmpty(), "created buffer should be empty");
		TEST_ASSERT(!buffer.isFull(), "created buffer should not be full");
	}
	
	void testConstructor1()
	{
		testConstructor(16, sizeof(int) * 16 * 8);
		testConstructor(16, sizeof(int) * 16 * 8 - sizeof(int));
	}
	
	void testConstructor2()
	{
		testConstructor(16, sizeof(int) * 16 * 8, 16 * 8 * 8);
		testConstructor(16, sizeof(int) * 16 * 8 - sizeof(int), 16 * 8 * 8);
	}
	
	void testPush(int width, int chunkSize, int capacity)
	{
		RingBuffer2D<int> buffer(width, chunkSize, capacity);
		
		for (int i = 0; i < (capacity * 3); i++) {
			int *ptr = buffer.push();
			for (int j = 0; j < width; j++) {
				ptr[j] = i;
			}
			
			if ((i + 1) > buffer.getCapacity()) {
				TEST_EQUALS(buffer.getCapacity(), buffer.getSize(),
						  "buffer size should be the same as capacity");
			} else {
				TEST_EQUALS((i + 1), buffer.getSize(),
						  "buffer size should increase after push");
			}
		}
	}
	
	void testPush()
	{
		testPush(16, sizeof(int) * 16 * 8, 16 * 8 * 8);
		testPush(16, sizeof(int) * 16 * 8 - 1, 16 * 8 * 8);
	}

	void testMark(int width, int chunkSize, int capacity)
	{
		RingBuffer2D<int> buffer(width, chunkSize, capacity);
		
		for (int i = 0; i < (capacity * 3); i++) {
			int *ptr = buffer.push();
			for (int j = 0; j < width; j++) ptr[j] = i;
			
			TEST_EQUALS(buffer.mark(), ((i + 1) % buffer.getCapacity()),
					  "mark() should return index of the next item to be inserted");
		}
	}
	
	void testMark()
	{
		testMark(16, sizeof(int) * 16 * 8, 16 * 8 * 8);
		testMark(16, sizeof(int) * 16 * 8 - 1, 16 * 8 * 8);
	}
	
	void testReservations(int width, int chunkSize, int capacity)
	{
		RingBuffer2D<int> buffer(width, chunkSize, capacity);
		int size = 10;
		
		int mark = buffer.mark();
		
		//TEST_EQUALS(buffer.size(mark), 0, "");
		
		for (int i = 0; i < size; i++) {
			fillRow(&buffer, i);
		}
		
		TEST_EQUALS(buffer.size(mark), 10, "");
		
		int handle = buffer.reserve(mark, size / 2);
		TEST_ASSERT(!buffer.isDirty(handle), "");
		
		for (int i = 0; i < capacity; i++) {
			fillRow(&buffer, i);
		}
		
		TEST_ASSERT(buffer.isDirty(handle), "");
	}
	
	void testReservations()
	{
		testReservations(16, sizeof(int) * 16 * 8, 16 * 8 * 8);
		testReservations(16, sizeof(int) * 16 * 8 - 1, 16 * 8 * 8);

		testReservations(16, sizeof(int) * 16 * 8, 16 * 8 * 31);
		testReservations(16, sizeof(int) * 16 * 8 - 1, 16 * 8 * 31);
	}
};

RUN_SUITE(RingBuffer2DTest);


#endif /* end of include guard: RINGBUFFERTEST_TEKSJAT1 */

