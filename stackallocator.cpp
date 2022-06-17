#include<iostream>
#include<vector>

template<int64_t Size>
class StackStorage {

public:
	StackStorage(StackStorage& other) = delete;
	void operator=(const StackStorage& other) = delete;
	StackStorage() = default;


	char* allocate(int64_t n, const int64_t align) {
		char* copy = finish;
		finish += n + align - ((reinterpret_cast<uintptr_t>(finish) + n) % align);
		return copy;
	}

	void deallocate(char* ptr, int64_t n, const int64_t align) {
		if (n + align - ((reinterpret_cast<uintptr_t>(finish) + n) % align) + ptr == finish) {
			finish = ptr;
		}
	}

private:
	char start[Size];
	char* finish{ start };
};


template<class C, int64_t Size>
class StackAllocator {
public:
	StackAllocator() = default;
	StackAllocator(StackStorage<Size>& other) : storage(&other) {}
	using value_type = C;
	template <class U>
	struct rebind { using other = StackAllocator<U, Size>; };
	~StackAllocator() {}
	template <class U>
	StackAllocator(StackAllocator<U, Size> const& other) : storage(other.storage) { storage->allocate(1, alignof(C)); }

	C* allocate(int64_t sz) {
		return static_cast<C*>(static_cast<void*>(storage->allocate(sz * sizeof(C), alignof(C))));
	}

	StackAllocator& operator=(StackAllocator const& other) {
		storage = other.storage;
		return *this;
	}

	void deallocate(C* ptr, int64_t sz) {
		storage->deallocate(static_cast<char*>(static_cast<void*>(ptr)), sz * sizeof(C), alignof(C));
	}
	template <class U> void destroy(U* const ptr) { ptr->~U(); }

	template <class U, int64_t sz>
	bool operator!=(StackAllocator<U, sz> const& other) const { return storage != other.storage; }

	template <class U, class ...A>
	void construct(U* const ptr, A&& ...args) {
		new (ptr) U(std::forward<A>(args)...);
	}
	template <class U, int64_t sz>
	bool operator==(StackAllocator<U, sz> const& other) const { return !(*this != other); }
private:
	StackStorage<Size>* storage{};
	template <class U, int64_t S>
	friend class StackAllocator;

};

template<typename T, typename Allocator = std::allocator<T>>
class List {
private:
	class Node {
	public:
		T value;
		Node* prev = this;
		Node* next = this;
		Node(Node* prev = nullptr, Node* next = nullptr) : value(T()), prev(prev), next(next) {}
		Node() = default;
		Node(const T& newValue) : value(newValue), prev(this), next(this) {}
		~Node() {}
		explicit Node(const T& value, Node* prev, Node* next) : value(value), prev(prev), next(next) {};
	};

	Node* root;
	size_t sz = 0;
	using NAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
	NAllocator alloc;
	void bind_nodes(Node* left, Node* right) {
		if (left != nullptr) left->next = right;
		if (right != nullptr) right->prev = left;
	}
	template<bool Con>
	class iter {
	public:
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = T;
		using difference_type = std::ptrdiff_t;
		using pointer = std::conditional_t<Con, const T*, T*>;
		using reference = std::conditional_t<Con, const T&, T&>;
		friend class List<T, Allocator>;
		iter() : ptr(nullptr) {};
		iter(Node* v) : ptr(v) {}
		iter(const iter<false>& other) : ptr(other.ptr) {};
		iter& operator++() {
			ptr = ptr->next;
			return *this;
		}

		iter operator++(int) {
			iter ans = *this;
			++* this;
			return ans;
		}
		Node* getNodePtr() const { return ptr; }


		iter& operator--() {
			ptr = ptr->prev;
			return *this;
		}
		bool operator==(const iter& other) const {
			if (other.ptr == nullptr || ptr == nullptr) return false;
			return ptr == other.ptr;
		}
		iter operator--(int) {
			iter ans = *this;
			--* this;
			return ans;
		}

		reference operator*() const {
			return ptr->value;
		}

		pointer operator->() const {
			return &ptr->value;
		}

		operator iter<true>() {
			return iter<true>(ptr);
		}

		bool operator!=(const iter& other) const {
			return !(*this == other);
		}
	private:
		Node* ptr;
	};
	Node* build_node() {
		Node* new_head = static_cast<Node*>(std::allocator_traits<NAllocator>::allocate(alloc, 1));
		if (new_head != nullptr) {
			new_head->next = new_head;
			new_head->prev = new_head;
		}
		return new_head;
	}

public:

	using iterator = iter<false>;
	using const_iterator = iter<true>;
	using reverse_iterator = std::reverse_iterator<iter<false>>;
	using const_reverse_iterator = std::reverse_iterator<iter<true>>;

