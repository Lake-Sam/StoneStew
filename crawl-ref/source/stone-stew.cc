/**
 * @file
 * @brief Stone Stew fork-specific NPC, town, and quest helpers.
**/

#include "AppHdr.h"

#include "stone-stew.h"

#include "artefact.h"
#include "branch.h"
#include "files.h"
#include "invent.h"
#include "items.h"
#include "libutil.h"
#include "macro.h"
#include "message.h"
#include "monster.h"
#include "notes.h"
#include "player.h"
#include "prompt.h"
#include "scroller.h"
#include "shopping.h"
#include "skills.h"
#include "stringutil.h"
#include "syscalls.h"
#include "travel.h"

#include <cstdio>

static const char *STONE_STEW_RELLAN_QUEST_KEY =
    "stone_stew_rellan_first_depth_quest";
static const char *STONE_STEW_RELLAN_SECOND_QUEST_KEY =
    "stone_stew_rellan_second_depth_quest";
static const char *STONE_STEW_BETHRA_QUEST_KEY =
    "stone_stew_bethra_road_coin_quest";
static const char *STONE_STEW_BETHRA_SECOND_QUEST_KEY =
    "stone_stew_bethra_second_purse_quest";
static const char *STONE_STEW_RELLAN_FIGHTING_TRAINING_KEY =
    "stone_stew_rellan_fighting_training";
static const char *STONE_STEW_TOWN_HOME_KEY = "stone_stew_town_home";
static const char *STONE_STEW_MID_DUNGEON_TOWN_NAME_KEY =
    "stone_stew_mid_dungeon_town_name";
static const char *STONE_STEW_LAIR_TOWN_NAME_KEY =
    "stone_stew_lair_town_name";
static const char *STONE_STEW_DUNGEON_TOWN_GUILD_KEY =
    "stone_stew_dungeon_town_guild";
static const char *STONE_STEW_LAIR_TOWN_GUILD_KEY =
    "stone_stew_lair_town_guild";
static const char *STONE_STEW_DUNGEON_TOWN_GUILD_JOINED_KEY =
    "stone_stew_dungeon_town_guild_joined";
static const char *STONE_STEW_LAIR_TOWN_GUILD_JOINED_KEY =
    "stone_stew_lair_town_guild_joined";
static const char *STONE_STEW_DUNGEON_TOWN_SURVEY_KEY =
    "stone_stew_dungeon_town_survey";
static const char *STONE_STEW_LAIR_TOWN_SURVEY_KEY =
    "stone_stew_lair_town_survey";
static const char *STONE_STEW_DUNGEON_TOWN_GUILD_RANK_KEY =
    "stone_stew_dungeon_town_guild_rank";
static const char *STONE_STEW_LAIR_TOWN_GUILD_RANK_KEY =
    "stone_stew_lair_town_guild_rank";
static const char *STONE_STEW_DUNGEON_TOWN_GUILD_BENEFIT_KEY =
    "stone_stew_dungeon_town_guild_benefit";
static const char *STONE_STEW_LAIR_TOWN_GUILD_BENEFIT_KEY =
    "stone_stew_lair_town_guild_benefit";
static const char *STONE_STEW_DUNGEON_WARDEN_FIGHTING_KEY =
    "stone_stew_dungeon_warden_fighting_training";
static const char *STONE_STEW_DUNGEON_BROKER_ARMOUR_KEY =
    "stone_stew_dungeon_broker_armour_training";
static const char *STONE_STEW_LAIR_HUNTER_DODGING_KEY =
    "stone_stew_lair_hunter_dodging_training";
static const char *STONE_STEW_LAIR_HEALER_STEALTH_KEY =
    "stone_stew_lair_healer_stealth_training";
static const char *STONE_STEW_TOWN_MORAL_TEMPLATE_KEY =
    "stone_stew_town_moral_template";
static const char *STONE_STEW_TOWN_MORAL_BRANCH_KEY =
    "stone_stew_town_moral_branch";
static const char *STONE_STEW_TOWN_MORAL_STATE_KEY =
    "stone_stew_town_moral_state";
static const char *STONE_STEW_TOWN_MORAL_TARGET_XL_KEY =
    "stone_stew_town_moral_target_xl";
static const char *STONE_STEW_TOWN_MORAL_OUTCOME_KEY =
    "stone_stew_town_moral_outcome";
static const char *STONE_STEW_TOWN_MORAL_IDENTITY_NAME_KEY =
    "stone_stew_town_moral_identity_name";
static const char *STONE_STEW_TOWN_MORAL_IDENTITY_ROLE_KEY =
    "stone_stew_town_moral_identity_role";
static const char *STONE_STEW_TOWN_MORAL_IDENTITY_TRAIT_KEY =
    "stone_stew_town_moral_identity_trait";
static const char *STONE_STEW_TOWN_MORAL_IDENTITY_PRESSURE_KEY =
    "stone_stew_town_moral_identity_pressure";
static const char *STONE_STEW_TOWN_MORAL_IDENTITY_VOICE_KEY =
    "stone_stew_town_moral_identity_voice";
static const char *STONE_STEW_LLM_NOTICE_KEY =
    "stone_stew_llm_notice";
static const int STONE_STEW_RELLAN_FIGHTING_TRAINING_COST = 60;
static const int STONE_STEW_RELLAN_FIGHTING_TRAINING_POINTS = 180;
static const int STONE_STEW_GUILD_BENEFIT_SKILL_POINTS = 220;

enum stone_stew_quest_state
{
    SSQ_UNOFFERED,
    SSQ_ACTIVE,
    SSQ_COMPLETED,
    SSQ_FAILED,
};

struct stone_stew_quest_def
{
    const char *prop_key;
    const char *prereq_key;
    int prereq_state;
    const char *giver;
    const char *legacy_giver;
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
    const char *failed;
    int reward_gold;
    bool (*complete)();
    bool (*fail)();
};

struct stone_stew_guild_def
{
    const char *name;
    const char *focus;
    const char *theme;
    const char *join_pitch;
};

struct stone_stew_guild_survey_def
{
    const char *prop_key;
    branch_type target_branch;
    int target_depth;
    const char *title;
    const char *offer;
    const char *accepted;
    const char *incomplete;
    const char *ready;
    const char *completed;
    int reward_gold;
};

struct stone_stew_training_def
{
    const char *prop_key;
    const char *trainer;
    branch_type branch;
    skill_type skill;
    const char *label;
    int cost;
    int skill_points;
    const char *offer;
    const char *declined;
    const char *poor;
    const char *complete;
};

struct stone_stew_moral_choice
{
    string label;
    string player_line;
    string result;
    int reward_mod = 0;
};

struct stone_stew_moral_template
{
    string id;
    string branches = "any";
    string giver;
    string title;
    string offer;
    string objective;
    string reward;
    string risk;
    string failure;
    string accepted;
    string incomplete;
    int target_xl_delta = 1;
    int base_gold = 20;
    int gold_per_xl = 5;
    stone_stew_moral_choice choices[3];
};

struct stone_stew_npc_identity
{
    string name;
    string role;
    string trait;
    string pressure;
    string voice;
};

static string _stone_stew_current_town_name();

static bool _stone_stew_rellan_complete()
{
    return you.experience_level >= 2;
}

static bool _stone_stew_rellan_second_complete()
{
    return you.experience_level >= 3;
}

static bool _stone_stew_bethra_complete()
{
    return you.gold >= 40;
}

static bool _stone_stew_bethra_second_complete()
{
    return you.gold >= 75;
}

static const stone_stew_training_def STONE_STEW_TRAINING[] =
{
    {
        STONE_STEW_DUNGEON_WARDEN_FIGHTING_KEY,
        "Lantern Warden",
        BRANCH_DWARF,
        SK_FIGHTING,
        "Fighting",
        120,
        220,
        "\"Road stance, shield shoulder, panic breath,\" the warden says. "
        "\"I can drill the basics into you.\"",
        "\"Keep your coin, then. Keep your balance too.\"",
        "\"Training costs 120 gold,\" the warden says. \"Come back solvent.\"",
        "\"Better,\" the warden says. \"The road hits back. Hit first.\"",
    },
    {
        STONE_STEW_DUNGEON_BROKER_ARMOUR_KEY,
        "Town Broker",
        BRANCH_DWARF,
        SK_ARMOUR,
        "Armour",
        140,
        220,
        "\"Most people wear armour like a locked door,\" the broker says. "
        "\"Pay me and I will show you where it hinges.\"",
        "\"No fitting, no fee,\" the broker says.",
        "\"A proper fitting costs 140 gold,\" the broker says.",
        "\"There,\" the broker says. \"Less clatter, fewer bruises.\"",
    },
    {
        STONE_STEW_LAIR_HUNTER_DODGING_KEY,
        "Root Hunter",
        BRANCH_FOREST,
        SK_DODGING,
        "Dodging",
        160,
        240,
        "\"Roots trip the proud,\" the hunter says. \"Pay me and I will teach "
        "your feet to listen.\"",
        "\"The roots can wait,\" the hunter says.",
        "\"Trailwork costs 160 gold,\" the hunter says.",
        "\"Good. You moved before the branch asked twice.\"",
    },
    {
        STONE_STEW_LAIR_HEALER_STEALTH_KEY,
        "Lair Healer",
        BRANCH_FOREST,
        SK_STEALTH,
        "Stealth",
        150,
        230,
        "\"Quiet bodies need fewer stitches,\" the healer says. "
        "\"I can teach you how not to announce your wounds.\"",
        "\"Noise remains free,\" the healer says.",
        "\"Quiet instruction costs 150 gold,\" the healer says.",
        "\"Softer steps. Fewer salves. A fair trade.\"",
    },
};

