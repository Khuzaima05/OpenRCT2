/*****************************************************************************
 * Copyright (c) 2014 Ted John
 * OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
 *
 * This file is part of OpenRCT2.
 *
 * OpenRCT2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#include "../addresses.h"
#include "../audio/audio.h"
#include "../audio/mixer.h"
#include "../config.h"
#include "../hook.h"
#include "../interface/viewport.h"
#include "../openrct2.h"
#include "../scenario.h"
#include "../world/map_animation.h"
#include "../world/sprite.h"
#include "ride.h"
#include "ride_data.h"
#include "track.h"
#include "vehicle.h"

rct_xyz16 *unk_F64E20 = (rct_xyz16*)0x00F64E20;

// Size: 0x09
typedef struct {
	uint16 x;			// 0x00
	uint16 y;			// 0x02
	uint16 z;			// 0x04
	uint8 direction;	// 0x06
	uint8 var_07;
	uint8 var_08;
} rct_vehicle_info;

static void vehicle_update(rct_vehicle *vehicle);

/**
*
*  rct2: 0x006BB9FF
*/
void vehicle_update_sound_params(rct_vehicle* vehicle)
{
	if (!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_SCENARIO_EDITOR) && (!(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_DESIGNER) || RCT2_GLOBAL(0x0141F570, uint8) == 6)) {
		if (vehicle->sound1_id != (uint8)-1 || vehicle->sound2_id != (uint8)-1) {
			if (vehicle->sprite_left != (sint16)0x8000) {
				RCT2_GLOBAL(0x009AF5A0, sint16) = vehicle->sprite_left;
				RCT2_GLOBAL(0x009AF5A2, sint16) = vehicle->sprite_top;
				RCT2_GLOBAL(0x009AF5A4, sint16) = vehicle->sprite_right;
				RCT2_GLOBAL(0x009AF5A6, sint16) = vehicle->sprite_bottom;
				sint16 v4 = RCT2_GLOBAL(0x00F438A4, rct_viewport*)->view_x;
				sint16 v5 = RCT2_GLOBAL(0x00F438A4, rct_viewport*)->view_y;
				sint16 v6 = RCT2_GLOBAL(0x00F438A4, rct_viewport*)->view_width / 4;
				sint16 v7 = RCT2_GLOBAL(0x00F438A4, rct_viewport*)->view_height / 4;
				if (!RCT2_GLOBAL(0x00F438A8, rct_window*)->classification) {
					v4 -= v6;
					v5 -= v7;
				}
				if (v4 < RCT2_GLOBAL(0x009AF5A4, sint16) && v5 < RCT2_GLOBAL(0x009AF5A6, sint16)) {
					sint16 t8 = RCT2_GLOBAL(0x00F438A4, rct_viewport*)->view_width + v4;
					sint16 t9 = RCT2_GLOBAL(0x00F438A4, rct_viewport*)->view_height + v5;
					if (!RCT2_GLOBAL(0x00F438A8, rct_window*)->classification) {
						t8 += v6 + v6;
						t9 += v7 + v7;
					}
					if (t8 >= RCT2_GLOBAL(0x009AF5A0, sint16) && t9 >= RCT2_GLOBAL(0x009AF5A2, sint16)) {
						uint16 v9 = sub_6BC2F3(vehicle);
						rct_vehicle_sound_params* i;
						for (i = &gVehicleSoundParamsList[0]; i < gVehicleSoundParamsListEnd && v9 <= i->var_A; i++);
						if (i < &gVehicleSoundParamsList[countof(gVehicleSoundParamsList)]) {
							if (gVehicleSoundParamsListEnd < &gVehicleSoundParamsList[countof(gVehicleSoundParamsList)]) {
								gVehicleSoundParamsListEnd++;
							}
							rct_vehicle_sound_params* j = gVehicleSoundParamsListEnd - 1;
							while (j >= i) {
								j--;
								*(j + 1) = *j;
							}
							i->var_A = v9;
							int pan_x = (RCT2_GLOBAL(0x009AF5A0, sint16) / 2) + (RCT2_GLOBAL(0x009AF5A4, sint16) / 2) - RCT2_GLOBAL(0x00F438A4, rct_viewport*)->view_x;
							pan_x >>= RCT2_GLOBAL(0x00F438A4, rct_viewport*)->zoom;
							pan_x += RCT2_GLOBAL(0x00F438A4, rct_viewport*)->x;

							uint16 screenwidth = RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_WIDTH, uint16);
							if (screenwidth < 64) {
								screenwidth = 64;
							}
							i->pan_x = ((((pan_x << 16) / screenwidth) - 0x8000) >> 4);

							int pan_y = (RCT2_GLOBAL(0x009AF5A2, sint16) / 2) + (RCT2_GLOBAL(0x009AF5A6, sint16) / 2) - RCT2_GLOBAL(0x00F438A4, rct_viewport*)->view_y;
							pan_y >>= RCT2_GLOBAL(0x00F438A4, rct_viewport*)->zoom;
							pan_y += RCT2_GLOBAL(0x00F438A4, rct_viewport*)->y;

							uint16 screenheight = RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_HEIGHT, uint16);
							if (screenheight < 64) {
								screenheight = 64;
							}
							i->pan_y = ((((pan_y << 16) / screenheight) - 0x8000) >> 4);

							sint32 v19 = vehicle->velocity;

							rct_ride_type* ride_type = GET_RIDE_ENTRY(vehicle->ride_subtype);
							uint8 test = ride_type->vehicles[vehicle->vehicle_type].var_5A;

							if (test & 1) {
								v19 *= 2;
							}
							if (v19 < 0) {
								v19 = -v19;
							}
							v19 >>= 5;
							v19 *= 5512;
							v19 >>= 14;
							v19 += 11025;
							v19 += 16 * vehicle->var_BF;
							i->frequency = (uint16)v19;
							i->id = vehicle->sprite_index;
							i->volume = 0;
							if (vehicle->x != (sint16)0x8000) {
								int tile_idx = (((vehicle->y & 0xFFE0) * 256) + (vehicle->x & 0xFFE0)) / 32;
								rct_map_element* map_element;
								for (map_element = RCT2_ADDRESS(RCT2_ADDRESS_TILE_MAP_ELEMENT_POINTERS, rct_map_element*)[tile_idx]; map_element->type & MAP_ELEMENT_TYPE_MASK; map_element++);
								if (map_element->base_height * 8 > vehicle->z) { // vehicle underground
									i->volume = 0x30;
								}
							}
						}
					}
				}
			}
		}
	}
}

/**
*
*  rct2: 0x006BC2F3
*/
int sub_6BC2F3(rct_vehicle* vehicle)
{
	int result = 0;
	rct_vehicle* vehicle_temp = vehicle;
	do {
		result += vehicle_temp->friction;
	} while (vehicle_temp->next_vehicle_on_train != (uint16)-1 && (vehicle_temp = GET_VEHICLE(vehicle_temp->next_vehicle_on_train)));
	sint32 v4 = vehicle->velocity;
	if (v4 < 0) {
		v4 = -v4;
	}
	result += ((uint16)v4) >> 13;
	rct_vehicle_sound* vehicle_sound = &gVehicleSoundList[0];

	while (vehicle_sound->id != vehicle->sprite_index) {
		vehicle_sound++;

		if (vehicle_sound >= &gVehicleSoundList[countof(gVehicleSoundList)]) {
			return result;
		}
	}
	return result + 300;
}

