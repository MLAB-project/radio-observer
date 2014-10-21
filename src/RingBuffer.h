/**
 * \file   RingBuffer.h
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-05-15
 *
 * \brief  Header file for the RingBuffer class.
 */

#ifndef RINGBUFFER_HSQMLSDG
#define RINGBUFFER_HSQMLSDG


#include <limits>


/**
 * \todo Write documentation for class RingBuffer.
 *
 * Items enter at the HEAD of the buffer and exit at the TAIL.
 */
template<class T>
class RingBuffer {
private:
	int  capacity_;
	
	T   *items_;
	
	T   *head_;
	int  size_;
	
public:
	void resize(int capacity)
	{
		if (capacity_ > capacity)
			return;
		
		// Alloc new items.
		T *newItems = NULL;
		if (capacity > 0) newItems = new T[capacity];
		
		// Copy old items.
		int newSize = size_;
		if (newSize > capacity) newSize = capacity;
		
		//
		//                        head
		//                         |
		// old items    |--- 2 ----|--------------|-- 1 --|
		// new items    |-- 1 --|--- 2 ----|--------------------|
		//                                 |
		//                                head
		//
		if (newSize > 0) {
			int offset = normalize(head() - newSize);
			int remaining = newSize;
			
			// Copy section 1
			int chunk = capacity_ - offset;
			if (chunk > remaining) chunk = remaining;
			remaining -= chunk;
			
			for (int i = 0; i < chunk; i++)
				newItems[i] = items_[offset + i];
			
			// Copy section 2
			if (remaining > 0) {
				for (int i = 0; i < remaining; i++)
					newItems[chunk + i] = items_[i];
			}
		}
		
		// Clean up old items.
		delete [] items_;
		items_    = newItems;
		capacity_ = capacity;
		head_     = items_;
		size_     = newSize;
	}
	
	RingBuffer() :
		capacity_(0), items_(NULL), head_(NULL), size_(0)
	{}
	
	/**
	 * \brief Constructor.
	 */
	RingBuffer(int capacity) :
		capacity_(0), items_(NULL), head_(NULL), size_(0)
	{
		assert(capacity >= 0);
		resize(capacity);
	}
	
	/**
	 * \brief Copy constructor.
	 */
	RingBuffer(const RingBuffer& other) :
		capacity_(0), items_(NULL), head_(NULL), size_(0)
	{
		resize(other.capacity_);
		
		if (capacity_ > 0) {
			head_ = items_ + (other.head_ - other.items_);
			size_ = other.size_;
			
			T *tail = head_ - size_;
			T *otherTail = other.head_ - size_;
			
			if (tail >= items_) {
				memcpy(tail, otherTail, sizeof(T) * size_);
			} else {
				tail      += capacity_;
				otherTail += capacity_;

				size_t tailSize = size_ - (head_ - items_);
				
				memcpy(tail, otherTail,
					  sizeof(T) * tailSize);
				memcpy(items_, other.items_,
					  sizeof(T) * (size_ - tailSize));
			}
		}
	}
	
	/**
	 * Destructor.
	 */
	~RingBuffer()
	{
		capacity_ = 0;
		delete [] items_;
		items_ = NULL;
		head_ = NULL;
		size_ = 0;
	}
	
	inline bool isEmpty()     const { return size_ == 0; }
	inline bool isFull()      const { return size_ == capacity_; }
	inline int  getCapacity() const { return capacity_; }
	inline int  getSize()     const { return size_; }
	
	inline void clear()
	{
		head_ = items_;
		size_ = 0;
	}
	
	inline void clear(const T& value)
	{
		clear();
		
		for (T *p = items_; p < (items_ + capacity_); p++)
			*p = value;
	}
	
	inline int normalize(int mark)
	{
		return mark % capacity_;
	}
	
	inline int head() { return head_ - items_; }
	
	inline T& at(int mark)
	{
		return items_[normalize(mark)];
	}
	
	inline void push(const T &item)
	{
		if (capacity_ < 1) return;
		
		*head_ = item;
		head_++;
		
		if (head_ >= (items_ + capacity_))
			head_ = items_;
		
		if (size_ < capacity_) size_++;
	}
	