static const int STONE_STEW_NUM_TRAINING =
    static_cast<int>(ARRAYSZ(STONE_STEW_TRAINING));

static const stone_stew_quest_def STONE_STEW_QUESTS[] =
{
    {
        STONE_STEW_RELLAN_QUEST_KEY,
        nullptr,
        SSQ_UNOFFERED,
        "Old Rellan",
        nullptr,
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
        "\"Too late for that lesson now,\" Old Rellan says.",
        25,
        _stone_stew_rellan_complete,
        nullptr,
    },
    {
        STONE_STEW_RELLAN_SECOND_QUEST_KEY,
        STONE_STEW_RELLAN_QUEST_KEY,
        SSQ_COMPLETED,
        "Old Rellan",
        nullptr,
        "Second Footing",
        "\"The dungeon has noticed you now,\" Old Rellan says. "
        "\"Learn whether your feet still obey when it pushes back.\"",
        "Reach experience level 3, then return to Old Rellan.",
        "40 gold pieces.",
        "Low to moderate. You may need to explore beyond the safest rooms.",
        "None yet, but later quest types may fail.",
        "Quest accepted: reach experience level 3, then return to Old Rellan.",
        "\"Still too green,\" Old Rellan says. \"Come back once you reach experience level 3.\"",
        "\"Good. Fear is quieter when it has a name,\" Old Rellan says.",
        "\"I have taught you what I can from this gate,\" Old Rellan says.",
        "\"That lesson has passed you by,\" Old Rellan says.",
        40,
        _stone_stew_rellan_second_complete,
        nullptr,
    },
    {
        STONE_STEW_BETHRA_QUEST_KEY,
        nullptr,
        SSQ_UNOFFERED,
        "Bertha of the Cot",
        "Bethra of the Cot",
        "Road Coin",
        "\"A town does not run on warnings alone,\" Bertha says. "
        "\"Show me you can make the dungeon pay for your boots.\"",
        "Return to Bertha once you have at least 40 gold pieces.",
        "15 gold pieces.",
        "Low. Explore D:1, gather loose gold, and return when your purse is heavy enough.",
        "None yet, but later quest types may fail.",
        "Quest accepted: gather at least 40 gold pieces, then return to Bertha.",
        "\"Not enough coin-song yet,\" Bertha says. \"Come back with at least 40 gold pieces.\"",
        "\"There, you have learned the sound of survival,\" Bertha says.",
        "\"No more errands from the inn today,\" Bertha says.",
        "\"That road has gone cold,\" Bertha says.",
        15,
        _stone_stew_bethra_complete,
        nullptr,
    },
    {
        STONE_STEW_BETHRA_SECOND_QUEST_KEY,
        STONE_STEW_BETHRA_QUEST_KEY,
        SSQ_COMPLETED,
        "Bertha of the Cot",
        "Bethra of the Cot",
        "Heavy Purse",
        "\"Coin is not safety,\" Bertha says, \"but it buys blankets, boots, "
        "and the sort of soup that remembers you.\"",
        "Return to Bertha once you have at least 75 gold pieces.",
        "35 gold pieces.",
        "Low. This rewards steady early exploration and restraint.",
        "None yet, but later quest types may fail.",
        "Quest accepted: gather at least 75 gold pieces, then return to Bertha.",
        "\"That purse still whispers,\" Bertha says. \"Bring me at least 75 gold pieces.\"",
        "\"There it is. A purse with a spine,\" Bertha says.",
        "\"No more purse-work today,\" Bertha says. \"Go spend wisely.\"",
        "\"That purse-work is past saving,\" Bertha says.",
        35,
        _stone_stew_bethra_second_complete,
        nullptr,
    },
};

static const int STONE_STEW_NUM_QUESTS =
    sizeof(STONE_STEW_QUESTS) / sizeof(STONE_STEW_QUESTS[0]);

static string _stone_stew_town_moral_key(const char *base)
{
    return string(base) + "_" + make_stringf("%d", you.where_are_you);
}

static string _stone_stew_pick_identity_field(const char *base_key,
                                             const char **values,
                                             int count)
{
    const string key = _stone_stew_town_moral_key(base_key);
    if (!you.props.exists(key))
        you.props[key] = values[random2(count)];

    return you.props[key].get_string();
}

static stone_stew_npc_identity _stone_stew_current_moral_identity()
{
    static const char *starter_names[] =
    {
        "Mavren", "Sella", "Odrik", "Tamsin", "Veyra", "Corren"
    };
    static const char *dungeon_names[] =
    {
        "Brann", "Istre", "Kelro", "Marn", "Ossin", "Velka"
    };
    static const char *lair_names[] =
    {
        "Aster", "Brynn", "Elow", "Mirel", "Sorrel", "Thane"
    };
    static const char *starter_roles[] =
    {
        "gate neighbor", "inn witness", "market runner", "well keeper"
    };
    static const char *dungeon_roles[] =
    {
        "ledger keeper", "lamp tender", "road clerk", "watch witness"
    };
    static const char *lair_roles[] =
    {
        "root gatherer", "trail witness", "salve mixer", "beast-path caller"
    };
    static const char *traits[] =
    {
        "too honest for comfort", "fond of bad jokes", "careful with names",
        "nervous around easy answers", "polite when frightened"
    };
    static const char *pressures[] =
    {
        "owes a favor to someone in town",
        "has heard the same lie from both sides",
        "wants the matter settled before blood follows",
        "fears the town will pretend nothing happened"
    };
    static const char *voices[] =
    {
        "dry and practical", "warm but worried", "ceremonial by habit",
        "blunt, then apologetic", "quietly amused"
    };

    const char **names = starter_names;
    int num_names = ARRAYSZ(starter_names);
    const char **roles = starter_roles;
    int num_roles = ARRAYSZ(starter_roles);

    if (you.where_are_you == BRANCH_DWARF)
    {
        names = dungeon_names;
        num_names = ARRAYSZ(dungeon_names);
        roles = dungeon_roles;
        num_roles = ARRAYSZ(dungeon_roles);
    }
    else if (you.where_are_you == BRANCH_FOREST)
    {
        names = lair_names;
        num_names = ARRAYSZ(lair_names);
        roles = lair_roles;
        num_roles = ARRAYSZ(lair_roles);
    }

    stone_stew_npc_identity identity;
    identity.name = _stone_stew_pick_identity_field(
        STONE_STEW_TOWN_MORAL_IDENTITY_NAME_KEY, names, num_names);
    identity.role = _stone_stew_pick_identity_field(
        STONE_STEW_TOWN_MORAL_IDENTITY_ROLE_KEY, roles, num_roles);
    identity.trait = _stone_stew_pick_identity_field(
        STONE_STEW_TOWN_MORAL_IDENTITY_TRAIT_KEY, traits, ARRAYSZ(traits));
    identity.pressure = _stone_stew_pick_identity_field(
        STONE_STEW_TOWN_MORAL_IDENTITY_PRESSURE_KEY, pressures,
        ARRAYSZ(pressures));
    identity.voice = _stone_stew_pick_identity_field(
        STONE_STEW_TOWN_MORAL_IDENTITY_VOICE_KEY, voices, ARRAYSZ(voices));
    return identity;
}

static string _stone_stew_substitute(string text)
{
    const stone_stew_npc_identity identity =
        _stone_stew_current_moral_identity();
    text = replace_all(text, "{town}", _stone_stew_current_town_name());
    text = replace_all(text, "{branch}", branches[you.where_are_you].longname);
    text = replace_all(text, "{xl}", make_stringf("%d", you.experience_level));
    text = replace_all(text, "{npc_name}", identity.name);
    text = replace_all(text, "{npc_role}", identity.role);
    text = replace_all(text, "{npc_trait}", identity.trait);
    text = replace_all(text, "{npc_pressure}", identity.pressure);
    text = replace_all(text, "{npc_voice}", identity.voice);
    return text;
}

static string _stone_stew_current_town_tag()
{
    if (you.where_are_you == BRANCH_DWARF)
        return "dungeon_town";

    if (you.where_are_you == BRANCH_FOREST)
        return "lair_town";

    return "starter";
}

static bool _stone_stew_template_matches_current_town(
    const stone_stew_moral_template& tmpl)
{
    const vector<string> tags = split_string(",", tmpl.branches, true);
    const string current = _stone_stew_current_town_tag();
    for (const string& tag : tags)
        if (tag == "any" || tag == current)
            return true;

    return false;
}

static string _stone_stew_unescape(string text)
{
    text = replace_all(text, "\\n", "\n");
    text = replace_all(text, "\\\"", "\"");
    text = replace_all(text, "\\\\", "\\");
    return text;
}

