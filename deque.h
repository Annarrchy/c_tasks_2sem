#include <iostream>
#include <iterator>

template <typename type>
class Deque {
public:

	Deque() : arr(new type * [2]), siz(0), capacity(2) {
		size_t s = block_capacity * sizeof(type);
		arr[0] = reinterpret_cast<type*>(new uint64_t[s]);
		arr[1] = reinterpret_cast<type*>(new uint64_t[s]);
		init = block::init(0, block_capacity - 1);
		fin = block::fin(0, block_capacity - 1);

	};

	size_t size() const {
		return siz;
	};
	Deque(const Deque& old) :arr(new type* [old.capacity]), siz(old.siz), capacity(old.capacity) {
		uint64_t s = 0, f = 0;
		fin = new block(*old.fin);
		uint64_t finish = fin->amount;
		init = new block(*old.init);
		uint64_t start = init->amount, i = start;
		while (i <= finish) {
			arr[i] = reinterpret_cast<type*>(new uint64_t[block_capacity * sizeof(type)]);
			if (i == start) {
				s = init->curr;
				f = block_capacity;
			}
			else {
				s = 0;
				if (i == finish) f = fin->curr + 1;
			}
			for (uint64_t j = s; j < f; ++j) {
				new (arr[i] + j) type(old.arr[i][j]);
			}
			++i;
		}
	};

	template<bool Con>
	class Iterat {

	public:
		Iterat(type** iter, uint64_t index) : iterat(iter), ind(index) {};
		Iterat& operator++() {
			if (ind + 1 == block_capacity) {
				++iterat;
				ind = 0;
			}
			else {
				++ind;
			}
			return *this;
		};

		Iterat& operator--() {
			if (ind) --ind;
			else {
				--iterat;
				ind = block_capacity - 1;
			}
			return *this;
		};

		Iterat<Con> operator+(const int& n) const {
			type** it = iterat + (n + ind) / block_capacity;
			uint64_t i = (n + ind) % block_capacity;
			return Iterat<Con>(it, i);
		};

		Iterat<Con> operator-(const int& n) const {
			uint64_t new_n = static_cast<uint64_t>(n);
			
			if (new_n > ind) {
				return Iterat<Con>(iterat - (new_n - ind) / block_capacity - 1, block_capacity - ((new_n - ind) % block_capacity));
			}
			return Iterat<Con>(iterat, ind - new_n);
		};
		bool operator==(const Iterat<Con>& second) const {
			return (iterat == second.iterat && ind == second.ind);
		};
		std::conditional_t<Con, const type&, type&> operator*() const {
			return *(*iterat + ind);
		};

		uint64_t operator-(const Iterat<Con>& second) const {
			return ind - second.ind + static_cast<uint64_t>((iterat - second.iterat)) * block_capacity;
		};
		bool operator!=(const Iterat<Con>& second) const {
			return !(*this == second);
		};
		bool operator<(const Iterat<Con>& second) const {
			return ((iterat == second.iterat && ind < second.ind) || (iterat < second.iterat));
		};
		bool operator<=(const Iterat<Con>& second) const {
			return (*this < second || *this == second);
		};
		std::conditional_t<Con, const type*, type*> operator->() {
			return *iterat + ind;
		};
		bool operator>=(const Iterat<Con>& second) const {
			return !(*this < second);
		};
		bool operator>(const Iterat<Con>& second) const {
			return !(*this <= second);
		};

	private:
		type** iterat;
		uint64_t ind;
	};

	Iterat<false> begin() {
		return Iterat<false>(arr + init->amount, init->curr);
	}

	Iterat<true> begin() const {
		return Iterat<true>(arr + init->amount, init->curr);
	}

	Iterat<true> cbegin() const {
		return Iterat<true>(arr + init->amount, init->curr);
	}

	Iterat<false> end() {
		if (fin->last != fin->curr) {
			return Iterat<false>(arr + fin->amount, fin->curr + 1);
		}
		return Iterat<false>(arr + fin->amount + 1, fin->first);
	}

	Iterat<true> end() const {
		if (fin->last != fin->curr) {
			return Iterat<true>(arr + fin->amount, fin->curr + 1);
		}
		return Iterat<true>(arr + fin->amount + 1, fin->first);
	}

	Iterat<true> cend() const {
		if (fin->last != fin->curr) {
			return Iterat<true>(arr + fin->amount, fin->curr + 1);
		}
		return Iterat<true>(arr + fin->amount + 1, fin->first);
	}

	std::reverse_iterator<Iterat<false>> rbegin() {
		return std::reverse_iterator(begin());
	}

	std::reverse_iterator<Iterat<true>> rbegin() const {
		return std::reverse_iterator(begin());
	}

	std::reverse_iterator<Iterat<true>> crbegin() const {
		return std::reverse_iterator(begin());
	}

	std::reverse_iterator<Iterat<false>> rend() {
		return std::reverse_iterator(end());
	}

	std::reverse_iterator<Iterat<true>> rend() const {
		return std::reverse_iterator(end());
	}

