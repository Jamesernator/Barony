#include "main.hpp"
#include "game.hpp"
#include "stat.hpp"
#include "entity.hpp"
#include "sound.hpp"
#include "items.hpp"
#include "net.hpp"
#include "monster.hpp"
#include "collision.hpp"
#include "player.hpp"
#include "magic/magic.hpp"

/*-------------------------------------------------------------------------------

act*

The following function describes an entity behavior. The function
takes a pointer to the entity that uses it as an argument.

-------------------------------------------------------------------------------*/

#define BOLTTRAP_INIT my->skill[0]
#define BOLTTRAP_SPELL my->skill[1]
#define BOLTTRAP_DIRECTION my->skill[3]

void actBoltTrap(Entity* my)
{
	if (!BOLTTRAP_INIT)
	{
		BOLTTRAP_INIT = 1;
		BOLTTRAP_SPELL = SPELL_ACID_SPRAY;
		//my->light = lightSphere(my->x / 16, my->y / 16, 3, 192);
	}

	// eliminate traps that have been destroyed.
	// check wall inside me.
	int checkx = static_cast<int>(my->x) >> 4;
	int checky = static_cast<int>(my->y) >> 4;
	if (!map.tiles[OBSTACLELAYER + checky * MAPLAYERS + checkx * MAPLAYERS * map.height])   // wall
	{
		my->removeLightField();
		list_RemoveNode(my->mynode);
		return;
	}

	if (multiplayer == CLIENT)
	{
		return;
	}

	if (my->ticks % TICKS_PER_SECOND == 0)
	{
		int oldir = 0;
		int x = 0, y = 0;
		switch (BOLTTRAP_DIRECTION)
		{
		case 0:
			x = 12;
			y = 0;
			oldir = BOLTTRAP_DIRECTION;
			BOLTTRAP_DIRECTION++;
			break;
		case 1:
			x = 0;
			y = 12;
			oldir = BOLTTRAP_DIRECTION;
			BOLTTRAP_DIRECTION++;
			break;
		case 2:
			x = -12;
			y = 0;
			oldir = BOLTTRAP_DIRECTION;
			BOLTTRAP_DIRECTION++;
			break;
		case 3:
			x = 0;
			y = -12;
			oldir = BOLTTRAP_DIRECTION;
			BOLTTRAP_DIRECTION++;
			break;
		}
		int u = std::min<int>(std::max<int>(0.0, (my->x + x) / 16), map.width - 1);
		int v = std::min<int>(std::max<int>(0.0, (my->y + y) / 16), map.height - 1);
		if (!map.tiles[OBSTACLELAYER + v * MAPLAYERS + u * MAPLAYERS * map.height])
		{
			Entity* entity = castSpell(my->getUID(), getSpellFromID(BOLTTRAP_SPELL), false, true);
			entity->x = my->x + x;
			entity->y = my->y + y;
			entity->z = my->z;
			entity->yaw = oldir * (PI / 2.f);
			double missile_speed = 4 * ((double)(((spellElement_t*)(getSpellFromID(BOLTTRAP_SPELL)->elements.first->element))->mana) / ((spellElement_t*)(getSpellFromID(BOLTTRAP_SPELL)->elements.first->element))->overload_multiplier);
			entity->vel_x = cos(entity->yaw) * (missile_speed);
			entity->vel_y = sin(entity->yaw) * (missile_speed);
		}
	}
}
