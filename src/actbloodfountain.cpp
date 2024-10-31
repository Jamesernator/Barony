/*-------------------------------------------------------------------------------

	BARONY
	File: actfountain.cpp
	Desc: behavior function for fountains

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include <random>
#include "main.hpp"
#include "game.hpp"
#include "stat.hpp"
#include "entity.hpp"
#include "monster.hpp"
#include "engine/audio/sound.hpp"
#include "items.hpp"
#include "net.hpp"
#include "collision.hpp"
#include "player.hpp"
#include "colors.hpp"
#include "scores.hpp"
#include "prng.hpp"
#include "mod_tools.hpp"

//Blood Fountain functions.
const std::vector<int> bloodFountainPotionDropChances =
{
	80,	//FOOD_BLOOD,
	1,	//POTION_BOOZE,
	1,	//POTION_JUICE,
	2,	//POTION_SICKNESS,
	1,	//POTION_CONFUSION,
	1,	//POTION_EXTRAHEALING,
	1,	//POTION_HEALING,
	1,	//POTION_CUREAILMENT,
	1,	//POTION_BLINDNESS,
	1,	//POTION_RESTOREMAGIC,
	1,	//POTION_INVISIBILITY,
	1,	//POTION_LEVITATION,
	1,	//POTION_SPEED,
	1,	//POTION_ACID,
	1,	//POTION_PARALYSIS,
	5	//POTION_POLYMORPH
};

const std::vector<std::pair<int, int>> bloodPotionStandardAppearanceMap =
{
	// second element is appearance.
	{ FOOD_BLOOD, 0 },
	{ POTION_BOOZE, 2 },
	{ POTION_JUICE, 3 },
	{ POTION_SICKNESS, 1 },
	{ POTION_CONFUSION, 0 },
	{ POTION_EXTRAHEALING, 0 },
	{ POTION_HEALING, 0 },
	{ POTION_CUREAILMENT, 0 },
	{ POTION_BLINDNESS, 0 },
	{ POTION_RESTOREMAGIC, 1 },
	{ POTION_INVISIBILITY, 0 },
	{ POTION_LEVITATION, 0 },
	{ POTION_SPEED, 0 },
	{ POTION_ACID, 0 },
	{ POTION_PARALYSIS, 1 },
	{ POTION_POLYMORPH, 0 },
	{ POTION_FIRESTORM, 0 },
	{ POTION_ICESTORM, 0 },
	{ POTION_THUNDERSTORM, 0 },
	{ POTION_STRENGTH, 0 }
};

std::mt19937 bloodFountainSeed(rand());
std::discrete_distribution<> bloodFountainDistribution(bloodFountainPotionDropChances.begin(), bloodFountainPotionDropChances.end());


std::pair<int, int> bloodFountainGeneratePotionDrop()
{
	auto keyPair = bloodPotionStandardAppearanceMap.at(bloodFountainDistribution(bloodFountainSeed));
	return std::make_pair(keyPair.first, keyPair.second);
}

/*-------------------------------------------------------------------------------

	act*

	The following function describes an entity behavior. The function
	takes a pointer to the entity that uses it as an argument.

	my->skill[0] is either 0 or 1. If it is 0, the fountain is dry and cannot be used
	my->skill[1] is either 0, 1, 2, or 3. It is set at the creation of the fountain.
		Those values correspond to what the fountain does:
		0 = spawn succubus, 1 = raise hunger, 2 = random potion effect, 3 = bless equipment
	my->skill[3] is a random potion effect. It is set at the creation of the fountain
-------------------------------------------------------------------------------*/

