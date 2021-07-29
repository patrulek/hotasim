#pragma once


#include <cmath>
#include <memory>

#include "structures.h"

#include "combat_manager.h"
#include "combat_unit.h"
#include "combat_state.h"
#include "combat_pathfinder.h"

class CombatManager;

enum class AIDifficulty {
	EASY /*80%, 100%*/, NORMAL /*130%, 160%, 200%*/
};

class CombatAI {
public:
	int playerUnitsSize;
	CombatUnit playerRelevantUnits[21];
	float playerUnitsFightValues[21];
	
	int aiUnitsSize;
	CombatUnit aiRelevantUnits[21];
	float aiUnitsFightValues[21];

	CombatAction possibleActions[512];
	CombatAction action;
	AIDifficulty difficulty;

	const CombatManager& combat_manager;
	std::unique_ptr<CombatPathfinder> pathfinder;

	CombatAI(const CombatManager& combat_manager);


	void generatePossibleActions(CombatAI& ai);

	void evaluateAction(CombatAI& ai, CombatAction action, CombatState& state);

	// E4580
	float multiplierModifier(CombatUnit& activeStack, int side);

	// 42770
	int calculateSingleUnitValue(CombatUnit& activeStack, int side);

	// 42B80
	int calculateUnitValue(CombatUnit& activeStack, int side);

	// 1EC40 (combat_mgr->get_current_active_side(), activeStack, diffDef, diffAtk, 0 or 1)
	int calculateSummedUnitValueForSide(CombatState& state, int side, int minMeleeDiffAtk, int minDiffDef, int something);

	// 1F1E0 (activeStack, 0, combat_mgr->get_current_active_side())
	void sub1F1E0();

	// 22130 ()
	void calledWhenTacticsState(CombatUnit& unit);



	// 35B10 (current_active_side, activeStack)
	void calculateUnitsFightValues(CombatAI& ai, CombatState& state);



	// 22100 (activeStack, 0, 0, combat_mgr->get_current_active_side())
	void generateRelevantUnitsLists(CombatAI& ai, CombatState& state);

	// part of 22370
	int unitPreconditions(CombatUnit& unit);

	// 22370 - pick ai action	
	void pickAction(CombatAI& ai, CombatState& state);


	float calculateUnitAttackFightValueModifier(const CombatUnit& unit) const;
	float calculateUnitDefenceFightValueModifier(const CombatUnit& unit) const;
	float calculateUnitFightValueModifier(const CombatUnit& unit) const;

	int calculateStackUnitFightValue(const CombatUnit& unit) const;
	int calculateHeroFightValue(const CombatHero& hero) const;
};


