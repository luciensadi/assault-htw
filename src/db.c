/*~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-
 ~  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        ~
 ~  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   ~
 ~                                                                         ~
 ~  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          ~
 ~  Chastain, Michael Quan, and Mitchell Tse.                              ~
 ~                                                                         ~
 ~  Ack 2.2 improvements copyright (C) 1994 by Stephen Dooley              ~
 ~  ACK!MUD is modified Merc2.0/2.1/2.2 code (c)Stephen Zepp 1998 Ver: 4.3 ~
 ~                                                                         ~
 ~  In order to use any part of this  PA  Diku Mud, you must comply with   ~
 ~  both the original Diku license in 'license.doc' as well the Merc       ~
 ~  license in 'license.txt', and the Ack!Mud license in 'ack_license.txt'.~
 ~  In particular, you may not remove any of these copyright notices.      ~
 ~                                                                         ~
 ~           _______      _____                                            ~
 ~          /  __  /\    / ___ \       222222        PA_MUD by Amnon Kruvi ~
 ~         /______/ /   / /___\ \            2       PA_MUD is modified    ~
 ~        / _______/   / _______ \           2       Ack!Mud, v4.3         ~
 ~       /_/          /_/       \_\        2                               ~
 ~                                      2                                  ~
 ~                                     2222222                             ~
 ~                                                                         ~
 ~                                                                         ~
 ~   Years of work have been invested to create DIKU, Merc, Ack and PA.    ~
 ~   Please show your respect by following the licenses, and issuing       ~
 ~   credits where due.                                                    ~
 ~                                                                         ~
 ~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-*/
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <stdarg.h>
#include "ack.h"
#include "hash.h"
#include "ssm.h"
#include <unistd.h>                                         /* for execl */
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include "tables.h"

#if !defined(macintosh)
extern int _filbuf args( (FILE *) );
#endif

/*
 * Globals.
 */
char bug_buf[2 * MAX_INPUT_LENGTH];
char log_buf[2 * MAX_INPUT_LENGTH];
char testerbuf[MSL] = "\0";
KILL_DATA kill_table[MAX_LEVEL];
TIME_INFO_DATA time_info;
SYS_DATA_TYPE sysdata;

char * quote_table[MAX_QUOTE];
int total_quotes;

bool booting_up;
bool area_resetting_global;
bool mem_log;
void insert_area args ( ( AREA_DATA *pArea) ); /* Auto-sort of areas */

const int convert_wearflags[] = {
BIT_24, BIT_14, BIT_8, BIT_19, BIT_4, BIT_21, BIT_22, BIT_13,
BIT_11, BIT_16, BIT_17, BIT_18, BIT_12, BIT_16, BIT_16, BIT_5,
BIT_7, BIT_16,
BIT_24, BIT_24, BIT_24, BIT_24, BIT_24, BIT_24, BIT_24,
BIT_24, BIT_24, BIT_24, BIT_24, BIT_24, BIT_24, BIT_24 };

/*
 * Locals.
 */
MOB_INDEX_DATA * mob_index_hash[MAX_KEY_HASH];
OBJ_INDEX_DATA * obj_index_hash[MAX_KEY_HASH];
ROOM_INDEX_DATA * room_index_hash[MAX_KEY_HASH];
char * string_hash[MAX_KEY_HASH];

AREA_DATA * area_used[MAX_AREAS];
AREA_DATA * area_load;
/*
 * replaced for SSM
 */

#if 0
char * string_space = NULL;
char * top_string = NULL;
char str_empty [1] = {0};
#endif

extern char str_empty[1];
extern long sOverFlowString;
extern long nOverFlowString;
extern bool Full;

int top_ed;
int top_help;
int top_mob_index;
int top_obj_index;
int top_room;

/*
 * Memory management.
 * Increase MAX_STRING if you have too.
 * Tune the others only if you understand what you're doing.
 */

#if 0

#define                 MAX_STRING      350000
/*#define                 MAX_PERM_BLOCK  131072
 #define                 MAX_MEM_LIST    12

 void *                  rgFreeList      [MAX_MEM_LIST];
 const int               rgSizeList      [MAX_MEM_LIST]  =
 {
 16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384, 32768-64, 65536-16
 };*/

int nAllocString;
int sAllocString;
#endif

extern long nAllocString;
extern long sAllocString;

int nAllocPerm;
int sAllocPerm;

extern long MAX_STRING;
void init_string_space(void);
void boot_done(void);

/*
 * Semi-locals.
 */
bool fBootDb;
FILE * fpArea;
char strArea[MAX_INPUT_LENGTH];
int area_revision = -1;

/*
 * Local booting procedures.
 */
void init_mm args( ( void ) );

void load_area args( ( FILE *fp ) );
void load_helps args( ( FILE *fp ) );
void load_mobiles args( ( FILE *fp ) );
void load_objects args( ( FILE *fp ) );
void load_resets args( ( FILE *fp ) );
void load_rooms args( ( FILE *fp ) );
void load_specials args( ( FILE *fp ) );
void load_objfuns args( ( FILE *fp ) );
void load_gold args( ( void ) );
void load_alliances args( ( void ) );
void load_changes args( ( void ) );
void load_logs args( ( void ) );
void read_map_from_file args( ( void ) );
void create_load_list args( ( void ) );
void load_buildings args( ( void ) );
void load_scores args( ( void ) );
void load_multiplay args( ( void ) );
void load_ranks args( ( void ) );
void load_quotes args( ( void ) );
void load_bans args( ( void ) );
void load_brands args( ( void ) );

void fix_exits args( ( void ) );
void check_resets args( ( void ) );

void copyover_recover args( ( void ) );
void init_dealer args( ( void ) );

#define SHOW_AREA \
    if (!previous_bug) \
    { \
        bug ("      In %s.",(int) pArea->filename); \
        previous_bug=1; \
    }

/*
 * Big mama top level function.
 */
void boot_db(bool fCopyOver) {
	int a;

	/*
	 * Init some data space stuff.
	 */

#if 0
	{
		if ( ( string_space = calloc( 1, MAX_STRING ) ) == NULL )
		{
			bug( "Boot_db: can't alloc %d string space.", MAX_STRING );
			exit( 1 );
		}
		top_string = string_space;
		fBootDb = TRUE;
	}
#endif

    init_string_space();
    fBootDb=TRUE;

    send_to_descrips( "Initialising Ack! Mud.  Please Wait....\n\r" );

    /*
     * Init random number generator.
     */
        init_mm( );

    /*
     * Set time and weather.
     */
        long lhour, lday, lmonth;

        lhour           = (current_time - 650336715)
                          / (PULSE_TICK / PULSE_PER_SECOND);
        time_info.hour  = lhour  % 24;
        lday            = lhour  / 24;
        time_info.day   = lday   % 35;
        lmonth          = lday   / 35;
        time_info.month = lmonth % 17;
        time_info.year  = lmonth / 17;

    /* Clear list of used areas */
    for (a=0; a< MAX_AREAS; a++)
        area_used[a]=NULL;

    /*
     * Read in all the socials.
     */
        load_social_table( );

    /*
     * Read in all the area files.
     */
        FILE *fpList;
        log_f( "Reading Area Files..." );

        if ( ( fpList = fopen( AREA_LIST, "r" ) ) == NULL )
        {
            perror( AREA_LIST );
            log_f( "Unable to open area.lst, aborting bootup." );
            kill( getpid(), SIGQUIT );
        }

        for ( ; ; )
        {
            strcpy( strArea, fread_word( fpList ) );
            if ( strArea[0] == '$' )
                break;

            if ( strArea[0] == '-' )
            {
                fpArea = stdin;
            }
            else
            {
                if ( ( fpArea = fopen( strArea, "r" ) ) == NULL )
                {
                    log_string( strArea );
                    kill( getpid(), SIGQUIT );
                }
            }

            for ( ; ; )
            {
                char *word;

                if ( fread_letter( fpArea ) != '#' )
                {
                    bug( "Boot_db: # not found.", 0 );
                    kill( getpid(), SIGQUIT );
                }

                word = fread_word( fpArea );

                if ( word[0] == '$'               )                 break;
                else if ( !str_cmp( word, "AREA"     ) ) load_area    (fpArea);
                else if ( !str_cmp( word, "HELPS"    ) ) load_helps   (fpArea);
                else if ( !str_cmp( word, "ROOMS"    ) ) load_rooms   (fpArea);
                else if ( !str_cmp( word, "MOBILES"  ) ) load_mobiles (fpArea);
                else if ( !str_cmp( word, "OBJECTS"  ) ) load_objects (fpArea);
                else if ( !str_cmp( word, "SPECIALS" ) ) load_specials(fpArea);
                else
                {
                    bug( "Boot_db: bad section name.", 0 );
                    exit( 1 );
                }
            }

            if ( fpArea != stdin )
                fclose( fpArea );
            fpArea = NULL;
        }
        fclose( fpList );
        	// Load Acid Spray object at an unreachable room... - Demortes WHY?!
        int i;
        extern OBJ_DATA *quest_obj[MAX_QUEST_ITEMS];
        for ( i=0; i<MAX_QUEST_ITEMS; i++ )
            quest_obj[i] = NULL;
        vehicle_weapon = create_object( get_obj_index(OBJ_VNUM_ACID_SPRAY), 0 );
        vehicle_weapon->x = 0;
        vehicle_weapon->y = 0;
        vehicle_weapon->z = 4;
        UNLINK(vehicle_weapon, first_obj, last_obj, next, prev);
        obj_to_room(vehicle_weapon,get_room_index(ROOM_VNUM_WMAP));

        //		Init history and guessing game variables.
        extern char *history1;
        extern char *history2;
        extern char *history3;
        extern char *history4;
        extern char *history5;
        extern char *history6;
        extern char *history7;
        extern char *history8;
        extern char *history9;
        extern char *history10;
        extern int guess_game;

        if ( history1 != NULL )
            free_string(history1);
        if ( history2 != NULL )
            free_string(history2);
        if ( history3 != NULL )
            free_string(history3);
        if ( history4 != NULL )
            free_string(history4);
        if ( history5 != NULL )
            free_string(history5);
        if ( history6 != NULL )
            free_string(history6);
        if ( history7 != NULL )
            free_string(history7);
        if ( history8 != NULL )
            free_string(history8);
        if ( history9 != NULL )
            free_string(history9);
        if ( history10 != NULL )
            free_string(history10);
        history1 = str_dup("");
        history2 = str_dup("");
        history3 = str_dup("");
        history4 = str_dup("");
        history5 = str_dup("");
        history6 = str_dup("");
        history7 = str_dup("");
        history8 = str_dup("");
        history9 = str_dup("");
        history10 = str_dup("");
        guess_game = 0;

        //Move on individual loading functions.
        fBootDb = FALSE;
        log_f( "Loading web data." );
        load_web_data();
        log_f( "Loading Building Table...." );
        load_building_t( );
        log_f( "Loading Wilderness...." );
        read_map_from_file( );
        log_f( "Creating Special Maps...." );
        create_special_map();
        log_f( "Loading Buildings...." );
        load_buildings();
        log_f( "Loading Objects...." );
        load_sobjects( 1 );
        log_f( "Loading High Scores...." );
        load_scores();
        load_ranks();
        log_f( "Loading alliances...." );
        load_alliances();
        log_f( "Loading changes." );
        load_changes();
        log_f( "Loading logs." );
        load_logs();
        log_f( "Creating item load list...." );
        create_load_list();
        /* loading of disabled commands - Wyn */
        log_f( "Loading Disabled Commands..." );
        load_disabled();
        log_f( "Loading Multiplay list...." );
        load_multiplay();
        log_f( "Loading quotes." );
        load_quotes( );
        booting_up = TRUE;
        booting_up = FALSE;
        log_f( "Loading banned sites." );
        load_bans( );
        log_f( "Loading Relevel Info." );
        do_loadrelevel( );

        log_f( "Loading imm brands." );
        load_brands( );
        log_f( "Loading System Data." );
        load_sysdata( );
        save_objects(0);

        //Vehicle handling, depending on Copyover or not
    if (fCopyOver)
    {
        extern bool disable_timer_abort;
        disable_timer_abort = TRUE;
        copyover_recover();
        disable_timer_abort = FALSE;
    }
    else
    {
        log_f( "Loading vehicles..." );
        load_vehicles(0);
    }
    return;
} //end dbboot()

