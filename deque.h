#include <iostream>
#include <iterator>

template <typename type>
class Deque {
public:

	Deque() try : arr(new type* [2]), siz(0), capacity(2) {
		size_t s = block_capacity * sizeof(type);
		try {
			arr[0] = reinterpret_cast<type*>(new uint64_t[s]);
		}
		catch (...) {
			delete[] arr[0];
			throw;
		}
		try {
			arr[1] = reinterpret_cast<type*>(new uint64_t[s]);
		}
		catch (...) {
			delete[] arr[1];
			delete[] arr[0];
			throw;
		}
		init = block::init(0, block_capacity - 1);
		fin = block::fin(0, block_capacity - 1);

	}
	catch (...) {
		delete[] arr;
		throw;
	}

	size_t size() const {
		return siz;
	};
	Deque(const Deque& old) try :arr(new type* [old.capacity]), siz(old.siz), capacity(old.capacity), init(old.init), fin(old.fin) {

		uint64_t s = 0, f = 0;
		uint64_t finish = fin.block_number;
		uint64_t start = init.block_number, i = start;

		while (i <= finish) {
			try {
				arr[i] = reinterpret_cast<type*>(new uint64_t[block_capacity * sizeof(type)]);
			}
			catch (...) {
				--i;
				while (i >= 0) {
					delete[] arr[i];
					--i;
				}
				throw;
			}
			if (i == start) {
				s = init.curr;
				f = block_capacity;
			}
			else {
				s = 0;
				if (i == finish) f = fin.curr + 1;
			}
			for (uint64_t j = s; j < f; ++j) {
				try {
					new (arr[i] + j) type(old.arr[i][j]);
				}
				catch (...) {
					j--;
					while (j >= s) {
						delete[](arr[i] + j);
						--j;
					}
					while (i >= 0) {
						delete[] arr[i];
						--i;
					}
					throw;
				}
			}
			++i;
		}
	}
	catch (...) {
		delete[] arr;
		throw;
	}


	template<bool Con>
	class Iterat {

	public:
		Iterat(type** iter, uint64_t index) : iterat(iter), ind(index) {};

		Iterat<Con> operator+(const int& n) const {
			type** it = iterat + (n + ind) / block_capacity;
			uint64_t i = (n + ind) % block_capacity;
			return Iterat<Con>(it, i);
		};
		Iterat<Con> operator+=(const int& n) {
			*this = *this + n;
			return *this;
		};
		Iterat& operator++() {
			*this += 1;
			return *this;
		};

		Iterat<Con> operator-(const int& n) const {
			uint64_t new_n = static_cast<uint64_t>(n);

			if (new_n > ind) {
				return Iterat<Con>(iterat - (new_n - ind) / block_capacity - 1, block_capacity - ((new_n - ind) % block_capacity));
			}
			return Iterat<Con>(iterat, ind - new_n);
		};
		Iterat<Con> operator-=(const int& n) {
			*this = *this - n;
			return *this;
		};
		Iterat& operator--() {
			*this -= 1;
			return *this;
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
		friend class Deque;
	};

	Iterat<false> begin() {
		return Iterat<false>(arr + init.block_number, init.curr);
	}

	Iterat<true> begin() const {
		return Iterat<true>(arr + init.block_number, init.curr);
	}

	Iterat<true> cbegin() const {
		return Iterat<true>(arr + init.block_number, init.curr);
	}

	Iterat<false> end() {
		if (fin.last != fin.curr) {
			return Iterat<false>(arr + fin.block_number, fin.curr + 1);
		}
		return Iterat<false>(arr + fin.block_number + 1, fin.first);
	}

	Iterat<true> end() const {
		if (fin.last != fin.curr) {
			return Iterat<true>(arr + fin.block_number, fin.curr + 1);
		}
		return Iterat<true>(arr + fin.block_number + 1, fin.first);
	}

	Iterat<true> cend() const {
		if (fin.last != fin.curr) {
			return Iterat<true>(arr + fin.block_number, fin.curr + 1);
		}
		return Iterat<true>(arr + fin.block_number + 1, fin.first);
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
		block::swap(init, ne.init);
		block::swap(fin, ne.fin);
		std::swap(*arr, *ne.arr);
		std::swap(siz, ne.siz);

		return *this;
	};
	std::pair<uint64_t, uint64_t> get_pos(uint64_t ind) const {
		if (init.curr + ind >= block_capacity) {
			ind -= (block_capacity - init.curr);
			return { init.block_number + 1 + ind / block_capacity, ind % block_capacity };
		}
		return { init.block_number, init.curr + ind };
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
				start_cap = capacity;
			}
			if (is_finish) {
				finish_cap = capacity;
			}
			type** new_arr = new type * [start_cap + capacity + finish_cap];
			