static void _stone_stew_set_template_field(stone_stew_moral_template& tmpl,
                                           const string& key,
                                           const string& value)
{
    if (key == "id")
        tmpl.id = value;
    else if (key == "branches")
        tmpl.branches = value.empty() ? "any" : value;
    else if (key == "giver")
        tmpl.giver = value;
    else if (key == "title")
        tmpl.title = value;
    else if (key == "offer")
        tmpl.offer = value;
    else if (key == "objective")
        tmpl.objective = value;
    else if (key == "reward")
        tmpl.reward = value;
    else if (key == "risk")
        tmpl.risk = value;
    else if (key == "failure")
        tmpl.failure = value;
    else if (key == "accepted")
        tmpl.accepted = value;
    else if (key == "incomplete")
        tmpl.incomplete = value;
    else if (key == "target_xl_delta")
        tmpl.target_xl_delta = max(1, atoi(value.c_str()));
    else if (key == "base_gold")
        tmpl.base_gold = max(0, atoi(value.c_str()));
    else if (key == "gold_per_xl")
        tmpl.gold_per_xl = max(0, atoi(value.c_str()));
    else if (starts_with(key, "choice"))
    {
        const int choice = key.size() > 6 ? key[6] - '1' : -1;
        if (choice < 0 || choice >= 3)
            return;

        const string suffix = key.size() > 8 ? key.substr(8) : "";
        if (suffix == "label")
            tmpl.choices[choice].label = value;
        else if (suffix == "player")
            tmpl.choices[choice].player_line = value;
        else if (suffix == "result")
            tmpl.choices[choice].result = value;
        else if (suffix == "reward_mod")
            tmpl.choices[choice].reward_mod = atoi(value.c_str());
    }
}

static bool _stone_stew_template_complete(
    const stone_stew_moral_template& tmpl)
{
    if (tmpl.id.empty() || tmpl.giver.empty() || tmpl.title.empty()
        || tmpl.offer.empty() || tmpl.objective.empty())
    {
        return false;
    }

    for (int i = 0; i < 3; ++i)
    {
        if (tmpl.choices[i].label.empty()
            || tmpl.choices[i].player_line.empty()
            || tmpl.choices[i].result.empty())
        {
            return false;
        }
    }

    return true;
}

static vector<stone_stew_moral_template> _stone_stew_load_moral_templates()
{
    vector<stone_stew_moral_template> templates;
    const string path = datafile_path("stone-stew/quest_templates.txt", false);
    FILE *fp = fopen_u(path.c_str(), "r");
    if (!fp)
        return templates;

    stone_stew_moral_template current;
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), fp))
    {
        string line = trimmed_string(buffer);
        if (line.empty() || line[0] == '#')
            continue;

        if (line == "---")
        {
            if (_stone_stew_template_complete(current))
                templates.push_back(current);
            current = stone_stew_moral_template();
            continue;
        }

        const string::size_type split = line.find('=');
        if (split == string::npos)
            continue;

        string key = trimmed_string(line.substr(0, split));
        string value = trimmed_string(line.substr(split + 1));
        _stone_stew_set_template_field(current, key,
                                       _stone_stew_unescape(value));
    }

    if (_stone_stew_template_complete(current))
        templates.push_back(current);

    fclose(fp);
    return templates;
}

static const stone_stew_moral_template *_stone_stew_current_moral_template()
{
    static vector<stone_stew_moral_template> templates =
        _stone_stew_load_moral_templates();
    static stone_stew_moral_template fallback;
    if (fallback.id.empty())
    {
        fallback.id = "fallback_moral_cache";
        fallback.branches = "any";
        fallback.giver = "townsperson";
        fallback.title = "A Borrowed Mercy";
        fallback.offer = "\"Two neighbors claim the same hidden cache,\" "
                         "the townsperson says. \"One needs it, one earned "
                         "it, and both have witnesses who lie.\"";
        fallback.objective = "Reach the target experience level, then return "
                             "to judge the cache.";
        fallback.reward = "Scaled gold based on your level and chosen "
                          "resolution.";
        fallback.risk = "Moderate. This asks you to survive more of the local "
                        "branch.";
        fallback.failure = "If the giver dies, the town problem dies with "
                           "them.";
        fallback.accepted = "You agree to return once the dungeon has made "
                            "your name heavier.";
        fallback.incomplete = "\"Not yet,\" the townsperson says. \"Come back "
                              "after the road has tested your judgment.\"";
        fallback.target_xl_delta = 1;
        fallback.base_gold = 20;
        fallback.gold_per_xl = 5;
        fallback.choices[0].label = "Give it to the needy claimant.";
        fallback.choices[0].player_line = "\"Need outweighs ledgers today.\"";
        fallback.choices[0].result = "The poor claimant weeps with relief. "
                                     "The other spits your name into the dust.";
        fallback.choices[0].reward_mod = -5;
        fallback.choices[1].label = "Give it to the legal claimant.";
        fallback.choices[1].player_line = "\"Proof matters, even when mercy "
                                          "complains.\"";
        fallback.choices[1].result = "The lawful claimant bows once. The "
                                     "hungry one leaves town before sunset.";
        fallback.choices[1].reward_mod = 0;
        fallback.choices[2].label = "Split the cache and shame them both.";
        fallback.choices[2].player_line = "\"You both brought lies to a "
                                          "starving table.\"";
        fallback.choices[2].result = "Neither claimant thanks you, which may "
                                     "be the closest this town comes to "
                                     "justice.";
        fallback.choices[2].reward_mod = 5;
    }

    if (templates.empty())
        return &fallback;

    vector<int> candidates;
    for (int i = 0; i < static_cast<int>(templates.size()); ++i)
        if (_stone_stew_template_matches_current_town(templates[i]))
            candidates.push_back(i);

    if (candidates.empty())
        return &fallback;

    const int branch = static_cast<int>(you.where_are_you);
    const string template_key =
        _stone_stew_town_moral_key(STONE_STEW_TOWN_MORAL_TEMPLATE_KEY);
    const string branch_key =
        _stone_stew_town_moral_key(STONE_STEW_TOWN_MORAL_BRANCH_KEY);
    const bool needs_pick = !you.props.exists(template_key)
                            || !you.props.exists(branch_key)
                            || you.props[branch_key].get_int()
                               != branch;
    if (needs_pick)
    {
        you.props[template_key] = candidates[random2(candidates.size())];
        you.props[branch_key] = branch;
    }

    int index = you.props[template_key].get_int();
    if (index < 0
        || index >= static_cast<int>(templates.size())
        || !_stone_stew_template_matches_current_town(templates[index]))
    {
        index = candidates[random2(candidates.size())];
        you.props[template_key] = index;
    }

    return &templates[index];
}

static int _stone_stew_town_moral_state()
{
    const string key = _stone_stew_town_moral_key(STONE_STEW_TOWN_MORAL_STATE_KEY);
    if (!you.props.exists(key))
        return SSQ_UNOFFERED;

    return you.props[key].get_int();
}

static void _stone_stew_set_town_moral_state(int state)
{
    you.props[_stone_stew_town_moral_key(STONE_STEW_TOWN_MORAL_STATE_KEY)] =
        state;
}

static int _stone_stew_town_moral_target_xl(
    const stone_stew_moral_template& tmpl)
{
    const string key =
        _stone_stew_town_moral_key(STONE_STEW_TOWN_MORAL_TARGET_XL_KEY);
    if (!you.props.exists(key))
    {
        you.props[key] = max(2, you.experience_level + tmpl.target_xl_delta);
    }

    return you.props[key].get_int();
}

static int _stone_stew_town_moral_reward(
    const stone_stew_moral_template& tmpl, int choice)
{
    const int target_xl = _stone_stew_town_moral_target_xl(tmpl);
    const int modifier = choice >= 0 && choice < 3
                         ? tmpl.choices[choice].reward_mod
                         : 0;
    return max(0, tmpl.base_gold + target_xl * tmpl.gold_per_xl + modifier);
}

static string _stone_stew_json_escape(const string& text)
{
    string out;
    for (char c : text)
    {
        switch (c)
        {
        case '\\':
            out += "\\\\";
            break;
        case '"':
            out += "\\\"";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            break;
        default:
            out += c;
            break;
        }
    }
    return out;
}

static string _stone_stew_json_unescape(string text)
{
    string out;
    bool escaped = false;
    for (char c : text)
    {
        if (escaped)
        {
            if (c == 'n')
                out += '\n';
            else
                out += c;
            escaped = false;
        }
        else if (c == '\\')
            escaped = true;
        else
            out += c;
    }
    return out;
}

static string _stone_stew_extract_json_response(const string& json)
{
    const string needle = "\"response\":\"";
    string::size_type start = json.find(needle);
    if (start == string::npos)
        return "";

    start += needle.size();
    string response;
    bool escaped = false;
    for (string::size_type i = start; i < json.size(); ++i)
    {
        const char c = json[i];
        if (escaped)
        {
            response += '\\';
            response += c;
            escaped = false;
        }
        else if (c == '\\')
            escaped = true;
        else if (c == '"')
            break;
        else
            response += c;
    }

    return trimmed_string(_stone_stew_json_unescape(response));
}

static string _stone_stew_sanitise_llm_line(string line)
{
    line = replace_all(line, "\n", " ");
    line = replace_all(line, "\r", " ");
    line = trimmed_string(line);
    if (line.size() > 180)
        line = line.substr(0, 177) + "...";

    return line;
}