/**
*
*  rct2: 0x006BBC6B
*/
void vehicle_sounds_update()
{
	if (RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_SOUND_DEVICE, uint32) != -1 && !gGameSoundsOff && gConfigSound.sound && !gOpenRCT2Headless) {
		RCT2_GLOBAL(0x00F438A4, rct_viewport*) = (rct_viewport*)-1;
		rct_viewport* viewport = (rct_viewport*)-1;
		rct_window* window = RCT2_GLOBAL(RCT2_ADDRESS_NEW_WINDOW_PTR, rct_window*);
		while (1) {
			window--;
			if (window < RCT2_ADDRESS(RCT2_ADDRESS_WINDOW_LIST, rct_window)) {
				break;
			}
			viewport = window->viewport;
			if (viewport && viewport->flags & VIEWPORT_FLAG_SOUND_ON) {
				break;
			}
		}
		RCT2_GLOBAL(0x00F438A4, rct_viewport*) = viewport;
		if (viewport != (rct_viewport*)-1) {
			if (window) {
				RCT2_GLOBAL(0x00F438A8, rct_window*) = window;
				RCT2_GLOBAL(RCT2_ADDRESS_VOLUME_ADJUST_ZOOM, uint8) = 0;
				if (viewport->zoom) {
					RCT2_GLOBAL(RCT2_ADDRESS_VOLUME_ADJUST_ZOOM, uint8) = 35;
					if (viewport->zoom != 1) {
						RCT2_GLOBAL(RCT2_ADDRESS_VOLUME_ADJUST_ZOOM, uint8) = 70;
					}
				}
			}
			gVehicleSoundParamsListEnd = &gVehicleSoundParamsList[0];
			for (uint16 i = RCT2_GLOBAL(RCT2_ADDRESS_SPRITES_START_VEHICLE, uint16); i != SPRITE_INDEX_NULL; i = g_sprite_list[i].vehicle.next) {
				vehicle_update_sound_params(&g_sprite_list[i].vehicle);
			}
			for(int i = 0; i < countof(gVehicleSoundList); i++){
				rct_vehicle_sound* vehicle_sound = &gVehicleSoundList[i];
				if (vehicle_sound->id != (uint16)-1) {
					for (rct_vehicle_sound_params* vehicle_sound_params = &gVehicleSoundParamsList[0]; vehicle_sound_params != gVehicleSoundParamsListEnd; vehicle_sound_params++) {
						if (vehicle_sound->id == vehicle_sound_params->id) {
							goto label26;
						}
					}
					if (vehicle_sound->sound1_id != (uint16)-1) {
						Mixer_Stop_Channel(vehicle_sound->sound1_channel);
					}
					if (vehicle_sound->sound2_id != (uint16)-1) {
						Mixer_Stop_Channel(vehicle_sound->sound2_channel);
					}
					vehicle_sound->id = (uint16)-1;
				}
			label26:
				;
			}

			for (rct_vehicle_sound_params* vehicle_sound_params = &gVehicleSoundParamsList[0]; ; vehicle_sound_params++) {
			label28:
				if (vehicle_sound_params >= gVehicleSoundParamsListEnd) {
					return;
				}
				uint8 vol1 = 0xFF;
				uint8 vol2 = 0xFF;
				sint16 pan_y = vehicle_sound_params->pan_y;
				if (pan_y < 0) {
					pan_y = -pan_y;
				}
				if (pan_y > 0xFFF) {
					pan_y = 0xFFF;
				}
				pan_y -= 0x800;
				if (pan_y > 0) {
					pan_y -= 0x400;
					pan_y = -pan_y;
					pan_y = pan_y / 4;
					vol1 = LOBYTE(pan_y);
					if ((sint8)HIBYTE(pan_y) != 0) {
						vol1 = 0xFF;
						if ((sint8)HIBYTE(pan_y) < 0) {
							vol1 = 0;
						}
					}
				}

				sint16 pan_x = vehicle_sound_params->pan_x;
				if (pan_x < 0) {
					pan_x = -pan_x;
				}
				if (pan_x > 0xFFF) {
					pan_x = 0xFFF;
				}
				pan_x -= 0x800;
				if (pan_x > 0) {
					pan_x -= 0x400;
					pan_x = -pan_x;
					pan_x = pan_x / 4;
					vol2 = LOBYTE(pan_x);
					if ((sint8)HIBYTE(pan_x) != 0) {
						vol2 = 0xFF;
						if ((sint8)HIBYTE(pan_x) < 0) {
							vol2 = 0;
						}
					}
				}

				if (vol1 >= vol2) {
					vol1 = vol2;
				}
				if (vol1 < RCT2_GLOBAL(RCT2_ADDRESS_VOLUME_ADJUST_ZOOM, uint8)) {
					vol1 = 0;
				} else {
					vol1 = vol1 - RCT2_GLOBAL(RCT2_ADDRESS_VOLUME_ADJUST_ZOOM, uint8);
				}

				rct_vehicle_sound* vehicle_sound = &gVehicleSoundList[0];
				while (vehicle_sound_params->id != vehicle_sound->id) {
					vehicle_sound++;
					if (vehicle_sound >= &gVehicleSoundList[countof(gVehicleSoundList)]) {
						vehicle_sound = &gVehicleSoundList[0];
						int i = 0;
						while (vehicle_sound->id != (uint16)-1) {
							vehicle_sound++;
							i++;
							if (i >= countof(gVehicleSoundList)) {
								vehicle_sound_params = (rct_vehicle_sound_params*)((int)vehicle_sound_params + 10);
								goto label28;
							}
						}
						vehicle_sound->id = vehicle_sound_params->id;
						vehicle_sound->sound1_id = (uint16)-1;
						vehicle_sound->sound2_id = (uint16)-1;
						vehicle_sound->volume = 0x30;
						break;
					}
				}

				int tempvolume = vehicle_sound->volume;
				if (tempvolume != vehicle_sound_params->volume) {
					if (tempvolume < vehicle_sound_params->volume) {
						tempvolume += 4;
					} else {
						tempvolume -= 4;
					}
				}
				vehicle_sound->volume = tempvolume;
				if (vol1 < tempvolume) {
					vol1 = 0;
				} else {
					vol1 = vol1 - tempvolume;
				}

				// do sound1 stuff, track noise
				rct_sprite* sprite = &g_sprite_list[vehicle_sound_params->id];
				int volume = sprite->vehicle.sound1_volume;
				volume *= vol1;
				volume = volume / 8;
				volume -= 0x1FFF;
				if (volume < -10000) {
					volume = -10000;
				}
				if (sprite->vehicle.sound1_id == (uint8)-1) {
					if (vehicle_sound->sound1_id != (uint16)-1) {
						vehicle_sound->sound1_id = -1;
						Mixer_Stop_Channel(vehicle_sound->sound1_channel);
					}
				} else {
					if (vehicle_sound->sound1_id == (uint16)-1) {
						goto label69;
					}
					if (sprite->vehicle.sound1_id != vehicle_sound->sound1_id) {
						Mixer_Stop_Channel(vehicle_sound->sound1_channel);
					label69:
						vehicle_sound->sound1_id = sprite->vehicle.sound1_id;
						vehicle_sound->sound1_pan = vehicle_sound_params->pan_x;
						vehicle_sound->sound1_volume = volume;
						vehicle_sound->sound1_freq = vehicle_sound_params->frequency;
						uint16 frequency = vehicle_sound_params->frequency;
						if (RCT2_ADDRESS(0x009AF51F, uint8)[2 * sprite->vehicle.sound1_id] & 2) {
							frequency = (frequency / 2) + 4000;
						}
						uint8 looping = RCT2_ADDRESS(0x009AF51E, uint8)[2 * sprite->vehicle.sound1_id];
						int pan = vehicle_sound_params->pan_x;
						vehicle_sound->sound1_channel = Mixer_Play_Effect(sprite->vehicle.sound1_id, looping ? MIXER_LOOP_INFINITE : MIXER_LOOP_NONE, DStoMixerVolume(volume), DStoMixerPan(pan), DStoMixerRate(frequency), 0);
						goto label87;
					}
					if (volume != vehicle_sound->sound1_volume) {
						vehicle_sound->sound1_volume = volume;
						Mixer_Channel_Volume(vehicle_sound->sound1_channel, DStoMixerVolume(volume));
					}
					if (vehicle_sound_params->pan_x != vehicle_sound->sound1_pan) {
						vehicle_sound->sound1_pan = vehicle_sound_params->pan_x;
						Mixer_Channel_Pan(vehicle_sound->sound1_channel, DStoMixerPan(vehicle_sound_params->pan_x));
					}
					if (!(RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_TICKS, uint32) & 3) && vehicle_sound_params->frequency != vehicle_sound->sound1_freq) {
						vehicle_sound->sound1_freq = vehicle_sound_params->frequency;
						uint16 frequency = vehicle_sound_params->frequency;
						if (RCT2_GLOBAL(0x009AF51F, uint8*)[2 * sprite->vehicle.sound1_id] & 2) {
							frequency = (frequency / 2) + 4000;
						}
						Mixer_Channel_Rate(vehicle_sound->sound1_channel, DStoMixerRate(frequency));
					}
				}
			label87: // do sound2 stuff, screams
				sprite = &g_sprite_list[vehicle_sound_params->id];
				volume = sprite->vehicle.sound2_volume;
				volume *= vol1;
				volume = (uint16)volume / 8;
				volume -= 0x1FFF;
				if (volume < -10000) {
					volume = -10000;
				}
				if (sprite->vehicle.sound2_id == (uint8)-1) {
					if (vehicle_sound->sound2_id != (uint16)-1) {
						vehicle_sound->sound2_id = -1;
						Mixer_Stop_Channel(vehicle_sound->sound2_channel);
					}
				} else {
					if (vehicle_sound->sound2_id == (uint16)-1) {
						goto label93;
					}
					if (sprite->vehicle.sound2_id != vehicle_sound->sound2_id) {
						Mixer_Stop_Channel(vehicle_sound->sound2_channel);
					label93:
						vehicle_sound->sound2_id = sprite->vehicle.sound2_id;
						vehicle_sound->sound2_pan = vehicle_sound_params->pan_x;
						vehicle_sound->sound2_volume = volume;
						vehicle_sound->sound2_freq = vehicle_sound_params->frequency;
						uint16 frequency = vehicle_sound_params->frequency;
						if (RCT2_ADDRESS(0x009AF51F, uint8)[2 * sprite->vehicle.sound2_id] & 1) {
							frequency = 12649;
						}
						frequency = (frequency * 2) - 3248;
						if (frequency > 25700) {
							frequency = 25700;
						}
						uint8 looping = RCT2_ADDRESS(0x009AF51E, uint8)[2 * sprite->vehicle.sound2_id];
						int pan = vehicle_sound_params->pan_x;
						vehicle_sound->sound2_channel = Mixer_Play_Effect(sprite->vehicle.sound2_id, looping ? MIXER_LOOP_INFINITE : MIXER_LOOP_NONE, DStoMixerVolume(volume), DStoMixerPan(pan), DStoMixerRate(frequency), 0);
						goto label114;
					}
					if (volume != vehicle_sound->sound2_volume) {
						Mixer_Channel_Volume(vehicle_sound->sound2_channel, DStoMixerVolume(volume));
						vehicle_sound->sound2_volume = volume;
					}
					if (vehicle_sound_params->pan_x != vehicle_sound->sound2_pan) {
						vehicle_sound->sound2_pan = vehicle_sound_params->pan_x;
						Mixer_Channel_Pan(vehicle_sound->sound2_channel, DStoMixerPan(vehicle_sound_params->pan_x));
					}
					if (!(RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_TICKS, uint32) & 3) && vehicle_sound_params->frequency != vehicle_sound->sound2_freq) {
						vehicle_sound->sound2_freq = vehicle_sound_params->frequency;
						if (!(RCT2_ADDRESS(0x009AF51F, uint8)[2 * sprite->vehicle.sound2_id] & 1)) {
							uint16 frequency = (vehicle_sound_params->frequency * 2) - 3248;
							if (frequency > 25700) {
								frequency = 25700;
							}
							Mixer_Channel_Rate(vehicle_sound->sound2_channel, DStoMixerRate(frequency));
						}
					}
				}
			label114:
				;
			}
		}
	}
}

/**
 *
 *  rct2: 0x006D4204
 */
void vehicle_update_all()
{
	uint16 sprite_index;
	rct_vehicle *vehicle;

	if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_SCENARIO_EDITOR)
		return;

	if ((RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & SCREEN_FLAGS_TRACK_DESIGNER) && RCT2_GLOBAL(0x0141F570, uint8) != 6)
		return;


	sprite_index = RCT2_GLOBAL(RCT2_ADDRESS_SPRITES_START_VEHICLE, uint16);
	while (sprite_index != SPRITE_INDEX_NULL) {
		vehicle = &(g_sprite_list[sprite_index].vehicle);
		sprite_index = vehicle->next;

		vehicle_update(vehicle);
	}
}

/**
 *
 *  rct2: 0x006D77F2
 */
static void vehicle_update(rct_vehicle *vehicle)
{
	RCT2_CALLPROC_X(0x006D77F2, 0, 0, 0, 0, (int)vehicle, 0, 0);
}

/**
 *
 *  rct2: 0x006D73D0
 * ax: verticalG
 * dx: lateralG
 * esi: vehicle
 */
void vehicle_get_g_forces(rct_vehicle *vehicle, int *verticalG, int *lateralG)
{
	int eax, ebx, ecx, edx, esi, edi, ebp;

	esi = (int)vehicle;
	RCT2_CALLFUNC_X(0x006D73D0, &eax, &ebx, &ecx, &edx, &esi, &edi, &ebp);

	if (verticalG != NULL) *verticalG = (sint16)(eax & 0xFFFF);
	if (lateralG != NULL) *lateralG = (sint16)(edx & 0xFFFF);
}

void vehicle_set_map_toolbar(rct_vehicle *vehicle)
{
	rct_ride *ride;
	int vehicleIndex;

	ride = GET_RIDE(vehicle->ride);

	while (vehicle->is_child) {
		vehicle = GET_VEHICLE(vehicle->prev_vehicle_on_ride);
	}

	for (vehicleIndex = 0; vehicleIndex < 32; vehicleIndex++)
		if (ride->vehicles[vehicleIndex] == vehicle->sprite_index)
			break;

	RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 0, uint16) = 2215;
	RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 2, uint16) = 1165;
	RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 4, uint16) = ride->name;
	RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 6, uint32) = ride->name_arguments;
	RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 10, uint16) = RideNameConvention[ride->type].vehicle_name + 2;
	RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 12, uint16) = vehicleIndex + 1;

	int arg0, arg1;
	ride_get_status(vehicle->ride, &arg0, &arg1);
	RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 14, uint16) = (uint16)arg0;
	RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 16, uint32) = (uint16)arg1;
}

rct_vehicle *vehicle_get_head(rct_vehicle *vehicle)
{
	rct_vehicle *prevVehicle;

	for (;;) {
		prevVehicle = GET_VEHICLE(vehicle->prev_vehicle_on_ride);
		if (prevVehicle->next_vehicle_on_train == SPRITE_INDEX_NULL)
			break;

		vehicle = prevVehicle;
	}

	return vehicle;
}

rct_vehicle *vehicle_get_tail(rct_vehicle *vehicle)
{
	uint16 spriteIndex;

	while ((spriteIndex = vehicle->next_vehicle_on_train) != SPRITE_INDEX_NULL) {
		vehicle = GET_VEHICLE(spriteIndex);
	}
	return vehicle;
}

int vehicle_is_used_in_pairs(rct_vehicle *vehicle)
{
	return vehicle->num_seats & VEHICLE_SEAT_PAIR_FLAG;
}

/**
 *
 *  rct2: 0x006DEF56
 */
