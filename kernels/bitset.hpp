/// @file bitset.hpp
/// @brief Sets of integers represented as bits in an array.

#ifndef KERNELS_BITSET_HPP
#define KERNELS_BITSET_HPP

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <unordered_set>

const size_t BITS_PER_WORD = 64; ///< Number of bits in each uint64_t word of the bitset.

/// @brief A set of integers represented as bits in an array (1 bit per integer).
/// If the i-th bit is 1, then i is in the set, otherwise it is not.
typedef struct bitset {
	uint64_t* data = nullptr; ///< Array of 64-bit words storing the bits.
							  ///< The i-th bit of the set is located at data[i/64] & (1 << (i%64)).
	size_t words = 0;		  ///< Number of 64-bit words allocated in data.
	uint64_t nb_bits = 0;	  ///< Number of bits in the bitset.

	/// @brief Construct a bitset with a given number of bits, all initialized to 0 (empty set).
	/// @param nb_bits The number of bits in the bitset.
	bitset(uint64_t nb_bits) {
		this->nb_bits = nb_bits;
		this->words = (nb_bits + BITS_PER_WORD - 1) / BITS_PER_WORD;
		this->data = (uint64_t*)calloc(this->words, sizeof(uint64_t));
	}

	/// @brief Transform a set of integers into a bitset. The integers must be in the range [0, nb_bits).
	/// @param elements The set of integers to be represented as a bitset.
	/// @param nb_bits The number of bits in the bitset (must be greater than the max element in the set).
	bitset(const std::unordered_set<int64_t>& elements, uint64_t nb_bits) : bitset(nb_bits) {
		for (int64_t elem : elements) {
			this->insert(elem);
		}
	}

	/// @brief Free the memory allocated for the bitset.
	~bitset() {
		if (this->data != nullptr) {
			free(this->data);
		}
	}

	/// @brief Insert an integer into the set.
	void insert(int64_t idx) {
		this->data[idx / BITS_PER_WORD] |= (1ULL << (idx & (BITS_PER_WORD - 1)));
	}

	/// @brief Remove an integer from the set.
	void remove(int64_t idx) {
		this->data[idx / BITS_PER_WORD] &= ~(1ULL << (idx & (BITS_PER_WORD - 1)));
	}

	bool contains(int64_t idx) const {
		return ((this->data[idx / BITS_PER_WORD] >> (idx & (BITS_PER_WORD - 1))) & 1ULL) != 0;
	}

	/// @brief Clear the set, i.e., remove all elements from it.
	void clear() {
		if (this->data != nullptr) {
			memset(this->data, 0, this->words * sizeof(uint64_t));
		}
	}

	/// @brief Check if the set is empty.
	bool empty() const {
		for (size_t i = 0; i < this->words; ++i) {
			if (this->data[i] != 0U) {
				return false;
			}
		}
		return true;
	}

	/// @brief Iterate over set bits in index order. Function `func` is called with the index of each set bit.
	/// @param func A callable that takes one int64_t parameter (the element).
	template <typename Func> void for_each(Func&& func) const {
		for (size_t w = 0; w < this->words; w++) {
			uint64_t word = this->data[w];
			// Iterate over all set bits in `word`
			while (word) {
				// Count the number of trailing zeros to find the index of the least significant set bit
				const int trailing_zeros = __builtin_ctzll(word);

				// Call the provided function with the index of the set bit
				int64_t idx = (int64_t)(static_cast<int64_t>(w) * BITS_PER_WORD) + trailing_zeros;
				if (idx >= this->nb_bits) {
					break;
				}
				func(idx);

				// Clear the least significant set bit.
				word &= word - 1;
			}
		}
	}
} bitset;

/// @brief Swap the contents of two bitsets.
/// This is a fast operation that only swaps pointers and metadata, without copying the bit data.
/// I made this to avoid dealing with c++ move/copy constructors stuff (which is insufferable)
/// @param a First bitset to swap.
/// @param b Second bitset to swap.
void bitset_swap(bitset& a, bitset& b) {
	std::swap(a.data, b.data);
	std::swap(a.words, b.words);
	std::swap(a.nb_bits, b.nb_bits);
}

