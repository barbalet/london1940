/****************************************************************
 
 city.c
 
 =============================================================
 
 Copyright 1996-2018 Tom Barbalet. All rights reserved.
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
 
 This software and Noble Ape are a continuing work of Tom Barbalet,
 begun on 13 June 1996. No apes or cats were harmed in the writing
 of this software.
 
 ****************************************************************/

#include "city.h"
#include "entity.h"

static noble_being beings[64];
static n_uint beings_number;

void city_init(n_byte2 * seed)
{
    beings_number = being_init_group(beings, seed, 50, 64);
}

void city_listen(noble_being * beings, n_uint beings_number, noble_being * local_being)
{
    being_listen_struct bls;
    n_uint loop = 0;

    if (local_being->delta.awake == 0) return;
    
    bls.max_shout_volume = 127;
    bls.local = local_being;
    /** clear shout values */
    if (local_being->changes.shout[SHOUT_CTR] > 0)
    {
        local_being->changes.shout[SHOUT_CTR]--;
    }
    
    while (loop < beings_number)
    {
        noble_being * other = &beings[loop];
        if (other!= local_being)
        {
            being_listen_loop_no_sim(other, (void *) &bls);
        }
        loop++;
    }
}

void city_sociability(noble_being * beings, n_uint beings_number, noble_being * local_being)
{
    drives_sociability_data dsd;
    n_uint loop = 0;

    dsd.beings_in_vacinity = 0;
    dsd.being = local_being;
    
    while (loop < beings_number)
    {
        noble_being * other = &beings[loop];
        if (other!= local_being)
        {
            drives_sociability_loop_no_sim(other, (void *) &dsd);
        }
        loop++;
    }
    being_crowding_cycle(local_being, dsd.beings_in_vacinity);
}


void city_cycle(void)
{
    n_uint loop = 0;
    n_int  max_honor = 0;

    while (loop < beings_number)
    {
        beings[loop].delta.awake = FULLY_AWAKE;
        loop++;
    }

    loop = 0;
    while (loop < beings_number)
    {
        being_cycle_universal(&beings[loop]);
        loop++;
    }
    
    loop = 0;
    while (loop < beings_number)
    {
        city_listen(beings, beings_number, &beings[loop]);
        loop++;
    }

    loop = 0;
    while (loop < beings_number)
    {
        episodic_cycle_no_sim(&beings[loop]);
        loop++;
    }
    /*
     if (local_being->delta.awake == 0) return;
     
     being_cycle_awake(local_sim, local_being);
     */
    
    loop = 0;
    while (loop < beings_number)
    {
        drives_hunger(&beings[loop]);
        loop++;
    }
    
    loop = 0;
    while (loop < beings_number)
    {
        city_sociability(beings, beings_number, &beings[loop]);
        loop++;
    }

    /*drives_sex(local_being, local_being->delta.awake, local_sim);*/
    
    loop = 0;
    while (loop < beings_number)
    {
        drives_fatigue(&beings[loop]);
        loop++;
    }
    
    
    loop = 0;
    while (loop < beings_number)
    {
        n_byte2 local_brain_state[3];
        noble_being * local = &beings[loop];
        if(being_brainstates(local, (local->delta.awake == 0), local_brain_state))
        {
            n_byte *local_brain = being_brain(local);
            if (local_brain != 0L)
            {
                brain_cycle(local_brain, local_brain_state);
            }
        }
        loop++;
    }
/*
 static void sim_brain_dialogue_loop(noble_simulation * local_sim, noble_being * local_being, void * data)
{
    n_byte     awake = 1;
    n_byte    *local_internal = being_braincode_internal(local_being);
    n_byte    *local_external = being_braincode_external(local_being);
    if(local_being->delta.awake == 0)
    {
        awake=0;
    }
    brain_dialogue(local_sim, awake, local_being, local_being, local_internal, local_external, being_random(local_being)%SOCIAL_SIZE);
    brain_dialogue(local_sim, awake, local_being, local_being, local_external, local_internal, being_random(local_being)%SOCIAL_SIZE);
}
 */
    
    loop = 0;
    while (loop < beings_number)
    {
        being_tidy_loop_no_sim(&beings[loop], &max_honor);
        loop++;
    }

/*
    being_loop(&sim, social_initial_loop, PROCESSING_LIGHT_WEIGHT);
*/
    if (max_honor)
    {
        loop = 0;
        while (loop < beings_number)
        {
            being_recalibrate_honor_loop_no_sim(&beings[loop]);
            loop++;
        }
    }
    /*
    being_loop(&sim, social_secondary_loop, PROCESSING_FEATHER_WEIGHT);

    {
        being_remove_loop2_struct * brls = being_remove_initial(&sim);
        if (sim.ext_death != 0L)
        {
            being_loop_no_thread(&sim, 0L, being_remove_loop1, 0L);
        }
        being_loop_no_thread(&sim, 0L, being_remove_loop2, brls);
        sim_being_remove_final(&sim, &brls);
    }
    */
}

static n_uint count = 0;

void city_draw(void)
{
    n_uint loop = 0;
    
    char value[200] = {0};
    
    gldraw_red();
    
    value[0] = '0'+ (count/1000) % 10;
    value[1] = '0'+ (count/100) % 10;
    value[2] = '0'+ (count/10) % 10;
    value[3] = '0'+ (count/1) % 10;
    value[4] = 0;

    
    count ++;
    
    while (loop < beings_number)
    {
        n_vect2 line_start;
        n_vect2 line_end;
        

        
        line_start.x = line_end.x = beings[loop].delta.location[0];
        line_start.y = line_end.y = beings[loop].delta.location[1];
        
        gldraw_string(value, line_start.x, line_start.y);

        
        line_start.x -= 10;
        line_end.x += 10;
        
        gldraw_line(&line_start, &line_end);
        
        line_start.x += 10;
        line_end.x -= 10;
        
        line_start.y -= 10;
        line_end.y += 10;
        
        gldraw_line(&line_start, &line_end);

        loop++;
    }
}