	iterator begin() {
		return iterator(root->next);
	}
	const_iterator begin() const {
		return const_iterator(root->next);
	}
	iterator end() {
		return iterator(root);
	}
	const_iterator end() const {
		return const_iterator(root);
	}
	const_iterator cbegin() const {
		return const_iterator(root->next);
	}
	const_iterator cend() const {
		return const_iterator(root);
	}
	reverse_iterator rbegin() {
		auto ans = reverse_iterator(root);
		return ans;
	}
	const_reverse_iterator rbegin() const {
		return const_reverse_iterator(root);
	}
	reverse_iterator rend() {
		return reverse_iterator(root->next);
	}
	const_reverse_iterator rend() const {
		return const_reverse_iterator(root->next);
	}
	const_reverse_iterator crbegin() const {
		return const_reverse_iterator(root);
	}
	const_reverse_iterator crend() const {
		return const_reverse_iterator(root->next);
	}
	void bind_n2(Node* n1, Node* n2, Node* n3) {
		bind_nodes(n1, n2);
		bind_nodes(n3, n1);
	}
	void insert_before(const_iterator pos) {
		--pos;
		Node* prev = (pos).ptr;
		Node* new_node = std::allocator_traits<NAllocator>::allocate(alloc, 1);
		try {
			++pos;
			std::allocator_traits<NAllocator>::construct(alloc, new_node, prev, (pos).ptr);
			bind_n2(new_node, pos.ptr, prev);
		}
		catch (...) {
			std::allocator_traits<NAllocator>::deallocate(alloc, new_node, 1);
			throw;
		}
		sz += 1;
	}

	explicit List(const Allocator& alloc = Allocator()) : alloc(alloc) {
		root = build_node();
	}

	bool empty() const { return sz == 0; }
	void Catch() {
		while (!empty()) {
			pop_front();
		}
		throw;
	}


	void insert_before_with_value(const_iterator pos, const T& val) {
		--pos;
		Node* prev = (pos).ptr;
		Node* new_node = std::allocator_traits<NAllocator>::allocate(alloc, 1);
		try {
			++pos;
			std::allocator_traits<NAllocator>::construct(alloc, new_node, val, prev, (pos).ptr);
			bind_n2(new_node, pos.ptr, prev);
		}
		catch (...) {
			std::allocator_traits<NAllocator>::deallocate(alloc, new_node, 1);
			throw;
		}
		sz += 1;
	}
	List(size_t newSz, const T& newValue, const Allocator& alloc = Allocator()) : List(alloc) {
		try {
			while (newSz != 0) {
				insert_before_with_value(end(), newValue);
				newSz -= 1;
			}
		}
		catch (...) {
			Catch();
		}

	}
	List(size_t newSz, const Allocator& alloc = Allocator()) : List(alloc) {
		try {
			while (newSz != 0) {
				insert_before(end());
				newSz -= 1;
			}
		}
		catch (...) {
			Catch();
		}
	}

	List(const List& other) :
		List(std::allocator_traits<Allocator>::select_on_container_copy_construction(other.alloc)) {
		try {
			for (auto it = other.begin(); it != other.end(); ++it) {
				push_back(*it);
			}
		}
		catch (...) {
			Catch();
		}
	}


	~List() {
		while (!empty()) {
			pop_front();
		}
		std::allocator_traits<NAllocator>::deallocate(alloc, root, 1);
	}

	List& operator=(const List& other) {
		int64_t num = 0;
		int64_t copy_size = sz;
		if (this == &other) return *this;
		try {
			const_iterator iter = other.begin();
			while (iter++ != other.end()) {
				push_back(*iter);
				num += 1;
			}
		}
		catch (...) {
			while (num != 0) {
				pop_back();
				num -= 1;
			}
			throw;
		}

		while (copy_size != 0) {
			pop_front();
			copy_size -= 1;
		}

		if (std::allocator_traits<NAllocator>::propagate_on_container_copy_assignment::value)
			alloc = other.alloc;

		return *this;
	}

	Allocator get_allocator() const {
		return alloc;
	}

	size_t size() const {
		return sz;
	}

	void erase_between(Node* prv, Node* nxt) {
		--sz;
		Node* v = prv->next;
		prv->next = nxt;
		nxt->prev = prv;
		std::allocator_traits<NAllocator>::destroy(alloc, v);
		std::allocator_traits<NAllocator>::deallocate(alloc, v, 1);
	}


	void push_back(const T& value) { insert_before_with_value(end(), value); }
	void push_front(const T& value) { insert_before_with_value(begin(), value); }
	void erase(const_iterator id) {
		Node* v = id.getNodePtr();
		erase_between(v->prev, v->next);
	}
	void pop_back() { erase(std::prev(end())); }
	void pop_front() { erase(begin()); }
	void insert(const_iterator pos, const T& value) { insert_before_with_value(pos, value); }

};