#pragma once


#include <cmath>
#include <memory>

#include "structures.h"

#include "combat_manager.h"
#include "combat_unit.h"
#include "combat_state.h"
#include "combat_pathfinder.h"
#include "combat_action.h"

class CombatManager;

enum class AIDifficulty {
	EASY /*80%, 100%*/, NORMAL /*130%, 160%, 200%*/
};

class CombatAI {
private:
	std::vector<int> hexes_fight_value_gain;

public:
	AIDifficulty difficulty;
	bool similar_army_strength{ true }; // > 2x fight_value for one side

	const CombatManager& combat_manager;
	std::unique_ptr<CombatPathfinder> pathfinder;

	CombatAI(const CombatManager& combat_manager);


	std::vector<int> chooseUnitToAttack(const CombatUnit& activeStack, const CombatHero& enemy_hero) const;
	int chooseHexToMoveForAttack(const CombatUnit& activeStack, const CombatUnit& target_unit) const;

	int calculateMeleeUnitAverageDamage(const CombatUnit& attacker, const CombatUnit& defender) const;
	int calculateCounterAttackMeleeUnitAverageDamage(const CombatUnit& attacker, const CombatUnit& defender) const;
	int calculateFightValueAdvantageAfterMeleeUnitAttack(const CombatUnit& attacker, const CombatUnit& defender) const;
	std::vector<int> calculateFightValueAdvantageOnHexes(const CombatUnit& activeStack, const CombatHero& enemy_hero, const CombatField& _field);

	//// E4580
	//float multiplierModifier(CombatUnit& activeStack, int side);

	//// 42770
	//int calculateSingleUnitValue(CombatUnit& activeStack, int side);

	//// 42B80
	//int calculateUnitValue(CombatUnit& activeStack, int side);

	//// 1EC40 (combat_mgr->get_current_active_side(), activeStack, diffDef, diffAtk, 0 or 1)
	//int calculateSummedUnitValueForSide(CombatState& state, int side, int minMeleeDiffAtk, int minDiffDef, int something);

	//// 1F1E0 (activeStack, 0, combat_mgr->get_current_active_side())
	//void sub1F1E0();

	//// 22130 ()
	//void calledWhenTacticsState(CombatUnit& unit);



	// 35B10 (current_active_side, activeStack)
	//void calculateUnitsFightValues(CombatAI& ai, CombatState& state);



	// 22100 (activeStack, 0, 0, combat_mgr->get_current_active_side())
	//void generateRelevantUnitsLists(CombatAI& ai, CombatState& state);

	// part of 22370
	//int unitPreconditions(CombatUnit& unit);

	// 22370 - pick ai action	
	//void pickAction(CombatAI& ai, CombatState& state);


	float calculateUnitAttackFightValueModifier(const CombatUnit& unit) const;
	float calculateUnitDefenceFightValueModifier(const CombatUnit& unit) const;
	float calculateUnitFightValueModifier(const CombatUnit& unit) const;

	int calculateStackUnitFightValue(const CombatUnit& unit) const;
	int calculateHeroFightValue(const CombatHero& hero) const;
};


