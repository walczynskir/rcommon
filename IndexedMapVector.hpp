#pragma once
#include <vector>
#include <unordered_map>
#include <stdexcept>

// template for indexed map + vector
template<typename Key, typename Value>
class IndexedMapVector {
public:
	using Pair = std::pair<Key, Value>;

	// Add new element (no duplicates allowed)
	void add(const Key& key, const Value& value) {
		if (m_indexByKey.find(key) != m_indexByKey.end()) {
			throw std::runtime_error("Key already exists");
		}
		m_indexByKey[key] = m_data.size();
		m_data.emplace_back(key, value);
	}

	// Access by index
	const Pair& atIndex(size_t index) const {
		return m_data.at(index);
	}

	// Access by key (throws if not found)
	Value& atKey(const Key& key) {
		return m_data.at(m_indexByKey.at(key)).second;
	}

	const Value& atKey(const Key& key) const {
		return m_data.at(m_indexByKey.at(key)).second;
	}

	// Try to find (returns nullptr if not found)
	Value* find(const Key& key) {
		auto it = m_indexByKey.find(key);
		if (it == m_indexByKey.end()) return nullptr;
		return &m_data[it->second].second;
	}

	// Find index by key, returns -1 if not found
	int indexOf(const Key& key) const {
		auto it = m_indexByKey.find(key);
		if (it != m_indexByKey.end()) {
			return static_cast<int>(it->second);
		}
		return -1; // not found
	}

	// check if Key exists
	bool contains(const Key& key) const {
		return m_indexByKey.find(key) != m_indexByKey.end();
	}

	// Number of stored elements
	size_t size() const { return m_data.size(); }
	bool empty() const { return size() == 0; }


	// Iteration support (like vector)
	auto begin() { return m_data.begin(); }
	auto end() { return m_data.end(); }
	auto begin() const { return m_data.begin(); }
	auto end() const { return m_data.end(); }
	// Number of stored elements

	// Remove all elements
	void clear() {
		m_data.clear();
		m_indexByKey.clear();
	}

	// Access by index (no bounds checking, like std::vector::operator[])
	Pair& operator[](size_t index) {
		return m_data[index];
	}

	const Pair& operator[](size_t index) const {
		return m_data[index];
	}
private:
	std::vector<Pair> m_data;
	std::unordered_map<Key, size_t> m_indexByKey;
};
