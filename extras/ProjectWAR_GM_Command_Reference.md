# ProjectWAR (RESTART) — GM / Admin Command Reference

Extracted directly from `WorldServer/Managers/Commands/CommandDeclarations.cs` on the `RESTART` branch. All commands are typed in-game chat prefixed with a `.` (e.g. `.kill`, `.teleport map ...`).

**In-game shortcut:** `.gmcommands <command path> [--long]` prints this same list live from the running server, in case the code changes after this doc was generated.

## Access levels

Commands are gated by an account's GM level (`FrameWork/NetWork/EGmLevel.cs`):

| Level | Value |
|---|---|
| Player | 1 |
| Staff | 2 |
| GM | 3 |
| Developer | 4 |
| Admin | 5 |

A command's listed level is the **minimum** level required to use it. A level of `0`/none listed means any player can use it.

---

## Command groups (`.<group> <subcommand> ...`)

### `.ability` — Ability commands
| Command | Level | Description |
|---|---|---|
| changelist | — | Displays a list of changes made to the career. |
| addstat | Developer | Increases a given stat by a given value. |
| buff | Developer | Sends a fake buff start packet (int buffId). |
| effect | Developer | Sends a cast player effect packet. |
| start | Developer | Sends a buff effect start packet. |
| end | Developer | Send a buff effect end packet. |
| send | Developer | Send an ability to the client. |
| cast | Developer | If possible, casts the ability of the specified ID. |
| buffcast | Developer | Invokes the buff of the specified ID. |
| zerostats | Developer | Sends zero stats to the client for debug purposes. |
| list | GM | Gets the complete ability list of target (creature). |

### `.add` — Addition commands
| Command | Level | Description |
|---|---|---|
| item | GM | Add item to player |
| money | Developer | Add money to player |
| tok | GM | Add tok to player |
| renown | GM | Add renown to player |
| influence | GM | Add Influence to player |
| xp | GM | Add xp to player |
| eligibility | GM | Reports on player's eligibility |
| zonelockbags | GM | Adds Zone Lock Bags |

### `.bot` — Bot commands
| Command | Level | Description |
|---|---|---|
| spawn | GM | Spawns a group of bots. |

### `.campaign` — RvR campaign commands
Built dynamically via `CommandsBuilder.BuildCommands(typeof(CampaignCommands))` — use `.gmcommands campaign` in-game for the live list.

### `.chapter` — Chapter modification commands
| Command | Level | Description |
|---|---|---|
| save | Developer | Save chapter position |
| explore | Developer | Set tok explore |
| tokentry | Developer | Set tok entry |

### `.check` — Debugging commands
| Command | Level | Description |
|---|---|---|
| groups | Developer | Check how many groups exist on the server. |
| objects | Developer | Check how many objects exist in the current region. |
| players | GM | Finds all players currently in range. |
| respawn | Developer | Find the closest respawn point for the specified realm. |
| logpackets | Developer | Toggles logging outgoing packet volume. |
| readpackets | Developer | Displays the volume of outgoing packets over the defined period. |
| los | GM | Starts/Stops line of sight monitoring for selected target. |
| population | GM | Finds all players in the game. |
| eligibility | GM | Reports on player's eligibility |
| contribution | GM | Gets the contribution of the player. |
| bagbonus | GM | Gets the bag bonus of the player. |
| bounty | GM | Gets the bounty of the player. |
| impacts | GM | Gets the player's impact matrix. |
| allcontribution | GM | Gets the contribution of all players in the battlefront. |
| keeps | GM | Checks all keeps have the minimum required child table records. |
| captain | GM | Returns captain for either realm in the current region. |
| oil | GM | Returns information about the current keep/oil. |

### `.database` — Database commands
| Command | Level | Description |
|---|---|---|
| itemsreload | Developer | Reload items information |
| characterreload | Developer | Reload character `<name>` |
| creaturesreload | Developer | Reload creatures in your region |
| gameobjectsreload | Developer | Reload game objects in your region |
| reloadabilities | Developer | Reload abilities. |
| reloadpetmodifiers | Developer | Reload pet modifiers |