			size_t s = block_capacity * sizeof(type);
			for (size_t i = 0; i < start_cap; ++i) {
				try {
					new_arr[i] = reinterpret_cast<type*>(new uint64_t[s]);
				}
				catch (...) {
					for (size_t p = 0; p < i; ++p) {
						delete[] new_arr[p];
					}
					delete[] new_arr;
					throw;
				}
			}
			for (size_t i = start_cap + capacity; i < start_cap + capacity + finish_cap; ++i) {
				try {
					new_arr[i] = reinterpret_cast<type*>(new uint64_t[s]);
				}
				catch (...) {
					for (size_t p = 0; p < i; ++p) {
						delete[] new_arr[p];
					}
					delete[] new_arr;
					throw;
				}
			}
			for (size_t i = 0; i < capacity; ++i) {
				new_arr[start_cap + i] = arr[i];
			}
			init.block_number += start_cap;
			fin.block_number += start_cap;
			delete[] arr;
			capacity = start_cap + capacity + finish_cap;
			arr = new_arr;
		
	}
	void push_back(type value) {
		if (siz != 0) {
			if (fin.last != fin.curr) {
				fin.curr += 1;
			}
			else {
				if (fin.block_number == capacity - 1) {
					reserve(0, true);
				}
				fin.curr = 0;
				fin.block_number += 1;
			}
		}
		try {
			arr[fin.block_number][fin.curr] = value;
		}
		catch (...) {
			if (siz != 0) {
				if (fin.curr == 0) {
					if (fin.block_number == 2 * capacity) {
						for (size_t i = capacity; i < 2 * capacity; ++i) {
							delete[] arr[i];
						}
						capacity -= capacity;
					}
					fin.curr = fin.last;
					fin.block_number -= 1;
				}
				else {
					fin.curr -= 1;
				}
			}
			throw;
		}
		siz += 1;
	}
	void pop_back() noexcept{

		if (siz == 0) return;
		(arr[fin.block_number] + fin.curr)->~type();
		if (siz != 1) {
			if (fin.curr != fin.first) {
				fin.curr -= 1;
			}
			else {
				fin.block_number -= 1;
				fin.curr = fin.last;
			}
		}
		siz -= 1;
	}

	void push_front(type value) {
		Deque<type> copy = *this;
		if (siz != 0) {
			if (init.last != init.curr) {
				init.curr -= 1;
			}
			else {
				if (init.block_number == 0) {
					reserve(true, 0);
				}
				init.block_number -= 1;
				init.curr = block_capacity - 1;
			}
		}
		try {
			arr[init.block_number][init.curr] = value;
		}
		catch (...) {
			if (siz != 0) {
				if (init.curr != block_capacity - 1) {
					init.curr += 1;
				}
				else {
					if (init.block_number == capacity - 1) {
						for (size_t i = 0; i < capacity; ++i) {
							delete[] arr[i];
						}
						capacity -= capacity;
						init.block_number = 0;
					}
					init.curr = init.first;
					init.block_number += 1;
				}
			}
			throw;
		}
		siz += 1;
	}
	void erase(Iterat<false> it) {
		Deque<type> copy = *this;
		try {
			for (auto i = it; i < copy.end() - 1; ++i) {
				type tmp = *(i + 1);
				*(i + 1) = *i;
				*i = tmp;
			}
			copy.pop_back();
			*this = copy;
		}
		catch (...) {
			copy.~Deque();
			throw;
		}
	}
	void pop_front() noexcept {
		if (siz == 0) return;

		(arr[init.block_number] + init.curr)->~type();
		if (siz != 1) {
			if (init.curr != init.first) {
				init.curr += 1;
			}
			else {
				init.block_number += 1;
				init.curr = init.last;
			}
		}
		siz -= 1;
	}
	void insert(Iterat<false> it, const type& val) {
		Deque<type> copy = *this;
		try {
			copy.push_back(val);
			for (auto i = --copy.end(); i > it; --i) {
				type tmp = *(i - 1);
				*(i - 1) = *i;
				*i = tmp;
			}
			*this = copy;
		}
		catch (...) {
			copy.~Deque();
			throw;
		}
	}
	~Deque() {
		for (uint64_t i = init.block_number; i <= fin.block_number; ++i) {
			for (uint64_t j = ((i == init.block_number) ? init.curr : 0);
				j < ((i == fin.block_number) ? fin.curr + 1 : block_capacity); ++j) {
				(arr[i] + j)->~type();
			}
		}
		for (size_t i = 0; i < capacity; ++i) {
			delete[] arr[i];
		}
		delete[] arr;
	}

	explicit Deque(int n, const type& value = type()) : Deque() {
		for (int i = 0; i < n; ++i) {
			(*this).push_front(value);
		}
	};


private:
	type** arr;
	size_t siz;
	size_t capacity;
	const static int block_capacity = 32;
	struct block {
		uint64_t block_number;
		uint64_t curr;
		uint64_t first;
		uint64_t last;
		static void swap(block& first, block& second) {
			std::swap(first.last, second.last);
			std::swap(first.curr, second.curr);
			std::swap(first.block_number, second.block_number);
			std::swap(first.first, second.first);
		}
		static block init(uint64_t n, uint64_t c) {
			return block(n, c, block_capacity - 1, 0);
		}
		static block fin(uint64_t n, uint64_t c) {
			return block(n, c, 0, block_capacity - 1);
		}
		block(uint64_t n, uint64_t c, uint64_t f, uint64_t l) : block_number(n), curr(c), first(f), last(l) {}
		block(const block& old) : block_number(old.block_number), curr(old.curr), first(old.first), last(old.last) {};
		~block() = default;
		block() = default;

	};

	block init;
	block fin;
};