	inline void push(const T *items, int count)
	{
		// If pushing more items that the buffer can hold,
		// clear the buffer and copy the last n items to the
		// buffer.
		if (count >= capacity_) {
			clear();
			int offset = count - capacity_;
			for (int i = 0; i < capacity_; i++) {
				items_[i] = items[offset + i];
			}
			return;
		}
		
		// Otherwise, start copying the pushed items to the
		// buffer at the head.
		int offset = head_ - items_;
		for (int i = 0; i < count; i++) {
			items_[offset] = items[i];
			offset = (offset + 1) % capacity_;
		}
		head_ = items_ + offset;
		size_ += count;
		if (size_ > capacity_) size_ = capacity_;
	}
};


template<class T>
class RingBuffer2D {
public:
	typedef T                 value_type;
	typedef value_type&       reference;
	typedef const value_type& const_reference;
	typedef value_type*       pointer;
	typedef const value_type* const_pointer;
	
	typedef int gen_type;
	static const gen_type GEN_WRAP_HIGH;
	static const gen_type GEN_WRAP_LOW;
	
private:
	int width_;          //< Width of a row in number of elements.
	int minCapacity_;    //< Desired (minimal) capacity of the buffer in number of rows.
	int chunkSizeLimit_; //< Maximal desired chunk size in bytes.
	
	int      capacity_;      //< Actual capacity of the buffer in number of rows.
	int      chunkElements_; //< Actual chunk size in number of elements (row width * row count).
	int      chunkRows_;     //< Actual chunk size in number of rows.
	int      chunkCount_;    //< Actual number of chunks.
	pointer *chunks_;        //< Array of pointers to chunks.
	
	struct ChunkItem {
		//RingBuffer2D<T> *buffer;
		pointer         *chunk; //< Points to an item in RingBuffer2D::chunks_.
		pointer          item;  //< Points to an item in the selected chunk.
		int              index; //< 
		gen_type         generation;
		
		ChunkItem() :
			chunk(NULL), item(NULL), generation(0)
		{ }

		ChunkItem(const RingBuffer2D<T> &buffer) :
			chunk(buffer.chunks_), item(*chunk), index(0), generation(0)
		{ }
		
		//ChunkItem(pointer *chunk, pointer item) :
		//	chunk(chunk), item(item), generation(0)
		//{ }
		//
		//ChunkItem(pointer *chunk, int item) :
		//	chunk(chunk), item(*chunk + item), generation(0)
		//{ }
		
		ChunkItem(const RingBuffer2D<T> &buffer, int index, int gen) :
			chunk(buffer.chunks_ + (index / buffer.chunkRows_)),
			item(*chunk + ((index % buffer.chunkRows_) * buffer.width_)),
			index(index),
			generation(gen)
		{
			assert(chunk >= buffer.chunks_);
			assert(chunk < (buffer.chunks_ + buffer.chunkCount_));
			
			assert(item >= *chunk);
			assert(item < (*chunk + buffer.chunkElements_));
			
			assert(index >= 0);
			assert(index < buffer.capacity_);
		}
		
		inline bool isBefore(const ChunkItem &other)
		{
			return ((generation < other.generation) ||
				   ((generation > GEN_WRAP_HIGH) && (other.generation < GEN_WRAP_LOW)));
		}
		
		inline ChunkItem advance(const RingBuffer2D<T> &buffer, int delta) const
		{
			return ChunkItem(buffer,
						  buffer.normalizeRowIndex(index + delta),
						  generation + delta);
		}
		
		inline ChunkItem advance(const RingBuffer2D<T> &buffer) const
		{
			ChunkItem result = *this;
			
			result.index++;
			result.index %= buffer.capacity_;
			result.generation++;
			
			result.item += buffer.width_;
			
			if (result.item >= (*result.chunk + buffer.chunkElements_)) {
				result.chunk++;
				if (result.chunk >= (buffer.chunks_ + buffer.chunkCount_))
					result.chunk = buffer.chunks_;
				result.item = *result.chunk;
			}
			
			return result;
		}
	};
	
	ChunkItem head_;
	int       size_;
	
	//inline ChunkItem advance(const ChunkItem &item) const
	//{
	//	ChunkItem result = item;
	//	
	//	result.generation++;
	//	result.item += width_;
	//	
	//	if (result.item >= (*result.chunk + chunkElements_)) {
	//		result.chunk++;
	//		if (result.chunk >= (chunks_ + chunkCount_))
	//			result.chunk = chunks_;
	//		result.item = *result.chunk;
	//	}
	//	
	//	return result;
	//}
	
