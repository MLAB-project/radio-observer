/**
 * \file   RingBuffer.h
 * \author Jan Milík <milikjan@fit.cvut.cz>
 * \date   2013-05-15
 *
 * \brief  Header file for the RingBuffer class.
 */

#ifndef RINGBUFFER_HSQMLSDG
#define RINGBUFFER_HSQMLSDG

/**
 * \todo Write documentation for class RingBuffer.
 *
 * Items enter at the HEAD of the buffer and exit at the TAIL.
 */
template<class T>
class RingBuffer {
private:
	T   *buffer_;
	int  capacity_;
	T   *head_;
	T   *tail_;
	
public:
	/**
	 * Constructor.
	 */
	RingBuffer(int capacity) :
		buffer_(NULL), capacity_(capacity), head_(NULL), tail_(NULL)
	{
		assert(capacity >= 0);

		if (capacity > 0) {
			buffer_ = new T[capacity];
			head_ = buffer_;
			tail_ = buffer_;
		}
	}
	/**
	 * Copy constructor.
	 */
	RingBuffer(const RingBuffer& other) :
		buffer_(NULL), capacity_(other.capacity_), head_(NULL), tail_(NULL)
	{
		if (capacity_ > 0) {
			buffer_ = new T[capacity_];
			head_ = buffer_ + (other.head_ - other.buffer_);
			tail_ = buffer_ + (other.tail_ - other.buffer_);
			
			if (head_ >= tail_) {
				memcpy(tail_, other.tail_, sizeof(T) * getSize());
			} else {
				memcpy(tail_, other.tail_,
					  sizeof(T) * (buffer_ + capacity_ - tail_));
				memcpy(tail_, other.buffer_,
					  sizeof(T) * (head_ - buffer_));
			}
		}
	}
	/**
	 * Destructor.
	 */
	~RingBuffer()
	{
		delete [] buffer_;
		buffer_ = NULL;
		capacity_ = 0;
		head_ = NULL;
		tail_ = NULL;
	}
	
	inline bool isEmpty()     const { return head_ == tail_; }
	inline bool isFull()      const
	{
		if (head_ + 1 == buffer_ + capacity_)
			return (tail_ == buffer_);
		return (head_ + 1 == tail_);
	}
	inline int  getCapacity() const { return capacity_; }
	
	int  getSize()     const
	{
		if (head_ >= tail_)
			return head_ - tail_;
		return capacity_ - (tail_ - head_);
	}
	
	bool tryPop(T *item)
	{
		if (isEmpty()) return false;
		
		if (item != NULL)
			*item = *tail_;
		tail_++;
		if (tail_ >= buffer_ + capacity_)
			tail_ = buffer_;

		return true;
	}
	
	T pop(T defaultValue)
	{
		T result;
		if (tryPop(&result)) return result;
		return defaultValue;
	}
	
	void push(const T &item)
	{
		if (isFull()) tryPop(NULL);
		
		*head_ = item;
		head_++;
		
		if (head_ >= buffer_ + capacity_)
			head_ = buffer_;
	}
};


template<class T>
class RingBuffer2D {
private:
	typedef T                 value_type;
	typedef value_type&       reference;
	typedef const value_type& const_reference;
	typedef value_type*       pointer;
	typedef const value_type* const_pointer;
	
	int width_;          //< Width of a row in number of elements.
	int minCapacity_;    //< Desired (minimal) capacity of the buffer in number of rows.
	int chunkSizeLimit_; //< Maximal desired chunk size in bytes.
	
	int      capacity_;      //< Actual capacity of the buffer in number of rows.
	int      chunkElements_; //< Actual chunk size in number of elements (row width * row count).
	int      chunkRows_;     //< Actual chunk size in number of rows.
	int      chunkCount_;    //< Actual number of chunks.
	pointer *chunks_;        //< Array of pointers to chunks.
	
	struct ChunkItem {
		pointer *chunk; //< Points to an item in RingBuffer2D::chunks_.
		pointer  item;  //< Points to an item in the selected chunk.
		
		ChunkItem() :
			chunk(NULL), item(NULL)
		{ }
		
		ChunkItem(pointer *chunk, pointer item) :
			chunk(chunk), item(item)
		{ }

		ChunkItem(pointer *chunk, int item) :
			chunk(chunk), item(*chunk + item)
		{ }
	};
	
	ChunkItem head_;
	int       size_;
	
	inline ChunkItem advance(const ChunkItem item) const
	{
		ChunkItem result = item;
		
		result.item += width_;
		
		if (result.item >= (*result.chunk + chunkElements_)) {
			result.chunk++;
			if (result.chunk >= (chunks_ + chunkCount_))
				result.chunk = chunks_;
			result.item = *result.chunk;
		}
		
		return result;
	}
	
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
	
	inline ChunkItem getItem(int rowIndex)
	{
		int chunkIndex = rowIndex / chunkRows_;
		int itemIndex = (rowIndex % chunkRows_) * width_;
		
		return ChunkItem(chunks_ + chunkIndex, itemIndex);
	}
	
	inline int getRowIndex(const ChunkItem &item)
	{
		int chunkIndex = item.chunk - chunks_;
		int itemIndex = (item.item - *item.chunk) / width_;
		
		return (chunkIndex * chunkRows_) + itemIndex;
	}

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
		head_ = ChunkItem(chunks_, chunks_[0]);
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
		dispose();
		
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
	
	pointer push()
	{
		pointer result = head_.item;
		head_ = advance(head_);
		
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
		return getItem(rowIndex).item;
	}
	
	int mark()
	{
		return getRowIndex(head_);
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


#endif /* end of include guard: RINGBUFFER_HSQMLSDG */