/*
AI FLOW:

22370 - start; check if shooter/shooter-siege or flyer
	77740 - something with blue highlight
	42160 - ???
		C8DE0 - get unit number shots and check if arrow tower
	hota.dll + C86D0 - check if arrow tower, ballista or 150 (cannon? undefined?)
	22100 - calculate relevant lists
		35B10 - calculate units fight values for both sides
			42610 - sprawdzamy czy arrowtower i coœ tam
			42270 - diff btw cur_atk melee and base_atk melee
				42130 - get cur_atk melee
			42380 - diff btw cur_def and base_def
				422B0 - get cur_def
			--- do tej pory mamy wyliczon¹ NAJMNIEJSZ¥ ró¿nicê melee_atk i melee_def ---
			1EC40 - calculateSummedUnitValueForSide
				42B80 - calculate unit-stack fight value
					[IF SHOOTER (marksman)] 
					D9460 -
					691E0 - 
						E7230 - 
						425A0 -
					[END IF]
					42770 - get unit fight value multiplier (ta funkcja wylicza nowe fight value dla jednostki na bazie aktywnych spelli i statystyk)
						42130 - get melee diff atk
						422B0 - get diff def
					--- tutaj mamy chyba porównanie def z min def ---
						E4580 - some magic
						[IF SHOOTER (marskman)]
						3FE60 - 
						D9460 - 
						42710 - 
							691E0 - 
						[END IF]
					--- tutaj mamy wyliczony jakiœ multiplier z atk (przy podwójnym ataku prawdopodobnie mno¿ymy x2, tak by³o dla marksman) i def
						218154 - sqrt z wyliczonego multipliera
					--- tutaj mamy wyliczony fight value dla jednej jednostki obecnego stacka w danym stanie bitwy ---
				--- tutaj mno¿ymy zsumowane hp stacka przez wyliczone fight value ---
					217F94 - konwersja wyliczonej wartoœci na int
				--- zwracamy tego inta
		1F2C0 - ustawienie kolejki ruchu probably
			1F500 - check blind/stone/paralyze
			489F0 - get unit speed (check slow etc)
			11B3A0 - dodaje stack do listy
				217492 - zwraca adres listy w której bêd¹ przechowywane jednostki
				AF880 - ustawiamy w zwróconym adresie pierwsz¹ jednostkê gracza
				232E0 - ???
				AF240 - zdejmujemy ze stosu 8 bajtów
				20B0F0 - kasujê tê listê (pewnie gdzieœ j¹ przepisujemy)
					219BB0 - ??
				14D2B0 - ??
			23340 - ustawiamy jednostki w kolejce, jak bêd¹ jechaæ
				237C0 - robi swap
			20B0F0 - kasujê tê listê (pewnie gdzieœ j¹ przepisujemy)
		22CA0 (1 (var), activeStack, 1 (const), stack_ptr, 0) - coœ ze sprawdzanie siege waepon; sprawdzaliœmy index clone i coœ z rysowaniem
		21800 - przygotowaliœmy array combat hexów (wyzerowanie pointerów)
			21670 - sprawdzamy obstacles na combat hexach (dok³adnie firewall - 2168F)
			21710 - sprawdzamy czy jest fosa
			203E0 - tutaj w pêtli lecimy po jednostkach playera
				42DA0 - coœ z morale?? liczymy hp stacka
				42610 -
				42770 - calc single unit fight value
				--- robimy negacjê tej wartoœci; sprawdzamy blind/stone/paraylyze ---
				42610 - 
				489F0 (2x) - sprawdzanie slow; return speed
				B2B00 - available for left square = 1
					[IF SOMETHING]
					B11C0 -
					[END IF]
					B3160 - to mo¿e byæ pathfinding do danej jednostki
						[IF SOMETHING]
						74520 -
						[END IF]
						--- ustaw "available for left/right square" na 0 dla wszystkich hexów po kolei ---
						B2FF0 - sprawdzamy obstacles na combat hexach (dok³adnie quicksand - B30E4)
						14DE10 - ??
						AF240 - zdejmujemy 8 bajtów ze stosu
						B3FD0 - ??
						AF240 - 
						--- jakaœ magia po drodze ---
						[WHILE SOMETHING]
						B3910 - jakaœ magia
							34840 - 
						B3FD0 -
						124280 (x2) - coœ z adjacent squares (chyba pobieramy pola przylegaj¹ce do danej jednostki)
						46960 - tutaj ciœniemy po adjacent squares danej jednostki i sprawdzamy, czy wokó³ niej s¹ jakieœ przeszkody
							695F0 - check obstacles na combat hexach (dok³adnie "local obstacles" - 6965A); dodatkowo sprawdzamy czy jednostka jest podwójnej d³ugoœci i sprawdzamy adjacent squares dla drugiego hexa
						[END WHILE]
						E7230 - ustawienie pointera do œcie¿ki do jednostki ??? albo wybór kolejnej jednostki do wyliczenia œcie¿ki
				358C0 (player_unit, activeStack, 0, 0) - 
					42DA0 (x2) - liczba jednostek w stacku * hp
					357E0 - 
						35600 - 
							225C0 -
						42CF0 - 
							42770 -
							--- z jakiegoœ powodu przemno¿yliœmy przez 1000 fight value, a w³aœciwie hp/liczbê jednostek ---
							217F94 
				--- tutaj negujemy zwrócon¹ wartoœæ hp * 1000 ---
			1FCE0 - 
				1FAA0 - 
					D9460 - 
				42DA0 - zwraca liczbê hp/jednostek activeStack 
				42610 -
				42CF0 - zwraca fight value activeStacka 
				--- negujemy tê wartoœæ; sprawdzamy czy blind/stone/paralyze activeStack ---
			B2B00 - iteracja po combat hexach i robienie coœz nimi
				B3160 - 
			35D10 - 
			36490 - 
				35F50 - 
			10C7C0 - 
			21400 - check special terrain / antimagic garrison
			1F700 - (check tactics phase) - tu gdzieœ ustawiliœmy combat action i wybraliœmy combathex do ruchu
				B3160 - 
*/