/*
* tips:
* - ai najpierw wybiera do ataku jednostki kt�re s� bli�ej (tj liczba tur jaka jest potrzebna do doj�cia jest najmniejsza)
*	- p�niej je�li liczba tur jest r�wna, to wybiera albo silniejszy stack, albo ten kt�remu zada wi�cej dmg
* 
AI FLOW:

22370 - start; check if shooter/shooter-siege or flyer
	77740 - something with blue highlight
	42160 - ???
		C8DE0 - get unit number shots and check if arrow tower
	hota.dll + C86D0 - check if arrow tower, ballista or 150 (cannon? undefined?)
	22100 - calculate relevant lists
		35B10 - calculate units fight values for both sides
			42610 - sprawdzamy czy arrowtower i co� tam
			42270 - diff btw cur_atk melee and base_atk melee
				42130 - get cur_atk melee
			42380 - diff btw cur_def and base_def
				422B0 - get cur_def
			--- do tej pory mamy wyliczon� NAJMNIEJSZ� r�nic� melee_atk i melee_def ---
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
					--- tutaj mamy chyba por�wnanie def z min def ---
						E4580 - some magic
						[IF SHOOTER (marskman)]
						3FE60 - 
						D9460 - 
						42710 - 
							691E0 - 
						[END IF]
					--- tutaj mamy wyliczony jaki� multiplier z atk (przy podw�jnym ataku prawdopodobnie mno�ymy x2, tak by�o dla marksman) i def
						218154 - sqrt z wyliczonego multipliera
					--- tutaj mamy wyliczony fight value dla jednej jednostki obecnego stacka w danym stanie bitwy ---
				--- tutaj mno�ymy zsumowane hp stacka przez wyliczone fight value ---
					217F94 - konwersja wyliczonej warto�ci na int
				--- zwracamy tego inta
		1F2C0 - ustawienie kolejki ruchu probably
			1F500 - check blind/stone/paralyze
			489F0 - get unit speed (check slow etc)
			11B3A0 - dodaje stack do listy
				217492 - zwraca adres listy w kt�rej b�d� przechowywane jednostki
				AF880 - ustawiamy w zwr�conym adresie pierwsz� jednostk� gracza
				232E0 - ???
				AF240 - zdejmujemy ze stosu 8 bajt�w
				20B0F0 - kasuj� t� list� (pewnie gdzie� j� przepisujemy)
					219BB0 - ??
				14D2B0 - ??
			23340 - ustawiamy jednostki w kolejce, jak b�d� jecha�
				237C0 - robi swap
			20B0F0 - kasuj� t� list� (pewnie gdzie� j� przepisujemy)
		22CA0 (1 (var), activeStack, 1 (const), stack_ptr, 0) - co� ze sprawdzanie siege waepon; sprawdzali�my index clone i co� z rysowaniem
		21800 - przygotowali�my array combat hex�w (wyzerowanie pointer�w) - tutaj ustawili�my na jaki hex p�jdziemy i combat action
			21670 - sprawdzamy obstacles na combat hexach (dok�adnie firewall - 2168F)
			21710 - sprawdzamy czy jest fosa
			203E0 - tutaj w p�tli lecimy po jednostkach playera
				42DA0 - co� z clone?? liczymy hp stacka
				42610 -
				42770 - calc single unit fight value
				--- robimy negacj� tej warto�ci; sprawdzamy blind/stone/paraylyze ---
				42610 - 
				489F0 (2x) - sprawdzanie slow; return speed
				B2B00 - available for left square = 1   --- 1FE320: zapisanie combat path 
					[IF SOMETHING]
					B11C0 -
					[END IF]
					B3160 - to mo�e by� pathfinding do danej jednostki
						[IF SOMETHING]
						74520 -
						[END IF]
						--- ustaw "available for left/right square" na 0 dla wszystkich hex�w po kolei ---
						B2FF0 - sprawdzamy obstacles na combat hexach (dok�adnie quicksand - B30E4)
						14DE10 - ??
						AF240 - zdejmujemy 8 bajt�w ze stosu
						B3FD0 - ??
						AF240 - 
						--- jaka� magia po drodze ---
						[WHILE SOMETHING]
						B3910 - jaka� magia
							34840 - 
						B3FD0 -
						124280 (x2) - co� z adjacent squares (chyba pobieramy pola przylegaj�ce do danej jednostki)
						46960 - tutaj ci�niemy po adjacent squares danej jednostki i sprawdzamy, czy wok� niej s� jakie� przeszkody
							695F0 - check obstacles na combat hexach (dok�adnie "local obstacles" - 6965A); dodatkowo sprawdzamy czy jednostka jest podw�jnej d�ugo�ci i sprawdzamy adjacent squares dla drugiego hexa
						[END WHILE]
						E7230 - ustawienie pointera do �cie�ki do jednostki ??? albo wyb�r kolejnej jednostki do wyliczenia �cie�ki
				358C0 (player_unit, activeStack, 0, 0) - 
					42DA0 (x2) - liczba jednostek w stacku * hp
					357E0 - 
						35600 - 
							225C0 -
						42CF0 - 
							42770 -
							--- z jakiego� powodu przemno�yli�my przez 1000 fight value, a w�a�ciwie hp/liczb� jednostek ---
							217F94 
				--- tutaj negujemy zwr�con� warto�� hp * 1000 ---
			1FCE0 - 
				1FAA0 - 
					D9460 - 
				42DA0 - zwraca liczb� hp/jednostek activeStack 
				42610 -
				42CF0 - zwraca fight value activeStacka 
				--- negujemy t� warto��; sprawdzamy czy blind/stone/paralyze activeStack ---
			B2B00 - iteracja po combat hexach i robienie co�z nimi
				B3160 - 
			35D10 - 
			36490 - 
				35F50 - 
			10C7C0 - 
			21400(0, some_calc_value, activeStack) - check special terrain / antimagic garrison
			1F700(jednym z argument�w jest hex na kt�ry idziemy) - (check tactics phase) - tu gdzie� ustawili�my combat action i wybrali�my combathex do ruchu; przekazali�my numer hexa jako parametr
				B3160 - 
				124280 - pobieranie kolejnego hexa z pathfindingu??



##################

20665 -> tutaj ustawiamy na hexie fight_value_diff

##################

10C7C0 (fight_value jakie zyskamy (ebx), 75 (ecx), 100 (edx))
	eax = 2184AC - nie wiem, sprawdzamy jakie� b��dy

--- liczymy ---
eax (3C49) = 15433
esi = 100 (esi) - 75 (edi) = 25
esi++;

eax /= esi -> 15433 / 26 = 593 r.15
eax = edx (eax = 15)
eax += edi -> 15 + 75 = 90  (tutaj bierzemy ile to jest %, 75-100 % warto�ci)

--- ju� po --

ecx(90) = ecx * ebx = 90 * 3535 = 318150
edx:eax = ecx * ecx
edx >>= 5
edx >>= 31
edx = edx + eax

--- decompile ---

some_val = sub_10C7C0(fight_val_diff, 75, 100);
int64 new_val = fight_val_diff * some_val;
new_val *= 1374389535;
new_val >> 32; (bierzemy 4 hword bajty)

new_val /= 32; // >>=5
int new_val2 = new_val >>= 32; (tutaj, je�li new_val by�o ujemne, wynik = 1)
new_val += new_val2;

new_val /= liczba_tur_w_jakie_dojdziemy


##################

21800 -
--- bla bla ---

21BCE - tutaj zwracamy jak�� warto�� z kt�r� p�niej co� robimy (wynik zapisujemy w [epb-20]);
			tutaj liczymy r�nic� w "zadanych fight value" - "straconych fight value" (w wyniku kontrataku);

porownujemy t� warto�� z ebp-48

Poni�sze warto�ci mamy ju� przed sub_10C7C0
Mo�liwe �e ciekawe: [ebp-40 (mo�liwe �e liczba tur zanim dojdziemy do targetu)], [ebp-44 (target_hex)], 
						  [ebp-48 (wygl�da na warto�� fight value jak� stracimy stoj�c dok�adnie w tym polu],
						  [ebp-4C (chyba dmg z kontrataku)], [ebp-50 (wygl�da na dmg jednostki w stack)]

Wcze�niej jest sub_10C7C0 i jaki� jump
ecx = [ebp-44] - ten sam target
21DAA - [ebp-1C] = ecx - tutaj ustawiamy target hexa na jaki zmierzamy

218B5 - tutaj ustawiamy r�nic� fight value na hexach, tylko w przypadku gdy player zada wi�cej fight_value ni� kontra ai

##################

35D10 - tak, wygl�da na obliczenie zadanych obra�en + kontratak
[ebp-50] - ile zosta�o jednostek w atakowanym stacku (57)
[ebp-4C] - ile zosta�o naszemu stackowi hp (500)

36490 - tutaj ustawiamy pozosta�e warto�ci
[ebp-48] - 1666 (kij wie, co to jest)
[ebp-44] - 36 (hex_target)
[ebp-40] - 5 (liczba tur zanim dojdziemy)

35F50 (przekazujemy pozycj� atakowanej jednostki) - chyba wyznaczamy tutaj hex na kt�ry b�dziemy si� kierowa�; zwracamy true, je�li si� uda�o wybra�
---liczymy---
lecimy po hexach dooko�a atakowanej jednostki (tutaj ju� chyba mamy wyliczone wcze�niej fight_value dla danego hexa)

co� tam checking extended_attack
co� tam checking no_retaliation
co� tam checking attack_all_around (dla targetowanej jednostki)
co� tam checking extended_attack (dla targetowanej jednostki)

zapisuje wynik w [esi+1C], [esi+20] (target_hex), [esi+24] (liczba tur)

wyb�r tutaj wygl�da tak:
	- je�li niewybrane �adne pole do ataku 
		- wybierz bie��ce
	- w przeciwnym razie 
		- je�li ile tur minimum trzeba �eby doj�� do jednostki (361C3) jest taka sama dla tego pola jak minimum
			- je�li fight_value_gain na tym wybranym wcze�niej hexie jest mniejsze ni� sprawdzanego pola (361D0)
				- sprawd� nast�pne pole
			- w przeciwnym razie je�li jest wi�ksze
				- wybierz sprawdzane pole
			- je�li r�wne
				- je�li jednostka = kawalerzysta lub czempion
					- wybierz pole, kt�re jest dalej
				- w przeciwnym przypadku
					- je�li pole jest bli�ej
						- wybierz bli�sze pole
					- je�li pole jest dalej lub r�wnej d�ugo��i
						- sprawd� nast�pne pole

		- w przeciwnym razie sprawd� nast�pne pole

35D10 -
	424A0 - liczymy jaki zadamy dmg jednostce
--- liczymy ---
dmg_dealt = sub_424A0
ecx (left_hp) = ecx (current_hp) - dmg_dealt
ebx = hp jednostki
[ebp+08] = left_hp
...
eax = ile zosta�o unit�w w stacku (437.5dmg ... zosta�o 57)
edx = ile hp zosta�o (i 2hp)

36490 - tutaj jakby�my wybrali hex z kt�rego b�dziemy atakowa� (liczyli�my, kt�ry b�dzie dla nas najlepszy do ataku)

42DA0 - ile hp ma stack

42DA0
---liczymy---
check clone (if clone hp = 1)
eax = get_target_unit_hp
edx = get_target_healthlost
eax *= target_stack_number - edx // (total stack health)

424A0 (activeStack.totalHealth, 0, target_unit); - obliczanie zadanego dmg w unit


##################

1FCE0 -
	1FAA0 -
for(every unit in ai hero)
	42D40 - 
	42610(0, unit_health * unit_number - health_lost, 0);
	42CF0(eax = 0, ecx = 0, edx = 0)
		42770(0, eax, ecx, edx);
--- liczymy ---
unit_stack_fight_value = sub_42Cf0;
eax = -unit_stack_fight_value;

check if can move
forend



42CF0 - zwr�cili�my unit stack fight value; 

42DA0 - get_unit_health*unit_number-healthlost
---liczymy---
check if this->unit isclone
---
edx = this->unit_hp
eax *= this->unit_alive;
return eax

1FAA0 - sprawdzamy kilka rzeczy, w tym spelle (pewnie po to, �eby wiedzie�, czy si� rozprasza�)
--- liczymy ---
eax = 9*ebx;
eax = 4*eax + ebx; // 37ebx
eax *= 32; // 1184ebx
eax -= ebx; // 1183ebx
edx = 3eax; // 3549ebx
eax = [combat_mgr + 4ebx + 54BC] = liczba stack�w(unit�w) u herosa(playera)

check fire_immunity
check extended_attack

	sub_D9460(126) - sprawdzamy czy enemy ma orb_of_inhibition

iterate through enemy spells
	check if have: frost_ring/fireball/inferno/meteor shower

D9460(id artefaktu) - sprawdzamy czy hero posiada artefakt

##################

203E0 - tutaj w p�tli lecimy po jednostkach playera
	42DA0 - co� z morale?? liczymy hp stacka
	42610 -
	42770 - calc single unit fight value
	--- tutaj negujemy zwr�con� warto�� ---
	fight_value = sub_42770();
	fight_value = -fight_value;
	unit->sub_42610(0); // sprawdzanie co� ze strzelajkami; chyba czy mog� strzela�


42160 - canShoot?? (return 1 je�li si� nie uda poni�sze, 0 else)
	unit->sub_D9460(137 # id bow_of_sharpshooter #) - check if has artifact bow_of_sharpshooter?
	unit->sub_691E0(unit_hex_pos, unknown_but_0) - sprawdzamy blokad� strzelajki
	check_forgetfullness


691E0 - co� z pozycj� strzelajki
--- liczymy ---
for(i = 0; i < 6; ++i) - dla ka�dego adjacent_hexa
eax (new_unit_hex_pos) = 3B(adj_size) * unit_hex_pos
edi = (combat_mgr + new_unit_hex_pos * 4B(ptr_size) + adjacent_squares) = ptr_to_adjacent_square_struct_for_unit_pos
edx = *edi (which is top-right) * 8 - *edi;
edx *= 16;
ecx = combat_square_of_top_right_adjacent_to_unit_hex_pos

	sub_E7230(ecx) - sprawdzamy czy co� blokuje strzelajk�
	--- liczymy ---
	al = stack_side_of_unit_at_this_square(); // 0 / 1 / -1

	sub_425A0(0) - nie wiem
forend

#################

358C0 - ile stracimy fight_value (je�li inna jednostka najpierw zaatakuje)
--- liczymy ---
total health for both stacks

	sub_357E0() - tutaj liczymy r�nic� w "zadanych fight value" - "straconych fight value" (w wyniku kontrataku);
					- fight_value = base_fight_value * fight_value_multiplier * zadany_dmg / hp_atakowanej_jednostki
		sub_35600() - tutaj obliczamy ile zadamy obra�e� z kontrataku
		--- tutaj mamy wyliczony dmg w jednostk� ---
			sub_225C0() - sprawdzamy, czy powinni�my otrzyma� dmg zwrotny (czy atakowali�my ifryta i czy mamy fire immunity)
			--- liczymy ---
			check if attacked ifryt_sultanski
		left_hp = total_health - dmg
		wyliczamy ile zostanie jednostek w stacku i ile lu�nego hp
			sub_424A0() - modyfikator dmg (bless/curse); liczymy skontratakowany dmg jednostki (min+max)/2*liczba_w_stacku
				sub_43C60() - tutaj og�lnie obliczamy jaki zadamy dmg uwzgl�dniaj�c WSZYSTKIE modyfikatory
					sub_43560() - zwracamy dodatkowy dmg (jak ni�ej)
						sub_43040() - sprawdzamy czy kawalerzysta/czempion; funkcja zwraca dodatkowy dmg (np. wynikaj�cy z r�nicy atk-def, lub specjalnych umiejek)
							sub_42130() - sprawdzamy jakie� jednostki z cove
							sub_422B0() - pobierz def jednostki (kontratakowanej)
						
						dodatkowy dmg = (diff_atk) * 0.05
							sub_E4520() - 

					dodatkowy dmg = 35 // bazowy 142 (57 * (2+3)/2 = 57 * 2.5 =  floor(142.5))
				dodatkowy dmg = 35;
				total_dmg = dmg + dodatkowy_dmg;

					sub_438B0() - tutaj uwzgl�dniamy luck (modyfikator 1.5x)

			tutaj liczymy ile zostanie hp po kontrataku
			4256A -> ecx -= edx
		3574E -> eax -= edx (liczymy ile zostanie hp po kontrataku)

		(najpeirw dla atakowanej jednostki)
		sub_42CF0() - tutaj co� wyliczamy (chyba fight_value jakie utracimy po ataku)
		
		(teraz dla active stacka; na bazie zadanych obra�e�)
		sub_42CF0() - z jakiego� powodu tutaj czasem zamiast liczy� dmg * fight_value, liczymy dmg * 1000 (kiedy fight value gracza > 2*fight_value_enemy)
							enemy przy takiej przewadze liczy dmg*100

#####

1FCE0

1FAA0 - sprwadzamy, czy mo�emy wycastowa� spell, czy jeste�my strzelajkami i wyliczenie jakiego� fight_value (8040 dla 500 ch�op�w na 200 imp�w)
- sprawdzamy czy gracz spellbook,																									
- sprawdzamy czy jest orb of inhibition (po kt�rejkolwiek ze stron)
- sprwadzanie czy mamy spelle (quick sand / 

wyliczenie hp w stacku
sprawdzenie czy strzelajki
wyliczenie fight_value activeStack

#####

og�lny algorytm jest taki:

(218B5)
- oblicz fight_value_gain (tj. jed� po wszystkich jednostkach gracza i symuluj, tak jakby on atakowa� pierwszy);  
 - je�li -fight_value_gain < 0 (tj. ai wi�cej straci ni� zyska) ustaw na wszystkich hexach to kt�rych jednostka gracza ma dost�p do ataku t� warto�� (2067B)
 #####
 1FCE0
 #####
 - p�niej sprawdzanie jaki jest dystans od nas do jednostki kt�r� chcemy zaatakowa�
 #####
 35D10 - ile zadamy obra�e�/zabijemy/ile nam zostanie hp
 36490 - tutaj wybieramy odpowiedni hex (idziemy zgodnie ze wskaz�wkami zegara)
 #####
 Po drodze jeszcze jak idziemy sprawdzamy czy si� nie zatrzyma�, �eby nas wroga jednostka nie mog�a zaatakowa�
 #####
 

- oblicz dla ka�dego hexa warto�� r�wn�: fight_value jakie zada ka�da jednostka gracza (tj. dmg jednostki, p�niej kontratak i z tego r�nic� w 
		zadanym fight_value) dla ka�dego hexa na mapie, tak jakby gracz atakowa� pierwszy
- oblicz dla ka�dej jednostki gracza r�nic� fight value (tak jakby ai atakowa�o pierwsze) bior�c pod uwag� kontratak itp.
- uwzgl�dnij drugie i pierwsze
- zr�b jakie� obliczenia
- cocojambo i do przodu

*/