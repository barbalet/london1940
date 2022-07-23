/****************************************************************

 occupations.c

 =============================================================

 Copyright 1996-2022 Tom Barbalet. All rights reserved.

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

 This software is a continuing work of Tom Barbalet, begun on
 13 June 1996. No apes or cats were harmed in the writing of
 this software.

 ****************************************************************/

#include "occupations.h"
#include "toolkit.h"

typedef enum
{
    OG_UNKNOWN = 0,
    OG_FEMALE,
    OG_MALE,
} occupations_gender;

typedef enum
{
    OT_UNKNOWN = 0,
    OT_ONE,
    OT_TWO,
    OT_THREE
} occupations_teir;

typedef enum // 107 - 56 = 51 occupations
{
    OR_ABBESS = 0,
    OR_ABBOTT,
    OR_ABLE_SEAMAN,
    OR_ORDINARY_SEAMAN,
    OR_ACADEMIC,
    OR_ACCOUNTANT,
    OR_ACOLYTE,
    OR_ACTUARY,
    OR_ADVOCATE,
    OR_BAILIFF,
    OR_BAKEMAKER,
    OR_BALLER,
    OR_CAB_DRIVER,
    OR_CADDY,
    OR_CONSTABLE,
    OR_OFFICEROFTHEPARISH,
    OR_DOMESTIC_SERVANT,
    OR_DOORMAN,
    OR_DRAPER,
    OR_DRAUGHTSMAN,
    OR_DRAWBOY,
    OR_DAIRYMAN,
    OR_DANTER,
    OR_DAYLABORER,
    OR_DAYMAN,
    OR_DESIGNER,
    OR_DEVILLER,
    OR_DIE_SINKER,
    OR_DIPPER,
    OR_DISH_THROWER,
    OR_DISH_TURNER,
    OR_DOCK_FOYBOATMAN,
    OR_DOCKER,
    OR_DOCKMASTER,
    OR_DOFFER,
    OR_DOG_BREAKER,
    OR_DOG_WHIPPER,
    OR_DOORKEEPER,
    OR_DOTTLER,
    OR_UTENSIL_MAKER,
    OR_DRAGSMAN,
    OR_DRAINER,
    OR_DISTILLER,
    OR_DISTRIBUTOR,
    OR_WEAVER,
    OR_SEAMSTRESS,
    OR_UNDER_GARDNER,
    OR_GARDNER,
    OR_UNDERLAY_STITCHER,
    OR_UNDERPRESSER,
    OR_UNDERSTUDY,
    OR_UNDERTAKER,
    OR_LAST_ONE,
} occupations_raw;

typedef enum
{
    OH_ALCOHOL,
    OH_RELIGION,
    OH_ANIMALS,
    OH_SEAMAN,
    OH_DOCKS,
    OH_ECONOMY,
    OH_EDUCATOR_UNIVERSITY,
    OH_LAW,
    OH_TRANSPORT,
    OH_ERRANDS,
    OH_GARDENS,
    OH_SHOES,
    OH_CLEANING,
    OH_DEATH,
    OH_HOME,
    OH_TEXTILES,
    OH_ARCHITECTURE,
    OH_METALWORK,
    OH_PRINTING,
    OH_POTTERY,
    OH_POLICE,
    OH_DAIRY,
    OH_LABOR,
    OH_THEATER,
    OH_CARRIAGE,
    OH_WOODWORK,
} occupations_heirarchy;

typedef struct{
    occupations_raw       raw;
    occupations_gender    gender;
    occupations_teir      tier;
    occupations_heirarchy heirarchy;
} occupations_definition;