/*
 * Snarf an 'area' header line.
 */
void load_area(FILE *fp) {
	AREA_DATA *pArea;
	char letter;
	int a;

	GET_FREE(pArea, area_free);
	pArea->name = fread_string(fp);
	pArea->offset = 0;
	pArea->modified = 0;
	pArea->min_vnum = 0;
	pArea->max_vnum = MAX_VNUM;
	pArea->area_num = 0;
	pArea->filename = str_dup(strArea);
	pArea->owner = str_empty;
	pArea->can_read = str_dup("all");
	pArea->can_write = str_dup("all");
	pArea->keyword = str_dup("none");
	pArea->first_area_room = NULL;
	pArea->last_area_room = NULL;
	pArea->first_area_help_text = NULL;
	pArea->last_area_help_text = NULL;
	pArea->first_area_object = NULL;
	pArea->last_area_object = NULL;
	area_revision = -1;

	for (;;) {
		letter = fread_letter(fp);
		if (letter == '#') {
			ungetc(letter, fp);
			break;
		}

		switch (letter) {
		case 'O':
			pArea->owner = fread_string(fp);
			break;
		case 'Q':
			area_revision = fread_number(fp);
			break;
		case 'R':
			free_string(pArea->can_read);
			pArea->can_read = fread_string(fp);
			break;
		case 'W':
			free_string(pArea->can_write);
			pArea->can_write = fread_string(fp);
			break;
		case 'X':
			pArea->offset = fread_number(fp);
			break;
		case 'N':
			pArea->area_num = fread_number(fp);
			break;
		case 'K':
			free_string(pArea->keyword);
			pArea->keyword = fread_string(fp);
			break;
		case 'V':
			pArea->min_vnum = fread_number(fp);
			pArea->max_vnum = fread_number(fp);
			break;

		}
	}

	if (pArea->area_num == 0) {
		/* Find a unique area number */
		for (a = 0; a < MAX_AREAS; a++)
			if (area_used[a] == NULL)
				break;
		pArea->area_num = a;
	}

	area_used[pArea->area_num] = pArea;
	area_load = pArea;

	insert_area(pArea); /* Use this instead of the above */
	return;
}

/*
 * Snarf a help section.
 */
void load_helps(FILE *fp) {
	HELP_DATA *pHelp;
	BUILD_DATA_LIST *pList;

	for (;;) {
		GET_FREE(pHelp, help_free);
		pHelp->level = fread_number(fp);
		pHelp->keyword = fread_string(fp);
		if (pHelp->keyword[0] == '$')
			break;
		pHelp->text = fread_string(fp);

		/* greeting text handled in comm.c now -S- */

		LINK(pHelp, first_help, last_help, next, prev);
		/* MAG Mod */
		GET_FREE(pList, build_free);
		pList->data = pHelp;
		LINK(pList, area_load->first_area_help_text,
				area_load->last_area_help_text, next, prev);

		top_help++;
	}

	return;
}

void load_quotes(void) {
	FILE *quotefp;
	char quote_file_name[MSL] = "\0";
	char buf[MSL] = "\0";
	int i;
	extern int total_quotes;

	sprintf(quote_file_name, "%s", QUOTE_FILE);

	sprintf(buf, "Loading %s\n\r", quote_file_name);
	log_string( buf );;

	if ((quotefp = fopen(quote_file_name, "r")) == NULL) {
		log_f("Load quote Table: fopen");
		perror("failed open of quotes.txt in load_quote_table");
		sprintf(quote_table[0],
				"\nThank you for playing " mudnamecolor ", please come again.");
		total_quotes = 1;
	} else {
		for (i = 0; i < MAX_QUOTE; i++) {
			quote_table[i] = fread_string(quotefp);
			if (!str_cmp(quote_table[i], "#END"))
				break;
		}
		total_quotes = i;
	}
	fclose(quotefp);
	return;
}

void load_bans(void) {

	FILE *bansfp;
	char bans_file_name[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];

	sprintf(bans_file_name, "%s", BANS_FILE);
	sprintf(buf, "Loading %s\n\r", bans_file_name);
	log_string( buf );;

	if ((bansfp = fopen(bans_file_name, "r")) == NULL) {
		log_f("Load bans Table: fopen");
		perror("failed open of bans_table.dat in load_bans_table");
	} else {
		fpArea = bansfp;
		sprintf(strArea, "%s", bans_file_name);

		for (;;) {
			char *word;
			BAN_DATA *pban;
			word = fread_string(bansfp);
			if (!str_cmp(word, "#BAN")) {
				sh_int get_bool = FALSE;

				GET_FREE(pban, ban_free);
				get_bool = fread_number(bansfp);
				if (get_bool == 1)
					pban->newbie = TRUE;
				else
					pban->newbie = FALSE;
				pban->name = fread_string(bansfp);
				pban->banned_by = fread_string(bansfp);
				pban->note = fread_string(bansfp);
				LINK(pban, first_ban, last_ban, next, prev);
				free_string(word);
			} else if (!str_cmp(word, "#END")) {
				free_string(word);
				break;
			} else {
				free_string(word);
				log_f("Load_bans: bad section.");
				break;
			}
		}

		fclose(bansfp);
		fpArea = NULL;
		sprintf(buf, "Done Loading %s", bans_file_name);
		log_string( buf );;

	}
}

/*
 * Snarf a room section.
 */
void load_rooms(FILE *fp) {
	ROOM_INDEX_DATA *pRoomIndex;
	BUILD_DATA_LIST *pList;

	if (area_load == NULL) {
		bug("Load_rooms: no #AREA seen yet.", 0);
		hang("Loading Rooms in db.c");
	}

	for (;;) {
		sh_int vnum;
		char letter;
		int iHash;

		letter = fread_letter(fp);
		if (letter != '#') {
			bug("Load_rooms: # not found.", 0);
			hang("Loading Rooms in db.c");
		}

		vnum = fread_number(fp);
		if (vnum == 0)
			break;

		fBootDb = FALSE;
		if (get_room_index(vnum) != NULL) {
			bug("Load_rooms: vnum %d duplicated.", vnum);
			hang("Loading Rooms in db.c");
		}
		fBootDb = TRUE;

		GET_FREE(pRoomIndex, rid_free);
		pRoomIndex->area = area_load;
		pRoomIndex->vnum = vnum;
		for (;;) {
			letter = fread_letter(fp);

			if (letter == 'S')
				break;

			else {
				bug("Load_rooms: vnum %d has flag not 'DES'.", vnum);
				hang("Loading Rooms in db.c");
			}
		}

		iHash = vnum % MAX_KEY_HASH;
		SING_TOPLINK(pRoomIndex, room_index_hash[iHash], next);
		GET_FREE(pList, build_free);
		pList->data = pRoomIndex;
		LINK(pList, area_load->first_area_room, area_load->last_area_room, next,
				prev);

		top_room++;
	}

	return;
}

/*
 * Snarf spec proc declarations.
 */
void load_specials(FILE *fp) {
	for (;;) {
		char letter;

		switch (letter = fread_letter(fp)) {
		default:
			bug("Load_specials: letter '%c' not *, M, or S.", letter);
			hang("Loading Specials in db.c");
			return;
			break;
		case 'S':
			return;
		case '*':
			break;
		}
		/* NB. Comments will not be saved when using areasave - MAG. */
		fread_to_eol(fp);
	}
}

