#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

#include "structures.h"

#include "combat_field.h"


namespace HotaMechanics {
	class CombatUnit;
};

namespace HotaMechanics::Utils {

	BaseStats baseStats(int _atk, int _def, int _pow, int _kgd);
	CombatStats combatStats(int _min, int _max, int _spd, int _shots);
	PrimaryStats primaryStats(int _hp, int _mana);

	int getRandomInt(int _min, int _max);
	float getRandomFloat(float _min, float _max);

	std::vector<int> getCombatFieldTemplate(const Constants::CombatFieldTemplate _field_template);


	template<typename Printable>
	std::string to_string(const Printable& _printable) {
		//if (std::is_fundamental_v<Printable>)
		//	return std::to_string((std::remove_reference_t<Printable>)_printable);
		//else
			return _printable.toString();
	}

	constexpr size_t ceil(const double _val) {
		return (size_t)_val;
	}

	template<typename T, size_t Size>
	struct FixedVector {
		
		std::array<T, Size> data;

		FixedVector() {
			data.fill(0);
		}
		FixedVector(std::initializer_list<int16_t> _il) {
			data.fill(0);
			for (auto val : _il)
				pushBack(val);
		}

		bool operator==(const FixedVector& _rhs) const {
			return getSize() == _rhs.getSize() && equalValues(_rhs);
		}

		template<class Array>
		inline void from(const Array& _arr) {
			std::copy(std::begin(_arr), std::begin(_arr) + _arr.getSize(), std::begin(data));
			data[Size - 1] = _arr.getSize();
		}
		template<>
		inline void from<std::array<int16_t, 6>>(const std::array<int16_t, 6>& _arr) {
			std::copy(std::begin(_arr), std::begin(_arr) + _arr.size(), std::begin(data));
			data[Size - 1] = _arr.size();
		}
		template<>
		inline void from<std::array<int16_t, 188>>(const std::array<int16_t, 188>& _arr) {
			std::copy(std::begin(_arr), std::begin(_arr) + _arr.size(), std::begin(data));
			data[Size - 1] = _arr.size();
		}
		template<>
		inline void from<FixedVector<T, Size>>(const FixedVector<T, Size>& _arr) {
			std::copy(std::begin(_arr.data), std::begin(_arr.data) + _arr.getSize(), std::begin(data));
			data[Size - 1] = _arr.getSize();
		}
		inline const bool empty() const { return data[Size - 1] == 0; }
		inline void pushBack(const T& _val, const bool _real_push = true) { data[data[Size - 1]] = _val;  data[Size - 1] += _real_push; }
		inline T& popBack() { return data[--data[Size - 1]]; }
		inline void clear() { data[Size - 1] = 0; }
		inline T& operator[](int _idx) { return data[_idx]; }

		inline void sort(const bool _reverse = false) {
			std::sort(std::begin(data), std::begin(data) + getSize(),
				[_reverse](auto v, auto v2) { return (!_reverse * v) - (_reverse * v) < (!_reverse * v2) - (_reverse * v2); });
		}
		inline void setSize(const int16_t _size) { data[Size - 1] = _size; }

		inline const bool equalValues(const FixedVector& _rhs) const {
			for (int i = 0; i < getSize(); ++i)
				if (_rhs.data[i] != data[i])
					return false;
			return true;
		}
		inline const int16_t getSize() const { return data[Size - 1]; }
	};

	using FieldArray = FixedVector<int16_t, Constants::FIELD_SIZE + 1>;

	//template<int16_t hex, typename... Args>
	//static const std::array<CombatHex, 32> make_hexarray(const CombatHex& t, const Args&... _args) {
	//	if constexpr (sizeof...(_args) == 31)
	//		return std::array<CombatHex, 32>{t, _args...};

	//	return make_hexarray<hex - 1>(CombatHex(hex - 1), t, _args...);
	//}

	// make_hexarray<int, Types...> ->
	//		if make_hexarray<0, Types...> return array
	//		else if make_hexarray<1...187, Types...> return make_array<1...187, Types...>(types...)


	// make_hexarray<188>() -> make_hexarray<>(T&& hex)

	//template<typename... Args>
	//const HexArray make_hexarray(Args... _args) {
	//	return make_hexarray<CombatHex>(_args...);
	//}


	//template<int16_t Size, class Type = CombatHex, typename... Args>
	//const HexArray make_hexarray(Type _t, Args... _args) {
	//	if constexpr (Size == 0) {
	//		return HexArray{ _t, _args... };
	//	}

	//	return make_hexarray<Size - 1, Type>(CombatHex(Size - 1), _t, _args...);
	//}

	//// make_hexarray<32>()
	//template<int16_t hex = Constants::FIELD_SIZE + 1>
	//const HexArray make_hexarray() {
	//	return make_hexarray<hex - 1>(CombatHex(hex - 1));
	//}


	//template<typename T, std::size_t N = Constants::FIELD_SIZE + 1, std::size_t index_t = N, typename... Ts>
	//HexArray make_hexarray(T t, Ts... ts)
	//{
	//	if constexpr (index_t <= 1) {
	//		return HexArray {t, ts...};
	//	}
	//	else {
	//		return make_hexarray<T, N, index_t - 1>(CombatHex(index_t - 1), t, ts...);
	//	}
	//}

	//template<std::size_t index_t = Constants::FIELD_SIZE + 1>
	//HexArray make_hexarray()
	//{
	//	return make_hexarray(CombatHex(index_t - 1));
	//}

	//template<class T, std::size_t N, std::size_t hex = N, typename ...Args>
	//static const std::array<T, N> make_hexarray(const T& t, const Args&... args) {
	//	std::cerr << hex << std::endl;

	//	if constexpr( hex <= 1 )
	//		return std::array<T, N>{ t, args... };

	//	return make_hexarray<T, N, hex - 1>(t, args...);
	//};

 //template<typename T, std::size_t N = Constants::FIELD_SIZE + 1, std::size_t index_t = N, typename... Ts>
 //HexArray make_hexarray(T t, Ts... ts)
 //{
	// if constexpr (index_t <= 1) {
	//	 return HexArray{ t, ts... };
	// }
	// else {
	//	 return make_hexarray<T, N, index_t - 1>(CombatHex(index_t - 1), t, ts...);
	// }
 //}

 //template<std::size_t index_t = Constants::FIELD_SIZE + 1>
 //HexArray make_hexarray()
 //{
	// return make_hexarray(CombatHex(index_t - 1));
 //}


}; // HotaMechanics::Utils
