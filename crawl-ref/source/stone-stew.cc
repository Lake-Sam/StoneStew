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
#include "scroller.h"
#include "travel.h"

static const char *STONE_STEW_RELLAN_QUEST_KEY =
    "stone_stew_rellan_first_depth_quest";
static const char *STONE_STEW_BETHRA_QUEST_KEY =
    "stone_stew_bethra_road_coin_quest";
static const char *STONE_STEW_TOWN_HOME_KEY = "stone_stew_town_home";

struct stone_stew_quest_def
{
    const char *prop_key;
    const char *giver;
    const char *title;
    const char *offer;
    const char *objective;
    const char *reward;
    const char *risk;
    const char *failure;
    const char *accepted;
    const char *incomplete;
    const char *ready;
    const char *completed;
    int reward_gold;
    bool (*complete)();
};

static bool _stone_stew_rellan_complete()
{
    return you.experience_level >= 2;
}

static bool _stone_stew_bethra_complete()
{
    return you.gold >= 40;
}

static const stone_stew_quest_def STONE_STEW_QUESTS[] =
{
    {
        STONE_STEW_RELLAN_QUEST_KEY,
        "Old Rellan",
        "First Depths",
        "\"Step past the gate and survive long enough to learn something,\" "
        "Old Rellan says.",
        "Reach experience level 2, then return to Old Rellan.",
        "25 gold pieces.",
        "Low. You only need to survive ordinary D:1 exploration.",
        "None yet, but later quest types may fail.",
        "Quest accepted: reach experience level 2, then return to Old Rellan.",
        "\"Not yet,\" Old Rellan says. \"Come back once you reach experience level 2.\"",
        "\"There. Now you have heard the dungeon answer back,\" Old Rellan says.",
        "\"No more errands today,\" Old Rellan says. \"Spend that coin before it spends you.\"",
        25,
        _stone_stew_rellan_complete,
    },
    {
        STONE_STEW_BETHRA_QUEST_KEY,
        "Bethra of the Cot",
        "Road Coin",
        "\"A town does not run on warnings alone,\" Bethra says. "
        "\"Show me you can make the dungeon pay for your boots.\"",
        "Return to Bethra once you have at least 40 gold pieces.",
        "15 gold pieces.",
        "Low. Explore D:1, gather loose gold, and return when your purse is heavy enough.",
        "None yet, but later quest types may fail.",
        "Quest accepted: gather at least 40 gold pieces, then return to Bethra.",
        "\"Not enough coin-song yet,\" Bethra says. \"Come back with at least 40 gold pieces.\"",
        "\"There, you have learned the sound of survival,\" Bethra says.",
        "\"No more errands from the inn today,\" Bethra says.",
        15,
        _stone_stew_bethra_complete,
    },
};

static const int STONE_STEW_NUM_QUESTS =
    sizeof(STONE_STEW_QUESTS) / sizeof(STONE_STEW_QUESTS[0]);

bool stone_stew_is_town_npc(const monster& mon)
{
    return mon.wont_attack()
           && (mon.mname == "Mara the Coinwise"
               || mon.mname == "Old Rellan"
               || mon.mname == "Bethra of the Cot"
               || mon.mname == "Gate Warden"
               || mon.mname == "townsperson");
}

static int _stone_stew_quest_state(const stone_stew_quest_def& quest)
{
    if (!you.props.exists(quest.prop_key))
        return 0;

    return you.props[quest.prop_key].get_int();
}

static void _stone_stew_set_quest_state(const stone_stew_quest_def& quest,
                                        int state)
{
    you.props[quest.prop_key] = state;
}

static string _stone_stew_quest_offer_text(const stone_stew_quest_def& quest)
{
    string text = "<yellow>";
    text += quest.title;
    text += "</yellow>\n\n";
    text += "Giver: ";
    text += quest.giver;
    text += "\nObjective: ";
    text += quest.objective;
    text += "\nReward: ";
    text += quest.reward;
    text += "\nRisk: ";
    text += quest.risk;
    text += "\nFailure: ";
    text += quest.failure;
    text += "\n\n<lightgrey>Press a/y/Enter to accept, d/n/Esc to decline.</lightgrey>";
    return text;
}