	std::reverse_iterator<Iterat<true>> crend() const {
		return std::reverse_iterator(end());
	}
	using iterator = Iterat<false>;
	using const_iterator = Iterat<true>;
	Deque& operator=(const Deque& old) {

		Deque<type> ne = old;
		std::swap(capacity, ne.capacity);
		block::swap(*init, *ne.init);
		block::swap(*fin, *ne.fin);
		std::swap(*arr, *ne.arr);
		std::swap(siz, ne.siz);

		return *this;
	};
	std::pair<uint64_t, uint64_t> get_pos(uint64_t ind) const {
		if (init->curr + ind >= block_capacity) {
			ind -= (block_capacity - init->curr);
			return { init->amount + 1 + ind / block_capacity, ind % block_capacity };
		}
		return { init->amount, init->curr + ind };
	};
	type& operator[](uint64_t ind) {
		std::pair<uint64_t, uint64_t> pos = get_pos(ind);
		return arr[pos.first][pos.second];
	};
	const type& operator[](uint64_t ind) const {
		std::pair<uint64_t, uint64_t> pos = get_pos(ind);
		return arr[pos.first][pos.second];
	};
	type& at(uint64_t ind) {
		if (ind >= siz) {
			throw std::out_of_range("at");
		}
		return (*this)[ind];
	};
	const type& at(uint64_t ind) const {
		if (ind >= siz) {
			throw std::out_of_range("at");
		}
		return (*this)[ind];
	};
	void reserve(bool is_start, bool is_finish) {
		size_t start_cap = 0, finish_cap = 0;
		if (is_start) {
			start_cap = (siz + block_capacity - 1) / block_capacity;
		}
		if (is_finish) {
			finish_cap = (siz + block_capacity - 1) / block_capacity;
		}
		type** new_arr = new type * [start_cap + capacity + finish_cap];
		size_t s = block_capacity * sizeof(type);
		for (size_t i = 0; i < start_cap; ++i) {
			new_arr[i] = reinterpret_cast<type*>(new uint64_t[s]);
		}
		for (size_t i = start_cap + capacity; i < start_cap + capacity + finish_cap; ++i) {
			new_arr[i] = reinterpret_cast<type*>(new uint64_t[s]);
		}
		for (size_t i = 0; i < capacity; ++i) {
			new_arr[start_cap + i] = arr[i];
		}
		init->amount += start_cap;
		fin->amount += start_cap;
		delete arr;
		capacity = start_cap + capacity + finish_cap;
		arr = new_arr;
	}
	void push_back(type value) {
		if (siz != 0) {
			if (fin->last != fin->curr) {
				fin->curr += 1;
			}
			else {
				if (fin->amount == capacity - 1) {
					reserve(0, true);
				}
				fin->curr = 0;
				fin->amount += 1;
			}
		}
		new(arr[fin->amount] + fin->curr) type(value);
		siz += 1;
	}
	void pop_back() {
		if (siz == 0) return;
		(arr[fin->amount] + fin->curr)->~type();
		if (siz != 1) {
			if (fin->curr != fin->first) {
				fin->curr -= 1;
			}
			else {
				fin->amount -= 1;
				fin->curr = fin->last;
			}
		}
		siz -= 1;
	}

	void push_front(type value) {
		if (siz != 0) {
			if (init->last != init->curr) {
				init->curr -= 1;
			}
			else {
				if (init->amount == 0) {
					reserve(true, 0);
				}
				init->amount -= 1;
				init->curr = block_capacity - 1;
			}
		}
		new(arr[init->amount] + init->curr) type(value);
		siz += 1;
	}
	void erase(Iterat<false> it) {
		for (auto i = it; i < end() - 1; ++i) {
			type tmp = *(i + 1);
			*(i + 1) = *i;
			*i = tmp;
		}
		pop_back();
	}
	void pop_front() {
		if (siz == 0) return;

		(arr[init->amount] + init->curr)->~type();
		if (siz != 1) {
			if (init->curr != init->first) {
				init->curr += 1;
			}
			else {
				init->amount += 1;
				init->curr = init->last;
			}
		}
		siz -= 1;
	}
	void insert(Iterat<false> it, const type& val) {
		push_back(val);
		for (auto i = --end(); i > it; --i) {
			type tmp = *(i - 1);
			*(i - 1) = *i;
			*i = tmp;
		}
	}

	explicit Deque(int n, const type& value = type()) : siz(static_cast<size_t>(n)) {
		uint64_t ne = static_cast<uint64_t>(n);
		size_t s = block_capacity * sizeof(type);
		if (ne > block_capacity) {
			init = block::init(0, 0);
			capacity = (static_cast<size_t>(ne) + block_capacity - 1) / block_capacity;
			fin = block::fin(capacity - 1, (ne - 1) % block_capacity);
		}
		else {
			init = block::init(0, block_capacity - ne);
			fin = block::fin(0, block_capacity - 1);
			capacity = 2;
		}
		arr = new type * [capacity];
		for (size_t i = 0; i < capacity; ++i) {
			arr[i] = reinterpret_cast<type*>(new uint8_t[s]);
		}
		for (uint64_t i = init->amount; i <= fin->amount; ++i) {
			uint64_t j = 0, e = 0;
			if (i == init->amount) {
				j = init->curr, e = block_capacity;
			}
			else if (i == fin->amount) {
				e = fin->curr + 1;
			}
			else {
				e = block_capacity;
			}
			for (uint64_t p = j; p < e; ++p) {
				new (arr[i] + p) type(value);
			}
		}
	};


private:
	type** arr;
	size_t siz;
	size_t capacity;
	const static int block_capacity = 32;
	struct block {
		uint64_t amount;
		uint64_t curr;
		uint64_t first;
		uint64_t last;
		static void swap(block& first, block& second) {
			std::swap(first.last, second.last);
			std::swap(first.curr, second.curr);
			std::swap(first.amount, second.amount);
			std::swap(first.first, second.first);
		}
		static block* init(uint64_t n, uint64_t c) {
			return new block(n, c, block_capacity - 1, 0);
		}
		static block* fin(uint64_t n, uint64_t c) {
			return new block(n, c, 0, block_capacity - 1);
		}
		block(uint64_t n, uint64_t c, uint64_t f, uint64_t l) : amount(n), curr(c), first(f), last(l) {}
		block(const block& old) : amount(old.amount), curr(old.curr), first(old.first), last(old.last) {};


	};

	block* init;
	block* fin;
};