#define VALID_RESET 0
#define INVAL_ROOM  1
#define INVAL_OBJ   2
#define INVAL_MOB   3
#define INVAL_GEN   4


/*
 * Create an instance of a mobile.
 */
CHAR_DATA *create_mobile( MOB_INDEX_DATA * pMobIndex )
{
    CHAR_DATA *mob;
   
    if( pMobIndex == NULL )
    {
        bug( "Create_mobile: NULL pNpcIndex.", 0 );
        pMobIndex = get_mob_index(1);
    }
    
    GET_FREE(mob, char_free);
    clear_char( mob );
    mob->pIndexData = pMobIndex;

    mob->name           = str_dup(pMobIndex->player_name);
    mob->short_descr    = str_dup(pMobIndex->short_descr);
    mob->long_descr     = str_dup(pMobIndex->long_descr);
    mob->description    = str_dup(pMobIndex->description);

    mob->act            = pMobIndex->act;
    mob->level          = pMobIndex->level;
    mob->sex            = pMobIndex->sex;
    
    mob->max_hit        = mob->level * 15 + number_range( mob->level * mob->level / 2, mob->level * mob->level );
    mob->hit            = mob->max_hit;
    
    mob->next = NULL;
    mob->prev = NULL;
    LINK(mob, first_char, last_char, next, prev);
    pMobIndex->count++;

    return mob;
}

/*
 * Create an instance of an object.
 */
OBJ_DATA *create_object( OBJ_INDEX_DATA *pObjIndex, int level )
{
    static OBJ_DATA obj_zero;
    OBJ_DATA *obj;
    int looper;
    int pLevel;

    /*
    if ( level < 1 )
      level = 1;
    */

    if ( pObjIndex == NULL )
    {
        bug( "Create_object: NULL pObjIndex.", 0 );
        pObjIndex = get_obj_index(1);
        //      hang( "Creating Object in db.c" );
    }

    GET_FREE(obj, obj_free);
    *obj                = obj_zero;
    obj->pIndexData     = pObjIndex;
    obj->in_room        = NULL;

    if (level < 1)
    {
        obj->level = pObjIndex->level;
    }
    else
    {
        obj->level = level;
    }

    obj->wear_loc       = -1;

    obj->name           = str_dup(pObjIndex->name);
    obj->short_descr    = str_dup(pObjIndex->short_descr);
    obj->description    = str_dup(pObjIndex->description);
    obj->owner      = str_dup("Nobody");
    obj->item_type      = pObjIndex->item_type;
    obj->extra_flags    = pObjIndex->extra_flags;
    obj->wear_flags     = pObjIndex->wear_flags;
    obj->first_in_carry_list = NULL;
    obj->next_in_carry_list = NULL;
    obj->prev_in_carry_list = NULL;
    obj->next = NULL;
    obj->prev = NULL;
    obj->attacker = NULL;
    obj->weight = pObjIndex->weight;
    obj->heat = pObjIndex->heat;
    obj->x = 0;
    obj->y = 0;
    obj->z = 4;
    obj->in_building = NULL;
    if ( obj->item_type == ITEM_BOMB )
        obj->bomb_data = make_bomb(obj);
    else
        obj->bomb_data = NULL;

    obj->quest_map = 0;
    obj->quest_timer = 0;
    for ( looper = 0; looper < MAX_OBJECT_VALUES; looper++ )
    {
        obj->value[looper]       = pObjIndex->value[looper];
    }

    /* Ok now that we have an actual object */
    {
        if (obj->level == 1)
            pLevel = 2;
        else
            pLevel = obj->level;
    }

    if (pLevel < 1) pLevel = 1;                             /* Should not happen, but make sure it's valid */

    /*
     * Mess with object properties.
     */
    switch ( obj->item_type )
    {
    default:
        bugf( "create_object: vnum %d bad type %d.", pObjIndex->vnum,
              obj->item_type );
        break;
    case ITEM_LIGHT:
    case ITEM_DRONE:
    case ITEM_IMPLANT:
    case ITEM_INSTALLATION:
    case ITEM_BOARD:
    case ITEM_MATERIAL:
    case ITEM_AMMO:
    case ITEM_WEAPON:
    case ITEM_BOMB:
    case ITEM_ARMOR:
    case ITEM_TELEPORTER:
    case ITEM_BLUEPRINT:
    case ITEM_SUIT:
    case ITEM_MEDPACK:
    case ITEM_TOKEN:
    case ITEM_FLAG:
    case ITEM_DART_BOARD:
    case ITEM_ELEMENT:
    case ITEM_CONTAINER:
    case ITEM_WEAPON_UP:
    case ITEM_PIECE:
    case ITEM_COMPUTER:
    case ITEM_LOCATOR:
    case ITEM_SKILL_UP:
    case ITEM_PART:
    case ITEM_DISK:
    case ITEM_TRASH:
    case ITEM_ASTEROID:
    case ITEM_BACKUP_DISK:
    case ITEM_VEHICLE_UP:
    case ITEM_TOOLKIT:
    case ITEM_SCAFFOLD:
    case ITEM_ORE:
    case ITEM_BIOTUNNEL:
    case ITEM_BATTERY:
    case ITEM_RECREATIONAL:
        break;

    }

    if ( sysdata.killfest && !IS_SET(obj->extra_flags,ITEM_NOSAVE) )
        SET_BIT(obj->extra_flags,ITEM_NOSAVE);

    LINK(obj, first_obj, last_obj, next, prev);

    return obj;
}

BOMB_DATA * make_bomb(OBJ_DATA *obj) {
	BOMB_DATA *bomb;
	GET_FREE(bomb, bomb_free);
	bomb->obj = obj;
	bomb->is_free = FALSE;
	LINK(bomb, first_bomb, last_bomb, next, prev);
	return bomb;
}

/*
 * Clear a new character.
 */
void clear_char(CHAR_DATA *ch) {
	static CHAR_DATA ch_zero;

	*ch = ch_zero;
	ch->name = &str_empty[0];
	ch->prompt = &str_empty[0];
	ch->logon = current_time;
	ch->position = POS_STANDING;
	ch->hit = STARTING_HP;
	ch->max_hit = STARTING_HP;
	ch->act_build = 0;
	ch->build_vnum = -1;
	/*    ch->pcdata->recall_vnum = 3001;     */

	return;
}

/*
 * Free a character.
 */
void free_char(CHAR_DATA *ch) {
	CHAR_DATA *rch;

	ch->is_quitting = TRUE;
	while (ch->first_carry != NULL)
		extract_obj(ch->first_carry);
	if (ch->in_vehicle)
		extract_vehicle(ch->in_vehicle, FALSE);

	if (map_ch[ch->x][ch->y][ch->z] == ch)
		map_ch[ch->x][ch->y][ch->z] = ch->next_in_room;

	for (rch = first_char; rch; rch = rch->next) {
		if (rch->next_in_room == ch)
			rch->next_in_room = ch->next_in_room;
		if (rch->leader == ch)
			rch->leader = NULL;
		if (rch->reply == ch)
			rch->reply = NULL;
		if (rch->victim == ch)
			rch->victim = rch;
	}

	if (ch->current_brand) {
		PUT_FREE(ch->current_brand, brand_data_free);
	}

	if (ch->pcdata != NULL) {
		QUEUE_DATA *q;
		QUEUE_DATA *q_next;
		PAGER_DATA *p;
		PAGER_DATA *p_next;
		for (p = ch->pcdata->pager; p; p = p_next) {
			p_next = p->next;
			extract_pager(p);
		}
		for (q = ch->pcdata->queue; q; q = q_next) {
			q_next = q->next;
			extract_queue(q);
		}
		ch->pcdata->queue = NULL;
		ch->pcdata->last_queue = NULL;
		//	if (IMC)
		//		imc_freechardata( ch );
		ch->pcdata->is_free = FALSE;
		PUT_FREE(ch->pcdata, pcd_free);
	}

	PUT_FREE(ch, char_free);
	return;
}

/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index( int vnum )
{
    MOB_INDEX_DATA *pMobIndex;

    for ( pMobIndex  = mob_index_hash[vnum % MAX_KEY_HASH]; pMobIndex != NULL;
        pMobIndex  = pMobIndex->next )
    {
        if ( pMobIndex->vnum == vnum )
            return pMobIndex;
    }

    if ( fBootDb )
    {
        bug( "Get_mob_index: bad vnum %d.", vnum );
        hang("Get Mob Index in db.c");
    }

    return NULL;
}


/*
 * Translates mob virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index(int vnum) {
	OBJ_INDEX_DATA *pObjIndex;

	for (pObjIndex = obj_index_hash[vnum % MAX_KEY_HASH]; pObjIndex != NULL;
			pObjIndex = pObjIndex->next) {
		if (pObjIndex == NULL)
			continue;
		if (pObjIndex->vnum == vnum)
			return pObjIndex;
	}

	if (fBootDb) {
		bug("Get_obj_index: bad vnum %d.", vnum);
		hang("Get Object Index in db.c");
	}

	return NULL;
}

/*
 * Translates mob virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *get_room_index(int vnum) {
	ROOM_INDEX_DATA *pRoomIndex;

	for (pRoomIndex = room_index_hash[vnum % MAX_KEY_HASH]; pRoomIndex != NULL;
			pRoomIndex = pRoomIndex->next) {
		if (pRoomIndex->vnum == vnum)
			return pRoomIndex;
	}

	if (fBootDb) {
		bug("Get_room_index: bad vnum %d.", vnum);
		/*	exit( 1 );   */
	}

	return NULL;
}

/*
 * Read a letter from a file.
 */
char fread_letter(FILE *fp) {
	char c;

	do {
		c = getc(fp);
	} while (isspace(c));

	return c;
}