	inline void dispose()
	{
		if ((chunkCount_ < 1) || (chunks_ == NULL)) return; 
		
		for (pointer *chunk = chunks_; chunk < (chunks_ + chunkCount_); chunk++) {
			delete [] *chunk;
			*chunk = NULL;
		}
		
		delete [] chunks_;
		chunks_ = NULL;
		
		chunkCount_ = 0;
		capacity_ = 0;
		size_ = 0;
		head_ = ChunkItem();
	}
	
	//inline ChunkItem getItem(int rowIndex)
	//{
	//	int chunkIndex = rowIndex / chunkRows_;
	//	int itemIndex = (rowIndex % chunkRows_) * width_;
	//	
	//	return ChunkItem(chunks_ + chunkIndex, itemIndex);
	//}
	
	//inline int getRowIndex(const ChunkItem &item)
	//{
	//	int chunkIndex = item.chunk - chunks_;
	//	int itemIndex = (item.item - *item.chunk) / width_;
	//	
	//	return (chunkIndex * chunkRows_) + itemIndex;
	//}

	inline int normalizeRowIndex(int value)
	{
		while (value < 0) {
			value += capacity_;
		}
		
		value = value % capacity_;
		
		return value;
	}

public:
	RingBuffer2D() :
		width_(0), minCapacity_(0), chunkSizeLimit_(0),
		capacity_(0), chunkElements_(0), chunkRows_(0), chunkCount_(0), chunks_(NULL),
		head_(), size_(0)
	{}
	
	RingBuffer2D(int width, int chunkSize) :
		width_(width), minCapacity_(0), chunkSizeLimit_(chunkSize),
		capacity_(0), chunkElements_(0), chunkRows_(0), chunkCount_(0), chunks_(NULL),
		head_(), size_(0)
	{
		int rowSize = sizeof(T) * width_;
		chunkRows_ = chunkSizeLimit_ / rowSize;
		if ((chunkSizeLimit_ % rowSize) != 0) chunkRows_++;
		
		chunkElements_ = chunkRows_ * width;
	}
	
	RingBuffer2D(int width, int chunkSize, int capacity) :
		width_(width), minCapacity_(0), chunkSizeLimit_(chunkSize),
		capacity_(0), chunkElements_(0), chunkRows_(0), chunkCount_(0), chunks_(NULL),
		head_(), size_(0)
	{
		int rowSize = sizeof(T) * width_;
		chunkRows_ = chunkSizeLimit_ / rowSize;
		if ((chunkSizeLimit_ % rowSize) != 0) chunkRows_++;
		
		chunkElements_ = chunkRows_ * width;
		
		resize(capacity);
	}
	
	~RingBuffer2D()
	{
		dispose();
	}
	
	inline int getWidth()    const { return width_; }
	inline int getCapacity() const { return capacity_; }
	inline int getSize()     const { return size_; }
	inline bool isEmpty()    const { return (size_ == 0); }
	inline bool isFull() const
	{
		return ((size_ >= capacity_) && (capacity_ > 0));
	}
	
	void clear()
	{
		//head_       = ChunkItem(chunks_, chunks_[0]);
		head_ = ChunkItem(*this);
		size_ = 0;
	}
	
	/**
	 * \brief Resizes the ring buffer, discarding all current data in process.
	 */
	void resize(int width, int chunkSize, int capacity)
	{
		dispose();

		width_          = width;
		chunkSizeLimit_ = chunkSize;
		
		// Calculate chunk metrics
		int rowSize = sizeof(T) * width_;
		chunkRows_ = chunkSizeLimit_ / rowSize;
		if ((chunkSizeLimit_ % rowSize) != 0) chunkRows_++;
		
		chunkElements_ = chunkRows_ * width;
		
		// Calculate capacities/sizes
		minCapacity_ = capacity;
		
		chunkCount_ = minCapacity_ / chunkRows_;
		if ((minCapacity_ % chunkRows_) > 0) chunkCount_ += 1;
		
		capacity_ = chunkCount_ * chunkRows_;
		
		// Allocate memory
		chunks_ = new pointer[chunkCount_];
		for (int i = 0; i < chunkCount_; i++) {
			chunks_[i] = new T[chunkElements_];
		}
		
		clear();
	}
	
