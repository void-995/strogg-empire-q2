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

static const char *G_Feedback_PlaceString(int rank)
{
	static char	buffer[64];
	char *place;

	if (rank == 1) {
		place = "1st";
	} else if (rank == 2) {
		place = "2nd";
	} else if (rank == 3) {
		place = "3rd";
	} else if (rank == 11) {
		place = "11th";
	} else if (rank == 12) {
		place = "12th";
	} else if (rank == 13) {
		place = "13th";
	} else if (rank % 10 == 1) {
		place = va("%ist", rank);
	} else if (rank % 10 == 2) {
		place = va("%ind", rank);
	} else if (rank % 10 == 3) {
		place = va("%ird", rank);
	} else {
		place = va("%ith", rank);
	}

	Q_snprintf(buffer, sizeof(buffer), "%s", place);

	return buffer;
}

void G_Feedback_Init()
{
	gi.soundindex("misc/hit_3.wav");
	gi.soundindex("misc/hit_2.wav");
	gi.soundindex("misc/hit_1.wav");
	gi.soundindex("misc/hit_0.wav");
	gi.soundindex("misc/kill.wav");

	gi.soundindex("misc/excellent.wav");
	gi.soundindex("misc/impressive.wav");

	gi.soundindex("misc/lostlead.wav");
	gi.soundindex("misc/tiedlead.wav");
	gi.soundindex("misc/takenlead.wav");
}

void G_Feedback_ClientHit(edict_t *attacker, edict_t *victim, int damage_given)
{
	if (attacker->client && victim->client && attacker != victim) {
		if (level.framenum >= attacker->client->next_feedback_time) {
			if (damage_given < 25) {
				gi.sound(attacker, CHAN_FEEDBACK, gi.soundindex("misc/hit_3.wav"), 1.f, ATTN_STATIC, 0);
			} else if (damage_given < 50) {
				gi.sound(attacker, CHAN_FEEDBACK, gi.soundindex("misc/hit_2.wav"), 1.f, ATTN_STATIC, 0);
			} else if (damage_given < 75) {
				gi.sound(attacker, CHAN_FEEDBACK, gi.soundindex("misc/hit_1.wav"), 1.f, ATTN_STATIC, 0);
			} else {
				gi.sound(attacker, CHAN_FEEDBACK, gi.soundindex("misc/hit_0.wav"), 1.f, ATTN_STATIC, 0);
			}

			attacker->client->next_feedback_time = level.framenum + 0.03125f * HZ;
		}
	}
}

void G_Feedback_ClientKilled(edict_t *attacker, edict_t *victim)
{
	char attacker_message[MAX_STRING_CHARS];
	char victim_message[MAX_STRING_CHARS];

	int i, ranks_count, attacker_rank, victim_rank;
	gclient_t *ranks[MAX_CLIENTS];

	if (attacker->client && victim->client) {
		memset(attacker_message, 0, MAX_STRING_CHARS);
		memset(victim_message, 0, MAX_STRING_CHARS);

		attacker_rank = -1;
		victim_rank = -1;

		ranks_count = G_CalcRanks(ranks);

		for (i = 0; i < ranks_count; i++) {
			if (ranks[i] == attacker->client) {
				attacker_rank = i;
			}

			if (ranks[i] == victim->client) {
				victim_rank = i;
			}
		}

		if (attacker != victim) {
			gi.sound(attacker, CHAN_FEEDBACK, gi.soundindex("misc/kill.wav"), 1.f, ATTN_STATIC, 0);

			Q_snprintf(attacker_message, MAX_STRING_CHARS, "You fragged %s\n%s place with %d", victim->client->pers.netname, G_Feedback_PlaceString(attacker_rank + 1), attacker->client->resp.score);
			Q_snprintf(victim_message, MAX_STRING_CHARS, "Fragged by %s\n", attacker->client->pers.netname);

			if (!G_IsControlledByAI(attacker)) {
				gi.centerprintf(attacker, "%s", attacker_message);
			}
		}

		Q_snprintf(victim_message, MAX_STRING_CHARS, "%s%s place with %d", victim_message, G_Feedback_PlaceString(victim_rank + 1), victim->client->resp.score);

		if (!G_IsControlledByAI(victim)) {
			gi.centerprintf(victim, "%s", victim_message);
		}
	}
}