static string _stone_stew_llm_flavour(const monster& mon, const string& topic)
{
    const stone_stew_npc_identity identity =
        _stone_stew_current_moral_identity();
    const string npc_name = mon.mname == "townsperson"
                            ? identity.name
                            : mon.name(DESC_PLAIN);
    const string prompt = make_stringf(
        "Write one short in-character line for a Dungeon Crawl Stone Soup fork "
        "NPC. NPC name: %s. Town: %s. Topic: %s. Allowed facts only: towns, "
        "guilds, priests, quests, shops, training, gold, artefacts, Dungeon, "
        "Lair, and the known DCSS gods. Do not invent mechanics. Keep under "
        "24 words.",
        npc_name.c_str(), _stone_stew_current_town_name().c_str(),
        topic.c_str());

    const string body = "{\"model\":\"qwen3:1.7b\",\"stream\":false,"
                        "\"options\":{\"think\":false},\"prompt\":\""
                        + _stone_stew_json_escape(prompt) + "\"}";
    const string command = "curl --silent --max-time 3 "
                           "-H \"Content-Type: application/json\" "
                           "-d \"" + _stone_stew_json_escape(body) + "\" "
                           "http://127.0.0.1:11434/api/generate";

#ifdef WIN32
    FILE *pipe = _popen(command.c_str(), "r");
#else
    FILE *pipe = popen(command.c_str(), "r");
#endif
    if (!pipe)
        return "";

    string output;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), pipe))
        output += buffer;

#ifdef WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif

    return _stone_stew_sanitise_llm_line(
        _stone_stew_extract_json_response(output));
}

static void _stone_stew_maybe_print_llm_flavour(const monster& mon,
                                                const string& topic)
{
    const string line = _stone_stew_llm_flavour(mon, topic);
    if (!line.empty())
        mprf("%s says, \"%s\"", mon.name(DESC_THE).c_str(), line.c_str());
    else if (!you.props.exists(STONE_STEW_LLM_NOTICE_KEY))
    {
        you.props[STONE_STEW_LLM_NOTICE_KEY] = true;
        mpr("<darkgrey>Stone Stew LLM fallback: no localhost Ollama response.</darkgrey>");
    }
}

static string _stone_stew_moral_offer_text(
    const stone_stew_moral_template& tmpl)
{
    const int target_xl = _stone_stew_town_moral_target_xl(tmpl);
    string text = "<yellow>";
    text += _stone_stew_substitute(tmpl.title);
    text += "</yellow>\n\n";
    text += _stone_stew_substitute(tmpl.offer);
    text += "\n\nGiver: ";
    text += tmpl.giver == "townsperson"
            ? _stone_stew_current_moral_identity().name
            : tmpl.giver;
    if (tmpl.giver == "townsperson")
    {
        const stone_stew_npc_identity identity =
            _stone_stew_current_moral_identity();
        text += ", ";
        text += identity.role;
        text += "\nNPC: ";
        text += identity.trait;
        text += "; ";
        text += identity.pressure;
        text += ". Voice: ";
        text += identity.voice;
        text += ".";
    }
    text += "\nObjective: ";
    text += replace_all(_stone_stew_substitute(tmpl.objective),
                        "{target_xl}", make_stringf("%d", target_xl));
    text += "\nReward: ";
    text += _stone_stew_substitute(tmpl.reward);
    text += "\nRisk: ";
    text += _stone_stew_substitute(tmpl.risk);
    text += "\nFailure: ";
    text += _stone_stew_substitute(tmpl.failure);
    text += "\n\n<lightgrey>Press a/y/Enter to accept, d/n/Esc to decline.</lightgrey>";
    return text;
}

