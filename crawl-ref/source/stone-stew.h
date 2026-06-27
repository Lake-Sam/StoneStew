#pragma once

#include <string>

#include "branch-type.h"
#include "coord-def.h"
#include "movement-type.h"

class monster;

bool stone_stew_is_town_npc(const monster& mon);
bool stone_stew_talk_to_town_npc(monster& mon);
bool stone_stew_town_npc_attack_warning(const monster& mon);
void stone_stew_display_quest_log();
std::string stone_stew_branch_name(branch_type branch, bool long_name);
bool stone_stew_town_npc_can_move_to(const monster& mon, const coord_def& target);
bool stone_stew_town_npc_prepare_move(monster& mon, const coord_def& target,
                                      movement_type mvflags);