void sub_6DEF56(rct_vehicle *cableLift)
{
	RCT2_CALLPROC_X(0x006DEF56, 0, 0, 0, 0, (int)cableLift, 0, 0);
}

rct_vehicle *cable_lift_segment_create(int rideIndex, int x, int y, int z, int direction, uint16 var_44, uint32 var_24, bool head)
{
	rct_ride *ride = GET_RIDE(rideIndex);
	rct_vehicle *current = &(create_sprite(1)->vehicle);
	current->sprite_identifier = SPRITE_IDENTIFIER_VEHICLE;
	current->ride = rideIndex;
	current->ride_subtype = 0xFF;
	if (head) {
		move_sprite_to_list((rct_sprite*)current, SPRITE_LINKEDLIST_OFFSET_VEHICLE);
		ride->cable_lift = current->sprite_index;
	}
	current->is_child = head ? 0 : 1;
	current->var_44 = var_44;
	current->var_24 = var_24;
	current->sprite_width = 10;
	current->sprite_height_negative = 10;
	current->sprite_height_positive = 10;
	current->friction = 100;
	current->num_seats = 0;
	current->speed = 20;
	current->acceleration = 80;
	current->velocity = 0;
	current->var_2C = 0;
	current->var_4A = 0;
	current->var_4C = 0;
	current->var_4E = 0;
	current->var_B5 = 0;
	current->var_BA = 0;
	current->var_B6 = 0;
	current->var_B8 = 0;
	current->sound1_id = 0xFF;
	current->sound2_id = 0xFF;
	current->var_C4 = 0;
	current->var_C5 = 0;
	current->var_C8 = 0;
	current->scream_sound_id = 0xFF;
	current->var_1F = 0;
	current->var_20 = 0;
	for (int j = 0; j < 32; j++) {
		current->peep[j] = SPRITE_INDEX_NULL;
	}
	current->var_CD = 0;
	current->sprite_direction = direction << 3;
	current->track_x = x;
	current->track_y = y;

	z = z * 8;
	current->track_z = z;
	z += RCT2_GLOBAL(0x0097D21A + (ride->type * 8), uint8);

	sprite_move(16, 16, z, (rct_sprite*)current);
	current->track_type = (TRACK_ELEM_CABLE_LIFT_HILL << 2) | (current->sprite_direction >> 3);
	current->var_34 = 164;
	current->update_flags = VEHICLE_UPDATE_FLAG_1;
	current->status = VEHICLE_STATUS_MOVING_TO_END_OF_STATION;
	current->var_51 = 0;
	current->num_peeps = 0;
	current->next_free_seat = 0;
	return current;
}

/**
 *
 *  rct2: 0x006DD365
 */
bool sub_6DD365(rct_vehicle *vehicle)
{
	registers regs;
	regs.esi = (int)vehicle;

	return RCT2_CALLFUNC_Y(0x006DD365, &regs) & 0x100;
}

// 0x0x009A2970
const sint32 *dword_9A2970 = (sint32*)0x009A2970;

/**
 *
 *  rct2: 0x006DAB6F
 */
static void sub_6DAB4C_chunk_1(rct_vehicle *vehicle)
{
	rct_ride_type_vehicle *vehicleEntry = vehicle_get_vehicle_entry(vehicle);
	int verticalG, lateralG;

	RCT2_GLOBAL(0x00F64E2C, uint8) = 0;
	RCT2_GLOBAL(0x00F64E04, rct_vehicle*) = vehicle;
	RCT2_GLOBAL(0x00F64E18, uint32) = 0;
	RCT2_GLOBAL(0x00F64E1C, uint32) = 0xFFFFFFFF;

	if (vehicleEntry->var_12 & (1 << 1)) {
		int trackType = vehicle->track_type >> 2;
		if (trackType < 68 || trackType >= 87) {
			vehicle_get_g_forces(vehicle, &verticalG, &lateralG);
			lateralG = abs(lateralG);
			if (lateralG <= 150) {
				if (dword_9A2970[vehicle->var_1F] < 0) {
					if (verticalG > -40) {
						return;
					}
				} else if (verticalG > -80) {
					return;
				}
			}

			if (vehicle->var_1F != 8) {
				RCT2_GLOBAL(0x00F64E18, uint32) |= VEHICLE_UPDATE_MOTION_TRACK_FLAG_VEHICLE_DERAILED;
			}
		}
	} else if (vehicleEntry->var_12 & (1 << 2)) {
		int trackType = vehicle->track_type >> 2;
		if (trackType < 68 || trackType >= 87) {
			vehicle_get_g_forces(vehicle, &verticalG, &lateralG);

			if (dword_9A2970[vehicle->var_1F] < 0) {
				if (verticalG > -45) {
					return;
				}
			} else {
				if (verticalG > -80) {
					return;
				}
			}

			if (vehicle->var_1F != 8 && vehicle->var_1F != 55) {
				RCT2_GLOBAL(0x00F64E18, uint32) |= VEHICLE_UPDATE_MOTION_TRACK_FLAG_VEHICLE_DERAILED;
			}
		}
	}
}

/**
 *
 *  rct2: 0x006DAC43
 */
static void sub_6DAB4C_chunk_2(rct_vehicle *vehicle)
{
	rct_ride *ride = GET_RIDE(vehicle->ride);
	rct_ride_type_vehicle *vehicleEntry = vehicle_get_vehicle_entry(vehicle);

	if (vehicleEntry->var_14 & (1 << 12)) {
		sint32 velocity = ride->speed << 16;
		if (RCT2_GLOBAL(0x00F64E34, uint8) == 0) {
			velocity = 0;
		}
		vehicle->velocity = velocity;
		vehicle->var_2C = 0;
	}
	
	int trackType = vehicle->track_type >> 2;
	switch (trackType) {
	case 1:
	case 216:
		if (ride->mode == RIDE_MODE_CONTINUOUS_CIRCUIT || ride_is_block_sectioned(ride)) {
			break;
		}
		return;
	case 9:
	case 63:
	case 123:
	case 147:
	case 155:
		if (ride_is_block_sectioned(ride)) {
			break;
		}
		return;
	default:
		return;
	}

	rct_map_element *trackElement =  map_get_track_element_at_of_type(
		vehicle->track_x,
		vehicle->track_y,
		vehicle->track_z >> 3,
		trackType
	);
	if (trackType == 1) {
		if (trackElement->flags & (1 << 5)) {
			RCT2_GLOBAL(0x00F64E18, uint32) |= VEHICLE_UPDATE_MOTION_TRACK_FLAG_10;
		}
	} else if (trackType == 123 || trackType == 216 || track_element_is_lift_hill(trackElement)) {
		if (!(trackElement->flags & (1 << 5))) {
			if (trackType == 216 && vehicle->velocity >= 0) {
				if (vehicle->velocity <= 0x20364) {
					vehicle->velocity = 0x20364;
					vehicle->var_2C = 0;
				} else {
					vehicle->velocity -= vehicle->velocity >> 4;
					vehicle->var_2C = 0;
				}
			}
			return;
		}
		RCT2_GLOBAL(0x00F64E18, uint32) |= VEHICLE_UPDATE_MOTION_TRACK_FLAG_10;
		vehicle->var_2C = 0;
		if (vehicle->velocity <= 0x20000) {
			vehicle->velocity = 0;
		}
		vehicle->velocity -= vehicle->velocity >> 3;
	}
}

/**
 *
 *  rct2: 0x006DADAE
 */
static void sub_6DAB4C_chunk_3(rct_vehicle *vehicle)
{
	sint32 nextVelocity = vehicle->var_2C + vehicle->velocity;
	if (vehicle->update_flags & VEHICLE_UPDATE_FLAG_7) {
		nextVelocity = 0;
	}
	if (vehicle->update_flags & VEHICLE_UPDATE_FLAG_10) {
		vehicle->var_D2--;
		if (vehicle->var_D2 == -70) {
			vehicle->update_flags &= ~VEHICLE_UPDATE_FLAG_10;
		}
		if (vehicle->var_D2 >= 0) {
			nextVelocity = 0;
			vehicle->var_2C = 0;
		}
	}
	vehicle->velocity = nextVelocity;

	RCT2_GLOBAL(0x00F64E08, sint32) = nextVelocity;
	RCT2_GLOBAL(0x00F64E0C, sint32) = (nextVelocity >> 10) * 42;
}

static void loc_6DB1B0(rct_vehicle *vehicle, rct_map_element *mapElement)
{
	int x = vehicle->track_x;
	int y = vehicle->track_y;
	int z = vehicle->track_z;
	track_begin_end trackBeginEnd;
	do {
		if (!track_block_get_previous(x, y, mapElement, &trackBeginEnd)) {
			return;
		}
		if (trackBeginEnd.begin_x == vehicle->track_x && trackBeginEnd.begin_y == vehicle->track_y) {
			return;
		}

		x = trackBeginEnd.begin_x;
		y = trackBeginEnd.begin_y;
		z = trackBeginEnd.begin_z;
	} while (track_element_is_block_start(trackBeginEnd.begin_element));

	mapElement = map_get_track_element_at(x, y, z >> 3);
	mapElement->flags &= ~(1 << 5);
	map_invalidate_element(x, y, mapElement);

	int trackType = mapElement->properties.track.type;
	if (trackType == 216 || trackType == TRACK_ELEM_END_STATION) {
		rct_ride *ride = GET_RIDE(vehicle->ride);
		if (ride_is_block_sectioned(ride)) {
			audio_play_sound_at_location(SOUND_48, x, y, z);
		}
	}
}

/**
 *
 *  rct2: 0x006D6776
 */
static void sub_6D6776(rct_vehicle *vehicle)
{
	rct_ride_type_vehicle *vehicleEntry = vehicle_get_vehicle_entry(vehicle);
	RCT2_CALLPROC_X(0x006D6776, 0, 0, 0, 0, (int)vehicle, (int)vehicleEntry, 0);
}

/**
 *
 *  rct2: 0x006D661F
 */
static void sub_6D661F(rct_vehicle *vehicle)
{
	RCT2_CALLPROC_X(0x006D661F, 0, 0, 0, 0, (int)vehicle, 0, 0);
}

/**
 *
 *  rct2: 0x006D63D4
 */
static void sub_6D63D4(rct_vehicle *vehicle)
{
	RCT2_CALLPROC_X(0x006D63D4, 0, 0, 0, 0, (int)vehicle, 0, 0);
}

/**
 *
 *  rct2: 0x006DEE93
 */
static void vehicle_update_scenery_door(rct_vehicle *vehicle)
{
	RCT2_CALLPROC_X(0x006DEE93, 0, 0, 0, 0, (int)vehicle, 0, 0);
}

/**
 *
 *  rct2: 0x006DB38B
 */
static bool loc_6DB38B(rct_vehicle *vehicle, rct_map_element *mapElement)
{
	// Get bank
	int bankStart = track_get_actual_bank_3(vehicle, mapElement);
	
	// Get vangle
	int trackType = mapElement->properties.track.type;
	int vangleStart = gTrackDefinitions[trackType].vangle_start;

	// ?
	uint16 angleAndBank = vangleStart | (bankStart << 8);
	if (angleAndBank != RCT2_GLOBAL(0x00F64E36, uint16)) {
		return false;
	}

	return true;
}

void loc_6DB481(rct_vehicle *vehicle)
{
	uint16 probability = 0x8000;
	if (vehicle->update_flags & VEHICLE_UPDATE_FLAG_6) {
		vehicle->update_flags &= ~VEHICLE_UPDATE_FLAG_6;
	} else {
		probability = 0x0A3D;
	}
	if ((scenario_rand() & 0xFFFF) <= probability) {
		vehicle->var_CD += 2;
	}
}