class stone_stew_moral_offer_popup : public formatted_scroller
{
public:
    stone_stew_moral_offer_popup(const stone_stew_moral_template& tmpl)
        : formatted_scroller(FS_PREWRAPPED_TEXT,
                             _stone_stew_moral_offer_text(tmpl))
    {
        set_tag("stone-stew-moral-offer");
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

static int _stone_stew_choose_moral_outcome(
    const stone_stew_moral_template& tmpl)
{
    mprf("<yellow>%s</yellow>", _stone_stew_substitute(tmpl.title).c_str());
    for (int i = 0; i < 3; ++i)
    {
        mprf("<lightgrey>%d</lightgrey> %s",
             i + 1, _stone_stew_substitute(tmpl.choices[i].label).c_str());
    }
    mpr("<lightgrey>Esc</lightgrey> Decide later");

    while (true)
    {
        const int key = getchm();
        if (key_is_escape(key) || key == ' ')
            return -1;

        if (key >= '1' && key <= '3')
            return key - '1';

        mpr("Choose one of the three responses, or Esc.");
    }
}

static string _stone_stew_moral_log_entry(
    const stone_stew_moral_template& tmpl)
{
    const int state = _stone_stew_town_moral_state();
    if (state != SSQ_ACTIVE)
        return "";

    const int target_xl = _stone_stew_town_moral_target_xl(tmpl);
    string text = "<yellow>";
    text += _stone_stew_substitute(tmpl.title);
    text += "</yellow>\n";
    text += "Giver: ";
    text += tmpl.giver == "townsperson"
            ? _stone_stew_current_moral_identity().name
            : tmpl.giver;
    text += " in ";
    text += _stone_stew_current_town_name();
    text += "\nObjective: ";
    text += replace_all(_stone_stew_substitute(tmpl.objective),
                        "{target_xl}", make_stringf("%d", target_xl));
    text += "\nReward: ";
    text += _stone_stew_substitute(tmpl.reward);
    text += "\nRisk: ";
    text += _stone_stew_substitute(tmpl.risk);
    text += "\nStatus: ";
    text += you.experience_level >= target_xl ? "ready for judgment."
                                              : "active.";
    text += "\n\n";
    return text;
}

static bool _stone_stew_townsperson_moral_quest(const monster& mon)
{
    const stone_stew_moral_template *tmpl =
        _stone_stew_current_moral_template();
    if (!tmpl || mon.mname != tmpl->giver)
        return false;

    const int state = _stone_stew_town_moral_state();
    if (state == SSQ_UNOFFERED)
    {
        _stone_stew_maybe_print_llm_flavour(mon, "offering a local moral problem");
        stone_stew_moral_offer_popup offer(*tmpl);
        offer.show();

        if (!offer.accepted())
        {
            mpr("You decline the work for now.");
            return true;
        }

        _stone_stew_town_moral_target_xl(*tmpl);
        _stone_stew_set_town_moral_state(SSQ_ACTIVE);
        mpr(_stone_stew_substitute(tmpl->accepted));
        mpr("You can review accepted quests with <lightgrey>Ctrl+T</lightgrey>.");
        return true;
    }

    if (state == SSQ_ACTIVE)
    {
        const int target_xl = _stone_stew_town_moral_target_xl(*tmpl);
        if (you.experience_level < target_xl)
        {
            string incomplete = replace_all(_stone_stew_substitute(tmpl->incomplete),
                                            "{target_xl}",
                                            make_stringf("%d", target_xl));
            mpr(incomplete);
            return true;
        }

        _stone_stew_maybe_print_llm_flavour(mon, "asking the player to judge a local dispute");
        const int choice = _stone_stew_choose_moral_outcome(*tmpl);
        if (choice < 0)
        {
            mpr("You leave the dispute unresolved for now.");
            return true;
        }

        mpr(_stone_stew_substitute(tmpl->choices[choice].player_line));
        mpr(_stone_stew_substitute(tmpl->choices[choice].result));
        const int reward = _stone_stew_town_moral_reward(*tmpl, choice);
        if (reward > 0)
        {
            mprf("The town pays you %d gold pieces.", reward);
            you.add_gold(reward);
        }
        you.props[_stone_stew_town_moral_key(STONE_STEW_TOWN_MORAL_OUTCOME_KEY)]
            = choice;
        _stone_stew_set_town_moral_state(SSQ_COMPLETED);
        take_note(Note(NOTE_USER_NOTE, 0, 0, "",
                       make_stringf("Resolved %s in %s.",
                                    tmpl->title.c_str(),
                                    _stone_stew_current_town_name().c_str())),
                  true);
        return true;
    }

    mpr("This town's matter has already found its ending.");
    return true;
}

static bool _stone_stew_townsperson_moral_followup()
{
    if (_stone_stew_town_moral_state() != SSQ_COMPLETED
        || !you.props.exists(_stone_stew_town_moral_key(
            STONE_STEW_TOWN_MORAL_OUTCOME_KEY)))
    {
        return false;
    }

    const int choice = you.props[_stone_stew_town_moral_key(
        STONE_STEW_TOWN_MORAL_OUTCOME_KEY)].get_int();
    const stone_stew_npc_identity identity =
        _stone_stew_current_moral_identity();

    if (choice == 0)
    {
        mprf("\"Mercy has a long shadow,\" %s the %s says. "
             "\"Some sleep warmer for it. Some do not sleep at all.\"",
             identity.name.c_str(), identity.role.c_str());
    }
    else if (choice == 1)
    {
        mprf("\"Law is a cold blanket,\" %s the %s says. "
             "\"Still, cold blankets keep some people alive.\"",
             identity.name.c_str(), identity.role.c_str());
    }
    else if (choice == 2)
    {
        mprf("\"No one forgave you,\" %s the %s says. "
             "\"That may be how I know you judged fairly.\"",
             identity.name.c_str(), identity.role.c_str());
    }
    else
    {
        mprf("%s the %s watches the town with changed eyes.",
             identity.name.c_str(), identity.role.c_str());
    }

    return true;
}

static string _stone_stew_make_town_name(const char *prop_key,
                                         const char **prefixes,
                                         int num_prefixes,
                                         const char **suffixes,
                                         int num_suffixes)
{
    if (!you.props.exists(prop_key))
    {
        const string prefix = prefixes[random2(num_prefixes)];
        const string suffix = suffixes[random2(num_suffixes)];
        const bool possessive = prefix.size() >= 2
                                && prefix.substr(prefix.size() - 2) == "'s";
        you.props[prop_key] = possessive ? prefix + " " + suffix
                                         : prefix + suffix;
    }

    return you.props[prop_key].get_string();
}

static string _stone_stew_mid_dungeon_town_name()
{
    static const char *prefixes[] =
    {
        "Moss", "Copper", "Ash", "Lantern", "Root", "Brine",
        "Ember", "Stone", "Nerul's", "Grey"
    };
    static const char *suffixes[] =
    {
        "gate", "hall", "Rest", "Stair", "Market", "Well",
        "Hearth", "Crossing", "Watch", "Haven"
    };

    return _stone_stew_make_town_name(STONE_STEW_MID_DUNGEON_TOWN_NAME_KEY,
                                      prefixes, ARRAYSZ(prefixes),
                                      suffixes, ARRAYSZ(suffixes));
}

static string _stone_stew_lair_town_name()
{
    static const char *prefixes[] =
    {
        "Moss", "Fern", "Root", "Green", "Thorn", "Rain",
        "Bark", "Willow", "Hunter's", "Deep"
    };
    static const char *suffixes[] =
    {
        "gate", "well", "watch", "hollow", "rest", "den",
        "hearth", "grove", "stand", "shade"
    };

    return _stone_stew_make_town_name(STONE_STEW_LAIR_TOWN_NAME_KEY,
                                      prefixes, ARRAYSZ(prefixes),
                                      suffixes, ARRAYSZ(suffixes));
}

static string _stone_stew_current_town_name()
{
    if (you.where_are_you == BRANCH_FOREST)
        return _stone_stew_lair_town_name();

    return _stone_stew_mid_dungeon_town_name();
}

std::string stone_stew_branch_name(branch_type branch, bool long_name)
{
    if (branch == BRANCH_DWARF)
        return _stone_stew_mid_dungeon_town_name();

    if (branch == BRANCH_FOREST)
        return _stone_stew_lair_town_name();

    return long_name ? branches[branch].longname : branches[branch].abbrevname;
}

static int _stone_stew_persistent_index(const char *prop_key, int count)
{
    if (!you.props.exists(prop_key))
        you.props[prop_key] = random2(count);

    int index = you.props[prop_key].get_int();
    if (index < 0 || index >= count)
    {
        index = 0;
        you.props[prop_key] = index;
    }

    return index;
}

static const stone_stew_guild_def *_stone_stew_current_guild()
{
    static const stone_stew_guild_def dungeon_guilds[] =
    {
        {
            "Lamplighters' Compact",
            "safe-road scouting, warnings, and rescue contracts",
            "lanterns, toll ledgers, and maps of half-cleared stairs",
            "\"We pay for eyes that come back,\" the factor says."
        },
        {
            "Copper Stair Fellowship",
            "merchant errands, appraisals, and guarded deliveries",
            "coin scales, sealed crates, and cautious trade routes",
            "\"Every road has a price. Members learn which prices are fair.\""
        },
        {
            "Grey Writ Company",
            "bounties, dungeon surveys, and practical monster work",
            "notice boards, weapon racks, and contracts stamped in grey wax",
            "\"No glory clauses. Work, report, collect.\""
        },
    };
    static const stone_stew_guild_def lair_guilds[] =
    {
        {
            "Rootwarden Circle",
            "beast paths, herbal remedies, and Lair survival",
            "root charms, poultice bowls, and maps scratched into bark",
            "\"The Lair kills the loud and the lost. We teach neither.\""
        },
        {
            "Green Hunt Lodge",
            "tracking, trophies, and dangerous-beast contracts",
            "hide frames, spear racks, and careful sketches of tracks",
            "\"Take only contracts you can walk away from.\""
        },
        {
            "Venomwise Lodge",
            "poison lore, antidote work, and swamp-road preparation",
            "drying herbs, glass vials, and venom notes in a steady hand",
            "\"Most venom is a question. Members learn the answer.\""
        },
    };

    if (you.where_are_you == BRANCH_DWARF)
    {
        return &dungeon_guilds[
            _stone_stew_persistent_index(STONE_STEW_DUNGEON_TOWN_GUILD_KEY,
                                         ARRAYSZ(dungeon_guilds))];
    }

    if (you.where_are_you == BRANCH_FOREST)
    {
        return &lair_guilds[
            _stone_stew_persistent_index(STONE_STEW_LAIR_TOWN_GUILD_KEY,
                                         ARRAYSZ(lair_guilds))];
    }

    return nullptr;
}

static const char *_stone_stew_current_guild_joined_key()
{
    if (you.where_are_you == BRANCH_DWARF)
        return STONE_STEW_DUNGEON_TOWN_GUILD_JOINED_KEY;

    if (you.where_are_you == BRANCH_FOREST)
        return STONE_STEW_LAIR_TOWN_GUILD_JOINED_KEY;

    return nullptr;
}

static bool _stone_stew_current_guild_joined()
{
    const char *key = _stone_stew_current_guild_joined_key();
    return key && you.props.exists(key) && you.props[key].get_bool();
}

static const char *_stone_stew_current_guild_rank_key()
{
    if (you.where_are_you == BRANCH_DWARF)
        return STONE_STEW_DUNGEON_TOWN_GUILD_RANK_KEY;

    if (you.where_are_you == BRANCH_FOREST)
        return STONE_STEW_LAIR_TOWN_GUILD_RANK_KEY;

    return nullptr;
}

static const char *_stone_stew_current_guild_benefit_key()
{
    if (you.where_are_you == BRANCH_DWARF)
        return STONE_STEW_DUNGEON_TOWN_GUILD_BENEFIT_KEY;

    if (you.where_are_you == BRANCH_FOREST)
        return STONE_STEW_LAIR_TOWN_GUILD_BENEFIT_KEY;

    return nullptr;
}

static int _stone_stew_current_guild_rank()
{
    const char *key = _stone_stew_current_guild_rank_key();
    if (!key || !you.props.exists(key))
        return 0;

    return you.props[key].get_int();
}

static void _stone_stew_set_current_guild_rank(int rank)
{
    const char *key = _stone_stew_current_guild_rank_key();
    if (!key)
        return;

    you.props[key] = max(_stone_stew_current_guild_rank(), rank);
}

static bool _stone_stew_current_guild_benefit_used()
{
    const char *key = _stone_stew_current_guild_benefit_key();
    return key && you.props.exists(key) && you.props[key].get_bool();
}

static skill_type _stone_stew_current_guild_benefit_skill()
{
    if (you.where_are_you == BRANCH_FOREST)
        return SK_DODGING;

    return SK_FIGHTING;
}

static string _stone_stew_skill_name(skill_type skill)
{
    switch (skill)
    {
    case SK_ARMOUR:
        return "Armour";
    case SK_DODGING:
        return "Dodging";
    case SK_FIGHTING:
        return "Fighting";
    case SK_STEALTH:
        return "Stealth";
    default:
        return "skill";
    }
}

static string _stone_stew_current_guild_rank_name()
{
    if (!_stone_stew_current_guild_joined())
        return "outsider";

    if (_stone_stew_current_guild_rank() >= 1)
        return "trusted hand";

    return "probationary member";
}

static const stone_stew_guild_survey_def *_stone_stew_current_survey()
{
    static const stone_stew_guild_survey_def dungeon_survey =
    {
        STONE_STEW_DUNGEON_TOWN_SURVEY_KEY,
        BRANCH_DUNGEON,
        11,
        "Guild Survey: Lower Road",
        "\"The board needs a reliable mark below the town road,\" the factor says. "
        "\"Reach D:11, come back alive, and we will call that useful ink.\"",
        "Guild work accepted: reach D:11, then return to the Guild Factor.",
        "\"The lower mark is still blank,\" the factor says. \"Reach D:11, then return.\"",
        "\"There. The lower road has your bootprint on it,\" the factor says.",
        "\"No more survey writs are posted here today,\" the factor says.",
        55,
    };
    static const stone_stew_guild_survey_def lair_survey =
    {
        STONE_STEW_LAIR_TOWN_SURVEY_KEY,
        BRANCH_LAIR,
        4,
        "Guild Survey: Deep Paths",
        "\"We need fresh signs from the deeper paths,\" the factor says. "
        "\"Reach Lair:4, return with your skin attached, and the hall pays.\"",
        "Guild work accepted: reach Lair:4, then return to the Guild Factor.",
        "\"The deep paths still want your witness,\" the factor says. \"Reach Lair:4, then return.\"",
        "\"Good. The Lair below has been seen and named,\" the factor says.",
        "\"The lodge has no more survey writs for you today,\" the factor says.",
        75,
    };

    if (you.where_are_you == BRANCH_DWARF)
        return &dungeon_survey;

    if (you.where_are_you == BRANCH_FOREST)
        return &lair_survey;

    return nullptr;
}

static int _stone_stew_survey_state(const stone_stew_guild_survey_def& survey)
{
    if (!you.props.exists(survey.prop_key))
        return SSQ_UNOFFERED;

    return you.props[survey.prop_key].get_int();
}

static void _stone_stew_set_survey_state(
    const stone_stew_guild_survey_def& survey, int state)
{
    you.props[survey.prop_key] = state;
}

static bool _stone_stew_survey_complete(
    const stone_stew_guild_survey_def& survey)
{
    const level_id target(survey.target_branch, survey.target_depth);
    return level_id::current() == target || you.level_visited(target);
}

static string _stone_stew_survey_target_name(
    const stone_stew_guild_survey_def& survey)
{
    return string(branches[survey.target_branch].abbrevname) + ":"
           + make_stringf("%d", survey.target_depth);
}

static string _stone_stew_survey_log_entry(
    const stone_stew_guild_survey_def& survey)
{
    const int state = _stone_stew_survey_state(survey);
    if (state == SSQ_UNOFFERED)
        return "";

    const stone_stew_guild_def *guild = _stone_stew_current_guild();
    string text = "<yellow>";
    text += survey.title;
    text += "</yellow>\n";
    text += "Giver: ";
    text += guild ? guild->name : "Local guild";
    text += "\nObjective: Reach ";
    text += _stone_stew_survey_target_name(survey);
    text += ", then return to the Guild Factor.";
    text += "\nReward: ";
    text += make_stringf("%d gold pieces.", survey.reward_gold);
    text += "\nRisk: Moderate. This requires real exploration beyond town.";
    text += "\nFailure: None yet, but later guild contracts may fail.";
    text += "\nStatus: ";

    if (state == SSQ_ACTIVE)
        text += _stone_stew_survey_complete(survey) ? "ready to turn in." : "active.";
    else if (state == SSQ_COMPLETED)
        text += "completed.";
    else if (state == SSQ_FAILED)
        text += "failed.";
    else
        text += "not accepted.";

    text += "\n\n";
    return text;
}

bool stone_stew_is_town_npc(const monster& mon)
{
    return mon.wont_attack()
           && (mon.mname == "Mara the Coinwise"
               || mon.mname == "Old Rellan"
               || mon.mname == "Bertha of the Cot"
               || mon.mname == "Bethra of the Cot"
               || mon.mname == "Gate Warden"
               || mon.mname == "townsperson"
               || mon.mname == "Town Priest"
               || mon.mname == "Town Broker"
               || mon.mname == "Guild Factor"
               || mon.mname == "Lantern Warden"
               || mon.mname == "Root Hunter"
               || mon.mname == "Lair Healer");
}

static bool _stone_stew_mon_is_giver(const monster& mon,
                                     const stone_stew_quest_def& quest)
{
    return mon.mname == quest.giver
           || quest.legacy_giver && mon.mname == quest.legacy_giver;
}

static int _stone_stew_quest_state(const stone_stew_quest_def& quest)
{
    if (!you.props.exists(quest.prop_key))
        return SSQ_UNOFFERED;

    return you.props[quest.prop_key].get_int();
}

static void _stone_stew_set_quest_state(const stone_stew_quest_def& quest,
                                        int state)
{
    you.props[quest.prop_key] = state;
}

static bool _stone_stew_quest_prereq_met(const stone_stew_quest_def& quest)
{
    if (!quest.prereq_key)
        return true;

    if (!you.props.exists(quest.prereq_key))
        return false;

    return you.props[quest.prereq_key].get_int() >= quest.prereq_state;
}

static bool _stone_stew_quest_failed(const stone_stew_quest_def& quest)
{
    return quest.fail && quest.fail();
}

static int _stone_stew_quest_state_by_key(const char *key)
{
    for (int i = 0; i < STONE_STEW_NUM_QUESTS; ++i)
        if (STONE_STEW_QUESTS[i].prop_key == key)
            return _stone_stew_quest_state(STONE_STEW_QUESTS[i]);

    return SSQ_UNOFFERED;
}

static string _stone_stew_quest_offer_text(const stone_stew_quest_def& quest)
{
    string text = "<yellow>";
    text += quest.title;
    text += "</yellow>\n\n";
    text += quest.offer;
    text += "\n\n";
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
    if (quest.prereq_key)
        text += "\nPrerequisite: complete earlier work for this giver.";
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

    if (state == SSQ_ACTIVE)
    {
        text += quest.complete() ? "ready to turn in." : "active.";
    }
    else if (state == SSQ_COMPLETED)
        text += "completed.";
    else if (state == SSQ_FAILED)
        text += "failed.";
    else
        text += "not accepted.";

    text += "\n\n";
    return text;
}

static string _stone_stew_quest_log_text()
{
    string text = "<white>Stone Stew Quest Log</white>\n\n";
    bool found = false;

    if (const stone_stew_moral_template *tmpl =
            _stone_stew_current_moral_template())
    {
        const string entry = _stone_stew_moral_log_entry(*tmpl);
        if (!entry.empty())
        {
            text += entry;
            found = true;
        }
    }

    for (int i = 0; i < STONE_STEW_NUM_QUESTS; ++i)
    {
        const stone_stew_quest_def& quest = STONE_STEW_QUESTS[i];
        if (_stone_stew_quest_state(quest) == SSQ_UNOFFERED)
            continue;

        text += _stone_stew_quest_log_entry(quest);
        found = true;
    }

    static const stone_stew_guild_survey_def survey_defs[] =
    {
        {
            STONE_STEW_DUNGEON_TOWN_SURVEY_KEY,
            BRANCH_DUNGEON,
            11,
            "Guild Survey: Lower Road",
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            55,
        },
        {
            STONE_STEW_LAIR_TOWN_SURVEY_KEY,
            BRANCH_LAIR,
            4,
            "Guild Survey: Deep Paths",
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            75,
        },
    };

    for (int i = 0; i < static_cast<int>(ARRAYSZ(survey_defs)); ++i)
    {
        const string entry = _stone_stew_survey_log_entry(survey_defs[i]);
        if (!entry.empty())
        {
            text += entry;
            found = true;
        }
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
    stone_stew_quest_offer_popup offer(quest);
    offer.show();

    if (!offer.accepted())
    {
        mpr("You decline the work for now.");
        return true;
    }

    _stone_stew_set_quest_state(quest, SSQ_ACTIVE);
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

enum stone_stew_dialogue_action
{
    SSDA_TALK,
    SSDA_QUEST,
    SSDA_TRADE,
    SSDA_TRAINING,
    SSDA_GUILD,
    SSDA_GUILD_WORK,
    SSDA_GUILD_BENEFIT,
};

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

    if (mon.mname == "Bertha of the Cot" || mon.mname == "Bethra of the Cot")
    {
        mpr("\"Beds are for stories, not statistics,\" Bertha says. "
            "\"Rest easy here; the town keeps its own watch.\"");
        return true;
    }

    if (mon.mname == "Gate Warden")
    {
        mpr("\"Steel stays sheathed inside the gate,\" the warden says. "
            "\"Past it, mind your own skin.\"");
        return true;
    }

    if (mon.mname == "Town Priest")
    {
        mprf("\"%s keeps shrines for travelers who still have choices,\" "
             "the priest says. \"Pray if the god's eye finds you.\"",
             _stone_stew_current_town_name().c_str());
        return true;
    }

    if (mon.mname == "Town Broker")
    {
        mprf("\"%s buys stories before it buys steel,\" the broker says. "
             "\"Bring stranger goods when the roads below open wider.\"",
             _stone_stew_current_town_name().c_str());
        return true;
    }

    if (mon.mname == "Guild Factor")
    {
        const stone_stew_guild_def *guild = _stone_stew_current_guild();
        if (guild)
        {
            mprf("\"%s keeps a chapter of the %s,\" the factor says. "
                 "\"Ask about Guild work if you want your name in the book.\"",
                 _stone_stew_current_town_name().c_str(), guild->name);
        }
        else
        {
            mpr("\"Guild writs are sealed for now,\" the factor says. "
                "\"Earn a name in the deep roads and doors will notice.\"");
        }
        return true;
    }

    if (mon.mname == "Lantern Warden")
    {
        mprf("\"Welcome to %s,\" the warden says. "
             "\"No blades drawn inside the lamps.\"",
             _stone_stew_current_town_name().c_str());
        return true;
    }

    if (mon.mname == "Root Hunter")
    {
        mprf("\"%s sits where the beasts remember the path,\" "
             "the hunter says. \"Learn the wind before you follow blood.\"",
             _stone_stew_lair_town_name().c_str());
        return true;
    }

    if (mon.mname == "Lair Healer")
    {
        mprf("\"Venom, claw, fever, fear,\" the healer says. "
             "\"%s has salves for some of them.\"",
             _stone_stew_lair_town_name().c_str());
        return true;
    }

    if (mon.mname == "townsperson")
    {
        if (_stone_stew_townsperson_moral_followup())
            return true;

        const stone_stew_npc_identity identity =
            _stone_stew_current_moral_identity();
        if (you.where_are_you == BRANCH_DWARF
            || you.where_are_you == BRANCH_FOREST)
        {
            mprf("%s the %s gives you a cautious nod. "
                 "\"%s is safe ground, if you keep it that way.\"",
                 identity.name.c_str(), identity.role.c_str(),
                 _stone_stew_current_town_name().c_str());
        }
        else
        {
            mprf("%s the %s gives you a cautious nod. "
                 "They seem %s, and %s.",
                 identity.name.c_str(), identity.role.c_str(),
                 identity.trait.c_str(), identity.pressure.c_str());
        }
        return true;
    }

    return false;
}

static bool _stone_stew_town_npc_quest(const monster& mon)
{
    if (_stone_stew_townsperson_moral_quest(mon))
        return true;

    bool has_quest = false;
    bool all_done = true;
    bool blocked_by_prereq = false;

    for (int i = 0; i < STONE_STEW_NUM_QUESTS; ++i)
    {
        const stone_stew_quest_def& quest = STONE_STEW_QUESTS[i];
        if (!_stone_stew_mon_is_giver(mon, quest))
            continue;

        has_quest = true;
        if (!_stone_stew_quest_prereq_met(quest))
        {
            blocked_by_prereq = true;
            continue;
        }

        const int state = _stone_stew_quest_state(quest);

        if (state == SSQ_UNOFFERED)
            return _stone_stew_offer_quest(quest);

        if (state == SSQ_ACTIVE)
        {
            all_done = false;
            if (_stone_stew_quest_failed(quest))
            {
                _stone_stew_set_quest_state(quest, SSQ_FAILED);
                mpr(quest.failed);
                return true;
            }

            if (quest.complete())
            {
                mpr(quest.ready);
                mprf("%s pays you %d gold pieces.", quest.giver, quest.reward_gold);
                you.add_gold(quest.reward_gold);
                _stone_stew_set_quest_state(quest, SSQ_COMPLETED);
                return true;
            }

            mpr(quest.incomplete);
            return true;
        }

        if (state == SSQ_FAILED)
        {
            all_done = false;
            mpr(quest.failed);
            return true;
        }

        if (state == SSQ_COMPLETED)
            continue;
    }

    if (!has_quest)
        mpr("They have no work for you yet.");
    else if (blocked_by_prereq)
        mpr("They are not ready to offer you more work yet.");
    else if (all_done)
    {
        const stone_stew_quest_def *last_completed = nullptr;
        for (int i = 0; i < STONE_STEW_NUM_QUESTS; ++i)
        {
            const stone_stew_quest_def& quest = STONE_STEW_QUESTS[i];
            if (_stone_stew_mon_is_giver(mon, quest))
            {
                if (_stone_stew_quest_state(quest) == SSQ_COMPLETED)
                    last_completed = &quest;
            }
        }
        if (last_completed)
            mpr(last_completed->completed);
    }

    return true;
}

static bool _stone_stew_town_npc_has_quest(const monster& mon)
{
    if (mon.mname == _stone_stew_current_moral_template()->giver)
        return _stone_stew_town_moral_state() != SSQ_COMPLETED;

    for (int i = 0; i < STONE_STEW_NUM_QUESTS; ++i)
        if (_stone_stew_mon_is_giver(mon, STONE_STEW_QUESTS[i]))
            return true;

    return false;
}

static bool _stone_stew_mara_will_buy(const item_def& item)
{
    return item.defined()
           && is_artefact(item)
           && !is_unrandom_artefact(item)
           && item.is_identified()
           && !item_is_equipped(item);
}

static bool _stone_stew_has_mara_sale_item()
{
    for (int i = 0; i < ENDOFPACK; ++i)
        if (_stone_stew_mara_will_buy(you.inv[i]))
            return true;

    return false;
}

static int _stone_stew_randart_sale_price(const item_def& item)
{
    const int shop_value = item_value(item, true);
    const int art_value = artefact_value(item);

    return max(1, min(350, shop_value / 5 + art_value));
}

static bool _stone_stew_mara_buy_randart()
{
    if (!_stone_stew_has_mara_sale_item())
    {
        mpr("\"I buy identified strange artefacts,\" Mara says. "
            "\"Not heirlooms, not mysteries, and not what you are wearing.\"");
        return true;
    }

    const int slot = prompt_invent_item(
        "Sell which identified random artefact?",
        menu_type::invlist, OSEL_ANY);

    if (slot < 0)
        return true;

    item_def& item = you.inv[slot];
    if (!_stone_stew_mara_will_buy(item))
    {
        if (!item.defined())
            mpr("\"Empty hands sell poorly,\" Mara says.");
        else if (!is_artefact(item))
            mpr("\"That is no artefact,\" Mara says.");
        else if (is_unrandom_artefact(item))
            mpr("\"Some things are too singular to fence,\" Mara says.");
        else if (!item.is_identified())
            mpr("\"Bring me a known thing, not a riddle,\" Mara says.");
        else if (item_is_equipped(item))
            mpr("\"Take it off first. I do not buy from someone's body,\" Mara says.");
        else
            mpr("\"Not that one,\" Mara says.");

        return true;
    }

    const int payout = _stone_stew_randart_sale_price(item);
    const string item_name = item.name(DESC_YOUR);
    const string prompt = make_stringf("Sell %s to Mara for %d gold?",
                                       item_name.c_str(), payout);

    if (!yesno(prompt.c_str(), true, 'n'))
    {
        mpr("\"Keep it, then,\" Mara says. \"The dungeon may yet change its mind.\"");
        return true;
    }

    mprf("Mara buys %s for %d gold.", item_name.c_str(), payout);
    you.add_gold(payout);
    dec_inv_item_quantity(slot, 1);
    return true;
}

static bool _stone_stew_town_npc_has_trade(const monster& mon)
{
    return mon.mname == "Mara the Coinwise";
}

static bool _stone_stew_rellan_training_available(const monster& mon)
{
    return mon.mname == "Old Rellan"
           && _stone_stew_quest_state_by_key(STONE_STEW_RELLAN_QUEST_KEY)
              >= SSQ_COMPLETED
           && !you.props.exists(STONE_STEW_RELLAN_FIGHTING_TRAINING_KEY)
           && you.skills[SK_FIGHTING] < 5;
}

static const stone_stew_training_def *_stone_stew_paid_training_for(
    const monster& mon)
{
    for (int i = 0; i < STONE_STEW_NUM_TRAINING; ++i)
    {
        const stone_stew_training_def& training = STONE_STEW_TRAINING[i];
        if (mon.mname == training.trainer
            && you.where_are_you == training.branch
            && !you.props.exists(training.prop_key))
        {
            return &training;
        }
    }

    return nullptr;
}

static bool _stone_stew_town_npc_has_training(const monster& mon)
{
    return _stone_stew_rellan_training_available(mon)
           || _stone_stew_paid_training_for(mon);
}

static bool _stone_stew_town_npc_has_guild(const monster& mon)
{
    return mon.mname == "Guild Factor" && _stone_stew_current_guild();
}

static bool _stone_stew_town_npc_has_guild_work(const monster& mon)
{
    return mon.mname == "Guild Factor"
           && _stone_stew_current_guild()
           && _stone_stew_current_guild_joined()
           && _stone_stew_current_survey();
}

static bool _stone_stew_town_npc_trade(const monster& mon)
{
    if (mon.mname == "Mara the Coinwise")
    {
        mpr("Mara appraises your pack with professional interest.");
        return _stone_stew_mara_buy_randart();
    }

    mpr("They are not trading right now.");
    return true;
}

static bool _stone_stew_town_npc_training(const monster& mon)
{
    if (_stone_stew_rellan_training_available(mon))
    {
        const string prompt = make_stringf(
            "Pay Old Rellan %d gold for a Fighting drill?",
            STONE_STEW_RELLAN_FIGHTING_TRAINING_COST);

        if (you.gold < STONE_STEW_RELLAN_FIGHTING_TRAINING_COST)
        {
            mprf("\"Training costs %d gold,\" Old Rellan says. "
                 "\"Come back with coin enough to respect the lesson.\"",
                 STONE_STEW_RELLAN_FIGHTING_TRAINING_COST);
            return true;
        }

        if (!yesno(prompt.c_str(), true, 'n'))
        {
            mpr("\"Another time, then,\" Old Rellan says.");
            return true;
        }

        you.del_gold(STONE_STEW_RELLAN_FIGHTING_TRAINING_COST);
        change_skill_points(SK_FIGHTING,
                            STONE_STEW_RELLAN_FIGHTING_TRAINING_POINTS,
                            true);
        you.skills_to_show.insert(SK_FIGHTING);
        you.props[STONE_STEW_RELLAN_FIGHTING_TRAINING_KEY] = 1;
        mpr("\"Good,\" Old Rellan says. \"Keep your shoulders under your fear.\"");
        mpr("Your Fighting skill improves from Old Rellan's drill.");
        return true;
    }

    if (const stone_stew_training_def *training =
            _stone_stew_paid_training_for(mon))
    {
        mpr(training->offer);
        const string prompt = make_stringf(
            "Pay %s %d gold for %s training?",
            training->trainer, training->cost, training->label);

        if (you.gold < training->cost)
        {
            mpr(training->poor);
            return true;
        }

        if (!yesno(prompt.c_str(), true, 'n'))
        {
            mpr(training->declined);
            return true;
        }

        you.del_gold(training->cost);
        change_skill_points(training->skill, training->skill_points, true);
        you.skills_to_show.insert(training->skill);
        you.props[training->prop_key] = 1;
        mpr(training->complete);
        mprf("Your %s skill improves from the lesson.",
             _stone_stew_skill_name(training->skill).c_str());
        return true;
    }

    mpr("They are not offering training right now.");
    return true;
}

static bool _stone_stew_town_npc_guild(const monster& mon)
{
    if (mon.mname != "Guild Factor")
    {
        mpr("They are not handling guild business right now.");
        return true;
    }

    const stone_stew_guild_def *guild = _stone_stew_current_guild();
    if (!guild)
    {
        mpr("There is no guild chapter here yet.");
        return true;
    }

    mprf("<yellow>%s</yellow>", guild->name);
    mprf("Focus: %s.", guild->focus);
    mprf("Chapter hall: %s.", guild->theme);

    if (_stone_stew_current_guild_joined())
    {
        mprf("\"You are already on the rolls of the %s,\" the factor says. "
             "\"Check Guild Work if you want a posted contract.\"",
             guild->name);
        mprf("Standing: %s.", _stone_stew_current_guild_rank_name().c_str());
        return true;
    }

    mpr(guild->join_pitch);
    const string prompt = make_stringf("Join the %s?", guild->name);
    if (!yesno(prompt.c_str(), true, 'n'))
    {
        mpr("\"No ink spent, then,\" the factor says.");
        return true;
    }

    const char *key = _stone_stew_current_guild_joined_key();
    if (key)
        you.props[key].get_bool() = true;

    mprf("You join the %s.", guild->name);
    mpr("You can now ask this Guild Factor about Guild Work.");
    return true;
}

static bool _stone_stew_town_npc_guild_work(const monster& mon)
{
    if (mon.mname != "Guild Factor")
    {
        mpr("They are not handling guild contracts right now.");
        return true;
    }

    if (!_stone_stew_current_guild_joined())
    {
        mpr("\"Join the chapter before asking for writs,\" the factor says.");
        return true;
    }

    const stone_stew_guild_survey_def *survey = _stone_stew_current_survey();
    if (!survey)
    {
        mpr("There is no guild work posted here yet.");
        return true;
    }

    const int state = _stone_stew_survey_state(*survey);
    if (state == SSQ_UNOFFERED)
    {
        mprf("<yellow>%s</yellow>", survey->title);
        mpr(survey->offer);
        mprf("Objective: Reach %s, then return to the Guild Factor.",
             _stone_stew_survey_target_name(*survey).c_str());
        mprf("Reward: %d gold pieces.", survey->reward_gold);

        if (!yesno("Accept this guild survey contract?", true, 'n'))
        {
            mpr("\"The board keeps its nails,\" the factor says.");
            return true;
        }

        _stone_stew_set_survey_state(*survey, SSQ_ACTIVE);
        mpr(survey->accepted);
        mpr("You can review accepted quests with <lightgrey>Ctrl+T</lightgrey>.");
        return true;
    }

    if (state == SSQ_ACTIVE)
    {
        if (_stone_stew_survey_complete(*survey))
        {
            mpr(survey->ready);
            mprf("The guild pays you %d gold pieces.", survey->reward_gold);
            you.add_gold(survey->reward_gold);
            _stone_stew_set_survey_state(*survey, SSQ_COMPLETED);
            _stone_stew_set_current_guild_rank(1);
            mprf("Your standing rises to %s with this guild.",
                 _stone_stew_current_guild_rank_name().c_str());
            return true;
        }

        mpr(survey->incomplete);
        return true;
    }

    if (state == SSQ_COMPLETED)
    {
        mpr(survey->completed);
        return true;
    }

    mpr("That guild contract is no longer available.");
    return true;
}

static bool _stone_stew_town_npc_has_guild_benefit(const monster& mon)
{
    return mon.mname == "Guild Factor"
           && _stone_stew_current_guild()
           && _stone_stew_current_guild_joined()
           && _stone_stew_current_guild_rank() >= 1
           && !_stone_stew_current_guild_benefit_used();
}

static bool _stone_stew_town_npc_guild_benefit(const monster& mon)
{
    if (mon.mname != "Guild Factor")
    {
        mpr("They are not handling guild benefits right now.");
        return true;
    }

    const stone_stew_guild_def *guild = _stone_stew_current_guild();
    if (!guild || !_stone_stew_current_guild_joined())
    {
        mpr("You are not on this guild chapter's rolls.");
        return true;
    }

    if (_stone_stew_current_guild_rank() < 1)
    {
        mpr("\"Finish a posted contract first,\" the factor says.");
        return true;
    }

    if (_stone_stew_current_guild_benefit_used())
    {
        mpr("\"The chapter has already trained you for this posting,\" the factor says.");
        return true;
    }

    const skill_type skill = _stone_stew_current_guild_benefit_skill();
    const string skill_name = _stone_stew_skill_name(skill);
    const string prompt = make_stringf(
        "Claim your %s training benefit from the %s?",
        skill_name.c_str(), guild->name);

    if (!yesno(prompt.c_str(), true, 'n'))
    {
        mpr("\"The benefit stays on the ledger,\" the factor says.");
        return true;
    }

    change_skill_points(skill, STONE_STEW_GUILD_BENEFIT_SKILL_POINTS, true);
    you.skills_to_show.insert(skill);

    const char *key = _stone_stew_current_guild_benefit_key();
    if (key)
        you.props[key].get_bool() = true;

    mprf("The %s grants you practical %s training.",
         guild->name, skill_name.c_str());
    return true;
}

static string _stone_stew_dialogue_action_name(stone_stew_dialogue_action action)
{
    switch (action)
    {
    case SSDA_TALK:
        return "Talk";
    case SSDA_QUEST:
        return "Quest";
    case SSDA_TRADE:
        return "Trade";
    case SSDA_TRAINING:
        return "Training";
    case SSDA_GUILD:
        return "Guild";
    case SSDA_GUILD_WORK:
        return "Guild Work";
    case SSDA_GUILD_BENEFIT:
        return "Guild Benefit";
    }

    return "";
}

static vector<stone_stew_dialogue_action> _stone_stew_dialogue_actions(
    const monster& mon)
{
    vector<stone_stew_dialogue_action> actions;

    actions.push_back(SSDA_TALK);

    if (_stone_stew_town_npc_has_quest(mon))
        actions.push_back(SSDA_QUEST);

    if (_stone_stew_town_npc_has_trade(mon))
        actions.push_back(SSDA_TRADE);

    if (_stone_stew_town_npc_has_training(mon))
        actions.push_back(SSDA_TRAINING);

    if (_stone_stew_town_npc_has_guild(mon))
        actions.push_back(SSDA_GUILD);

    if (_stone_stew_town_npc_has_guild_work(mon))
        actions.push_back(SSDA_GUILD_WORK);

    if (_stone_stew_town_npc_has_guild_benefit(mon))
        actions.push_back(SSDA_GUILD_BENEFIT);

    return actions;
}

static void _stone_stew_show_dialogue_options(
    const vector<stone_stew_dialogue_action>& actions)
{
    for (unsigned i = 0; i < actions.size(); ++i)
    {
        mprf("<lightgrey>%u</lightgrey> %s",
             i + 1, _stone_stew_dialogue_action_name(actions[i]).c_str());
    }
    mpr("<lightgrey>Esc</lightgrey> Leave");
}

static bool _stone_stew_run_dialogue_action(stone_stew_dialogue_action action,
                                            const monster& mon)
{
    switch (action)
    {
    case SSDA_TALK:
        return _stone_stew_town_npc_talk(mon);
    case SSDA_QUEST:
        return _stone_stew_town_npc_quest(mon);
    case SSDA_TRADE:
        return _stone_stew_town_npc_trade(mon);
    case SSDA_TRAINING:
        return _stone_stew_town_npc_training(mon);
    case SSDA_GUILD:
        return _stone_stew_town_npc_guild(mon);
    case SSDA_GUILD_WORK:
        return _stone_stew_town_npc_guild_work(mon);
    case SSDA_GUILD_BENEFIT:
        return _stone_stew_town_npc_guild_benefit(mon);
    }

    return true;
}

static bool _stone_stew_try_shortcut_action(
    int key, const monster& mon,
    const vector<stone_stew_dialogue_action>& actions)
{
    stone_stew_dialogue_action action = SSDA_TALK;
    bool matched = true;

    switch (toalower(key))
    {
    case 't':
        action = SSDA_TALK;
        break;
    case 'q':
        action = SSDA_QUEST;
        break;
    case 'r':
        action = SSDA_TRADE;
        break;
    case '$':
        action = SSDA_TRADE;
        break;
    case 'n':
        action = SSDA_TRAINING;
        break;
    case 'g':
        action = SSDA_GUILD;
        break;
    case 'w':
        action = SSDA_GUILD_WORK;
        break;
    case 'b':
        action = SSDA_GUILD_BENEFIT;
        break;
    default:
        matched = false;
        break;
    }

    if (!matched)
        return false;

    for (stone_stew_dialogue_action offered : actions)
    {
        if (offered == action)
        {
            _stone_stew_run_dialogue_action(action, mon);
            return true;
        }
    }

    mpr("That option is not available from this person.");
    return true;
}

bool stone_stew_talk_to_town_npc(monster& mon)
{
    if (!stone_stew_is_town_npc(mon))
        return false;

    stop_running();
    mprf("You speak with %s.", mon.name(DESC_THE).c_str());
    const vector<stone_stew_dialogue_action> actions =
        _stone_stew_dialogue_actions(mon);
    _stone_stew_show_dialogue_options(actions);

    while (true)
    {
        const int key = getchm();
        if (key_is_escape(key) || key == ' ' || toalower(key) == 'q')
        {
            mpr("You step back from the conversation.");
            return true;
        }

        if (key >= '1' && key < '1' + static_cast<int>(actions.size()))
        {
            return _stone_stew_run_dialogue_action(actions[key - '1'], mon);
        }

        if (_stone_stew_try_shortcut_action(key, mon, actions))
            return true;

        mpr("Choose one of the listed options, or Esc.");
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

    if (mon.mname == "Bertha of the Cot" || mon.mname == "Bethra of the Cot")
        return 3;

    if (mon.mname == "townsperson")
        return 9;

    if (mon.mname == "Town Priest"
        || mon.mname == "Town Broker"
        || mon.mname == "Guild Factor"
        || mon.mname == "Lantern Warden"
        || mon.mname == "Root Hunter"
        || mon.mname == "Lair Healer")
    {
        return 4;
    }

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