	void resize(int capacity)
	{
		resize(width_, chunkSizeLimit_, capacity);
		
		//dispose();
		//
		//// Calculate capacities/sizes
		//minCapacity_ = capacity;
		//
		//chunkCount_ = minCapacity_ / chunkRows_;
		//if ((minCapacity_ % chunkRows_) > 0) chunkCount_ += 1;
		//
		//capacity_ = chunkCount_ * chunkRows_;
		//
		//// Allocate memory
		//chunks_ = new pointer[chunkCount_];
		//for (int i = 0; i < chunkCount_; i++) {
		//	chunks_[i] = new T[chunkElements_];
		//}
		//
		//clear();
	}
	
	pointer push()
	{
		pointer result = head_.item;
		head_ = head_.advance(*this);
		
		if (!isFull()) size_++;
		assert(size_ <= capacity_);
		
		FOR_EACH(reservations_, it) {
			if (it->alive && isInRange(mark(), it->start, it->end))
				it->dirty = true;
		}
		
		return result;
	}
	
	pointer at(int mark)
	{
		int rowIndex = normalizeRowIndex(mark);
		//return getItem(rowIndex).item;
		return ChunkItem(*this, rowIndex, 0).item;
	}
	
	int mark()
	{
		//return getRowIndex(head_);
		return head_.index;
	}
	
	//// RESERVATIONS //////////////////////////////////////////////
private:
	struct Reservation {
		int start;
		int end;

		bool alive;
		bool dirty;

		Reservation() :
			start(0), end(0), alive(false), dirty(false)
		{}
		
		void init(int start, int end) {
			this->start = start;
			this->end = 0;
			this->alive = true;
			this->dirty = false;
		}
		
		void free() {
			alive = false;
		}
	};
	
	vector<Reservation> reservations_;
	vector<int>         freeReservations_;
	
public:
	/**
	 * \brief Returns number of elements (rows) between \c start and \c end.
	 */
	int size(int start, int end) {
		start = normalizeRowIndex(start);
		end = normalizeRowIndex(end);
		
		if (end > start)
			return (end - start);
		return (capacity_ - start) + end;
	}
	
	/**
	 * \brief Returns number of elements (rows) between \c start and head of the buffer.
	 */
	int size(int start) {
		start = normalizeRowIndex(start);
		
		int headIndex = mark();
		return size(start, headIndex);
	}
	
	bool isInRange(int index, int start, int end) {
		index = normalizeRowIndex(index);
		start = normalizeRowIndex(start);
		end = normalizeRowIndex(end);
		
		if (end > start)
			return ((index >= start) && (index < end));
		
		return ((index >= start) || (index < end));
	}
	
	/**
	 * \brief Creates a reservation and returns a handle to it.
	 *
	 * \return integer handle to the created reservation
	 *
	 * \note This method is not thread-safe.
	 */
	int reserve(int start, int end) {
		int handle = -1;
		
		start = normalizeRowIndex(start);
		end   = normalizeRowIndex(end);
		
		if (freeReservations_.size() > 0) {
			handle = freeReservations_.back();
			freeReservations_.pop_back();
		} else {
			handle = reservations_.size();
			reservations_.push_back(Reservation());
		}
		
		reservations_[handle].init(start, end);

		return handle;
	}
	
	/**
	 * \brief Frees a reservation previously created by reserve().
	 *
	 * \param handle handle to the reservation returned by the reserve() method
	 *
	 * \note This method is not thread-safe.
	 */
	bool freeReservation(int handle) {
		if ((handle < 0) || (handle >= (int)reservations_.size()))
			return false;
		reservations_[handle].free();
		freeReservations_.push_back(handle);
		return true;
	}
	
	/**
	 * \brief Returns \c true if the reservation with the specified handle has been written to.
	 */
	bool isDirty(int handle) {
		assert((handle >= 0) && (handle < (int)reservations_.size()));
		return reservations_[handle].dirty;
	}
};

template<class T>
const typename RingBuffer2D<T>::gen_type RingBuffer2D<T>::GEN_WRAP_HIGH = numeric_limits<gen_type>::max() - (numeric_limits<gen_type>::max() / 4);

template<class T>
const typename RingBuffer2D<T>::gen_type RingBuffer2D<T>::GEN_WRAP_LOW = numeric_limits<gen_type>::min() - (numeric_limits<gen_type>::min() / 4);


template<class T, class Container>
class RingBuffer2D_ {
public:
	typedef Container container_type;
	typedef T*        value_type;

private:
	int       width_;
	Container container_;
};


#endif /* end of include guard: RINGBUFFER_HSQMLSDG */