/// @brief A thread-safe version of bitset, using atomic operations.
typedef struct atomic_bitset {
	std::atomic<uint64_t>* data; ///< Array of atomic 64-bit words storing the bits.
								 ///< The i-th bit of the set is located at data[i/64] & (1 << (i%64)).
	size_t words;				 ///< Number of 64-bit words allocated in data.
	uint64_t nb_bits;			 ///< Number of bits in the bitset.

	/// @brief Construct an empty atomic_bitset with a given number of bits.
	/// @param nb_bits The number of bits in the atomic_bitset.
	atomic_bitset(uint64_t nb_bits) : data(nullptr), words(0), nb_bits(nb_bits) {
		words = (nb_bits + BITS_PER_WORD - 1) / BITS_PER_WORD;
		data = new std::atomic<uint64_t>[words];
		memset(data, 0, words * sizeof(std::atomic<uint64_t>));
	}

	/// @brief Construct an atomic_bitset from a set of integers. The integers must be in the range [0, nb_bits).
	/// @param indices The set of integers to be represented as an atomic_bitset.
	/// @param nb_bits The number of bits in the atomic_bitset (must be greater than the max element in the set).
	atomic_bitset(const std::unordered_set<int64_t>& indices, uint64_t nb_bits) : atomic_bitset(nb_bits) {
		for (int64_t idx : indices) {
			insert(idx);
		}
	}

	/// @brief Free the memory allocated for the atomic_bitset.
	~atomic_bitset() {
		delete[] data;
		data = nullptr;
	}

	/// @brief Insert an integer into the set.
	void insert(int64_t elem) {
		assert(0 <= elem && elem < nb_bits);
		const size_t word_idx = elem / BITS_PER_WORD;
		const uint64_t mask = (1ULL << (elem & (BITS_PER_WORD - 1)));
		data[word_idx].fetch_or(mask, std::memory_order_acq_rel);
	}

	/// @brief Remove an integer from the set.
	void remove(int64_t elem) {
		assert(0 <= elem && elem < nb_bits);
		const size_t word_idx = elem / BITS_PER_WORD;
		const uint64_t mask = ~(1ULL << (elem & (BITS_PER_WORD - 1)));
		data[word_idx].fetch_and(mask, std::memory_order_acq_rel);
	}

	/// @brief Check if an integer is in the set.
	/// @param elem The integer to check for membership in the set.
	/// @return true if elem is in the set, false otherwise.
	bool contains(int64_t elem) const {
		assert(0 <= elem && elem < nb_bits);
		const size_t word_idx = elem / BITS_PER_WORD;
		const uint64_t mask = (1ULL << (elem & (BITS_PER_WORD - 1)));
		return (data[word_idx].load(std::memory_order_acquire) & mask) != 0ULL;
	}

	/// @brief Clear the set, i.e., remove all elements from it. (Not thread-safe)
	void clear() {
		memset(data, 0, words * sizeof(std::atomic<uint64_t>));
	}

	/// @brief Check if the set is empty.
	bool empty() const {
		for (size_t i = 0; i < words; ++i) {
			if (data[i].load(std::memory_order_acquire) != 0ULL) {
				return false;
			}
		}
		return true;
	}

	/// @brief Iterate over set bits in index order. Function `func` is called with the index of each set bit.
	/// @param func A callable that takes one int64_t parameter (the element).
	template <typename Func> void for_each(Func&& func) const {
		for (size_t w = 0; w < words; w++) {
			uint64_t word = data[w].load(std::memory_order_acquire);
			// Iterate over all set bits in `word`
			while (word) {
				// Count the number of trailing zeros to find the index of the least significant set bit
				const int trailing_zeros = __builtin_ctzll(word);

				// Call the provided function with the index of the set bit
				int64_t idx = (int64_t)(static_cast<int64_t>(w) * BITS_PER_WORD) + trailing_zeros;
				if (idx >= nb_bits) {
					break;
				}
				func(idx);

				// Clear the least significant set bit.
				word &= word - 1;
			}
		}
	}

	/// @brief Iterate over set bits in parallel. Function `func` is called with the index of each set bit.
	/// @param func A callable that takes one int64_t parameter (the element).
	template <typename Func> void parallel_for_each(Func&& func) const {
#pragma omp parallel for schedule(guided)
		for (size_t w = 0; w < words; w++) {
			uint64_t word = data[w].load(std::memory_order_acquire);
			// Iterate over all set bits in `word`
			while (word) {
				// Count the number of trailing zeros to find the index of the least significant set bit
				const int trailing_zeros = __builtin_ctzll(word);

				// Call the provided function with the index of the set bit
				int64_t idx = (int64_t)(static_cast<int64_t>(w) * BITS_PER_WORD) + trailing_zeros;
				if (idx >= nb_bits) {
					break;
				}
				func(idx);

				// Clear the least significant set bit.
				word &= word - 1;
			}
		}
	}
} atomic_bitset;

/// @brief Swap the contents of two atomic_bitsets.
/// This is a fast operation that only swaps pointers and metadata, without copying the bit data.
/// @param a First atomic_bitset to swap.
/// @param b Second atomic_bitset to swap.
void atomic_bitset_swap(atomic_bitset& a, atomic_bitset& b) {
	std::swap(a.data, b.data);
	std::swap(a.words, b.words);
	std::swap(a.nb_bits, b.nb_bits);
}

#endif // KERNELS_BITSET_HPP