/*
 * Read a number from a file.
 */
int fread_number(FILE *fp) {
	int number;
	bool sign;
	char c;

	do {
		c = getc(fp);
	} while (isspace(c));

	number = 0;

	sign = FALSE;
	if (c == '+') {
		c = getc(fp);
	} else if (c == '-') {
		sign = TRUE;
		c = getc(fp);
	}

	if (!isdigit(c)) {
		char error_buf[MSL] = "\0";
		sprintf(error_buf, "%c", c);
		bug_string("Fread_number: looking for a digit, found a %s.", error_buf);
		hang("Error in fread_number");
	}

	while (isdigit(c)) {
		number = number * 10 + c - '0';
		c = getc(fp);
	}

	if (sign)
		number = 0 - number;

	if (c == '|')
		number += fread_number(fp);
	else if (c != ' ')
		ungetc(c, fp);

	return number;
}

/*
 * Read a Long number from a file.
 */
long_int fread_long_number(FILE *fp) {
	long_int number;
	bool sign;
	char c;

	do {
		c = getc(fp);
	} while (isspace(c));

	number = 0;

	sign = FALSE;
	if (c == '+') {
		c = getc(fp);
	} else if (c == '-') {
		sign = TRUE;
		c = getc(fp);
	}

	if (!isdigit(c)) {
		char error_buf[MSL] = "\0";
		sprintf(error_buf, "%c", c);
		bug_string("Fread_number: looking for a digit, found a %s.", error_buf);
		hang("Error in fread_number");
	}

	while (isdigit(c)) {
		number = number * 10 + c - '0';
		c = getc(fp);
	}

	if (sign)
		number = 0 - number;

	if (c == '|')
		number += fread_number(fp);
	else if (c != ' ')
		ungetc(c, fp);

	return number;
}

/*
 * Read to end of line (for comments).
 */

/* Spec: fixed to handle EOF more gracefully */

void fread_to_eol(FILE *fp) {
	char c;

	do {
		if (feof(fp)) {
			bugf("fread_to_eol: EOF encountered on read.");
			if (fBootDb)
				exit(1);
			return;
		}
		c = getc(fp);
	} while (c != '\n' && c != '\r');

	do {
		c = getc(fp);
	} while (c == '\n' || c == '\r');

	ungetc(c, fp);
	return;
}

/* Same as above, but returns the rest of the line */

/* Spec: fixed to handle EOF more gracefully */

char * fsave_to_eol(FILE *fp) {
	char string[MAX_INPUT_LENGTH + 1];
	int a;
	char c;

	a = 0;
	do {
		string[a++] = getc(fp);

	} while (a < MAX_INPUT_LENGTH && string[a - 1] != '\n'
			&& string[a - 1] != '\r' && string[a - 1] != EOF);

	if (string[a - 1] == EOF) {
		bugf("fsave_to_eol: EOF");
		string[a - 1] = 0;
		return str_dup(string);
	}

	if (a == MAX_INPUT_LENGTH)
		fread_to_eol(fp);
	else {
		c = string[a - 1];
		while (c == '\n' || c == '\r')
			c = getc(fp);

		if (c == EOF)
			bugf("fsave_to_eol: EOF");
		else
			ungetc(c, fp);
	}
	string[a] = '\0';

	return str_dup(string);
}

/*
 * Read one word (into static buffer).
 */
char *fread_word(FILE *fp) {
	static char word[MAX_INPUT_LENGTH];
	char *pword;
	char cEnd;

	do {
		if (feof(fp)) {
			bug("fread_word: EOF encountered on read.\n\r", 0);
			if (fBootDb)
				kill(getpid(), SIGQUIT);
			strcpy(word, "");
			return word;
		}
		cEnd = getc(fp);
	} while (isspace(cEnd));

	if (cEnd == '\'' || cEnd == '"') {
		pword = word;
	} else {
		word[0] = cEnd;
		pword = word + 1;
		cEnd = ' ';
	}

	for (; pword < word + MAX_INPUT_LENGTH; pword++) {
		if (feof(fp)) {
			bug("fread_word: EOF encountered on read.\n\r", 0);
			if (fBootDb)
				kill(getpid(), SIGQUIT);
			*pword = '\0';
			return word;
		}
		*pword = getc(fp);
		if (cEnd == ' ' ? isspace(*pword) : *pword == cEnd) {
			if (cEnd == ' ')
				ungetc(*pword, fp);
			*pword = '\0';
			return word;
		}
	}

	bug("Fread_word: word too long", 0);
	kill(getpid(), SIGQUIT);
	return NULL;
}

void *_getmem(int size, const char *caller, int log) {
	void *mem;

	if (!(mem = malloc(size))) {
		fprintf(stderr, "Out of memory.\n");
		raise(SIGSEGV);
	}
	memset(mem, 0, size);

	if (log && mem_log)
		log_f("getmem(%d)=%p from %s", size, mem, caller);

	return mem;
}

#if 0
/*
 * Duplicate a string into dynamic memory.
 * Fread_strings are read-only and shared.
 */
#define STRING_FREELIST
#ifdef STRING_FREELIST
#define MAX_SIZE_LIST   13
static const int sizelist[MAX_SIZE_LIST] =
{	8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768};
struct text_data
{
	struct text_data *next;
	char *text;
};
static struct text_data *text_free[MAX_SIZE_LIST] =
{
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL
};
static struct text_data *tdf_free = NULL;
#endif
char *str_dup( const char *str )
{
	char *str_new;
#ifdef STRING_FREELIST
	sh_int size;
	int len;
#endif

	if ( !str || !*str )
	return &str_empty[0];

	if ( str >= string_space && str < top_string )
	return (char *) str;

#ifdef STRING_FREELIST
	len = strlen(str)+1;
	for ( size = 0; size < MAX_SIZE_LIST; size++ )
	if ( len < sizelist[size] )
	break;
	if ( size < MAX_SIZE_LIST )
	{
		if ( text_free[size] != NULL )
		{
			struct text_data *tdf = text_free[size];

			text_free[size] = tdf->next;
			str_new = tdf->text;
			tdf->next = tdf_free;
			tdf_free = tdf;
		}
		else
		str_new = getmem( sizelist[size] );
	}
	else
	str_new = getmem( len );
#else
	str_new = getmem( strlen(str) + 1 );
#endif
	strcpy( str_new, str );
	return str_new;
}

/*
 * Free a string.
 * Null is legal here to simplify callers.
 * Read-only shared strings are not touched.
 */
void free_string( char *pstr )
{
#ifdef STRING_FREELIST
	sh_int size;
	int len;
#endif

	if ( pstr == NULL
			|| pstr == &str_empty[0]
			|| ( pstr >= string_space && pstr < top_string ) )
	return;

#ifdef STRING_FREELIST
	len = strlen(pstr)+1;
	for ( size = 0; size < MAX_SIZE_LIST; size++ )
	if ( len < sizelist[size] )
	break;
	if ( size < MAX_SIZE_LIST )
	{
		struct text_data *tdf;

		if ( tdf_free )
		{
			tdf = tdf_free;
			tdf_free = tdf->next;
		}
		else
		tdf = getmem(sizeof(*tdf));
		tdf->text = pstr;
		tdf->next = text_free[size];
		text_free[size] = tdf;
	}
	else
	dispose( pstr, len );
#else
	dispose( pstr, strlen(pstr)+1 );
#endif
	return;
}
#endif

void perm_update() {
	/* output perm usage to log file */
	FILE *po;
	char *strtime;
	po = fopen("perm.out", "a");

	strtime = ctime(&current_time);
	strtime[strlen(strtime) - 1] = '\0';

	fprintf(po, "%s :: Perms   %5d blocks  of %7d bytes.\n\r", strtime,
			nAllocPerm, sAllocPerm);
	fclose(po);
	return;
}

void do_memory(CHAR_DATA *ch, char *argument) {
	char buf[MAX_STRING_LENGTH];
	extern int obj_count;
	extern int active_building_count;
	extern int vehicle_count;
	int usage_now = get_user_seconds();

	if (!str_cmp(argument, "defrag")) {
		send_to_char("Defragmenting SSM heap.", ch);
		log_f("SSM: %s called defrag_heap.", ch->name);
		defrag_heap();
		return;
	}

	if (!str_cmp(argument, "log")) {
		if (get_trust(ch) < MAX_LEVEL) {
			send_to_char("Not at your level.\n\r", ch);
			return;
		}

		if (mem_log) {
			mem_log = FALSE;
			send_to_char("Memory logging is now OFF.\n\r", ch);
			log_f("%s turned off memory logging", ch->name);
			return;
		} else {
			mem_log = TRUE;
			send_to_char(
					"Memory logging is now ON.. remember to turn it off!\n\r",
					ch);
			log_f("%s turned on memory logging", ch->name);
			return;
		}
	}

    sprintf(buf, "ExDes      %5d\n\r", top_ed);
    send_to_char(buf, ch);
    sprintf(buf, "Helps      %5d\n\r", top_help);
    send_to_char(buf, ch);
    sprintf(buf, "Mobs       %5d\n\r", top_mob_index);
    send_to_char(buf, ch);
    sprintf(buf, "Objs       %5d\n\r", top_obj_index);
    send_to_char(buf, ch);
    sprintf(buf, "Objects:   %5d\n\r", obj_count);
    send_to_char(buf, ch);
    sprintf(buf, "Vehicles:  %5d\n\r", vehicle_count);
    send_to_char(buf, ch);
    sprintf(buf, "Buildings: %ld (%d active)\n\r", building_count,
            active_building_count);
    send_to_char(buf, ch);
    sprintf(buf, "Rooms:     %5d\n\r", top_room);
    send_to_char(buf, ch);
    sprintf(buf, "CPU Time:  %5d\n\r", usage_now);
    send_to_char(buf, ch);

#if 0
	sprintf( buf, "Strings %5d strings of %7d bytes (max %d).\n\r",
			nAllocString, sAllocString, MAX_STRING );
#endif

	sprintf(buf, "Shared String Info:\n\r");
	send_to_char(buf, ch);
	sprintf(buf, "Strings           %5ld strings of %7ld bytes (max %ld).\n\r",
			nAllocString, sAllocString, MAX_STRING);
	send_to_char(buf, ch);
	sprintf(buf, "Overflow Strings  %5ld strings of %7ld bytes.\n\r",
			nOverFlowString, sOverFlowString);
	send_to_char(buf, ch);
	if (Full) {
		send_to_char("Shared String Heap is full, increase MAX_STRING.\n\r",
				ch);
		sprintf(buf, "Overflow high-water-mark is %ld bytes.\n\r", hwOverFlow);
		send_to_char(buf, ch);
	}

	sprintf(buf, "Perms   %5d blocks  of %7d bytes.\n\r", nAllocPerm,
			sAllocPerm);
	send_to_char(buf, ch);
	return;
}

