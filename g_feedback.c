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
		if (damage_given < 25) {
			gi.sound(attacker, CHAN_FEEDBACK, gi.soundindex("misc/hit_3.wav"), 1.f, ATTN_STATIC, 0);
		} else if (damage_given < 50) {
			gi.sound(attacker, CHAN_FEEDBACK, gi.soundindex("misc/hit_2.wav"), 1.f, ATTN_STATIC, 0);
		} else if (damage_given < 75) {
			gi.sound(attacker, CHAN_FEEDBACK, gi.soundindex("misc/hit_1.wav"), 1.f, ATTN_STATIC, 0);
		} else {
			gi.sound(attacker, CHAN_FEEDBACK, gi.soundindex("misc/hit_0.wav"), 1.f, ATTN_STATIC, 0);
		}
	}
}

void G_Feedback_ClientKilled(edict_t *attacker, edict_t *victim)
{
	if (attacker->client && victim->client && attacker != victim) {
		gi.sound(attacker, CHAN_FEEDBACK, gi.soundindex("misc/kill.wav"), 1.f, ATTN_STATIC, 0);
		
		gi.centerprintf(attacker, "You fragged %s", victim->client->pers.netname);
		gi.centerprintf(victim, "Fragged by %s", attacker->client->pers.netname);
	}
}