static string _stone_stew_quest_log_entry(const stone_stew_quest_def& quest)
{
    const int state = _stone_stew_quest_state(quest);
    string text = "<yellow>";
    text += quest.title;
    text += "</yellow>\n";
    text += "Giver: ";
    text += quest.giver;
    text += "\nObjective: ";
    text += quest.objective;
    text += "\nReward: ";
    text += quest.reward;
    text += "\nRisk: ";
    text += quest.risk;
    text += "\nFailure: ";
    text += quest.failure;
    text += "\nStatus: ";

    if (state == 1)
    {
        text += quest.complete() ? "ready to turn in." : "active.";
    }
    else
        text += "completed.";

    text += "\n\n";
    return text;
}

static string _stone_stew_quest_log_text()
{
    string text = "<white>Stone Stew Quest Log</white>\n\n";
    bool found = false;

    for (int i = 0; i < STONE_STEW_NUM_QUESTS; ++i)
    {
        const stone_stew_quest_def& quest = STONE_STEW_QUESTS[i];
        if (_stone_stew_quest_state(quest) == 0)
            continue;

        text += _stone_stew_quest_log_entry(quest);
        found = true;
    }

    if (!found)
        text += "<lightgrey>No accepted quests.</lightgrey>\n\nTalk to townspeople in Stone Stew towns to find work.\n";

    return text;
}

class stone_stew_quest_offer_popup : public formatted_scroller
{
public:
    stone_stew_quest_offer_popup(const stone_stew_quest_def& quest)
        : formatted_scroller(FS_PREWRAPPED_TEXT, _stone_stew_quest_offer_text(quest))
    {
        set_tag("stone-stew-quest-offer");
        set_title(formatted_string::parse_string("<white>Quest Offer</white>"));
        set_more(formatted_string::parse_string("<lightgrey>a</lightgrey> Accept  <lightgrey>d</lightgrey> Decline"));
    }

    bool accepted() const { return m_accepted; }

private:
    maybe_bool process_key(int ch) override
    {
        const int key = toalower(ch);
        if (key == 'a' || key == 'y' || key == CK_ENTER)
        {
            m_accepted = true;
            return false;
        }

        if (key == 'd' || key == 'n' || key_is_escape(key))
        {
            m_accepted = false;
            return false;
        }

        return formatted_scroller::process_key(ch);
    }

    bool m_accepted = false;
};

static bool _stone_stew_offer_quest(const stone_stew_quest_def& quest)
{
    mpr(quest.offer);
    stone_stew_quest_offer_popup offer(quest);
    offer.show();

    if (!offer.accepted())
    {
        mpr("You decline the work for now.");
        return true;
    }

    _stone_stew_set_quest_state(quest, 1);
    mpr(quest.accepted);
    mpr("You can review accepted quests with <lightgrey>Ctrl+T</lightgrey>.");
    return true;
}

void stone_stew_display_quest_log()
{
    formatted_scroller quest_log(FS_PREWRAPPED_TEXT | FS_EASY_EXIT);
    quest_log.set_tag("stone-stew-quests");
    quest_log.set_more();
    quest_log.add_text(_stone_stew_quest_log_text());
    quest_log.show();
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
    bool has_quest = false;
    bool all_done = true;

    for (int i = 0; i < STONE_STEW_NUM_QUESTS; ++i)
    {
        const stone_stew_quest_def& quest = STONE_STEW_QUESTS[i];
        if (mon.mname != quest.giver)
            continue;

        has_quest = true;
        const int state = _stone_stew_quest_state(quest);

        if (state == 0)
            return _stone_stew_offer_quest(quest);

        if (state == 1)
        {
            all_done = false;
            if (quest.complete())
            {
                mpr(quest.ready);
                mprf("%s pays you %d gold pieces.", quest.giver, quest.reward_gold);
                you.add_gold(quest.reward_gold);
                _stone_stew_set_quest_state(quest, 2);
                return true;
            }

            mpr(quest.incomplete);
            return true;
        }

        if (state == 2)
            continue;
    }

    if (!has_quest)
        mpr("They have no work for you yet.");
    else if (all_done)
    {
        for (int i = 0; i < STONE_STEW_NUM_QUESTS; ++i)
        {
            const stone_stew_quest_def& quest = STONE_STEW_QUESTS[i];
            if (mon.mname == quest.giver)
            {
                mpr(quest.completed);
                break;
            }
        }
    }

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