/*void do_status( CHAR_DATA *ch, char *argument )
 {
 char buf[MAX_STRING_LENGTH];

 send_to_char( "             Status Report for Ack! Mud:\n\r", ch );
 send_to_char( "             ---------------------------\n\r\n\r", ch );
 do_time( ch, "" );

 send_to_char( "\n\r", ch );
 send_to_char( "The following counts are for *distinct* mobs/objs/rooms, not a count\n\r" , ch);
 send_to_char( "of how many are actually in the game at this time.\n\r", ch );
 send_to_char( "NB. Areas count will include areas used as help files.\n\r\n\r", ch );
 sprintf( buf, "Helps   %5d\n\r", top_help      ); send_to_char( buf, ch );
 sprintf( buf, "Objs    %5d\n\r", top_obj_index ); send_to_char( buf, ch );
 sprintf( buf, "Rooms   %5d\n\r", top_room      ); send_to_char( buf, ch );

 return;
 }*/

/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy(int number) {
	switch (number_bits(2)) {
	case 0:
		number -= 1;
		break;
	case 3:
		number += 1;
		break;
	}

	return UMAX(1, number);
}

/*
 * Generate a random number.
 */
int number_range(int from, int to) {
	int power;
	int number;

	if ((to = to - from + 1) <= 1)
		return from;

	for (power = 2; power < to; power <<= 1)
		;

	while ((number = number_mm() & (power - 1)) >= to)
		;

	return from + number;
}

/*
 * Generate a percentile roll.
 */
int number_percent(void) {
	int percent;

	while ((percent = number_mm() & (128 - 1)) > 99)
		;

	return 1 + percent;
}

/*
 * Generate a random door.
 */
int number_door(void) {
	int door;

	while ((door = number_mm() & (8 - 1)) > 5)
		;

	return door;
}

int number_bits(int width) {
	return number_mm() & ((1 << width) - 1);
}

/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */
static int rgiState[2 + 55];

void init_mm() {
	int *piState;
	int iState;

	piState = &rgiState[2];

	piState[-2] = 55 - 55;
	piState[-1] = 55 - 24;

	piState[0] = ((int) current_time) & ((1 << 30) - 1);
	piState[1] = 1;
	for (iState = 2; iState < 55; iState++) {
		piState[iState] = (piState[iState - 1] + piState[iState - 2])
				& ((1 << 30) - 1);
	}
	return;
}

int number_mm(void) {
	int *piState;
	int iState1;
	int iState2;
	int iRand;

	piState = &rgiState[2];
	iState1 = piState[-2];
	iState2 = piState[-1];
	iRand = (piState[iState1] + piState[iState2]) & ((1 << 30) - 1);
	piState[iState1] = iRand;
	if (++iState1 == 55)
		iState1 = 0;
	if (++iState2 == 55)
		iState2 = 0;
	piState[-2] = iState1;
	piState[-1] = iState2;
	return iRand >> 6;
}

/*
 * Roll some dice.
 */
int dice(int number, int size) {
	int idice;
	int sum;

	switch (size) {
	case 0:
		return 0;
	case 1:
		return number;
	}

	for (idice = 0, sum = 0; idice < number; idice++)
		sum += number_range(1, size);

	return sum;
}

/*
 * Simple linear interpolation.
 */
int interpolate(int level, int value_00, int value_32) {
	return value_00 + level * (value_32 - value_00) / 32;
}

/*
 * Append a string to a file.
 */
void append_file(CHAR_DATA *ch, char *file, char *str) {
	FILE *fp;

	if (str[0] == '\0')
		return;

	fclose(fpReserve);
	if ((fp = fopen(file, "a")) == NULL) {
		perror(file);
		send_to_char("Could not open the file!\n\r", ch);
	} else {
		fprintf(fp, "%15s: %s\n", ch->name, str);
		fclose(fp);
	}

	fpReserve = fopen(NULL_FILE, "r");
	return;
}

/*
 * Nice little functions that limit the amount of typing you have to do with
 * each and every log File entry and bug report.---Flar
 */
void bugf(char * fmt, ...) {
	char buf[MSL] = "\0";
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	bug(buf, 0);
}

void log_f(char * fmt, ...) {
	char buf[2 * MSL];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	log_string(buf);
}

/*
 * Reports a bug.
 */
void bug(const char *str, int param) {
	char buf[MAX_STRING_LENGTH];
	FILE *fp;

	if (fpArea != NULL) {
		int iLine;
		int iChar;

		if (fpArea == stdin) {
			iLine = 0;
		} else {
			iChar = ftell(fpArea);
			fseek(fpArea, 0, 0);
			for (iLine = 0; ftell(fpArea) < iChar; iLine++) {
				while (getc(fpArea) != '\n')
					;
			}
			fseek(fpArea, iChar, 0);
		}

		sprintf(buf, "[*****] FILE: %s LINE: %d", strArea, iLine);
		log_string(buf);

		/*	if ( ( fp = fopen( SHUTDOWN_FILE, "a" ) ) != NULL )
		 {
		 fprintf( fp, "[*****] %s\n", buf );
		 fclose( fp );
		 } */
	}

	strcpy(buf, "[*****] BUG: ");
	sprintf(buf + strlen(buf), str, param);
	log_string(buf);

	fclose(fpReserve);
	if ((fp = fopen( BUG_FILE, "a")) != NULL) {
		fprintf(fp, "%s\n", buf);
		fclose(fp);
	}
	fpReserve = fopen(NULL_FILE, "r");

	return;
}

void bug_string(const char *str, const char *str2) {
	char buf[MAX_STRING_LENGTH];
	FILE *fp;

	if (fpArea != NULL) {
		int iLine;
		int iChar;

		if (fpArea == stdin) {
			iLine = 0;
		} else {
			iChar = ftell(fpArea);
			fseek(fpArea, 0, 0);
			for (iLine = 0; ftell(fpArea) < iChar; iLine++) {
				while (getc(fpArea) != '\n')
					;
			}
			fseek(fpArea, iChar, 0);
		}

		sprintf(buf, "[*****] FILE: %s LINE: %d", strArea, iLine);
		log_string(buf);

		/*	if ( ( fp = fopen( SHUTDOWN_FILE, "a" ) ) != NULL )
		 {
		 fprintf( fp, "[*****] %s\n", buf );
		 fclose( fp );
		 } */
	}

	strcpy(buf, "[*****] BUG: ");
	sprintf(buf + strlen(buf), str, str2);
	log_string(buf);

	fclose(fpReserve);
	if ((fp = fopen( BUG_FILE, "a")) != NULL) {
		fprintf(fp, "%s\n", buf);
		fclose(fp);
	}
	fpReserve = fopen(NULL_FILE, "r");

	return;
}

/*
 * Writes a string to the log.
 */
void log_string(const char *str) {
	char *strtime;

	strtime = ctime(&current_time);
	strtime[strlen(strtime) - 1] = '\0';
	fprintf( stderr, "%s :: %s\n", strtime, str);
	return;
}

/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain(void) {
	return;
}

void insert_area(AREA_DATA *pArea) {
	AREA_DATA *aTemp;

	for (aTemp = first_area; aTemp; aTemp = aTemp->next) {
		if (pArea->min_vnum > aTemp->max_vnum) {
			if (aTemp->next != NULL) {
				if (pArea->max_vnum < aTemp->next->min_vnum) {
					LINK_AFTER(pArea, aTemp, first_area, last_area, next, prev);
					return;
				}
			} else {
				LINK_AFTER(pArea, aTemp, first_area, last_area, next, prev);
				return;
			}
		} else if (pArea->max_vnum < aTemp->min_vnum) {
			LINK_BEFORE(pArea, aTemp, first_area, last_area, next, prev);
			return;
		}
	}
	LINK(pArea, first_area, last_area, next, prev);
}

void load_sobjects(int mode) {

	FILE *objectfp;
	char object_file_name[MAX_STRING_LENGTH] = {0};
	char buf[MAX_STRING_LENGTH];
	int i = 0;

	snprintf(object_file_name, sizeof(object_file_name), "%s", OBJECTS_FILE);
	if (mode == 3)
		snprintf(object_file_name, sizeof(object_file_name), "%s", OBJECTS_BACKUP_FILE);
	else if (mode == 4)
	snprintf(object_file_name, sizeof(object_file_name), OBJECTS_FEST_FILE);

	snprintf(buf, sizeof(buf), "Loading %s\n\r", object_file_name);
	log_string( buf );;

	if ((objectfp = fopen(object_file_name, "r")) == NULL) {
		log_f("Load object Table: fopen of %s", object_file_name);
		perror("failed open of objects.txt in load_sobject");
	}

	else {
		fpArea = objectfp;
		sprintf(strArea, "%s", object_file_name);

		for (;;) {
			char letter;
			i++;
			if (i > 100000)
				break;
			letter = fread_letter(objectfp);
			if (letter == 'O') {
				fread_object(objectfp);
				continue;
			}

			if (letter != '#') {
				break;
			}
		}
	}
	fclose(objectfp);
	fpArea = NULL;

}

