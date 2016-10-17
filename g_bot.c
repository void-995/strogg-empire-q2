/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include "g_local.h"
#include "m_player.h"

qboolean ClientConnect(edict_t *ent, char *userinfo);
void ClientBegin(edict_t *ent);
void ClientThink(edict_t *ent, usercmd_t *ucmd);
void ClientUserinfoChanged(edict_t *ent, char *userinfo);

qboolean G_IsControlledByAI(edict_t *ent)
{
	if (!ent->client) {
		return qfalse;
	}

	return ent->client->pers.ai_controlled; 
}

static void G_BotCheatWeapons(edict_t *self)
{
	int i;
	gitem_t *it;

	for (i = 0; i < ITEM_TOTAL; i++) {
        it = INDEX_ITEM(i);

        if (!it->pickup)
            continue;

        if (!(it->flags & IT_WEAPON))
            continue;

        self->client->inventory[i] += 1;
    }

    for (i = 0; i < ITEM_TOTAL; i++) {
        it = INDEX_ITEM(i);

        if (!it->pickup)
            continue;

        if (!(it->flags & IT_AMMO))
            continue;

        Add_Ammo(self, it, 1000);
    }
}

static void G_BotUseWeapon(edict_t *self, int weapon_index)
{
	gitem_t *it;

	it = INDEX_ITEM(weapon_index + ITEM_BLASTER - 1);
	it->use(self, it);
}

static void G_BotRunFrame(edict_t *self)
{
	int i;
	trace_t tr;
	usercmd_t ucmd;
	edict_t *player;
	edict_t *enemy;
	float player_distance;
	float enemy_distance;
	vec3_t eye;
	vec3_t target;
	vec3_t delta;
	vec3_t angles;

	VectorCopy(self->client->ps.viewangles, self->s.angles);
	VectorSet(self->client->ps.pmove.delta_angles, 0.f, 0.f, 0.f);

	memset(&ucmd, 0, sizeof(usercmd_t));

	ucmd.angles[PITCH] = ANGLE2SHORT(self->s.angles[PITCH]);
	ucmd.angles[YAW] = ANGLE2SHORT(self->s.angles[YAW]);
	ucmd.angles[ROLL] = ANGLE2SHORT(self->s.angles[ROLL]);

	if (self->deadflag || self->client->ps.pmove.pm_type == PM_SPECTATOR) {
		self->client->buttons = 0;
		ucmd.buttons = BUTTON_ATTACK;
	} else {
		if (random() < 0.125f) {
			G_BotCheatWeapons(self);

			if (random() < 0.125f) {
				G_BotUseWeapon(self, floor(random() * (WEAP_BFG - 2)) + 2);
			}
		}

		enemy = NULL;
		enemy_distance = 65536.f;

		for (i = 1; i <= game.maxclients; i++) {
			player = &g_edicts[i];

			if (player == self || player->deadflag || player->client->ps.pmove.pm_type == PM_SPECTATOR) {
				continue;
			}

			VectorCopy(self->s.origin, eye);
			eye[PLANE_Z] += self->viewheight;

			VectorCopy(player->s.origin, target);
			target[PLANE_Z] += player->viewheight * 0.5f;

			tr = gi.trace(eye, NULL, NULL, target, self, MASK_SHOT);

			if (tr.ent == player) {
				VectorSubtract(player->s.origin, self->s.origin, delta);
				player_distance = VectorLength(delta);

				if (enemy_distance > player_distance) {
					enemy_distance = player_distance;
					enemy = player;
				}
			}
		}

		if (enemy != NULL) {
			VectorCopy(self->s.origin, eye);
			eye[PLANE_Z] += self->viewheight;

			VectorCopy(enemy->s.origin, target);
			target[PLANE_Z] += enemy->viewheight * 0.5f;

			VectorSubtract(target, eye, delta);
			vectoangles(delta, angles);

			ucmd.angles[PITCH] = ANGLE2SHORT(angles[PITCH]);
			ucmd.angles[YAW] = ANGLE2SHORT(angles[YAW]);

			ucmd.forwardmove = enemy_distance - 192;

			if (abs(ucmd.forwardmove) > 64) {
				ucmd.forwardmove *= 64;
			}

			if (random() < 0.5f) {
				ucmd.sidemove = 1500 * crandom();
			}

			if (random() < 0.125f) {
				ucmd.upmove = 1000;
			} else  if (random() < 0.35f) {
				ucmd.upmove = -1000;
			}

			ucmd.buttons = random() < 0.95f ? BUTTON_ATTACK : 0;
		} else {
			if (level.framenum % 100 < 90) {
				if (random() < 0.5f) {
					ucmd.forwardmove = 1500 * crandom();
				}

				if (random() < 0.35f) {
					ucmd.sidemove = 1000 * crandom();
				}
			}
		}
	}

	ucmd.msec = 1000 / HZ;

	self->client->ping = 25 + floor(random() * 25) + 1;

	ClientThink(self, &ucmd);
}

void G_BotsRunFrame(void)
{
	int i;
	edict_t *player;

	for (i = 1; i <= game.maxclients; i++) {
		player = &g_edicts[i];

		if (G_IsControlledByAI(player)) {
			G_BotRunFrame(player);
		}
	}
}

edict_t *G_BotFindFreeClientEntity(void)
{
	int         i;
	edict_t     *player;
	gclient_t   *client;

	for (i = game.maxclients; i >= 1; i--) {
		player = &g_edicts[i];
		client = game.clients + (player - g_edicts - 1);

		if (!player->inuse && !client->pers.connected) {
			player->client = client;
			return player;
		}
	}

	return NULL;
}

void Cmd_Addbot_f(edict_t *ent)
{
	qboolean client_can_add_bots;
	char botuserinfo[MAX_STRING_CHARS];

	edict_t *bot;

	client_can_add_bots = ent->client->pers.admin || ent->client->pers.loopback;

	if (!client_can_add_bots) {
		gi.cprintf(ent, PRINT_HIGH, "You are not allowed to add bots.\n");
		return;
	}

	sprintf(botuserinfo, "\\rate\\25000\\msg\\1\\fov\\90\\skin\\%s/%s\\name\\%s\\hand\\0\\ip\\127.0.0.1", "male", "grunt", "Bot");

	bot = G_BotFindFreeClientEntity();

	if (bot != NULL) {
		if (ClientConnect(bot, botuserinfo)) {
			bot->client->pers.ai_controlled = qtrue;

			ClientUserinfoChanged(bot, botuserinfo);
			ClientBegin(bot);
		} else {
			gi.cprintf(ent, PRINT_HIGH, "Adding Bot Failed: Free client slot found, but couldn't connect it!\n");
		}
	} else {
		gi.cprintf(ent, PRINT_HIGH, "Adding Bot Failed: No free client slots found!\n");
	}
}