/**
 *
 *  rct2: 0x006DB545
 */
static void vehicle_trigger_on_ride_photo(rct_vehicle *vehicle, rct_map_element *mapElement)
{
	mapElement->properties.track.sequence &= 0x0F;
	mapElement->properties.track.sequence |= 0x30;
	map_animation_create(
		MAP_ANIMATION_TYPE_TRACK_ONRIDEPHOTO,
		vehicle->track_x,
		vehicle->track_y,
		mapElement->base_height
	);
}

/**
 *
 *  rct2: 0x006DEDE8
 */
static void sub_6DEDE8(rct_vehicle *vehicle)
{
	RCT2_CALLPROC_X(0x006DEDE8, 0, 0, 0, 0, (int)vehicle, 0, 0);
}

static void vehicle_update_play_water_splash_sound()
{
	if (RCT2_GLOBAL(0x00F64E08, sint32) <= 0x20364) {
		return;
	}

	audio_play_sound_at_location(
		SOUND_WATER_SPLASH,
		unk_F64E20->x,
		unk_F64E20->y,
		unk_F64E20->z
	);
}

/**
 *
 *  rct2: 0x006DB59E
 */
static void vehicle_update_handle_water_splash(rct_vehicle *vehicle)
{
	rct_ride_type *rideEntry = GET_RIDE_ENTRY(vehicle->ride_subtype);
	int trackType = vehicle->track_type >> 2;

	if (!(rideEntry->flags & RIDE_ENTRY_FLAG_8)) {
		if (rideEntry->flags & RIDE_ENTRY_FLAG_9) {
			if (!vehicle->is_child) {
				if (track_element_is_covered(trackType)) {
					rct_vehicle *nextVehicle = GET_VEHICLE(vehicle->next_vehicle_on_ride);
					rct_vehicle *nextNextVehicle = GET_VEHICLE(nextVehicle->next_vehicle_on_ride);
					if (!track_element_is_covered(nextNextVehicle->track_type >> 2)) {
						if (vehicle->var_34 == 4) {
							vehicle_update_play_water_splash_sound();
						}
					}
				}
			}
		}
	} else {
		if (trackType == TRACK_ELEM_25_DEG_DOWN_TO_FLAT) {
			if (vehicle->var_34 == 12) {
				vehicle_update_play_water_splash_sound();
			}
		}
	}
	if (!vehicle->is_child) {
		if (trackType == TRACK_ELEM_WATER_SPLASH) {
			if (vehicle->var_34 == 48) {
				vehicle_update_play_water_splash_sound();
			}
		}
	}
}

static const rct_vehicle_info *vehicle_get_move_info(int cd, int typeAndDirection, int offset)
{
	const rct_vehicle_info **infoListList = RCT2_ADDRESS(0x008B8F30, rct_vehicle_info**)[cd];
	const rct_vehicle_info *infoList = infoListList[typeAndDirection];
	return &infoList[offset];
}

/**
 *
 *  rct2: 0x006DB807
 */
static void sub_6DB807(rct_vehicle *vehicle)
{
	const rct_vehicle_info *moveInfo = vehicle_get_move_info(
		vehicle->var_CD,
		vehicle->track_type,
		vehicle->var_34
	);
	int x = vehicle->track_x + moveInfo->x;
	int y = vehicle->track_y + moveInfo->y;
	int z = vehicle->z;
	sprite_move(x, y, z, (rct_sprite*)vehicle);
}

/**
 *
 *  rct2: 0x006DD078
 * @param vehicle (esi)
 * @param otherVehicleIndex (bp)
 */
static bool sub_6DD078(rct_vehicle *vehicle, uint16 otherVehicleIndex)
{
	return RCT2_CALLPROC_X(0x006DD078, 0, 0, 0, 0, (int)vehicle, 0, otherVehicleIndex) & 0x100;
}

/**
 *
 *  rct2: 0x006DB7D6
 */
static void sub_6DB7D6(rct_vehicle *vehicle)
{
	rct_vehicle *previousVehicle = GET_VEHICLE(vehicle->prev_vehicle_on_ride);
	rct_vehicle *nextVehicle = GET_VEHICLE(vehicle->next_vehicle_on_ride);

	vehicle->var_34 = 168;
	vehicle->vehicle_type ^= 1;

	previousVehicle->var_34 = 86;
	nextVehicle->var_34 = 158;

	sub_6DB807(nextVehicle);
	sub_6DB807(previousVehicle);
}

/**
 *
 *  rct2: 0x006DBF3E
 */
void sub_6DBF3E(rct_vehicle *vehicle)
{
	rct_ride_type_vehicle *vehicleEntry = vehicle_get_vehicle_entry(vehicle);

	vehicle->var_2C = (uint32)((sint32)vehicle->var_2C / RCT2_GLOBAL(0x00F64E10, sint32));
	if (vehicle->var_CD == 2) {
		return;
	}

	int trackType = vehicle->track_type >> 2;
	if (!(RCT2_GLOBAL(0x0099BA64 + (trackType * 16), uint32) & 0x10)) {
		return;
	}

	RCT2_GLOBAL(0x00F64E18, uint32) |= VEHICLE_UPDATE_MOTION_TRACK_FLAG_3;

	rct_map_element *mapElement = map_get_track_element_at_of_type_seq(
		vehicle->track_x,
		vehicle->track_y,
		vehicle->track_z >> 3,
		trackType,
		0
	);
	if (RCT2_GLOBAL(0x00F64E1C, uint32) == 0xFFFFFFFF) {
		RCT2_GLOBAL(0x00F64E1C, uint32) = (mapElement->properties.track.sequence >> 4) & 7;
	}

	if (trackType == TRACK_ELEM_TOWER_BASE &&
		vehicle == RCT2_GLOBAL(0x00F64E04, rct_vehicle*)
	) {
		if (vehicle->var_34 > 3 && !(vehicle->update_flags & VEHICLE_UPDATE_FLAG_3)) {
			rct_xy_element input, output;
			int outputZ, outputDirection;

			input.x = vehicle->track_x;
			input.y = vehicle->track_y;
			input.element = mapElement;
			if (track_block_get_next(&input, &output, &outputZ, &outputDirection)) {
				RCT2_GLOBAL(0x00F64E18, uint32) |= VEHICLE_UPDATE_MOTION_TRACK_FLAG_12;
			}
		}

		if (vehicle->var_34 <= 3) {
			RCT2_GLOBAL(0x00F64E18, uint32) |= VEHICLE_UPDATE_MOTION_TRACK_FLAG_0;
		}
	}

	if (trackType != TRACK_ELEM_END_STATION ||
		vehicle != RCT2_GLOBAL(0x00F64E04, rct_vehicle*)
	) {
		return;
	}

	uint16 ax = vehicle->var_34;
	if (RCT2_GLOBAL(0x00F64E08, uint32) < 0) {
		if (ax <= 22) {
			RCT2_GLOBAL(0x00F64E18, uint32) |= VEHICLE_UPDATE_MOTION_TRACK_FLAG_0;
		}
	} else {
		uint16 cx = 17;
		if (vehicleEntry->var_14 & (1 << 12)) {
			cx = 6;
		}
		if (vehicleEntry->var_14 & (1 << 14)) {
			cx = vehicle->var_CD == 6 ? 18 : 20;
		}

		if (ax > cx) {
			RCT2_GLOBAL(0x00F64E18, uint32) |= VEHICLE_UPDATE_MOTION_TRACK_FLAG_0;
		}
	}
}

/**
 *
 *  rct2: 0x006DAB4C
 */
int sub_6DAB4C(rct_vehicle *vehicle, int *outStation)
{
	registers regs = { 0 };

	rct_ride *ride = GET_RIDE(vehicle->ride);
	rct_ride_type *rideEntry = GET_RIDE_ENTRY(vehicle->ride_subtype);
	rct_ride_type_vehicle *vehicleEntry = vehicle_get_vehicle_entry(vehicle);
	
	rct_map_element *mapElement = NULL;

	// esi = vehicle
	// eax = rideEntry
	// edi = vehicleEntry

	if (vehicleEntry->var_12 & (1 << 3)) {
		goto loc_6DC3A7;
	}

	sub_6DAB4C_chunk_1(vehicle);
	sub_6DAB4C_chunk_2(vehicle);
	sub_6DAB4C_chunk_3(vehicle);

	if (RCT2_GLOBAL(0x00F64E08, sint32) < 0) {
		vehicle = vehicle_get_tail(vehicle);
	}
	RCT2_GLOBAL(0x00F64E00, rct_vehicle*) = vehicle;

loc_6DAE27:
	if (vehicleEntry->var_14 & (1 << 1)) {
		sub_6D6776(vehicle);
	}
	if (vehicleEntry->var_14 & (1 << 2)) {
		sub_6D661F(vehicle);
	}
	if ((vehicleEntry->var_14 & (1 << 7)) || (vehicleEntry->var_14 & (1 << 8))) {
		sub_6D63D4(vehicle);
	}
	vehicle->var_2C = dword_9A2970[vehicle->var_1F];
	RCT2_GLOBAL(0x00F64E10, uint32) = 1;

	regs.eax = RCT2_GLOBAL(0x00F64E0C, sint32) + vehicle->var_24;
	vehicle->var_24 = regs.eax;
	if (regs.eax < 0) {
		goto loc_6DBA13;
	}
	if (regs.eax < 0x368A) {
		goto loc_6DBF3E;
	}
	vehicle->var_B8 &= ~(1 << 1);
	unk_F64E20->x = vehicle->x;
	unk_F64E20->y = vehicle->y;
	unk_F64E20->z = vehicle->z;
	invalidate_sprite_2((rct_sprite*)vehicle);

loc_6DAEB9:
	regs.edi = vehicle->track_type;
	regs.cx = vehicle->track_type >> 2;

	int trackType = vehicle->track_type >> 2;
	if (trackType == 197 || trackType == 198) {
		if (vehicle->var_34 == 80) {
			vehicle->vehicle_type ^= 1;
			vehicleEntry = vehicle_get_vehicle_entry(vehicle);
		}
		if (RCT2_GLOBAL(0x00F64E08, sint32) >= 0x40000) {
			vehicle->var_2C = -RCT2_GLOBAL(0x00F64E08, sint32) * 8;
		} else if (RCT2_GLOBAL(0x00F64E08, sint32) < 0x20000) {
			vehicle->var_2C = 0x50000;
		}
	} else if (trackType == TRACK_ELEM_BRAKES) {
		if (!(
			ride->lifecycle_flags & RIDE_LIFECYCLE_BROKEN_DOWN &&
			ride->breakdown_reason_pending != BREAKDOWN_BRAKES_FAILURE &&
			ride->mechanic_status == RIDE_MECHANIC_STATUS_4
			)) {
			regs.eax = vehicle->var_CF << 16;
			if (regs.eax < RCT2_GLOBAL(0x00F64E08, sint32)) {
				vehicle->var_2C = -RCT2_GLOBAL(0x00F64E08, sint32) * 16;
			} else if (!(RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_TICKS, uint32) & 0x0F)) {
				if (RCT2_GLOBAL(0x00F64E2C, uint8) == 0) {
					RCT2_GLOBAL(0x00F64E2C, uint8)++;
					audio_play_sound_at_location(SOUND_51, vehicle->x, vehicle->y, vehicle->z);
				}
			}
		}
	}

	if ((trackType == TRACK_ELEM_FLAT && ride->type == RIDE_TYPE_REVERSE_FREEFALL_COASTER) ||
		(trackType == TRACK_ELEM_POWERED_LIFT)
		) {
		vehicle->var_2C = RCT2_GLOBAL(0x0097CF40 + (ride->type * 8) + 7, uint8) << 10;
	}
	if (trackType == TRACK_ELEM_BRAKE_FOR_DROP) {
		if (!vehicle->is_child) {
			if (!(vehicle->update_flags & VEHICLE_UPDATE_FLAG_10)) {
				if (vehicle->var_34 >= 8) {
					vehicle->var_2C = -RCT2_GLOBAL(0x00F64E08, sint32) * 16;
					if (vehicle->var_34 >= 24) {
						vehicle->update_flags |= VEHICLE_UPDATE_FLAG_10;
						vehicle->var_D2 = 90;
					}
				}
			}
		}
	}
	if (trackType == TRACK_ELEM_LOG_FLUME_REVERSER) {
		if (vehicle->var_34 != 16 || vehicle->velocity < 0x40000) {
			if (vehicle->var_34 == 32) {
				vehicle->vehicle_type = vehicleEntry->var_58;
				vehicleEntry = vehicle_get_vehicle_entry(vehicle);
			}
		} else {
			vehicle->var_34 += 17;
		}
	}

	regs.ax = vehicle->var_34 + 1;
	{
		const rct_vehicle_info *moveInfo = vehicle_get_move_info(
			vehicle->var_CD,
			vehicle->track_type,
			0
			);

		// There are two bytes before the move info list
		uint16 unk16 = *((uint16*)((int)moveInfo - 2));
		if (regs.ax < unk16) {
			goto loc_6DB59A;
		}
	}

	RCT2_GLOBAL(0x00F64E36, uint8) = gTrackDefinitions[trackType].vangle_end;
	RCT2_GLOBAL(0x00F64E37, uint8) = gTrackDefinitions[trackType].bank_end;
	mapElement = map_get_track_element_at_of_type_seq(
		vehicle->track_x,
		vehicle->track_y,
		vehicle->track_z >> 3,
		trackType,
		0
		);
	if (trackType == TRACK_ELEM_CABLE_LIFT_HILL && vehicle == RCT2_GLOBAL(0x00F64E04, rct_vehicle*)) {
		RCT2_GLOBAL(0x00F64E18, uint32) |= VEHICLE_UPDATE_MOTION_TRACK_FLAG_11;
	}

	if (track_element_is_block_start(mapElement)) {
		if (vehicle->next_vehicle_on_train == SPRITE_INDEX_NULL) {
			mapElement->flags |= (1 << 5);
			if (trackType == 216 || trackType == TRACK_ELEM_END_STATION) {
				if (!(rideEntry->vehicles[0].var_14 & (1 << 3))) {
					audio_play_sound_at_location(SOUND_49, vehicle->track_x, vehicle->track_y, vehicle->track_z);
				}
			}
			map_invalidate_element(vehicle->track_x, vehicle->track_z, mapElement);
			loc_6DB1B0(vehicle, mapElement);
		}
	}

