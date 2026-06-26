# Stone Stew Design Notes

Stone Stew is a Dungeon Crawl Stone Soup-derived roguelike fork based on DCSS
0.34.1. The fork targets local tiles first; console support is not a project
goal. WebTiles can follow after the Windows/local tiles experience is stable.

## Core Direction

- Keep the game recognisably DCSS-derived, but broaden it toward towns, NPCs,
  quests, services, and a stronger gold economy.
- Start with a small town on D:1 plus the normal dungeon entrance.
- Later towns appear between branches. Larger cities may become separate hub
  areas with multiple floors, similar in spirit to the Ecumenical Temple.
- Towns are always safe from normal dungeon danger. Monsters may exist outside
  town boundaries on D:1.
- Altars should not appear before current DCSS timing.
- Guilds do not appear on D:1. They unlock later and provide quests/rewards,
  not passive perks. The player may join multiple guilds.

## NPCs

- Towns contain both permanent named NPCs for that run and procedural villagers.
- NPC identities are procedural per run, but persistent within that run.
- NPCs can remember the player across visits.
- Civilian NPCs should not normally die. Companion NPCs can die permanently.
- Civilian and service NPC movement should be constrained to the town area they
  belong to, so they cannot wander across the rest of D:1 or into the dungeon.
- Player attacks on friendly NPCs require a confirmation prompt.
- Guards are very strong rather than invincible.
- Town crimes are tracked per town. If the player murders townsfolk, that town
  becomes hostile permanently.

## Dialogue

- Development dialogue server: Ollama on localhost with Qwen3 1.7B.
- Release dialogue runtime: bundled llama.cpp, quantized GGUF model, and a game
  dialogue controller.
- Dialogue uses selectable player responses, not free text.
- The model writes presentation text only. The engine owns all facts, quest
  state, rewards, inventory transfer, pricing, and discovered knowledge.
- NPC dialogue is generated from a character sheet and cached where useful.
- Dialogue history should persist within a save/run, but new runs can generate
  new NPCs and new dialogue.
- If local model generation fails, authored fallback dialogue is used.
- NPCs should only know what the player has discovered.

## Quests

- D:1 quests should be low-risk and achievable.
- Quest types may include fetch, kill, escort, exploration, rescue, delivery,
  bounty, and Elder Scrolls-inspired multi-step stories.
- Quests can fail.
- Quest dialogue should preview the objective and expected reward before the
  player accepts.
- Quest difficulty and reward value should scale from dungeon depth, branch,
  monster/item difficulty, and the player's progression.
- Quest rewards may include gold, items, randarts, mutations, god piety,
  services, and training.

## Economy

- Gold becomes more important than in upstream DCSS.
- Merchants have finite gold.
- Merchants can buy randarts, but only in categories they handle. Some shops buy
  narrow categories such as swords or boots; others buy broader categories such
  as weapons, armour, or evocables.
- Sold randarts enter merchant stock for later buyback.
- Buyback costs more than the sale price.
- Bad or cursed randarts are worth very little, but not necessarily zero.
- DCSS 0.34 identifies artefacts automatically, so initial randart selling can
  rely on identified artefact data.

## First Milestone

1. Brand the fork as Stone Stew in visible metadata.
2. Add a D:1 arrival town vault with a town boundary and dungeon entrance.
3. Add a small fixed set of town NPC roles: guard, merchant, quest giver, inn
   keeper, and townsperson.
4. Add a tile-mode talk interaction that pauses the dungeon clock.
5. Add authored fallback dialogue and structured NPC character sheets.
6. Add one low-risk starter quest.
7. Add one merchant who can buy randarts by category with finite gold and
   buyback stock.
8. Add per-town hostility/crime state and attack confirmation for friendly NPCs.

## Deferred

- Ollama/Qwen integration.
- llama.cpp bundling.
- Large hub cities.
- Guild questlines.
- Companion recruitment.
- Priests/god-specific guilds after the player has chosen a god.
- WebTiles support.