void read_map_from_file(void) {
	FILE *objectfp;
	char object_file_name[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int i, j, v, z;

	sprintf(object_file_name, "%s", MAP_FILE);

	sprintf(buf, "Loading %s\n\r", object_file_name);
	log_string( buf );;

	v = 0;
	if ((objectfp = fopen(object_file_name, "r")) == NULL) {
		log_f("Load object Table: fopen");
		perror("failed open of map.txt in read_map_from_file");
	}

	else {
		fpArea = objectfp;
		sprintf(strArea, "%s", object_file_name);

		for (z = 0; z < Z_MAX; z++) {
			if (z == Z_AIR) {
				for (i = 0; i < MAX_MAPS; i++) {
					for (j = 0; j < MAX_MAPS; j++) {
						map_table.resource[i][j][z] = -1;
						map_table.type[i][j][z] = SECT_AIR;
					}
				}
				continue;
			} else if (planet_table[z].system == 0) {
				for (i = 0; i < MAX_MAPS; i++) {
					for (j = 0; j < MAX_MAPS; j++) {
						map_table.resource[i][j][z] = -1;
					}
				}
				continue;
			}
			for (i = 0; i < MAX_MAPS; i++) {
				for (j = 0; j < MAX_MAPS; j++) {
					if (v != -1)
						v = fread_number(objectfp);
					map_table.type[i][j][z] = v;
					map_table.resource[i][j][z] = -1;
					if (z == Z_GROUND) {
						if (j < BORDER_SIZE || i < BORDER_SIZE
								|| i > MAX_MAPS - BORDER_SIZE
								|| j > MAX_MAPS - BORDER_SIZE) {
							map_table.type[i][j][Z_UNDERGROUND] = SECT_NULL;
							map_table.type[i][j][Z_PAINTBALL] = SECT_NULL;
						}
					}
				}
			}
		}

	}
	fclose(objectfp);
	fpArea = NULL;
}

void load_alliances(void) {
	FILE *objectfp;
	char object_file_name[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int i;

	sprintf(object_file_name, "%s", ALLIANCES_FILE);

	sprintf(buf, "Loading %s\n\r", object_file_name);
	log_string( buf );;

	if ((objectfp = fopen(object_file_name, "r")) == NULL) {
		log_f("Load object Table: fopen");
		perror("failed open of alliances.txt in load_alliances");
	}

	else {
		fpArea = objectfp;
		sprintf(strArea, "%s", object_file_name);

		for (i = 0; i < MAX_ALLIANCE; i++) {
			if (fread_letter(objectfp) == '#')
				break;

			if (alliance_table[i].name != NULL)
				free_string(alliance_table[i].name);
			if (alliance_table[i].leader != NULL)
				free_string(alliance_table[i].leader);
			if (alliance_table[i].history != NULL)
				free_string(alliance_table[i].history);

			alliance_table[i].name = fread_string(objectfp);
			alliance_table[i].leader = fread_string(objectfp);
			alliance_table[i].members = fread_number(objectfp);
			alliance_table[i].kills = fread_number(objectfp);
			alliance_table[i].history = str_dup("");
		}
	}
	fclose(objectfp);
	fpArea = NULL;
}

BUILDING_DATA *create_building(int type) {
	static BUILDING_DATA bld_zero;
	BUILDING_DATA *bld;

	if (type < 0 || type > MAX_BUILDING) {
		return NULL;
	}
	building_count++;

	GET_FREE(bld, building_free);
	*bld = bld_zero;

	bld->x = 0;
	bld->y = 0;
	bld->z = 1;
	bld->next = NULL;
	bld->prev = NULL;
	bld->type = type;
	bld->name = str_dup(build_table[type].name);
	bld->exit[0] = FALSE;
	bld->exit[1] = FALSE;
	bld->exit[2] = FALSE;
	bld->exit[3] = FALSE;
	bld->maxhp = build_table[type].hp;
	bld->hp = bld->maxhp;
	bld->maxshield = build_table[type].shield;
	bld->shield = bld->maxshield;
	bld->value[0] = 0;
	bld->value[1] = 0;
	bld->value[2] = 0;
	bld->value[3] = 0;
	bld->value[4] = 0;
	bld->value[5] = 0;
	bld->value[6] = 0;
	bld->value[7] = 0;
	bld->value[8] = 0;
	bld->value[9] = 0;
	bld->value[10] = 0;
	bld->resources[0] = build_table[type].resources[0];
	bld->resources[1] = build_table[type].resources[1];
	bld->resources[2] = build_table[type].resources[2];
	bld->resources[3] = build_table[type].resources[3];
	bld->resources[4] = build_table[type].resources[4];
	bld->resources[5] = build_table[type].resources[5];
	bld->resources[6] = build_table[type].resources[6];
	bld->resources[7] = build_table[type].resources[7];
	bld->owned = str_dup("Amnon");
	bld->owner = NULL;
	bld->level = 1;
	bld->visible = TRUE;
	bld->attacker = str_dup("None");
	bld->protection = 0;
	bld->active = FALSE;
	//    bld->next_active = NULL;
	//    bld->prev_active = NULL;
	bld->next_owned = NULL;
	bld->prev_owned = NULL;
	bld->directories = 2;
	bld->real_dir = number_range(1, 2);
	bld->password = number_range(10000, 99999);
	bld->timer = 0;
	if (is_neutral(bld->type) || !str_cmp(bld->owned, "Nobody"))
		activate_building(bld, TRUE);

	LINK(bld, first_building, last_building, next, prev);
	return bld;
}

void load_vehicles(int mode) {
	CHAR_DATA *ch;
	FILE *fp;
	char object_file_name[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char *driver;
	VEHICLE_DATA *vhc;
	int i, x = 0;

	if (mode == 1)
		sprintf(object_file_name, "%s", VEHICLE_BACKUP_FILE);
	else if (mode == 2)
		sprintf(object_file_name, "%s", VEHICLE_FEST_FILE);
	else
		sprintf(object_file_name, "%s", VEHICLE_FILE);

	sprintf(buf, "Loading %s\n\r", object_file_name);
	log_string( buf );;

	if ((fp = fopen(object_file_name, "r")) == NULL) {
		log_f("Load Buildings Table: fopen");
		perror("failed open vehicles.txt in load_vehicles");
	} else {
		fpArea = fp;
		sprintf(strArea, "%s", object_file_name);

		for (;;) {
			x++;
			if (x >= 10000)
				break;
			if (fread_letter(fpArea) == '#')
				break;

			GET_FREE(vhc, vehicle_free);
			vhc->type = fread_number(fp);
			vhc->name = fread_string(fp);
			vhc->desc = fread_string(fp);
			vhc->x = fread_number(fp);
			vhc->y = fread_number(fp);
			vhc->z = fread_number(fp);
			vhc->hit = fread_number(fp);
			vhc->max_hit = fread_number(fp);
			vhc->ammo_type = fread_number(fp);
			vhc->ammo = fread_number(fp);
			vhc->max_ammo = fread_number(fp);
			vhc->fuel = fread_number(fp);
			vhc->max_fuel = fread_number(fp);
			vhc->flags = fread_number(fp);
			vhc->speed = fread_number(fp);
			vhc->range = fread_number(fp);
			vhc->scanner = fread_number(fp);
			vhc->timer = 0;
			driver = fread_string(fp);
			if ((ch = get_ch(driver)) != NULL && ch->in_vehicle == NULL) {
				vhc->x = ch->x;
				vhc->y = ch->y;
				vhc->z = ch->z;
				ch->in_vehicle = vhc;
				vhc->driving = ch;
			}
			for (i = 0; i < POWER_MAX; i++)
				vhc->power[i] = fread_number(fp);
			vhc->power[POWER_REPAIR] = 0;
			move_vehicle(vhc, vhc->x, vhc->y, vhc->z);
			LINK(vhc, first_vehicle, last_vehicle, next, prev);
			free_string(driver);
		}

	}
	fclose(fp);
	fpArea = NULL;
}

void load_buildings( void )
{
    FILE *fp;
    char object_file_name[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int i,x=0;
    BUILDING_DATA *bld;
    building_count = 0;
    sprintf( object_file_name, "%s", BUILDING_FILE );

    sprintf( buf, "Loading %s\n\r", object_file_name);
    log_string( buf );;

    if ( ( fp = fopen( object_file_name, "r" ) ) == NULL )
    {
        log_f( "Load Buildings Table: fopen" );
        perror( "failed open of buildings.txt in load_buildings" );
    }
    else
    {
        fpArea = fp;
        sprintf( strArea, "%s", object_file_name );

        for ( ; ; )
        {
            x++;
            if ( x >= 100000 )
                break;
            if ( fread_letter( fpArea ) == '#' )
            {
                /*		char *word;
                        word = fread_word( fp );
                        if ( !str_cmp(word,"revision") )
                        {
                            revision = fread_number(fp);
                        } */
                break;
            }
            building_count++;
            GET_FREE(bld, building_free);
            bld->type = fread_number(fp);
            bld->name = fread_string(fp);
            for ( i = 0; i<4; i++ )
                bld->exit[i] = fread_number(fp);
            bld->maxhp = fread_number(fp);
            bld->hp = fread_number(fp);
            bld->maxshield = fread_number(fp);
            bld->shield = fread_number(fp);
            for ( i = 0; i<11; i++ )
                bld->value[i] = fread_number(fp);
            for ( i = 0; i<8; i++ )
                bld->resources[i] = fread_number(fp);
            bld->owned = fread_string(fp);
            bld->owner = NULL;
            bld->x = fread_number(fp);
            bld->y = fread_number(fp);
            bld->z = fread_number(fp);
            bld->level = fread_number(fp);
            bld->visible = fread_number(fp);
            bld->directories = fread_number(fp);
            bld->real_dir = fread_number(fp);
            bld->password = fread_number(fp);
            bld->attacker = fread_string(fp);
            if ( bld->attacker == NULL )
                bld->attacker = str_dup("None");
            LINK(bld, first_building, last_building, next, prev);
            if ( bld->x >= 0 && bld->y >= 0 )
                map_bld[bld->x][bld->y][bld->z] = bld;
            if ( bld->directories == 0 )
            {
                bld->directories = 2;
                bld->real_dir = number_range(1,2);
                bld->password = number_range(10000,99999);
            }
            bld->value[8] = 0;
            bld->timer = 0;
            if ( bld->value[0] < 0 )
                bld->value[0] = -1;
            if ( bld->type == BUILDING_ARMORY && bld->value[0] == -1 )
                bld->value[0] = 0;
            if ( bld->z == Z_PAINTBALL || is_evil(bld) )
                activate_building(bld,TRUE);
        }

    }
    fclose( fp );
    fpArea = NULL;
}

void load_buildings_b( int mode )
{
    FILE *fp;
    char object_file_name[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int i;
    BUILDING_DATA *bld;
    building_count = 0;

    if ( mode == 0 )
        sprintf( object_file_name, "%s", BUILDING_BACKUP_FILE );
    else
        sprintf( object_file_name, "%s", BUILDING_FEST_FILE );

    sprintf( buf, "Loading %s\n\r", object_file_name);
    log_string( buf );

    if ( ( fp = fopen( object_file_name, "r" ) ) == NULL )
    {
        log_f( "Load Buildings Table: fopen" );
        perror( "failed open of buildings.bak in load_buildings_b" );
    }
    else
    {
        fpArea = fp;
        sprintf( strArea, "%s", object_file_name );

        for ( ; ; )
        {
            if ( fread_letter( fpArea ) == '#' )
            {
                break;
            }
            building_count++;
            GET_FREE(bld, building_free);
            bld->type = fread_number(fp);
            bld->name = fread_string(fp);
            for ( i = 0; i<4; i++ )
                bld->exit[i] = fread_number(fp);
            bld->maxhp = fread_number(fp);
            bld->hp = fread_number(fp);
            bld->maxshield = fread_number(fp);
            bld->shield = fread_number(fp);
            for ( i = 0; i<11; i++ )
                bld->value[i] = fread_number(fp);
            for ( i = 0; i<8; i++ )
                bld->resources[i] = fread_number(fp);
            bld->owned = fread_string(fp);
            bld->owner = NULL;
            bld->x = fread_number(fp);
            bld->y = fread_number(fp);
            bld->z = fread_number(fp);
            bld->level = fread_number(fp);
            bld->visible = fread_number(fp);
            bld->directories = fread_number(fp);
            bld->real_dir = fread_number(fp);
            bld->password = fread_number(fp);
            bld->attacker = fread_string(fp);
            if ( bld->attacker == NULL )
                bld->attacker = str_dup("None");
            LINK(bld, first_building, last_building, next, prev);
            if ( bld->x >= 0 && bld->y >= 0 )
                map_bld[bld->x][bld->y][bld->z] = bld;
            if ( bld->directories == 0 )
            {
                bld->directories = 2;
                bld->real_dir = number_range(1,2);
                bld->password = number_range(10000,99999);
            }
            bld->value[8] = 0;
            if ( (!str_infix(bld->name,"mine")) && bld->value[0] == 0 )
                bld->value[0] = -1;
            if ( bld->type == BUILDING_ARMORY && bld->value[0] == -1 )
                bld->value[0] = 0;
            if ( bld->z == Z_PAINTBALL || is_neutral(bld->type) || !str_cmp(bld->owned,"Nobody") )
                activate_building(bld,TRUE);
        }

    }
    fclose( fp );
    fpArea = NULL;
}

void load_multiplay(void) {
	FILE *fp;
	char object_file_name[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int i = 0;

	sprintf(object_file_name, "%s", MULTIPLAY_FILE);

	sprintf(buf, "Loading %s\n\r", object_file_name);
	log_string( buf );;

	if ((fp = fopen(object_file_name, "r")) == NULL) {
		log_f("Load Multiplay Table: fopen");
		perror("failed open of multiplay.txt in load_multiplay");
	} else {
		fpArea = fp;
		sprintf(strArea, "%s", object_file_name);

		for (i = 0; i < 30; i++) {
			if (multiplay_table[i].name != NULL)
				free_string(multiplay_table[i].name);
			if (multiplay_table[i].host != NULL)
				free_string(multiplay_table[i].host);
			multiplay_table[i].name = NULL;
			multiplay_table[i].host = NULL;
		}
		for (i = 0; i < 30; i++) {
			if (fread_letter(fpArea) == '#')
				break;

			multiplay_table[i].name = fread_string(fp);
			multiplay_table[i].host = fread_string(fp);
			if (!str_cmp(multiplay_table[i].name, "(null)")) {
				free_string(multiplay_table[i].name);
				multiplay_table[i].name = NULL;
			}
			if (!str_cmp(multiplay_table[i].host, "(null)")) {
				free_string(multiplay_table[i].host);
				multiplay_table[i].host = NULL;
			}
		}
	}
	fclose(fp);
	fpArea = NULL;
	return;
}

void load_scores(void) {
	FILE *fp;
	char object_file_name[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int i = 0;

	sprintf(object_file_name, "%s", SCORE_FILE);

	sprintf(buf, "Loading %s\n\r", object_file_name);
	log_string( buf );;

	if ((fp = fopen(object_file_name, "r")) == NULL) {
		log_f("Load scores Table: fopen");
		perror("failed open of scores.txt in load_scores");
	} else {
		fpArea = fp;
		sprintf(strArea, "%s", object_file_name);

		for (i = 0; i < 100; i++) {
			if (fread_letter(fpArea) == '#')
				break;

			if (score_table[i].name != NULL)
				free_string(score_table[i].name);
			if (score_table[i].killedby != NULL)
				free_string(score_table[i].killedby);
			score_table[i].name = fread_string(fp);
			score_table[i].killedby = fread_string(fp);
			score_table[i].kills = fread_number(fp);
			score_table[i].buildings = fread_number(fp);
			score_table[i].time = fread_number(fp);
		}
	}
	fclose(fp);
	fpArea = NULL;

	if ((fp = fopen( MAX_PLAYERS_FILE, "r")) == NULL) {
		log_f("Loading max players: fopen");
		perror("failed open of players.txt in load_scores");
	} else {
		extern int max_players_ever;
		fpArea = fp;
		sprintf(strArea, "%s", MAX_PLAYERS_FILE);
		max_players_ever = fread_number(fp);
	}
	fclose(fp);
	fpArea = NULL;
	return;
}

void load_ranks(void) {
	FILE *fp;
	char object_file_name[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int i = 0;

	sprintf(object_file_name, "%s", RANK_FILE);

	sprintf(buf, "Loading %s\n\r", object_file_name);
	log_string( buf );;

	if ((fp = fopen(object_file_name, "r")) == NULL) {
		log_f("Load scores Table: fopen");
		perror("failed open of ranks.txt in load_ranks");
	} else {
		fpArea = fp;
		sprintf(strArea, "%s", object_file_name);

		for (i = 0; i < 100; i++) {
			if (fread_letter(fpArea) == '#')
				break;

			if (rank_table[i].name != NULL)
				free_string(rank_table[i].name);
			rank_table[i].name = fread_string(fp);
			rank_table[i].rank = fread_number(fp);
		}
	}
	fclose(fp);
	fpArea = NULL;
}

VEHICLE_DATA *create_vehicle(int type) {
	int i;
	static VEHICLE_DATA vhc_zero;
	VEHICLE_DATA *vhc;

	if (type >= MAX_VEHICLE) {
		bug("Create_vehicle: type larger than max_vehicle", MAX_STRING);
		return NULL;
	}
	GET_FREE(vhc, vehicle_free);
	*vhc = vhc_zero;

	vhc->x = 0;
	vhc->y = 0;
	vhc->z = 1;
	vhc->next = NULL;
	vhc->prev = NULL;
	vhc->next_in_room = NULL;
	vhc->type = type;
	vhc->name = str_dup(vehicle_name[type]);
	vhc->desc = str_dup(vehicle_desc[type]);
	vhc->max_hit = 0;
	vhc->hit = 0;
	vhc->speed = 0;
	vhc->fuel = 0;
	vhc->max_fuel = 0;
	vhc->ammo_type = 0;
	vhc->ammo = 0;
	vhc->max_ammo = 0;
	vhc->flags = 0;
	vhc->driving = NULL;
	vhc->in_building = NULL;
	vhc->in_vehicle = NULL;
	vhc->vehicle_in = NULL;
	vhc->state = VEHICLE_STATE_NORMAL;
	vhc->power[0] = 0;
	for (i = 1; i < POWER_MAX; i++)
		vhc->power[i] = 100;
	vhc->power[POWER_REPAIR] = 0;

	LINK(vhc, first_vehicle, last_vehicle, next, prev);
	return vhc;
}

/*
 * Snarf a mob section.
 */
void load_mobiles( FILE *fp )
{
    MOB_INDEX_DATA *pMobIndex;
    BUILD_DATA_LIST *pList;

    for ( ; ; )
    {
	sh_int vnum;
	char letter;
	int iHash;

	letter                          = fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_mobiles: # not found.", 0 );
	    exit( 1 );
	}

	vnum                            = fread_number( fp );
	if ( vnum == 0 )
	    break;

	fBootDb = FALSE;
	if ( get_mob_index( vnum ) != NULL )
	{
	    bug( "Load_mobiles: vnum %d duplicated.", vnum );
	    exit( 1 );
	}
	fBootDb = TRUE;

	GET_FREE(pMobIndex, mid_free);
    pMobIndex->vnum                 = vnum;
    pMobIndex->area                 = area_load;
    pMobIndex->player_name          = fread_string( fp );
    pMobIndex->short_descr          = fread_string( fp );
    pMobIndex->long_descr           = fread_string( fp );
    pMobIndex->description          = fread_string( fp );
    
    pMobIndex->long_descr[0]        = UPPER(pMobIndex->long_descr[0]);
	pMobIndex->description[0]       = UPPER(pMobIndex->description[0]);

    pMobIndex->act                  = fread_number( fp );;
    pMobIndex->level                = fread_number( fp );;
    pMobIndex->sex                  = fread_number( fp );;
    
    letter                          = fread_letter( fp );

    // Should signal the end of the mob
	if ( letter != 'S' )
	{
	    bug( "Load_mobiles: vnum %d non-S.", vnum );
	    exit( 1 );
	}
	
	iHash                   = vnum % MAX_KEY_HASH;
	SING_TOPLINK(pMobIndex, mob_index_hash[iHash], next);
/* MAG Mod */
	GET_FREE(pList, build_free);
	pList->data     = pMobIndex;
	LINK(pList, area_load->first_area_mobile, area_load->last_area_mobile,
	     next, prev);

	top_mob_index++;
	kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;
    }

    return;
}

void load_objects(FILE *fp) {
	OBJ_INDEX_DATA *pObjIndex;
	BUILD_DATA_LIST *pList;
	sh_int looper;

	for (;;) {
		sh_int vnum;
		char letter;
		int iHash;

		letter = fread_letter(fp);
		if (letter != '#') {
			bug("Load_objects: # not found.", 0);
			hang("Loading Objects in db.c");
		}

		vnum = fread_number(fp);
		if (vnum == 0)
			break;

		fBootDb = FALSE;
		if (get_obj_index(vnum) != NULL) {
			bug("Load_objects: vnum %d duplicated.", vnum);
			hang("Loading Objects in db.c");
		}
		fBootDb = TRUE;

		GET_FREE(pObjIndex, oid_free);
		pObjIndex->vnum = vnum;
		pObjIndex->name = fread_string(fp);
		pObjIndex->short_descr = fread_string(fp);
		pObjIndex->description = fread_string(fp);

		pObjIndex->short_descr[0] = LOWER(pObjIndex->short_descr[0]);
		pObjIndex->description[0] = UPPER(pObjIndex->description[0]);

		pObjIndex->item_type = fread_number(fp);
		pObjIndex->extra_flags = fread_number(fp);
		if (area_revision < 15) {
			int temp_flags, index, new_flags = 0;
			temp_flags = fread_number(fp);
			for (index = 0; index < 32; index++) {
				if (IS_SET(temp_flags, (1 << index))) {
					SET_BIT(new_flags, convert_wearflags[index]);
				}
				pObjIndex->wear_flags = new_flags;
			}
		} else {
			pObjIndex->wear_flags = fread_number(fp);
		}
		for (looper = 0; looper < MAX_OBJECT_VALUES; pObjIndex->value[looper] =
				fread_number(fp), looper++)
			;

		if (IS_SET(pObjIndex->extra_flags, ITEM_RARE)
				&& pObjIndex->value[5] == 0)
			pObjIndex->value[5] = 7;

		pObjIndex->weight = fread_number(fp);
		pObjIndex->heat = fread_number(fp);
		pObjIndex->building = fread_number(fp);
		if (area_revision > 19)
			pObjIndex->image = fread_string(fp);
		if (!str_cmp(pObjIndex->image, "(null)")) {
			//		pObjIndex->image = NULL;
			free_string(pObjIndex->image);
			pObjIndex->image = NULL;
		}

		for (;;) {
			char letter;

			letter = fread_letter(fp);

			if (letter == 'L') {
				pObjIndex->level = fread_number(fp);
			}

			else {
				ungetc(letter, fp);
				break;
			}
		}
		iHash = vnum % MAX_KEY_HASH;
		SING_TOPLINK(pObjIndex, obj_index_hash[iHash], next);
		/* MAG Mod */
		GET_FREE(pList, build_free);
		pList->data = pObjIndex;
		LINK(pList, area_load->first_area_object, area_load->last_area_object,
				next, prev);

		top_obj_index++;
	}

	return;
}

void create_load_list() {
	OBJ_INDEX_DATA *pObj;
	int i, lev, r;
	int j[MAX_BUILDING_LEVEL + 1];
	int k[MAX_BUILDING_TYPES];
	int act = 0;
	extern int buildings_lists[MAX_BUILDING_TYPES][MAX_POSSIBLE_BUILDING];

	for (i = 0; i <= MAX_BUILDING_LEVEL; i++)
		j[i] = 1;

	for (i = MIN_LOAD_OBJ; i <= MAX_LOAD_OBJ; i++) {
		if ((pObj = get_obj_index(i)) == NULL)
			continue;
		r = pObj->level;
		if (r < 20)
			lev = 1;
		else if (r < 40)
			lev = 2;
		else if (r < 60)
			lev = 3;
		else if (r < 80)
			lev = 4;
		else
			lev = 5;
		load_list[lev][j[lev]].vnum = i;
		load_list[lev][j[lev]].rarity = r;
		load_list[lev][j[lev]].building = pObj->building;
		j[lev]++;
	}
	for (; i < MAX_BUILDING_LEVEL + 1; i++) {
		load_list[i][j[i]].vnum = -1;
		load_list[i][j[i]].rarity = -1;
		load_list[i][j[i]].building = -1;
	}
	for (i = 0; i < MAX_BUILDING_TYPES; i++)
		k[i] = 0;
	for (i = 1; i < MAX_BUILDING; i++) {
		act = build_table[i].act;
		buildings_lists[act][k[act]] = i;
		k[act]++;
	}
	for (i = 0; i < MAX_BUILDING_TYPES; i++)
		for (act = k[i]; act < MAX_BUILDING; act++)
			buildings_lists[i][act] = -1;
	return;
}

void load_building_t(void) {
	FILE *fp;
	char object_file_name[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int i = 0, j;
	int cur_revision;

	sprintf(object_file_name, "%s", BUILDING_TABLE_FILE);

	sprintf(buf, "Loading %s\n\r", object_file_name);
	log_string( buf );;

	if ((fp = fopen(object_file_name, "r")) == NULL) {
		log_f("Load Building Table: fopen");
		perror("failed open of building_table.txt in load_building_t");
	} else {
		fpArea = fp;
		sprintf(strArea, "%s", object_file_name);

		cur_revision = fread_number(fp);
		for (i = 0; i < MAX_POSSIBLE_BUILDING; i++) {
			if (fread_letter(fpArea) == '#')
				break;

			if (build_table[i].name != NULL)
				free_string(build_table[i].name);
			if (build_table[i].desc != NULL)
				free_string(build_table[i].desc);
			if (build_table[i].symbol != NULL)
				free_string(build_table[i].symbol);
			if (build_help_table[i].help != NULL)
				free_string(build_help_table[i].help);

			build_table[i].type = i;
			build_table[i].name = fread_string(fp);
			build_table[i].hp = fread_number(fp);
			build_table[i].shield = fread_number(fp);
			for (j = 0; j < 8; j++)
				build_table[i].resources[j] = fread_number(fp);
			build_table[i].requirements = fread_number(fp);
			build_table[i].requirements_l = fread_number(fp);
			build_table[i].desc = fread_string(fp);
			build_table[i].symbol = fread_string(fp);
			for (j = 0; j < MAX_BUILDON; j++)
				build_table[i].buildon[j] = fread_number(fp);
			build_table[i].militairy = fread_number(fp);
			build_table[i].rank = fread_number(fp);
			build_table[i].act = fread_number(fp);
			build_table[i].max = fread_number(fp);
			build_table[i].disabled = fread_number(fp);
			build_help_table[i].help = fread_string(fp);
		}
	}
	MAX_BUILDING = i;
	sprintf(buf, "Buildings: %d", MAX_BUILDING);
	log_string( buf );;
	fclose(fp);
	fpArea = NULL;
	return;
}

void load_web_data() {
	FILE * fp;

	if ((fp = fopen( WEB_DATA_FILE, "r")) != NULL) {
		fpArea = fp;
		if (web_data.last_killed_in_pit != NULL)
			free_string(web_data.last_killed_in_pit);
		if (web_data.last_kills_in_pit != NULL)
			free_string(web_data.last_kills_in_pit);
		if (web_data.highest_ranking_player != NULL)
			free_string(web_data.highest_ranking_player);
		if (web_data.newest_player != NULL)
			free_string(web_data.newest_player);
		if (!feof(fp))
			return;
		web_data.last_killed_in_pit = fread_string(fp);
		web_data.last_kills_in_pit = fread_string(fp);
		web_data.highest_ranking_player = fread_string(fp);
		web_data.newest_player = fread_string(fp);
		web_data.highest_rank = fread_number(fp);
		web_data.tot_players = fread_number(fp);
		web_data.num_players = 0;
	}
	fclose(fp);
	fpArea = NULL;
	return;
}

void reward_votes() {
	FILE * fp;
	bool ok = TRUE;
	CHAR_DATA *ch;
	char *buf;

	if ((fp = fopen("../data/votes.txt", "r")) != NULL) {
		fpArea = fp;
		sprintf(strArea, "../data/votes.txt");
		while (ok) {
			if (getchar() == EOF)
				break;
			buf = fread_string(fp);
			for (ch = first_char; ch; ch = ch->next) {
				if (!str_cmp(ch->pcdata->host, buf)) {
					send_to_char("Thank you for voting.\n\r", ch);
					continue;
				}
			}
		}
	}
	fclose(fp);
	fpArea = NULL;
	return;
}