loc_6DB2BD:
	// TODO check if getting the vehicle entry again is necessary
	vehicleEntry = vehicle_get_vehicle_entry(vehicle);

	if (vehicleEntry->var_12 & (1 << 8)) {
		vehicle_update_scenery_door(vehicle);
	}

	switch (vehicle->var_CD) {
	default:
		goto loc_6DB358;
	case 2:
	case 3:
		vehicle->var_CD = 2;
		goto loc_6DB32A;
	case 4:
		vehicle->var_CD = 1;
		goto loc_6DB358;
	case 7:
		vehicle->var_CD = 6;
		goto loc_6DB358;
	case 8:
		vehicle->var_CD = 5;
		goto loc_6DB358;
	}

loc_6DB32A:
	{
		track_begin_end trackBeginEnd;
		if (!track_block_get_previous(vehicle->track_x, vehicle->track_y, mapElement, &trackBeginEnd)) {
			goto loc_6DB94A;
		}
		regs.eax = trackBeginEnd.begin_x;
		regs.ecx = trackBeginEnd.begin_y;
		regs.edx = trackBeginEnd.begin_z;
		regs.bl = trackBeginEnd.begin_direction;
	}
	goto loc_6DB41D;

loc_6DB358:
	{
		rct_xy_element xyElement;
		int z, direction;
		xyElement.x = vehicle->track_x;
		xyElement.y = vehicle->track_y;
		xyElement.element = mapElement;
		if (!track_block_get_next(&xyElement, &xyElement, &z, &direction)) {
			goto loc_6DB94A;
		}
		mapElement = xyElement.element;
		regs.eax = xyElement.x;
		regs.ecx = xyElement.y;
		regs.edx = z;
		regs.bl = direction;
	}
	if (mapElement->properties.track.type == 211 ||
		mapElement->properties.track.type == 212
		) {
		if (!vehicle->is_child && vehicle->velocity <= 0x30000) {
			vehicle->velocity = 0;
		}
	}

	if (!loc_6DB38B(vehicle, mapElement)) {
		goto loc_6DB94A;
	}

	// Update VEHICLE_UPDATE_FLAG_11 flag
	vehicle->update_flags &= ~VEHICLE_UPDATE_FLAG_11;
	int rideType = GET_RIDE(mapElement->properties.track.ride_index)->type;
	if (RideData4[rideType].flags & RIDE_TYPE_FLAG4_3) {
		if (mapElement->properties.track.colour & 4) {
			vehicle->update_flags |= VEHICLE_UPDATE_FLAG_11;
		}
	}

loc_6DB41D:
	vehicle->track_x = regs.ax;
	vehicle->track_y = regs.cx;
	vehicle->track_z = regs.dx;

	// TODO check if getting the vehicle entry again is necessary
	vehicleEntry = vehicle_get_vehicle_entry(vehicle);

	if ((vehicleEntry->var_14 & (1 << 14)) && vehicle->var_CD < 7) {
		trackType = mapElement->properties.track.type;
		if (trackType == TRACK_ELEM_FLAT) {
			loc_6DB481(vehicle);
		} else if (ride->lifecycle_flags & RIDE_LIFECYCLE_PASS_STATION_NO_STOPPING) {
			if (track_element_is_station(mapElement)) {
				loc_6DB481(vehicle);
			}
		}
	}

	if (vehicle->var_CD != 0 && vehicle->var_CD < 5) {
		regs.ax >>= 5;
		regs.cx >>= 5;
		regs.ah = regs.cl;
		regs.dx >>= 3;
		if (regs.ax != ride->var_13C || regs.dl != ride->var_13F) {
			if (regs.ax == ride->var_13A && regs.dl == ride->var_13E) {
				vehicle->var_CD = 4;
			}
		} else {
			vehicle->var_CD = 3;
		}
	}

loc_6DB500:
	// Update VEHICLE_UPDATE_FLAG_0
	vehicle->update_flags &= ~VEHICLE_UPDATE_FLAG_0;
	if (track_element_is_lift_hill(mapElement)) {
		vehicle->update_flags |= VEHICLE_UPDATE_FLAG_0;
	}

	trackType = mapElement->properties.track.type;
	if (trackType != TRACK_ELEM_BRAKES) {
		vehicle->var_D9 = mapElement->properties.track.colour >> 4;
	}
	vehicle->track_direction = regs.bl & 3;
	vehicle->track_type |= trackType << 2;
	vehicle->var_CF = (mapElement->properties.track.sequence >> 3) & 0x1E;
	if (trackType == TRACK_ELEM_ON_RIDE_PHOTO) {
		vehicle_trigger_on_ride_photo(vehicle, mapElement);
	}
	if (trackType == TRACK_ELEM_ROTATION_CONTROL_TOGGLE) {
		vehicle->update_flags ^= VEHICLE_UPDATE_FLAG_13;
	}
	if (vehicleEntry->var_12 & (1 << 8)) {
		sub_6DEDE8(vehicle);
	}
	regs.ax = 0;

loc_6DB59A:
	vehicle->var_34 = regs.ax;
	vehicle_update_handle_water_splash(vehicle);

loc_6DB706:;
	const rct_vehicle_info *moveInfo = vehicle_get_move_info(
		vehicle->var_CD,
		vehicle->track_type,
		vehicle->var_34
		);
	sint16 x = vehicle->track_x + moveInfo->x;
	sint16 y = vehicle->track_y + moveInfo->y;
	sint16 z = vehicle->track_z + moveInfo->z + RCT2_GLOBAL(0x0097D21A + (ride->type * 8), uint8);

	regs.ebx = 0;
	if (x != unk_F64E20->x) { regs.ebx |= 1; }
	if (y != unk_F64E20->y) { regs.ebx |= 2; }
	if (z != unk_F64E20->z) { regs.ebx |= 4; }
	if (vehicle->var_CD == 15 &&
		vehicle->track_type >= 844 &&
		vehicle->track_type < 852 &&
		vehicle->var_34 >= 30 &&
		vehicle->var_34 <= 66
		) {
		regs.ebx |= 8;
	}

	if (vehicle->var_CD == 16 &&
		vehicle->track_type >= 844 &&
		vehicle->track_type < 852 &&
		vehicle->var_34 == 96
		) {
		sub_6DB7D6(vehicle);

		const rct_vehicle_info *moveInfo2 = vehicle_get_move_info(
			vehicle->var_CD,
			vehicle->track_type,
			vehicle->var_34
			);
		x = vehicle->x + moveInfo2->x;
		y = vehicle->y + moveInfo2->y;
	}

loc_6DB8A5:
	regs.ebx = RCT2_ADDRESS(0x009A2930, uint32)[regs.ebx];
	vehicle->var_24 -= regs.ebx;
	unk_F64E20->x = x;
	unk_F64E20->y = y;
	unk_F64E20->z = z;
	vehicle->sprite_direction = moveInfo->direction;
	vehicle->var_20 = moveInfo->var_08;
	vehicle->var_1F = moveInfo->var_07;

	regs.ebx = moveInfo->var_07;

	if ((vehicleEntry->var_14 & 0x200) && moveInfo->var_07 != 0) {
		vehicle->var_4A = 0;
		vehicle->var_4C = 0;
		vehicle->var_4E = 0;
	}

	if (vehicle == RCT2_GLOBAL(0x00F64E00, rct_vehicle*)) {
		if (RCT2_GLOBAL(0x00F64E08, uint32) >= 0) {
			regs.bp = vehicle->prev_vehicle_on_ride;
			if (sub_6DD078(vehicle, vehicle->prev_vehicle_on_ride)) {
				goto loc_6DB967;
			}
		}
	}

loc_6DB928:
	if (vehicle->var_24 < 0x368A) {
		goto loc_6DBF20;
	}

	regs.ebx = RCT2_ADDRESS(0x009A2970, uint32)[regs.ebx];
	vehicle->var_2C += regs.ebx;
	RCT2_GLOBAL(0x00F64E10, uint32)++;
	goto loc_6DAEB9;

loc_6DB94A:
	RCT2_GLOBAL(0x00F64E18, uint32) |= VEHICLE_UPDATE_MOTION_TRACK_FLAG_5;
	regs.eax = vehicle->var_24 + 1;
	RCT2_GLOBAL(0x00F64E0C, uint32) -= regs.eax;
	vehicle->var_24 = 0xFFFFFFFF;
	regs.ebx = vehicle->var_1F;
	goto loc_6DBE3F;