const occupations_definition occupations[OR_LAST_ONE] =
{
    { OR_ABBESS,             OG_FEMALE,    OT_ONE,     OH_RELIGION },
    { OR_ABBOTT,             OG_MALE,      OT_ONE,     OH_RELIGION },
    { OR_ABLE_SEAMAN,        OG_UNKNOWN,   OT_TWO,     OH_SEAMAN   },
    { OR_ORDINARY_SEAMAN,    OG_UNKNOWN,   OT_THREE,   OH_SEAMAN   },
    { OR_ACADEMIC,           OG_UNKNOWN,   OT_UNKNOWN, OH_EDUCATOR_UNIVERSITY },
    { OR_ACCOUNTANT,         OG_UNKNOWN,   OT_UNKNOWN, OH_ECONOMY },
    { OR_ACOLYTE,            OG_UNKNOWN,   OT_THREE,   OH_RELIGION },
    { OR_ACTUARY,            OG_UNKNOWN,   OT_TWO,     OH_ECONOMY },
    { OR_ADVOCATE,           OG_UNKNOWN,    OT_THREE,   OH_LAW },
    { OR_BAILIFF,            OG_UNKNOWN,    OT_TWO,     OH_LAW },
    { OR_BAKEMAKER,          OG_UNKNOWN,    OT_TWO,     OH_POTTERY },
    { OR_BALLER,             OG_UNKNOWN,    OT_THREE,   OH_POTTERY },
    { OR_CAB_DRIVER,         OG_UNKNOWN,    OT_UNKNOWN, OH_TRANSPORT },
    { OR_CADDY,              OG_MALE,       OT_THREE,   OH_ERRANDS },
    { OR_DAIRYMAN,           OG_MALE,       OT_TWO,     OH_DAIRY },
    { OR_DANTER,             OG_FEMALE,     OT_TWO,     OH_TEXTILES },
    { OR_DRAPER,             OG_UNKNOWN,    OT_ONE,     OH_TEXTILES },
    { OR_DRAUGHTSMAN,        OG_UNKNOWN,    OT_TWO,     OH_ARCHITECTURE },
    { OR_DRAWBOY,            OG_UNKNOWN,    OT_THREE,   OH_TEXTILES },
    { OR_DAYLABORER,         OG_UNKNOWN,    OT_THREE,   OH_LABOR },
    { OR_DAYMAN,             OG_MALE,       OT_THREE,   OH_LABOR },
    { OR_DESIGNER,           OG_UNKNOWN,    OT_ONE,     OH_POTTERY },
    { OR_CONSTABLE,          OG_MALE,       OT_TWO,     OH_POLICE },
    { OR_OFFICEROFTHEPARISH, OG_MALE,       OT_TWO,     OH_RELIGION},
    { OR_DEVILLER,           OG_UNKNOWN,    OT_THREE,    OH_PRINTING },
    { OR_DIE_SINKER,         OG_UNKNOWN,    OT_THREE,    OH_METALWORK },
    { OR_DIPPER,             OG_UNKNOWN,    OT_TWO,      OH_POTTERY   },
    { OR_DISH_THROWER,       OG_UNKNOWN,    OT_THREE,    OH_POTTERY   },
    { OR_DISH_TURNER,        OG_UNKNOWN,    OT_THREE,    OH_WOODWORK   },
    { OR_DOCK_FOYBOATMAN,    OG_MALE,       OT_TWO,      OH_DOCKS },
    { OR_DOCKER,             OG_UNKNOWN,    OT_THREE,    OH_DOCKS },
    { OR_DOCKMASTER,         OG_MALE,       OT_ONE,      OH_DOCKS },
    { OR_DOFFER,            OG_UNKNOWN,    OT_TWO,       OH_TEXTILES },
    { OR_DOG_BREAKER,       OG_UNKNOWN,    OT_THREE,     OH_ANIMALS},
    { OR_DOG_WHIPPER,       OG_UNKNOWN,    OT_THREE,     OH_RELIGION},
    { OR_DOORKEEPER,        OG_UNKNOWN,    OT_TWO,       OH_HOME },
    { OR_DOTTLER,           OG_UNKNOWN,    OT_THREE,     OH_POTTERY   },
    { OR_UTENSIL_MAKER,     OG_UNKNOWN,    OT_TWO,       OH_POTTERY   },
    { OR_DRAGSMAN,          OG_MALE,       OT_TWO,       OH_TRANSPORT},
    { OR_DRAINER,           OG_UNKNOWN,     OT_THREE,    OH_LABOR},
    { OR_DISTILLER,         OG_UNKNOWN,     OT_ONE,      OH_ALCOHOL},
    { OR_DISTRIBUTOR,        OG_UNKNOWN,    OT_TWO,     OH_RELIGION },
    { OR_WEAVER,             OG_UNKNOWN,    OT_TWO,     OH_TEXTILES },
    { OR_SEAMSTRESS,         OG_FEMALE,     OT_TWO,     OH_TEXTILES },
    { OR_DOMESTIC_SERVANT,   OG_UNKNOWN,    OT_THREE,   OH_HOME },
    { OR_DOORMAN,            OG_MALE,       OT_TWO,     OH_HOME },
    { OR_UNDER_GARDNER,      OG_UNKNOWN,    OT_THREE,   OH_GARDENS },
    { OR_GARDNER,            OG_UNKNOWN,    OT_TWO,     OH_GARDENS },
    { OR_UNDERPRESSER,      OG_UNKNOWN,    OT_THREE,   OH_TEXTILES },
    { OR_UNDERLAY_STITCHER, OG_UNKNOWN,    OT_THREE,   OH_SHOES },
    { OR_UNDERSTUDY,        OG_UNKNOWN,    OT_THREE,   OH_THEATER },
    { OR_UNDERTAKER,       OG_UNKNOWN,     OT_TWO,     OH_DEATH },
}; // 202 - 151 = 51 occupations