### `.equip` — Creature equipment modification
| Command | Level | Description |
|---|---|---|
| add | Developer | Add equipment to target `<Model, Slot, Save>` |
| remove | Developer | Remove equipment from target `<Slot, Save>` |
| clear | Developer | Remove all equipment from target `<Save>` |
| list | GM | Draw equipment list of target |

### `.go` — Game object commands
Built dynamically via `CommandsBuilder.BuildCommands(typeof(GoCommands))` — use `.gmcommands go` in-game for the live list.

### `.instance` — PVE instance commands
| Command | Level | Description |
|---|---|---|
| doorinfo | Staff | Returns LOS information about keep door |
| info | Staff | Instance info, optional `<instanceid>` |
| spawn | Developer | Spawn an instance NPC `<object id> <BossId> <spawngroup> <realm>` |
| convert | Developer | Converts selected object to an instance spawn `<BossId> <spawngroup> <realm>` |
| bossspawn | Developer | Spawn an instance boss NPC `<object id> <BossId> <spawngroup> <instanceid>` |
| bossconvert | Developer | Converts selected object to an instance boss spawn `<BossId> <spawngroup> <instanceid>` |
| reset | Developer | Resets the current PQ |
| opendoor | Developer | Opens door inside instance `<uniqueID> <instanceID> <open=1/close=0>` |

### `.lotd` — Land of the Dead expedition commands
Built dynamically via `CommandsBuilder.BuildCommands(typeof(LotdCommands))` — use `.gmcommands lotd` in-game for the live list.

### `.ward` — Ward fragment task counter commands
Built dynamically via `CommandsBuilder.BuildCommands(typeof(WardCommands))` — use `.gmcommands ward` in-game for the live list.

### `.modify` — Unit modification commands
| Command | Level | Description |
|---|---|---|
| speed | GM | Changes the speed of the targeted player (int 0-1000) |
| playername | GM | Changes a player's name |
| playernametemp | GM | Temporarily changes a player's name until server restart |
| guildleader | GM | Changes the leader of the guild `<newLeader, guildName>` |
| guildnamebyid | GM | Changes the name of the guild by ID `<guildID, guildName>` |
| level | Admin | Changes the level of the targeted player (int Rank) |
| renown | Admin | Changes the renown rank of a player `<playerName, RenownRank>` |
| stat | Developer | Changes your proficiency in your current crafting skill |
| access | Admin | Changes the access level of an account `<username, newAccessLevel>` |
| morale | Developer | Changes the morale of the selected player |
| faction | Developer | Changes the current faction of selected unit |
| influence | Developer | Change the Influence Chapter value |
| resource | Developer | Modify your career resource value |
| gatheringskill | GM | Changes your proficiency in your current gathering skill |
| craftingskill | GM | Changes your proficiency in your current crafting skill |
| keepguild | GM | Claims or removes claim on keep for a guild |
| contribution | GM | Sets a player's contribution |

### `.mount` — Mount commands
| Command | Level | Description |
|---|---|---|
| set | GM | Changes the mount of the selected unit (int Entry) |
| add | Developer | Adds a new mount to the database `<Entry, Speed, Name>` |
| remove | GM | Removes the mount of the selected unit |
| list | GM | Shows the list of all mounts |

### `.npc` — NPC commands
| Command | Level | Description |
|---|---|---|
| spawn | Developer | Spawn an NPC |
| remove | Developer | Delete the target `<0=World, 1=Database>` |
| go | Developer | NPC go to target `<X, Y, Z>` |
| come | Developer | Move target to your position |
| modify | Developer | Modify a column value `<columnname, value, 0=target/1=all>` |
| quote | Developer | Adds speech to targeted NPC by spawn |
| tint | Developer | Sets armor piece color `<slotIndex, pri_tint, sec_tint>` |
| animscript | Developer | Sets monster's animation script (temporary) |
| animationset | Developer | Sets monster's animation script (permanent, updates DB) |
| level | Developer | Sets NPC level to specified value |
| disable | Developer | Disables NPC from spawns (restorable via DB) |
| move | Developer | Makes NPC come to player and updates DB position |
| addtoevent | Developer | Adds NPC to event (currently non-functional) |
| health | Developer | Sets NPC/GO health to a specified percent |
| keepspawn | Developer | Spawn a keep NPC `<destroId, orderId, keepId (0=auto)>` |
| keepnpcmove | Developer | Moves a keep NPC |

