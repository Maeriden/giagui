#ifndef GIAGUI_CONTAINERS_HPP
#define GIAGUI_CONTAINERS_HPP


#include <unordered_set>
#include <unordered_map>


template<typename V>
using HashSet = std::unordered_set<V>;


template<typename K, typename V>
struct HashMap : public std::unordered_map<K, V>
{
	inline V* get(const K& key)
	{
		auto it = this->find(key);
		if(it != this->end())
			return &it->second;
		return nullptr;
	}
	
	inline const V* get(const K& key) const
	{
		auto it = this->find(key);
		if(it != this->end())
			return &it->second;
		return nullptr;
	}
	
	inline const V& get_or(const K& key, const V& fallback) const
	{
		auto it = this->find(key);
		if(it != this->end())
			return it->second;
		return fallback;
	}
};

#endif //GIAGUI_CONTAINERS_HPP
