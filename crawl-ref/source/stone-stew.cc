/**
 * @file
 * @brief Stone Stew fork-specific NPC, town, and quest helpers.
**/

#include "AppHdr.h"

#include "stone-stew.h"

#include "libutil.h"
#include "macro.h"
#include "message.h"
#include "monster.h"
#include "player.h"
#include "prompt.h"
#include "travel.h"

static const char *STONE_STEW_RELLAN_QUEST_KEY =
    "stone_stew_rellan_first_depth_quest";
static const char *STONE_STEW_TOWN_HOME_KEY = "stone_stew_town_home";

bool stone_stew_is_town_npc(const monster& mon)
{
    return mon.wont_attack()
           && (mon.mname == "Mara the Coinwise"
               || mon.mname == "Old Rellan"
               || mon.mname == "Bethra of the Cot"
               || mon.mname == "Gate Warden"
               || mon.mname == "townsperson");
}

static int _stone_stew_rellan_quest_state()
{
    if (!you.props.exists(STONE_STEW_RELLAN_QUEST_KEY))
        return 0;

    return you.props[STONE_STEW_RELLAN_QUEST_KEY].get_int();
}

static void _stone_stew_show_dialogue_options()
{
    mpr("<lightgrey>1</lightgrey> Talk");
    mpr("<lightgrey>2</lightgrey> Quest");
    mpr("<lightgrey>3</lightgrey> Trade/services");
    mpr("<lightgrey>Esc</lightgrey> Leave");
}

static bool _stone_stew_town_npc_talk(const monster& mon)
{
    if (mon.mname == "Mara the Coinwise")
    {
        mpr("\"Coin spends better than blood,\" Mara says. "
            "\"Bring me a strange artefact later and I will make a fair offer.\"");
        return true;
    }

    if (mon.mname == "Old Rellan")
    {
        mpr("\"The first stairs are never the first danger,\" Old Rellan says. "
            "\"Come back when you have earned a scar or two.\"");
        return true;
    }

    if (mon.mname == "Bethra of the Cot")
    {
        mpr("\"Beds are for stories, not statistics,\" Bethra says. "
            "\"Rest easy here; the town keeps its own watch.\"");
        return true;
    }

    if (mon.mname == "Gate Warden")
    {
        mpr("\"Steel stays sheathed inside the gate,\" the warden says. "
            "\"Past it, mind your own skin.\"");
        return true;
    }

    if (mon.mname == "townsperson")
    {
        mpr("The townsperson gives you a cautious nod.");
        return true;
    }

    return false;
}

static bool _stone_stew_town_npc_quest(const monster& mon)
{
    if (mon.mname != "Old Rellan")
    {
        mpr("They have no work for you yet.");
        return true;
    }

    const int state = _stone_stew_rellan_quest_state();
    if (state == 0)
    {
        mpr("\"Step past the gate and survive long enough to learn something,\" "
            "Old Rellan says. \"Reach experience level 2, then return.\"");
        if (yesno("Accept Old Rellan's quest?", false, 'y'))
        {
            you.props[STONE_STEW_RELLAN_QUEST_KEY] = 1;
            mpr("Quest accepted: reach experience level 2, then return to Old Rellan.");
        }
        else
            mpr("You decline the work for now.");
        return true;
    }

    if (state == 1)
    {
        if (you.experience_level >= 2)
        {
            mpr("\"There. Now you have heard the dungeon answer back,\" Old Rellan says.");
            mpr("Old Rellan pays you 25 gold pieces.");
            you.add_gold(25);
            you.props[STONE_STEW_RELLAN_QUEST_KEY] = 2;
        }
        else
        {
            mpr("\"Not yet,\" Old Rellan says. \"Come back once you reach experience level 2.\"");
        }
        return true;
    }

    mpr("\"No more errands today,\" Old Rellan says. \"Spend that coin before it spends you.\"");
    return true;
}

static bool _stone_stew_town_npc_services(const monster& mon)
{
    if (mon.mname == "Mara the Coinwise")
    {
        mpr("Mara appraises your pack with professional interest.");
        mpr("Randart selling is not implemented yet.");
    }
    else if (mon.mname == "Bethra of the Cot")
        mpr("Inn services are not implemented yet.");
    else
        mpr("They have no services to offer yet.");

    return true;
}

bool stone_stew_talk_to_town_npc(monster& mon)
{
    if (!stone_stew_is_town_npc(mon))
        return false;

    stop_running();
    mprf("You speak with %s.", mon.name(DESC_THE).c_str());
    _stone_stew_show_dialogue_options();

    while (true)
    {
        const int key = getchm();
        if (key_is_escape(key) || key == ' ' || toalower(key) == 'q')
        {
            mpr("You step back from the conversation.");
            return true;
        }

        switch (key)
        {
        case '1':
        case 't':
        case 'T':
            return _stone_stew_town_npc_talk(mon);
        case '2':
        case 'Q':
            return _stone_stew_town_npc_quest(mon);
        case '3':
        case 's':
        case 'S':
            return _stone_stew_town_npc_services(mon);
        default:
            mpr("Choose 1, 2, 3, or Esc.");
            break;
        }
    }
}

bool stone_stew_town_npc_attack_warning(const monster& mon)
{
    if (!stone_stew_is_town_npc(mon))
        return false;

    if (!_stone_stew_town_npc_talk(mon))
        mprf("%s steps back, unwilling to fight.", mon.name(DESC_THE).c_str());

    return true;
}

static int _stone_stew_town_npc_roam_radius(const monster& mon)
{
    if (mon.mname == "Gate Warden")
        return 5;

    if (mon.mname == "townsperson")
        return 9;

    return 7;
}

bool stone_stew_town_npc_can_move_to(const monster& mon, const coord_def& target)
{
    if (!stone_stew_is_town_npc(mon))
        return true;

    const coord_def home = mon.props.exists(STONE_STEW_TOWN_HOME_KEY)
                           ? mon.props[STONE_STEW_TOWN_HOME_KEY].get_coord()
                           : mon.pos();

    return home.distance_from(target) <= _stone_stew_town_npc_roam_radius(mon);
}

bool stone_stew_town_npc_prepare_move(monster& mon, const coord_def& target,
                                      movement_type mvflags)
{
    if (!stone_stew_is_town_npc(mon)
        || (mvflags & MV_INTERNAL)
        || (mvflags & MV_NO_MGRID_UPDATE))
    {
        return true;
    }

    if (!mon.props.exists(STONE_STEW_TOWN_HOME_KEY))
        mon.props[STONE_STEW_TOWN_HOME_KEY].get_coord() = mon.pos();

    return stone_stew_town_npc_can_move_to(mon, target);
}