### `.pq` — Public Quest commands
| Command | Level | Description |
|---|---|---|
| add | Developer | Spawn a PQ NPC `<object id, objective id, type>` |
| convert | Developer | Converts selected object to a PQ spawn `<objective id>` |
| next | GM | Advances to the next PQ stage |
| clear | GM | Despawns all NPCs of the current stage |
| reset | GM | Resets the current PQ |

### `.respawn` — Respawn modification commands
| Command | Level | Description |
|---|---|---|
| add | Developer | Add respawn point at your position `<1=Order/2=Destruction>` |
| modify | Developer | Modify an existing point to your position `<ID>` |
| remove | Developer | Delete an existing point `<ID>` |

### `.respec` — Respecialization commands
| Command | Level | Description |
|---|---|---|
| gathering | Developer | Respec gathering skill |
| crafting | Developer | Respec crafting skill |
| mastery | Staff | Resets your career mastery points |

### `.scenario` — Scenario commands
| Command | Level | Description |
|---|---|---|
| balance | Developer | Checks the current scenario's balance internals |
| domination | Developer | Checks the current scenario's domination internals |

### `.search` — Search commands
| Command | Level | Description |
|---|---|---|
| item | GM | Search an item by name |
| npc | GM | Search an NPC by name |
| gameobject | GM | Search a game object by name |
| quest | GM | Search a quest by name |
| zone | GM | Search a zone by name |
| inventory | GM | Search inventory of selected target (optional filter) |

### `.states` — State modification commands
| Command | Level | Description |
|---|---|---|
| add | Developer | Add state to target `<Id>` |
| remove | Developer | Remove state from target `<Id>` |
| list | GM | Show target's state list |

### `.teleport` — Teleportation commands
| Command | Level | Description |
|---|---|---|
| map | GM | Teleport to world coordinates in a zone `<ZoneID, X, Y, Z>` |
| center | GM | Teleport to a derived safe point in a map `<zone [order\|destruction\|neutral]>` |
| entry | GM | Teleport to the portal-arrival point for a map `<zone [order\|destruction\|neutral]>` |
| zonejump | GM | Alias of `.teleport entry` |
| info | GM | Display the derived safe/entry points for a map |
| appear | GM | Teleport to a player's location `<playerName>` |
| summon | GM | Summon a player/group to your location `<playerName [GROUP]>` |
| set | GM | Set offline/online player's DB coordinates `<player_name, ZoneID, X, Y, Z>` |
| objective | GM | Teleport to a BattlefieldObjective `<ObjectiveId>` |

### `.ticket` — Ticket management commands
| Command | Level | Description |
|---|---|---|
| listtickets | Developer | Lists current tickets |
| assign | Developer | Assigns a person to a ticket `<accountname, bugreportID>` |
| assignme | Developer | Assigns yourself to a ticket `<bugreportID>` |
| numberoftickets | Developer | Shows how many tickets currently exist |
| deleteticket | Developer | Deletes a ticket `<bugreportID>` |
| answer | Developer | Answers and closes a ticket `<reportID, message>` |

### `.waypoints` — Waypoint commands
| Command | Level | Description |
|---|---|---|
| add | Developer | Adds a waypoint at your position to your current target |
| remove | Developer | Remove a waypoint from target `<Id>` |
| move | Developer | Moves the specified waypoint to your position `<Id>` |
| list | Developer | Shows the list of waypoints |
| info | Developer | Shows information about the current waypoint |

### `.setting` — World settings commands
Built dynamically via `CommandsBuilder.BuildCommands(typeof(SettingCommands))` — use `.gmcommands setting` in-game for the live list.

---

## Standalone (top-level) commands

### Available to any player
| Command | Description |
|---|---|
| `.grouprefresh` | Tries to rediscover all players within your group. |
| `.scenariostatus` | Shows player count and score of all running scenarios. |
| `.scenarioscore` | Returns target's scenario scores. |
| `.gmlist` | Lists available GMs. |
| `.rules` | Sends a condensed list of in-game rules. |
| `.assist` | Switches to friendly target's target. |
| `.unlock` | Fixes stuck-in-combat problems preventing you from joining a scenario. |
| `.tellblock` | Blocks whispers from non-staff players outside your guild. |
| `.getstats` | Shows your own linear stat bonuses. |
| `.ping` | Returns an average of 10 latency packets. |
| `.sorenable` | Enables the SoR addon. |