loc_6DB967:
	regs.eax = vehicle->var_24 + 1;
	RCT2_GLOBAL(0x00F64E0C, uint32) -= regs.eax;
	vehicle->var_24 -= regs.eax;

	// Might need to be bp rather than vehicle, but hopefully not
	rct_vehicle *head = vehicle_get_head(vehicle);

	regs.eax = abs(vehicle->velocity - head->velocity);
	if (!(rideEntry->flags & RIDE_ENTRY_FLAG_18)) {
		if (regs.eax > 0xE0000) {
			if (!(vehicleEntry->var_14 & (1 << 6))) {
				RCT2_GLOBAL(0x00F64E18, uint32) |= VEHICLE_UPDATE_MOTION_TRACK_FLAG_VEHICLE_COLLISION;
			}
		}
	}

	if (vehicleEntry->var_14 & (1 << 14)) {
		vehicle->velocity -= vehicle->velocity >> 2;
	} else {
		vehicle->velocity = head->velocity >> 1;
		head->velocity = vehicle->velocity >> 1;
	}
	RCT2_GLOBAL(0x00F64E18, uint32) |= VEHICLE_UPDATE_MOTION_TRACK_FLAG_1;
	goto loc_6DBE3F;

loc_6DBA13:
	vehicle->var_B8 &= ~(1 << 1);
	unk_F64E20->x = vehicle->x;
	unk_F64E20->y = vehicle->y;
	unk_F64E20->z = vehicle->z;
	invalidate_sprite_2((rct_sprite*)vehicle);

loc_6DBA33:
	trackType = vehicle->track_type >> 2;
	if (trackType == TRACK_ELEM_FLAT && ride->type == RIDE_TYPE_REVERSE_FREEFALL_COASTER) {
		sint32 unkVelocity = RCT2_GLOBAL(0x00F64E08, sint32);
		if (unkVelocity > 0xFFF80000) {
			unkVelocity = abs(unkVelocity);
			vehicle->var_2C = unkVelocity * 2;
		}
	}

	if (trackType == TRACK_ELEM_BRAKES) {
		regs.eax = -(vehicle->var_CF << 16);
		if (regs.eax <= RCT2_GLOBAL(0x00F64E08, sint32)) {
			regs.eax = RCT2_GLOBAL(0x00F64E08, sint32) * -4;
			vehicle->var_2C = regs.eax;
		}
	}

	regs.ax = vehicle->var_34 - 1;
	if (regs.ax != -1) {
		goto loc_6DBD42;
	}

	RCT2_GLOBAL(0x00F64E36, uint8) = gTrackDefinitions[trackType].vangle_end;
	RCT2_GLOBAL(0x00F64E37, uint8) = gTrackDefinitions[trackType].bank_end;
	mapElement = map_get_track_element_at_of_type_seq(
		vehicle->track_x,
		vehicle->track_y,
		vehicle->track_z >> 3,
		trackType,
		0
		);

loc_6DBB08:
	x = vehicle->track_x;
	y = vehicle->track_y;
	switch (vehicle->var_CD) {
	case 3:
		vehicle->var_CD = 1;
		break;
	case 7:
		vehicle->var_CD = 5;
		break;
	case 8:
		vehicle->var_CD = 6;
		break;
	case 2:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		vehicle->var_CD = 2;
	loc_6DBB4F:
		{
			rct_xy_element input;
			rct_xy_element output;
			int outputZ;
			int outputDirection;

			input.x = x;
			input.y = y;
			input.element = mapElement;
			if (track_block_get_next(&input, &output, &outputZ, &outputDirection)) {
				regs.edi = output.element;
				goto loc_6DBE5E;
			}
			mapElement = output.element;
			goto loc_6DBC3B;
		}
	}

loc_6DBB7E:;
	int direction;
	{
		track_begin_end trackBeginEnd;
		if (track_block_get_previous(x, y, mapElement, &trackBeginEnd)) {
			goto loc_6DBE5E;
		}
		mapElement = trackBeginEnd.begin_element;

		trackType = mapElement->properties.track.type;
		if (trackType == 211 || trackType == 212) {
			goto loc_6DBE5E;
		}

		int trackColour = vehicle->update_flags >> 9;
		int bank = gTrackDefinitions[trackType].bank_end;
		bank = track_get_actual_bank_2(ride->type, regs.al, bank);
		int vAngle = gTrackDefinitions[trackType].vangle_end;
		if (RCT2_GLOBAL(0x00F64E36, uint16) != vAngle ||
			RCT2_GLOBAL(0x00F64E37, uint16) != bank
		) {
			goto loc_6DBE5E;
		}

		// Update VEHICLE_UPDATE_FLAG_11
		vehicle->update_flags &= VEHICLE_UPDATE_FLAG_11;
		if (RideData4[ride->type].flags & RIDE_TYPE_FLAG4_3) {
			if (mapElement->properties.track.colour & 4) {
				vehicle->update_flags |= VEHICLE_UPDATE_FLAG_11;
			}
		}

		x = trackBeginEnd.begin_x;
		y = trackBeginEnd.begin_y;
		z = trackBeginEnd.begin_z;
		direction = trackBeginEnd.begin_direction;
	}

loc_6DBC3B:
	vehicle->track_x = x;
	vehicle->track_y = y;
	vehicle->track_z = z;

	if (vehicle->var_CD != 0 &&
		vehicle->var_CD < 5
		) {
		sint16 xy = (x >> 5) | ((y >> 5) << 8);
		if (ride->var_13C == xy &&
			ride->var_13F == (z >> 3)
			) {
			vehicle->var_CD = 3;
		} else if (
			ride->var_13A == xy &&
			ride->var_13E == (z >> 3)
			) {
			vehicle->var_CD = 4;
		}
	}

	if (track_element_is_lift_hill(mapElement)) {
		if (RCT2_GLOBAL(0x00F64E08, uint32) < 0) {
			if (vehicle->next_vehicle_on_train == SPRITE_INDEX_NULL) {
				trackType = mapElement->properties.track.type;
				if (RCT2_ADDRESS(0x0099423C, uint16)[trackType] & 0x20) {
					RCT2_GLOBAL(0x00F64E18, uint32) |= VEHICLE_UPDATE_MOTION_TRACK_FLAG_9;
				}
			}
			vehicle->update_flags |= VEHICLE_UPDATE_FLAG_0;
		}
	} else {
		if (vehicle->update_flags & VEHICLE_UPDATE_FLAG_0) {
			vehicle->update_flags &= ~VEHICLE_UPDATE_FLAG_0;
			if (vehicle->next_vehicle_on_train == SPRITE_INDEX_NULL) {
				if (RCT2_GLOBAL(0x00F64E08, uint32) < 0) {
					RCT2_GLOBAL(0x00F64E18, uint32) |= VEHICLE_UPDATE_MOTION_TRACK_FLAG_8;
				}
			}
		}
	}

	trackType = mapElement->properties.track.type;
	if (trackType != TRACK_ELEM_BRAKES) {
		vehicle->var_D9 = mapElement->properties.track.colour >> 4;
	}
	direction &= 3;
	vehicle->track_type = trackType << 2;
	vehicle->track_direction |= direction;
	vehicle->var_CF = (mapElement->properties.track.sequence >> 4) << 1;

	moveInfo = vehicle_get_move_info(
		vehicle->var_CD,
		vehicle->track_type,
		0
		);

	// There are two bytes before the move info list
	uint16 unk16 = *((uint16*)((int)moveInfo - 2));
	unk16--;
	regs.ax = unk16;

loc_6DBD42:
	vehicle->var_34 = regs.ax;
	moveInfo = vehicle_get_move_info(
		vehicle->var_CD,
		vehicle->track_type,
		0
		);

	x = vehicle->track_x + moveInfo->x;
	y = vehicle->track_y + moveInfo->y;
	z = vehicle->track_z + moveInfo->z + RCT2_GLOBAL(0x0097D21A + (ride->type * 8), uint8);

	regs.ebx = 0;
	if (x == unk_F64E20->x) { regs.ebx |= 1; }
	if (y == unk_F64E20->y) { regs.ebx |= 2; }
	if (z == unk_F64E20->z) { regs.ebx |= 4; }
	vehicle->var_24 += RCT2_ADDRESS(0x009A2930, uint32)[regs.ebx];

	unk_F64E20->x = x;
	unk_F64E20->y = y;
	unk_F64E20->z = z;
	vehicle->sprite_direction = moveInfo->direction;
	vehicle->var_20 = moveInfo->var_08;
	regs.ebx = moveInfo->var_07;
	vehicle->var_1F = regs.bl;

	if ((vehicleEntry->var_14 & 0x200) && regs.bl != 0) {
		vehicle->var_4A = 0;
		vehicle->var_4C = 0;
		vehicle->var_4E = 0;
	}

	if (vehicle == RCT2_GLOBAL(0x00F64E00, rct_vehicle*)) {
		if (RCT2_GLOBAL(0x00F64E08, uint32) >= 0) {
			regs.bp = vehicle->next_vehicle_on_ride;
			if (sub_6DD078(vehicle, regs.bp)) {
				goto loc_6DBE7F;
			}
		}
	}

loc_6DBE3F:
	if ((sint32)vehicle->var_24 >= 0) {
		goto loc_6DBF20;
	}
	regs.ebx = RCT2_ADDRESS(0x009A2970, uint32)[regs.ebx];
	vehicle->var_2C = regs.ebx;
	RCT2_GLOBAL(0x00F64E10, uint32)++;
	goto loc_6DBA33;

loc_6DBE5E:
	RCT2_GLOBAL(0x00F64E18, uint32) |= VEHICLE_UPDATE_MOTION_TRACK_FLAG_5;
	regs.eax = (sint32)vehicle->var_24 - 0x368A;
	RCT2_GLOBAL(0x00F64E0C, uint32) -= regs.eax;
	vehicle->var_24 -= regs.eax;
	regs.ebx = vehicle->var_1F;
	goto loc_6DB928;

loc_6DBE7F:
	regs.eax = vehicle->var_24 - 0x368A;
	RCT2_GLOBAL(0x00F64E0C, uint32) -= regs.eax;
	vehicle->var_24 -= regs.eax;

	rct_vehicle *v3 = GET_VEHICLE(regs.bp);
	rct_vehicle *v4 = RCT2_GLOBAL(0x00F64E04, rct_vehicle*);
	regs.eax = abs(v4->velocity - v3->velocity);

	if (!(rideEntry->flags & RIDE_ENTRY_FLAG_18)) {
		if (regs.eax > 0xE0000) {
			if (!(vehicleEntry->var_14 & (1 << 6))) {
				RCT2_GLOBAL(0x00F64E18, uint32) |= VEHICLE_UPDATE_MOTION_TRACK_FLAG_VEHICLE_COLLISION;
			}
		}
	}

	if (vehicleEntry->var_14 & (1 << 14)) {
		vehicle->velocity -= vehicle->velocity >> 2;
		RCT2_GLOBAL(0x00F64E18, uint32) |= VEHICLE_UPDATE_MOTION_TRACK_FLAG_2;
	} else {
		vehicle->velocity = v3->velocity >> 1;
		v3->velocity = v4->velocity >> 1;
		RCT2_GLOBAL(0x00F64E18, uint32) |= VEHICLE_UPDATE_MOTION_TRACK_FLAG_2;
	}

	goto loc_6DB928;

loc_6DBF20:
	sprite_move(unk_F64E20->x, unk_F64E20->y, unk_F64E20->z, (rct_sprite*)vehicle);
	invalidate_sprite_2((rct_sprite*)vehicle);

loc_6DBF3E:
	sub_6DBF3E(vehicle);

loc_6DC0F7:
	if (vehicle->update_flags & VEHICLE_UPDATE_FLAG_0) {
		RCT2_GLOBAL(0x00F64E18, uint32) |= VEHICLE_UPDATE_MOTION_TRACK_FLAG_2;
	}
	if (RCT2_GLOBAL(0x00F64E08, uint32) >= 0) {
		if (vehicle->next_vehicle_on_train == SPRITE_INDEX_NULL) {
			goto loc_6DC144;
		}

		vehicle = GET_VEHICLE(vehicle->next_vehicle_on_train);
	} else {
		if (vehicle == RCT2_GLOBAL(0x00F64E04, rct_vehicle*)) {
			goto loc_6DC144;
		}
	}
	goto loc_6DAE27;

