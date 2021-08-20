#pragma once

#include <vector>
#include <string>
#include <iostream>

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

	std::vector<uint8_t> getCombatFieldTemplate(const Constants::CombatFieldTemplate _field_template);
	ArmyPermutation createBaseArmyPermutation(std::vector<UnitStack>& _army);

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

	//template<typename T>
	//struct VectorList {
	//	std::vector<T> data;
	//	std::vector<uint8_t> nexts;
	//	std::vector<uint8_t> prevs;
	//	uint8_t head{ 0xFF };
	//	uint8_t tail{ 0xFF };

	//	VectorList() {
	//		data.reserve(32);
	//		nexts.reserve(32);
	//		prevs.reserve(32);
	//	}

	//	void push_first(const T& _val) {
	//		head = tail = 0;
	//		data[0] = _val;
	//		nexts[0] = 0xFF;
	//		prevs[0] = 0xFF;
	//	}

	//	void push_back(const T& _val) {
	//		if (tail = 0xFF) {
	//			push_first(_val);
	//			return;
	//		}

	//		nexts[tail] = tail + 1;
	//		prevs[tail + 1] = tail;
	//		data[++tail] = _val;
	//	}

	//	void remove_first() {
	//		head = nexts[0];

	//		if( head )
	//	}

	//	void remove(const T& _val) {
	//		if (auto it = std::find(std::begin(data), std::end(data), _val); it != std::end(data)) {
	//			if (it == std::begin(data))
	//				remove_first();
	//		}
	//	}

	//	std::vector<T> get_list() {
	//		return data;
	//	}
	//};
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