### Developer+
| Command | Description |
|---|---|
| `.ping` (Developer variant) | Shows player ping. |
| `.debugmode` | Enables debugging messages (byte enableDebug). Requires Staff outside DEBUG builds. |

### GM+
| Command | Description |
|---|---|
| `.gmcommands <path> [--long]` | Lists available GM commands and explains them. |
| `.standard` | Assigns Standard Bearer title to the player. |
| `.ror` | Help files for RoR-specific features. |
| `.changename <newName>` | Requests a name change (one per account per month). |
| `.spooky` / `.notspooky` | Halloween event visual toggle. |
| `.morph` | Makes you morph. |
| `.togglerank` | Toggles whether your staff rank shows in chat. |
| `.name <Surname>` | Changes your last name (`clear` to remove). |
| `.info` | Prints general information about your current target. |
| `.doorinfo` | Prints door information about your current target. |
| `.aiinfo` | Sends info about the targeted creature's AI state. |
| `.kill` | Slays the targeted unit. |
| `.wound` | Wounds the targeted unit. |
| `.nuke <realm> <radius>` | Slays everyone in a radius. Realm: 0=all, 1=order, 2=destro. |
| `.clearboot` | Removes all players from the server. |
| `.revive` | Resurrects the targeted unit. |
| `.fly <0/1>` | Grants/removes the ability to fly. |
| `.skavenform <gutterrunner\|engineer\|ratogre\|off>` | Take/end a Skaven monster form. |
| `.announce <message>` | Sends a global message (optionally `.announce sound <id> <message>`). |
| `.shroud` | Makes you invisible to other players. |
| `.invincible` | Toggles invulnerability on the current target. |
| `.invmob` | Toggles invulnerability on the character. |
| `.xpmode <0/1>` | Toggles whether you gain XP. |
| `.gps` | Prints your current target's position. |
| `.setnpcmodel <modelID>` | Temporarily overlays an NPC model on target (0 to clear). |
| `.setvfxstate <objectID> <vfxID>` | Sets a game object's effect state. |
| `.getobjects` | Lists OIDs of all game objects in range. |
| `.packet2` | Custom command to check how a variable works. |
| `.advice` | Messages Advice with a generic name. |
| `.csr` | Sends a CSR-type message to your realm. |
| `.motd <message>` | Changes the Message of the Day. |
| `.findip <string>` | Lists all players with an IP starting with the given string. |
| `.getonlinechar <accountName>` | Looks for online characters from the given account. |
| `.getchar <charname>` | Lists all characters on the account for a given character name. |
| `.checklog <characterName>` | Checks the sanction history for a player. |
| `.note <characterName> <Reason>` | Creates a user note. |
| `.warn <characterName> <Reason>` | Issues a logged warning. |
| `.blockadvice <characterName>` | Blocks a player from using Advice. |
| `.mute <characterName> [duration] [durationString] [Reason]` | Mutes/unmutes a player. |
| `.eject <characterName> <Reason>` | Closes the client of the named player. |
| `.sever <characterName>` | Force-disconnects the player. |
| `.exile <characterName> <duration> <durationString> <Reason>` | Exiles the player. |
| `.permaban <characterName> <Reason>` | Permanently bans the player. |
| `.unban <characterName> <Reason>` | Lifts any ban on the player. |
| `.annihilate <accountName>` | Wipes the player's account (asks for confirmation). |
| `.getcharslots <accountId>` | Displays character names and slots for the account. |
| `.deletecharat <accountName> <slotId>` | Removes a character from the given account/slot. |
| `.togglequest <QuestID> <0=Memory/1=Database>` | Turns a quest on or off. |
| `.getguildid <GuildName>` | Gets the guild ID. |
| `.guildadd <playerName> <guildName>` | Adds a player to a guild. |
| `.guildremove <playerName>` | Removes a player from their guild. |
| `.blockname <name> <Equals\|StartsWith\|Contains>` | Prevents a name from being used. |
| `.unblockname <name>` | Removes a character name filter. |
| `.listblockednames` | Lists all blocked names. |
| `.removequests <player>` | Removes all quests from a player. |
| `.hide` | Hides you from `.gmlist`. |
| `.previewmodel <slotIndex> <modelID>` | Temporarily sets an equipped item's model. |