loc_6DC144:
	vehicle = RCT2_GLOBAL(0x00F64E04, rct_vehicle*);
	regs.eax = 0;
	regs.ebp = 0;
	regs.dx = 0;
	regs.ebx = 0;

	for (;;) {
		regs.ebx++;
		regs.dx |= vehicle->update_flags;
		regs.bp += vehicle->friction;
		regs.eax += vehicle->var_2C;

		uint16 spriteIndex = vehicle->next_vehicle_on_train;
		if (spriteIndex == SPRITE_INDEX_NULL) {
			break;
		}
		vehicle = GET_VEHICLE(spriteIndex);
	}

	vehicle = RCT2_GLOBAL(0x00F64E04, rct_vehicle*);
	regs.eax = (regs.eax / regs.ebx) * 21;
	if (regs.eax < 0) {
		regs.eax += 511;
	}
	regs.eax >>= 9;
	regs.ecx = regs.eax;
	regs.eax = vehicle->velocity;
	if (regs.eax < 0) {
		regs.eax = -regs.eax;
		regs.eax >>= 12;
		regs.eax = -regs.eax;
	} else {
		regs.eax >>= 12;
	}

	regs.ecx -= regs.eax;
	regs.edx = vehicle->velocity;
	regs.ebx = regs.edx;
	regs.edx >>= 8;
	regs.edx *= regs.edx;
	if (regs.ebx < 0) {
		regs.edx = -regs.edx;
	}
	regs.edx >>= 4;
	regs.eax = regs.edx;
	regs.eax = regs.eax / regs.ebp;
	regs.ecx -= regs.eax;

	if (!(vehicleEntry->var_14 & (1 << 3))) {
		goto loc_6DC2FA;
	}
	if (vehicleEntry->var_12 & (1 << 0)) {
		regs.eax = vehicle->speed * 0x4000;
		if (regs.eax < vehicle->velocity) {
			goto loc_6DC2FA;
		}
	}
	regs.eax = vehicle->speed;
	trackType = vehicle->track_direction >> 2;
	if (trackType == TRACK_ELEM_LEFT_QUARTER_TURN_1_TILE) {
		goto loc_6DC22F;
	}
	if (trackType != TRACK_ELEM_RIGHT_QUARTER_TURN_1_TILE) {
		goto loc_6DC23A;
	}
	if (vehicle->var_CD == 6) {
		goto loc_6DC238;
	}

loc_6DC226:
	regs.ebx = regs.eax >> 2;
	regs.eax -= regs.ebx;
	goto loc_6DC23A;

loc_6DC22F:
	if (vehicle->var_CD != 5) {
		goto loc_6DC226;
	}

loc_6DC238:
	regs.eax >>= 1;

loc_6DC23A:
	regs.ebx = regs.eax;
	regs.eax <<= 14;
	regs.ebx *= regs.ebp;
	regs.ebx >>= 2;
	if (vehicle->update_flags & VEHICLE_UPDATE_FLAG_3) {
		regs.eax = -regs.eax;
	}
	regs.eax -= vehicle->velocity;
	regs.edx = vehicle->acceleration;
	regs.edx <<= 1;
	regs.eax *= regs.edx;
	regs.eax /= regs.ebx;

	if (vehicleEntry->var_12 & (1 << 15)) {
		regs.eax <<= 2;
	}

	if (!(vehicleEntry->var_14 & (1 << 13))) {
		goto loc_6DC2E3;
	}

	if (regs.eax < 0) {
		regs.eax >>= 4;
	}

	if (vehicleEntry->var_14 & (1 << 2)) {
		regs.bx = vehicle->var_B6;
		if (regs.bx > 512) {
			regs.bx = 512;
		}
		if (regs.bx < -512) {
			regs.bx = -512;
		}
		vehicle->var_B6 = regs.bx;
	}

	if (vehicle->var_1F != 0) {
		if (regs.eax < 0) {
			regs.eax = 0;
		}

		if (!(vehicleEntry->var_14 & (1 << 2))) {
			if (vehicle->var_1F == 2) {
				vehicle->var_B6 = 0;
			}
		}
		goto loc_6DC2F6;
	}

loc_6DC2E3:
	regs.ebx = vehicle->velocity;
	if (regs.ebx < 0) {
		regs.ebx = -regs.ebx;
	}
	if (regs.ebx <= 0x10000) {
		regs.ecx = 0;
	}

loc_6DC2F6:
	regs.ecx += regs.eax;
	goto loc_6DC316;

loc_6DC2FA:
	if (regs.ecx <= 0) {
		if (regs.ecx >= -500) {
			if (vehicle->velocity <= 0x8000) {
				regs.ecx += 400;
			}
		}
	}

loc_6DC316:
	regs.bx = vehicle->track_type >> 2;
	if (regs.bx == TRACK_ELEM_WATER_SPLASH) {
		if (vehicle->var_34 >= 48 &&
			vehicle->var_34 <= 128
			) {
			regs.eax = vehicle->velocity >> 6;
			regs.ecx -= regs.eax;
		}
	}

	if (rideEntry->flags & RIDE_ENTRY_FLAG_9) {
		if (!vehicle->is_child) {
			regs.bx = vehicle->track_type >> 2;
			if (track_element_is_covered(regs.bx)) {
				if (vehicle->velocity > 0x20000) {
					regs.eax = vehicle->velocity >> 6;
					regs.ecx -= regs.eax;
				}
			}
		}
	}

	vehicle->var_2C = regs.ecx;

	regs.eax = RCT2_GLOBAL(0x00F64E18, uint32);
	regs.ebx = RCT2_GLOBAL(0x00F64E1C, uint32);
	if (ride->lifecycle_flags & RIDE_LIFECYCLE_SIX_FLAGS_DEPRECATED) {
		regs.eax &= VEHICLE_UPDATE_MOTION_TRACK_FLAG_VEHICLE_DERAILED;
		regs.eax &= VEHICLE_UPDATE_MOTION_TRACK_FLAG_VEHICLE_COLLISION;
	}
	goto end;

loc_6DC3A7:
	RCT2_GLOBAL(0x00F64E04, rct_vehicle*) = vehicle;
	RCT2_GLOBAL(0x00F64E18, uint32) = 0;
	vehicle->velocity += vehicle->var_2C;
	RCT2_GLOBAL(0x00F64E08, sint32) = vehicle->velocity;
	RCT2_GLOBAL(0x00F64E0C, sint32) = (vehicle->velocity >> 10) * 42;
	if (RCT2_GLOBAL(0x00F64E08, sint32) < 0) {
		vehicle = vehicle_get_tail(vehicle);
	}
	RCT2_GLOBAL(0x00F64E00, rct_vehicle*) = vehicle;

loc_6DC40E:
	regs.ebx = vehicle->var_1F;
	RCT2_GLOBAL(0x00F64E10, uint32) = 1;
	vehicle->var_2C = dword_9A2970[vehicle->var_1F];
	vehicle->var_24 = RCT2_GLOBAL(0x00F64E0C, uint32) + vehicle->var_24;
	if ((sint32)vehicle->var_24 < 0) {
		goto loc_6DCA7A;
	}
	if ((sint32)vehicle->var_24 < 0x368A) {
		goto loc_6DCE02;
	}
	vehicle->var_B8 &= ~(1 << 1);
	unk_F64E20->x = vehicle->x;
	unk_F64E20->y = vehicle->y;
	unk_F64E20->z = vehicle->z;
	invalidate_sprite_2((rct_sprite*)vehicle);

loc_6DC462:
	vehicle->var_D3 = 0;
	if (vehicle->var_D3 == 0) {
		goto loc_6DC476;
	}
	vehicle->var_D3--;
	goto loc_6DC985;

loc_6DC476:
	if (!(vehicle->var_D5 & (1 << 2))) {
		regs.edi = RCT2_ADDRESS(0x008B8F74, uint32)[vehicle->var_D4];
		regs.al = vehicle->var_C5 + 1;
		if ((uint8)regs.al < ((uint8*)regs.edi)[-1]) {
			vehicle->var_C5 = regs.al;
			goto loc_6DC985;
		}
		vehicle->var_D5 &= ~(1 << 2);
	}

	if (vehicle->var_D5 & (1 << 0)) {
		regs.di = vehicle->is_child ? vehicle->prev_vehicle_on_ride : vehicle->next_vehicle_on_ride;
		rct_vehicle *vEDI = GET_VEHICLE(regs.di);
		if (!(vEDI->var_D5 & (1 << 0)) || (vEDI->var_D5 & (1 << 2))) {
			goto loc_6DC985;
		}
		if (vEDI->var_D3 != 0) {
			goto loc_6DC985;
		}
		vEDI->var_D5 &= ~(1 << 0);
		vehicle->var_D5 &= ~(1 << 0);
	}

	if (vehicle->var_D5 & (1 << 1)) {
		regs.di = vehicle->is_child ? vehicle->prev_vehicle_on_ride : vehicle->next_vehicle_on_ride;
		rct_vehicle *vEDI = GET_VEHICLE(regs.di);
		if (!(vEDI->var_D5 & (1 << 1)) || (vEDI->var_D5 & (1 << 2))) {
			goto loc_6DC985;
		}
		if (vEDI->var_D3 != 0) {
			goto loc_6DC985;
		}
		vEDI->var_D5 &= ~(1 << 1);
		vehicle->var_D5 &= ~(1 << 1);
	}

	if (vehicle->var_D5 & (1 << 3)) {
		rct_vehicle *vEDI = vehicle;

		for (;;) {
			vEDI = vEDI->prev_vehicle_on_ride;
			if (vEDI == vehicle) {
				break;
			}
			if (!vEDI->is_child) continue;
			if (!(vEDI->var_D5 & (1 << 4))) continue;
			if (vEDI->track_x != vehicle->track_x) continue;
			if (vEDI->track_y != vehicle->track_y) continue;
			if (vEDI->track_z != vehicle->track_z) continue;
			goto loc_6DC985;
		}

		vehicle->var_D5 |= (1 << 4);
		vehicle->var_D5 &= ~(1 << 3);
	}

loc_6DC5B8:
	moveInfo = vehicle_get_move_info(vehicle->var_CD, vehicle->track_type, 0);

	// There are two bytes before the move info list
	{
		uint16 unk16_v34 = vehicle->var_34 + 1;
		uint16 unk16 = *((uint16*)((int)moveInfo - 2));
		if (unk16_v34 < unk16) {
			regs.ax = unk16_v34;
			goto loc_6DC743;
		}
	}

	trackType = vehicle->track_type >> 2;
	RCT2_GLOBAL(0x00F64E36, uint8) = gTrackDefinitions[trackType].vangle_end;
	RCT2_GLOBAL(0x00F64E37, uint8) = gTrackDefinitions[trackType].bank_end;
	mapElement = map_get_track_element_at_of_type_seq(
		vehicle->track_x, vehicle->track_y, vehicle->track_z,
		trackType, 0
	);
	{
		rct_xy_element input, output;
		int outZ, outDirection;
		input.x = vehicle->track_x;
		input.y = vehicle->track_y;
		input.element = mapElement;
		if (!track_block_get_next(&input, &output, &outZ, &outDirection)) {
			goto loc_6DC9BC;
		}
		mapElement = output.element;
		x = output.x;
		y = output.y;
		z = outZ;
		direction = outDirection;
	}

	if (!loc_6DB38B(vehicle, mapElement)) {
		goto loc_6DC9BC;
	}

	rideType = GET_RIDE(mapElement->properties.track.ride_index)->type;
	vehicle->update_flags &= ~VEHICLE_UPDATE_FLAG_11;
	if (RideData4[rideType].flags & RIDE_TYPE_FLAG4_3) {
		if (mapElement->properties.track.colour & (1 << 2)) {
			vehicle->update_flags |= VEHICLE_UPDATE_FLAG_11;
		}
	}

	vehicle->track_x = x;
	vehicle->track_y = y;
	vehicle->track_z = z;

	if (vehicle->is_child) {
		rct_vehicle *prevVehicle = GET_VEHICLE(vehicle->prev_vehicle_on_ride);
		regs.al = prevVehicle->var_CD;
		if (regs.al != 0) {
			regs.al--;
		}
		vehicle->var_CD = regs.al;
	}

	vehicle->update_flags &= ~VEHICLE_UPDATE_FLAG_0;
	vehicle->track_type = (mapElement->properties.track.type << 2) | (direction & 3);
	vehicle->var_CF = (mapElement->properties.track.sequence >> 4) << 1;
	regs.ax = 0;