void actBloodFountain(Entity* my)
{
	Entity* entity;

	// spray water
	if ( my->skill[0] > 0 )
	{
#define FOUNTAIN_AMBIENCE my->skill[7]
#ifdef USE_FMOD
		if ( FOUNTAIN_AMBIENCE == 0 )
		{
			FOUNTAIN_AMBIENCE--;
			my->stopEntitySound();
			my->entity_sound = playSoundEntityLocal(my, 672, 32);
		}
		if ( my->entity_sound )
		{
			bool playing = false;
			my->entity_sound->isPlaying(&playing);
			if ( !playing )
			{
				my->entity_sound = nullptr;
			}
		}
#else
		FOUNTAIN_AMBIENCE--;
		if ( FOUNTAIN_AMBIENCE <= 0 )
		{
			FOUNTAIN_AMBIENCE = TICKS_PER_SECOND * 6;
			playSoundEntityLocal(my, 672, 32 );
		}
#endif
		entity = spawnGib(my);
		entity->flags[INVISIBLE] = false;
		entity->y -= 2;
		entity->z = my->z - 8;
		entity->flags[SPRITE] = false;
		entity->flags[NOUPDATE] = true;
		entity->flags[UPDATENEEDED] = false;
		entity->skill[4] = 7;
		// @JAMESERNATOR_TODO Verify this sprite
		entity->sprite = 5;
		entity->yaw = (local_rng.rand() % 360) * PI / 180.0;
		entity->pitch = (local_rng.rand() % 360) * PI / 180.0;
		entity->roll = (local_rng.rand() % 360) * PI / 180.0;
		entity->vel_x = 0;
		entity->vel_y = 0;
		entity->vel_z = .25;
		entity->fskill[3] = 0.03;
	}
	else
	{
		my->stopEntitySound();
	}

	if ( my->ticks == 1 )
	{
		my->createWorldUITooltip();
	}

	// the rest of the function is server-side.
	if ( multiplayer == CLIENT )
	{
		return;
	}

	//Using the fountain (TODO: Monsters using it?).
	int i;
	for (i = 0; i < MAXPLAYERS; ++i)
	{
		if ( selectedEntity[i] == my || client_selected[i] == my )
		{
			if (inrange[i])   //Act on it only if the player (or monster, if/when this is changed to support monster interaction?) is in range.
			{
				auto& rng = my->entity_rng ? *my->entity_rng : local_rng;

				//First check that it's not depleted.
				if (my->skill[0] == 0)
				{
					//Depleted
					messagePlayer(i, MESSAGE_INTERACTION, Language::get(467));
				}
				else
				{
					if (players[i]->entity->flags[BURNING])
					{
						// @JAMESERNATOR_TODO: MESSAGE: You dowse yourself in the blood.
						messagePlayer(i, MESSAGE_INTERACTION, Language::get(9999999));
						players[i]->entity->flags[BURNING] = false;
						serverUpdateEntityFlag(players[i]->entity, BURNING);
						steamAchievementClient(i, "BARONY_ACH_HOT_SHOWER");
					}
					int potionDropQuantity = 0;
					if ( stats[i] && (stats[i]->type == GOATMAN || (stats[i]->playerRace == RACE_GOATMAN && stats[i]->stat_appearance == 0)) )
					{
						// drop some random potions.
						switch (rand() % 10)
						{
							case 0:
							case 1:
								potionDropQuantity = 2;
								break;
							case 2:
								potionDropQuantity = 3;
								break;
							case 3:
							case 4:
							case 5:
							case 6:
							case 7:
							case 8:
							case 9:
								potionDropQuantity = 1;
								break;
							default:
								break;
						}

						if (potionDropQuantity > 0)
						{
							steamStatisticUpdateClient(i, STEAM_STAT_BOTTLE_NOSED, STEAM_STAT_INT, 1);
						}

						if ( potionDropQuantity > 0 )
						{
							if ( stats[i]->playerRace == RACE_GOATMAN && stats[i]->stat_appearance == 0 )
							{
								steamStatisticUpdateClient(i, STEAM_STAT_BOTTLE_NOSED, STEAM_STAT_INT, 1);
							}
						}

						for ( int j = 0; j < potionDropQuantity; ++j )
						{
							std::pair<int, int> generatedPotion = bloodFountainGeneratePotionDrop();
							ItemType type = static_cast<ItemType>(generatedPotion.first);
							int appearance = generatedPotion.second;
							Item* item = newItem(type, EXCELLENT, 0, 1, appearance, false, NULL);
							if ( Entity* dropped = dropItemMonster(item, my, NULL) )
							{
								dropped->yaw = ((0 + local_rng.rand() % 360) / 180.f) * PI;
								dropped->vel_x = (0.75 + .025 * (local_rng.rand() % 11)) * cos(dropped->yaw);
								dropped->vel_y = (0.75 + .025 * (local_rng.rand() % 11)) * sin(dropped->yaw);
								dropped->vel_z = (-10 - local_rng.rand() % 20) * .01;
								dropped->flags[USERFLAG1] = false;
							}
						}
					}
					Compendium_t::Events_t::eventUpdateWorld(i, Compendium_t::CPDM_FOUNTAIN_USED, "fountain", 1);
					switch (my->skill[1])
					{
						case 0:
						{
							playSoundEntity(players[i]->entity, 52, 64);

							//Spawn monsters.
							Uint32 color = makeColorRGB(255, 128, 0);
							Entity* spawnedMonster = nullptr;

							if ( !strncmp(map.name, "Underworld", 10) )
							{
								Monster creature = CREATURE_IMP;
								if ( rng.rand() % 2 )
								{
									creature = DEMON;
								}
								for ( int c = 0; spawnedMonster == nullptr && c < 5; ++c )
								{
									switch ( c )
									{
										case 0:
											spawnedMonster = summonMonster(creature, my->x, my->y);
											break;
										case 1:
											spawnedMonster = summonMonster(creature, my->x + 16, my->y);
											break;
										case 2:
											spawnedMonster = summonMonster(creature, my->x - 16, my->y);
											break;
										case 3:
											spawnedMonster = summonMonster(creature, my->x, my->y + 16);
											break;
										case 4:
											spawnedMonster = summonMonster(creature, my->x, my->y - 16);
											break;
									}
								}
								if ( spawnedMonster )
								{
									spawnedMonster->seedEntityRNG(rng.getU32());

									if ( creature == DEMON)
									{
										// @JAMESERNATOR_TODO: MESSAGE: You summon a demon!
										messagePlayerColor(i, MESSAGE_INTERACTION, color, Language::get(999999));
										Stat* tmpStats = spawnedMonster->getStats();
										if ( tmpStats )
										{
											strcpy(tmpStats->name, "blood demon");
										}
									} else {
										// @JAMESERNATOR_TODO: MESSAGE: You summon an imp!
										messagePlayerColor(i, MESSAGE_INTERACTION, color, Language::get(999999999999));
									}
								}
							}
							else if ( currentlevel < 18 )
							{
								if ( spawnedMonster = summonMonster(CREATURE_IMP, my->x, my->y) )
								{
									spawnedMonster->seedEntityRNG(rng.getU32());
									// @JAMESERNATOR_TODO: MESSAGE: You summon an imp!
									messagePlayerColor(i, MESSAGE_INTERACTION, color, Language::get(9999999));
								}
							}
							else if ( currentlevel < 46 )
							{
								if ( rng.rand() % 2 )
								{
									if ( spawnedMonster = summonMonster(DEMON, my->x, my->y) )
									{
										spawnedMonster->seedEntityRNG(rng.getU32());
										Stat* tmpStats = spawnedMonster->getStats();
										if ( tmpStats )
										{
											strcpy(tmpStats->name, "blood demon");
										}
										// @JAMESERNATOR_TODO: MESSAGE: You summon a demon!
										messagePlayerColor(i, MESSAGE_INTERACTION, color, Language::get(999999));
									}
								}
								else
								{
									if ( spawnedMonster = summonMonster(DEMON, my->x, my->y) )
									{
										spawnedMonster->seedEntityRNG(rng.getU32());
										// @JAMESERNATOR_TODO: MESSAGE::  You summon an imp!
										messagePlayerColor(i, MESSAGE_INTERACTION, color, Language::get(9999));
									}
								}
							}
							else
							{
								if ( spawnedMonster = summonMonster(DEMON, my->x, my->y) )
								{
									spawnedMonster->seedEntityRNG(rng.getU32());
									// @JAMESERNATOR_TODO: MESSAGE: You summon a demon!
									messagePlayerColor(i, MESSAGE_INTERACTION, color, Language::get(99999));
								}
							}
							break;
						}
						case 1:
							Compendium_t::Events_t::eventUpdateWorld(i, Compendium_t::CPDM_FOUNTAIN_DRUNK, "fountain", 1);
							if ( stats[i] && stats[i]->type == VAMPIRE )
							{
								messagePlayer(i, MESSAGE_INTERACTION, Language::get(470));
								// @JAMESERNTOR_TODO: MESSAGE:: The blood refreshes you.
								messagePlayer(i, MESSAGE_INTERACTION, Language::get(99999));
								playSoundEntity(players[i]->entity, 52, 64);

								stats[i]->HUNGER += 1000;
								serverUpdateHunger(i);

								players[i]->entity->modHP(5);
							}
							else
							{
								players[i]->entity->modHP(-4);
								playSoundEntity(players[i]->entity, 28, 64);
								playSoundEntity(players[i]->entity, 249, 128);
								// @JAMESERNATOR_TODO: MESSAGE:: 's body couldn't support the powers of the fountain, and collapsed.
								players[i]->entity->setObituary(Language::get(9999));
								stats[i]->killer = KilledBy::FOUNTAIN;

								Uint32 color = makeColorRGB(255, 0, 0);
								// @JAMESERNATOR_TODO: MESSAGE:: The blood refreshes you.
								messagePlayerColor(i, MESSAGE_STATUS, color, Language::get(3183));
								if ( i >= 0 && players[i]->isLocalPlayer() )
								{
									cameravars[i].shakex += .1;
									cameravars[i].shakey += 10;
								}
								else if ( multiplayer == SERVER && i > 0 && !players[i]->isLocalPlayer() )
								{
									strcpy((char*)net_packet->data, "SHAK");
									net_packet->data[4] = 10; // turns into .1
									net_packet->data[5] = 10;
									net_packet->address.host = net_clients[i - 1].host;
									net_packet->address.port = net_clients[i - 1].port;
									net_packet->len = 6;
									sendPacketSafe(net_sock, -1, net_packet, i - 1);
								}
							}
							break;
						case 2:
						{
							//Potion effect. Potion effect is stored in my->skill[3], randomly chosen when the fountain is created.
							Compendium_t::Events_t::eventUpdateWorld(i, Compendium_t::CPDM_FOUNTAIN_DRUNK, "fountain", 1);
							messagePlayer(i, MESSAGE_INTERACTION, Language::get(470));
							Item* item = newItem(static_cast<ItemType>(POTION_WATER + my->skill[3]), static_cast<Status>(4), 0, 1, 0, false, NULL);
							useItem(item, i, my);
							// Long live the mystical fountain of TODO.
							break;
						}
						case 3:
						{
							// curse all equipment
							playSoundEntity(players[i]->entity, 52, 64);
							//playSoundEntity(players[i]->entity, 167, 64);
							Uint32 textcolor = makeColorRGB(255, 0, 0);

							// @JAMESERNATOR_TODO: MESSAGE:: The blood refreshes you.
							messagePlayerColor(i, MESSAGE_STATUS, textcolor, Language::get(999999));
							// @JAMESERNATOR_TODO: MESSAGE:: The fountain curses your equipment.
							messagePlayerColor(i, MESSAGE_STATUS, textcolor, Language::get(999999));

							if ( !stats[i] )
							{
								break;
							}

							if ( stats[i]->helmet )
							{
								stats[i]->helmet->beatitude--;
							}
							if ( stats[i]->breastplate )
							{
								stats[i]->breastplate->beatitude--;
							}
							if ( stats[i]->gloves )
							{
								stats[i]->gloves->beatitude--;
							}
							if ( stats[i]->shoes )
							{
								stats[i]->shoes->beatitude--;
							}
							if ( stats[i]->shield )
							{
								stats[i]->shield->beatitude--;
							}
							if ( stats[i]->weapon )
							{
								if ( stats[i]->weapon->type != POTION_EMPTY )
								{
									stats[i]->weapon->beatitude--;
								}
							}
							if ( stats[i]->cloak )
							{
								stats[i]->cloak->beatitude--;
							}
							if ( stats[i]->amulet )
							{
								stats[i]->amulet->beatitude--;
							}
							if ( stats[i]->ring )
							{
								stats[i]->ring->beatitude--;
							}
							if ( stats[i]->mask )
							{
								stats[i]->mask->beatitude--;
							}
							if ( multiplayer == SERVER && i > 0 && !players[i]->isLocalPlayer() )
							{
								strcpy((char*)net_packet->data, "BLES");
								net_packet->address.host = net_clients[i - 1].host;
								net_packet->address.port = net_clients[i - 1].port;
								net_packet->len = 4;
								sendPacketSafe(net_sock, -1, net_packet, i - 1);
							}
							break;
						}
						case 4:
						{
							// curse one piece of equipment
							playSoundEntity(players[i]->entity, 52, 64);
							//playSoundEntity(players[i]->entity, 167, 64);
							Uint32 textcolor = makeColorRGB(255, 0, 0);
							// @JAMESERNATOR_TODO: MESSAGE:: The blood refreshes you.
							messagePlayerColor(i, MESSAGE_STATUS, textcolor, Language::get(471));
							//Choose only one piece of equipment to bless.

							if ( !stats[i] )
							{
								break;
							}

							//First, Figure out what equipment is available.
							std::vector<std::pair<Item*, Uint32>> items;
							if ( stats[i]->helmet )
							{
								items.push_back(std::pair<Item*,int>(stats[i]->helmet, 0));
							}
							if ( stats[i]->breastplate )
							{
								items.push_back(std::pair<Item*,int>(stats[i]->breastplate, 1));
							}
							if ( stats[i]->gloves )
							{
								items.push_back(std::pair<Item*,int>(stats[i]->gloves, 2));
							}
							if ( stats[i]->shoes )
							{
								items.push_back(std::pair<Item*,int>(stats[i]->shoes, 3));
							}
							if ( stats[i]->shield )
							{
								items.push_back(std::pair<Item*,int>(stats[i]->shield, 4));
							}
							if ( stats[i]->weapon && stats[i]->weapon->type != POTION_EMPTY )
							{
								items.push_back(std::pair<Item*,int>(stats[i]->weapon, 5));
							}
							if ( stats[i]->cloak )
							{
								items.push_back(std::pair<Item*,int>(stats[i]->cloak, 6));
							}
							if ( stats[i]->amulet )
							{
								items.push_back(std::pair<Item*,int>(stats[i]->amulet, 7));
							}
							if ( stats[i]->ring )
							{
								items.push_back(std::pair<Item*,int>(stats[i]->ring, 8));
							}
							if ( stats[i]->mask )
							{
								items.push_back(std::pair<Item*,int>(stats[i]->mask, 9));
							}

							if ( items.size() )
							{
								// @JAMESERNATOR_TODO: MESSAGE:: The fountain curses your equipment.
								messagePlayerColor(i, MESSAGE_STATUS, textcolor, Language::get(999999));
								//Randomly choose a piece of equipment.
								std::pair<Item*, Uint32> chosen = items[rng.rand()%items.size()];
								chosen.first->beatitude--;
								Compendium_t::Events_t::eventUpdate(i, Compendium_t::CPDM_BLESSED_TOTAL, chosen.first->type, 1);

								if ( multiplayer == SERVER && i > 0 && !players[i]->isLocalPlayer() )
								{
									strcpy((char*)net_packet->data, "BLE1");
									SDLNet_Write32(chosen.second, &net_packet->data[4]);
									net_packet->address.host = net_clients[i - 1].host;
									net_packet->address.port = net_clients[i - 1].port;
									net_packet->len = 8;
									sendPacketSafe(net_sock, -1, net_packet, i - 1);
								}
							}
							//Does nothing if no valid items.
							break;
						}
						//heals or damages the player
						case 5:
						{
							//Blood fountain is good for vampires and not much for everyone else
							if (stats[i]->type != VAMPIRE)
							{

								messagePlayer(i, MESSAGE_INTERACTION, Language::get(470));
								// @JAMESERNATOR_TODO: MESSAGE:: Your body rejects the blood in pain.
								messagePlayer(i, MESSAGE_INTERACTION, Language::get(6236));
								playSoundEntity(players[i]->entity, 52, 64);
								stats[i]->HUNGER -= 100;
								players[i]->entity->modHP(-45);
								// @JAMESERNATOR_TODO: MESSAGE:: 's body couldn't support the powers of the fountain, and collapsed.
								players[i]->entity->setObituary(Language::get(6234));
								break;
							}
							//Does not break here.
						}
						case 6:
						{
							messagePlayer(i, MESSAGE_INTERACTION, Language::get(470));
							// @JAMESERNATOR_TODO: MESSAGE:: Your body welcomes the blood and heals.
							messagePlayer(i, MESSAGE_INTERACTION, Language::get(99999));
							playSoundEntity(players[i]->entity, 52, 64);
							stats[i]->HUNGER += 100;
							players[i]->entity->modHP(30);
							break;
						}
						case 7:
						{
							messagePlayer(i, MESSAGE_INTERACTION, Language::get(470));
							// @JAMESERNATOR_TODO: MESSAGE:: The clean blood becomes one with you.
							messagePlayer(i, MESSAGE_INTERACTION, Language::get(6238));
							playSoundEntity(players[i]->entity, 52, 64);
							stats[i]->HUNGER += 100;
							players[i]->entity->modHP(100);
							break;
						}
						default:
							break;
					}
					if ( potionDropQuantity > 0 )
					{
						playSoundEntity(my, 47 + local_rng.rand() % 3, 64);
					}
					if ( potionDropQuantity > 1 )
					{
						// @JAMESERNATOR_TODO: Why is this message different?
						// @JAMESERNATOR_TODO: MESSAGE:: The fountain curses your equipment.
						messagePlayerColor(i, MESSAGE_STATUS, uint32ColorGreen, Language::get(3245), potionDropQuantity);
					}
					else if ( potionDropQuantity == 1 )
					{
						// @JAMESERNATOR_TODO: Why is this message different?
						// @JAMESERNATOR_TODO: MESSAGE:: Your body rejects the blood in pain.
						messagePlayerColor(i, MESSAGE_STATUS, uint32ColorGreen, Language::get(3246));
					}
					messagePlayer(i, MESSAGE_INTERACTION, Language::get(474));
					my->skill[0] = 0; //Dry up fountain.
					serverUpdateEntitySkill(my, 0);
					//TODO: messagePlayersInSight() instead.
				}
				//Then perform the effect randomly determined when the fountain was created.
				return;
			}
		}
	}
}