### Staff+
| Command | Description |
|---|---|
| `.debugmode` (non-DEBUG build) | Enables debugging messages. |
| `.prevpos <millisecondDelta>` | Creates an object at a player's past position. |
| `.latency <latency>` | Overrides the server's opinion of your latency. |
| `.getguildlead <guildname>` | Returns the guild leader for a guild. |

### Admin
| Command | Description |
|---|---|
| `.boot` | Cleanly shuts the server down (saves all data, kicks all players, exits). |
| `.modify level` | Changes the level of the targeted player. |
| `.modify renown` | Changes the renown rank of a player. |
| `.modify access` | Changes the access level of an account. |

### Developer-only (dev/debug tooling)
| Command | Description |
|---|---|
| `.playeffect <a> <b>` | Play effect from `data/gamedata/effectlist.csv`. |
| `.playability <playerName> <effectID>` | Plays ability from `data/gamedata/effect.csv`. |
| `.playsound <soundID:delayInSeconds> ...` | Play sound(s) from `data/gamedata/audio_server.csv`. |
| `.save` | Performs a database save on the target. |
| `.objectivestate <oid>` | Sets VFX for an objective. |
| `.recreateplayer` | Requests player info be resent. |
| `.quest <QuestId> <Operation>` | Debug quests: 1=add, 2=finish, 3=delete. |
| `.givebag <Rarity><Item1><Item2>...` | Gives a character a bag (dev-only). |
| `.lockcasting <0/1>` | Prevents all casting. |
| `.mindread` | Causes target creature to broadcast its AI. |
| `.collatelength <n>` | Sets max send-buffer length before `Socket.Send()` fires. |
| `.scenariocmd` | Sends a custom command into the scenario handler. |
| `.scout` | Scouts for champions in the current region. |
| `.alltoks <n>` | Enables all toks. |
| `.packetlog <playername>` | Turns on packet logging for a player's account. |
| `.tokbestiary <playername>` | Resends the player's real bestiary/ward action counters. |
| `.tometactic` | Reports which tome tactic effects apply to the current target. |
| `.guildlogon <guild>` / `.guildlogoff <guild>` | Start/stop logging a guild's chat. |
| `.structure` | Creates a structure. |
| `.bolsterlevel <n>` | Changes max bolster level for T2/T3/T4. |
| `.playerdrop <n>` | Changes how medallions are generated for killed players. |
| `.beastmaster` | Changes your White Lion to Beastmaster. |
| `.setpet <n>` | Changes your pet model. |
| `.summonpet <n>` | Summons the requested pet. |
| `.spawnmobinstance <n>` | Spawns one or more mobs (not saved to DB). |
| `.sendkeepinfo` / `.sendkeepstatus` | Sends KeepInfo/KeepStatus messages. |
| `.sendcampaignstatus` | Sends a Campaign Status message. |
| `.dynamicvendor` | Creates a dynamic vendor. |
| `.creategatehouseportal` | Creates a portal to Southern Garrison gatehouse. |
| `.realmcaptain` | Makes the targeted player a realm captain. |
| `.spawnboss <protoid>` | Spawns a boss. |
| `.summongoremane`, `.summonkokrit`, `.summonbulbousone`, `.summonazukthul`, `.summonborzhar`, `.summongahlvoth`, `.summonzekaraz`, `.summonlordslaurith`, `.summonorcapult` | Spawns specific named bosses. |
| `.creategoldchest` | Summons a gold chest. |
| `.forcelockzone` | Force-locks a fort zone. |
| `.mailitem <a> <b> <c>` | Mails an item to a character. |
| `.testscoreboard` | Tests the scoreboard. |
| `.title <n>` | Sets a player's title. |
| `.bulkmail <file>` | Mails items from a file to characters. |

---

*Generated from the `RESTART` branch source (`WorldServer/Managers/Commands/CommandDeclarations.cs`) on 2026-09-09. Some groups (`campaign`, `go`, `lotd`, `ward`, `setting`) are built at runtime via reflection and aren't fully enumerated in source — run `.gmcommands <group>` in-game to see their live contents.*