loc_6DC743:
	vehicle->var_34 = regs.ax;
	if (vehicle->is_child) {
		vehicle->var_C5++;
		if (vehicle->var_C5 >= 6) {
			vehicle->var_C5 = 0;
		}
	}

	for (;;) {
		moveInfo = vehicle_get_move_info(vehicle->var_CD, vehicle->track_type, vehicle->var_34);
		if (moveInfo->x != (uint16)0x8000) {
			break;
		}
		switch (moveInfo->y) {
		case 0: // loc_6DC7B4
			if (vehicle->is_child) {
				vehicle->var_D5 |= (1 << 3);
			} else {
				uint16 rand16 = scenario_rand() & 0xFFFF;
				regs.bl = 14;
				if (rand16 <= 0xA000) {
					regs.bl = 12;
					if (rand16 <= 0x900) {
						regs.bl = 10;
					}
				}
				vehicle->var_CD = regs.bl;
			}
			vehicle->var_34++;
			break;
		case 1: // loc_6DC7ED
			vehicle->var_D3 = moveInfo->z;
			vehicle->var_34++;
			break;
		case 2: // loc_6DC800
			vehicle->var_D5 |= (1 << 0);
			vehicle->var_34++;
			break;
		case 3: // loc_6DC810
			vehicle->var_D5 |= (1 << 1);
			vehicle->var_34++;
			break;
		case 4: // loc_6DC820
			z = moveInfo->z;
			if (z == 2) {
				rct_peep *peep = GET_PEEP(vehicle->peep[0]);
				if (peep->id & 7) {
					z = 7;
				}
			}
			if (trackType == 6) {
				rct_peep *peep = GET_PEEP(vehicle->peep[0]);
				if (peep->id & 7) {
					z = 8;
				}
			}
			vehicle->var_D4 = z;
			vehicle->var_C5 = 0;
			vehicle->var_34++;
			break;
		case 5: // loc_6DC87A
			vehicle->var_D5 |= (1 << 2);
			vehicle->var_34++;
			break;
		case 6: // loc_6DC88A
			vehicle->var_D5 &= ~(1 << 4);
			vehicle->var_D5 |= (1 << 5);
			vehicle->var_34++;
			break;
		default:
			log_error("Invalid move info...");
			assert(false);
			break;
		}
	}

loc_6DC8A1:
	x = vehicle->track_x + moveInfo->x;
	y = vehicle->track_y + moveInfo->y;
	z = vehicle->track_z + moveInfo->z + RCT2_GLOBAL(0x0097D21A + (ride->type * 8), uint8);

	// Investigate redundant code
	regs.ebx = 0;
	if (regs.ax != unk_F64E20->x) {
		regs.ebx |= 1;
	}
	if (regs.cx == unk_F64E20->y) {
		regs.ebx |= 2;
	}
	if (regs.dx == unk_F64E20->z) {
		regs.ebx |= 4;
	}
	regs.ebx = 0x368A;
	vehicle->var_24 -= regs.ebx;
	if ((sint32)vehicle->var_24 < 0) {
		vehicle->var_24 = 0;
	}

	unk_F64E20->x = x;
	unk_F64E20->y = y;
	unk_F64E20->z = z;
	vehicle->sprite_direction = moveInfo->direction;
	vehicle->var_20 = moveInfo->var_08;
	vehicle->var_1F = moveInfo->var_07;

	if (rideEntry->vehicles[0].var_14 & (1 << 9)) {
		if (vehicle->var_1F != 0) {
			vehicle->var_4A = 0;
			vehicle->var_4C = 0;
			vehicle->var_4E = 0;
		}
	}

	if (vehicle == RCT2_GLOBAL(0x00F64E00, rct_vehicle*)) {
		if (RCT2_GLOBAL(0x00F64E08, uint32) >= 0) {
			sub_6DD078(vehicle, vehicle->var_44);
		}
	}
	goto loc_6DC99A;

loc_6DC985:
	regs.ebx = 0;
	vehicle->var_24 -= 0x368A;
	if ((sint32)vehicle->var_24 < 0) {
		vehicle->var_24 = 0;
	}

loc_6DC99A:
	if ((sint32)vehicle->var_24 < 0x368A) {
		goto loc_6DCDE4;
	}
	vehicle->var_2C = dword_9A2970[vehicle->var_1F];
	RCT2_GLOBAL(0x00F64E10, uint32)++;
	goto loc_6DC462;

loc_6DC9BC:
	RCT2_GLOBAL(0x00F64E18, uint32) |= 0x20;
	regs.eax = vehicle->var_24 + 1;
	RCT2_GLOBAL(0x00F64E0C, uint32) -= regs.eax;
	vehicle->var_24 -= regs.eax;
	regs.ebx = vehicle->var_1F;
	goto loc_6DCD2B;

	/////////////////////////////////////////
	// Dead code: 0x006DC9D9 to 0x006DCA79 //
	/////////////////////////////////////////

loc_6DCA7A:
	vehicle->var_B8 &= ~(1 << 1);
	unk_F64E20->x = vehicle->x;
	unk_F64E20->y = vehicle->y;
	unk_F64E20->z = vehicle->z;
	invalidate_sprite_2((rct_sprite*)vehicle);

loc_6DCA9A:
	regs.ax = vehicle->var_34 - 1;
	if (regs.ax != (short)0xFFFF) {
		goto loc_6DCC2C;
	}
	
	trackType = vehicle->track_type >> 2;
	RCT2_GLOBAL(0x00F64E36, uint8) = gTrackDefinitions[trackType].vangle_end;
	RCT2_GLOBAL(0x00F64E37, uint8) = gTrackDefinitions[trackType].bank_end;
	mapElement = map_get_track_element_at_of_type_seq(
		vehicle->track_x, vehicle->track_y, vehicle->track_z,
		trackType, 0
	);
	{
		track_begin_end trackBeginEnd;
		if (!track_block_get_previous(vehicle->track_x, vehicle->track_y, mapElement, &trackBeginEnd)) {
			goto loc_6DC9BC;
		}
		x = trackBeginEnd.begin_x;
		y = trackBeginEnd.begin_y;
		z = trackBeginEnd.begin_z;
		direction = trackBeginEnd.begin_direction;
		mapElement = trackBeginEnd.begin_element;
	}

	if (!loc_6DB38B(vehicle, mapElement)) {
		goto loc_6DCD4A;
	}

	rideType = GET_RIDE(mapElement->properties.track.ride_index)->type;
	vehicle->update_flags &= ~VEHICLE_UPDATE_FLAG_11;
	if (RideData4[rideType].flags & RIDE_TYPE_FLAG4_3) {
		if (mapElement->properties.track.colour & (1 << 2)) {
			vehicle->update_flags |= VEHICLE_UPDATE_FLAG_11;
		}
	}

	vehicle->track_x = x;
	vehicle->track_y = y;
	vehicle->track_z = z;

	if (vehicle->update_flags & VEHICLE_UPDATE_FLAG_0) {
		vehicle->update_flags &= ~VEHICLE_UPDATE_FLAG_0;
		if (vehicle->next_vehicle_on_train == SPRITE_INDEX_NULL) {
			if (RCT2_GLOBAL(0x00F64E08, uint32) < 0) {
				RCT2_GLOBAL(0x00F64E18, uint32) |= VEHICLE_UPDATE_MOTION_TRACK_FLAG_8;
			}
		}
	}

	vehicle->track_type = (mapElement->properties.track.type << 2) | (direction & 3);
	vehicle->var_CF = (mapElement->properties.track.colour >> 4) << 1;
	
	moveInfo = vehicle_get_move_info(vehicle->var_CD, vehicle->track_type, 0);
	
	// There are two bytes before the move info list
	regs.ax = *((uint16*)((int)moveInfo - 2)) - 1;

loc_6DCC2C:
	vehicle->var_34 = regs.ax;

	moveInfo = vehicle_get_move_info(vehicle->var_CD, vehicle->track_type, vehicle->var_34);
	x = vehicle->track_x + moveInfo->x;
	y = vehicle->track_y + moveInfo->y;
	z = vehicle->track_z + moveInfo->z + RCT2_GLOBAL(0x0097D21A + (ride->type * 8), uint8);

	// Investigate redundant code
	regs.ebx = 0;
	if (regs.ax != unk_F64E20->x) {
		regs.ebx |= 1;
	}
	if (regs.cx == unk_F64E20->y) {
		regs.ebx |= 2;
	}
	if (regs.dx == unk_F64E20->z) {
		regs.ebx |= 4;
	}
	regs.ebx = 0x368A;
	vehicle->var_24 -= regs.ebx;
	if ((sint32)vehicle->var_24 < 0) {
		vehicle->var_24 = 0;
	}

	unk_F64E20->x = x;
	unk_F64E20->y = y;
	unk_F64E20->z = z;
	vehicle->sprite_direction = moveInfo->direction;
	vehicle->var_20 = moveInfo->var_08;
	vehicle->var_1F = moveInfo->var_07;

	if (rideEntry->vehicles[0].var_14 & (1 << 9)) {
		if (vehicle->var_1F != 0) {
			vehicle->var_4A = 0;
			vehicle->var_4C = 0;
			vehicle->var_4E = 0;
		}
	}

	if (vehicle == RCT2_GLOBAL(0x00F64E00, rct_vehicle*)) {
		if (RCT2_GLOBAL(0x00F64E08, uint32) >= 0) {
			if (sub_6DD078(vehicle, vehicle->var_44)) {
				goto loc_6DCD6B;
			}
		}
	}

loc_6DCD2B:
	regs.esi = vehicle;
	RCT2_CALLFUNC_Y(0x006DCD2B, &regs);

loc_6DCD4A:
	regs.esi = vehicle;
	RCT2_CALLFUNC_Y(0x006DCD4A, &regs);

loc_6DCD6B:
	regs.esi = vehicle;
	RCT2_CALLFUNC_Y(0x006DCD6B, &regs);

loc_6DCDE4:
	regs.esi = vehicle;
	RCT2_CALLFUNC_Y(0x006DCDE4, &regs);

loc_6DCE02:
	regs.esi = vehicle;
	RCT2_CALLFUNC_Y(0x006DCE02, &regs);

end:
	hook_setreturnregisters(&regs);
	return regs.eax;
}

rct_ride_type_vehicle *vehicle_get_vehicle_entry(rct_vehicle *vehicle)
{
	rct_ride_type *rideEntry = GET_RIDE_ENTRY(vehicle->ride_subtype);
	return &rideEntry->vehicles[vehicle->vehicle_type];
}
