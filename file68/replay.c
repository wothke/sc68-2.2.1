#include <config68.h>

#include <stdlib.h>
#include <string.h>

#include "replay.h"

/*
* Replay module fetching based on inlined data. Used to ease JavaScript
* compilation (i.e. remove file system dependency): Later on this could be
* replaced by some JavaScript-side load on demand.
*/

typedef struct {
    int size;
    void **keys;
    void **values;
    unsigned int*lengths;
} Hash;

Hash * EMSCRIPTEN_KEEPALIVE hash_new (int size) {
    Hash *hash = calloc(1, sizeof (Hash));
    hash->size = size;
    hash->keys = calloc(size, sizeof (void *));
    hash->values = calloc(size, sizeof (void *));
    hash->lengths = calloc(size, sizeof (unsigned int));

    return hash;
}

int EMSCRIPTEN_KEEPALIVE hashKey (char *key) {
    int h= 0;
    int i= 0;
    while (key[i] != 0) {
        h ^= key[i++];
    }
    return h;
}

int EMSCRIPTEN_KEEPALIVE hash_index (Hash *hash, char *key) {
    int index = hashKey(key) % hash->size;
    while (hash->keys[index] && (strcmp(hash->keys[index], key) != 0))
        index = (index + 1) % hash->size;
    return index;
}

void EMSCRIPTEN_KEEPALIVE hash_insert (Hash *hash, void *key, unsigned char *value, unsigned int len) {
    int index = hash_index(hash, key);
    hash->keys[index] = key;
    hash->values[index] = (void*)value;
    hash->lengths[index] = len;
}

char * EMSCRIPTEN_KEEPALIVE hash_lookup (Hash *hash, char *key) {
    int index = hash_index(hash, key);
    return (char*)hash->values[index];
}


 unsigned int EMSCRIPTEN_KEEPALIVE hash_lookup_len (Hash *hash, char *key) {
    int index = hash_index(hash, key);
    return hash->lengths[index];
}

Hash *playerBase;

void EMSCRIPTEN_KEEPALIVE register_players() {
    playerBase = hash_new(84*2);
    hash_insert(playerBase, "aenigmatica", aenigmatica_bin, aenigmatica_bin_len);
    hash_insert(playerBase, "alteredbeast", alteredbeast_bin, alteredbeast_bin_len);
    hash_insert(playerBase, "armalyte", armalyte_bin, armalyte_bin_len);
    hash_insert(playerBase, "bendaglish_deli", bendaglish_deli_bin, bendaglish_deli_bin_len);
    hash_insert(playerBase, "big", big_bin, big_bin_len);
    hash_insert(playerBase, "blipblop", blipblop_bin, blipblop_bin_len);
    hash_insert(playerBase, "blipp079", blipp079_bin, blipp079_bin_len);
    hash_insert(playerBase, "bs22", bs22_bin, bs22_bin_len);
    hash_insert(playerBase, "cabal", cabal_bin, cabal_bin_len);
    hash_insert(playerBase, "chipmon2", chipmon2_bin, chipmon2_bin_len);
    hash_insert(playerBase, "cosotfmx", cosotfmx_bin, cosotfmx_bin_len);
    hash_insert(playerBase, "crapman1", crapman1_bin, crapman1_bin_len);
    hash_insert(playerBase, "crapman2", crapman2_bin, crapman2_bin_len);
    hash_insert(playerBase, "digitfmx", digitfmx_bin, digitfmx_bin_len);
    hash_insert(playerBase, "dmu", dmu_bin, dmu_bin_len);
    hash_insert(playerBase, "dwaga", dwaga_bin, dwaga_bin_len);
    hash_insert(playerBase, "dyter07", dyter07_bin, dyter07_bin_len);
    hash_insert(playerBase, "elf", elf_bin, elf_bin_len);
    hash_insert(playerBase, "fc13_14", fc13_14_bin, fc13_14_bin_len);
    hash_insert(playerBase, "fireice", fireice_bin, fireice_bin_len);
    hash_insert(playerBase, "gemx", gemx_bin, gemx_bin_len);
    hash_insert(playerBase, "harlequin", harlequin_bin, harlequin_bin_len);
    hash_insert(playerBase, "hippel_coso_note_deli", hippel_coso_note_deli_bin, hippel_coso_note_deli_bin_len);
    hash_insert(playerBase, "hpn_jam", hpn_jam_bin, hpn_jam_bin_len);
    hash_insert(playerBase, "huelsbeck", huelsbeck_bin, huelsbeck_bin_len);
    hash_insert(playerBase, "huntforredoct", huntforredoct_bin, huntforredoct_bin_len);
    hash_insert(playerBase, "ivanhoe", ivanhoe_bin, ivanhoe_bin_len);
    hash_insert(playerBase, "jamcrackerpro", jamcrackerpro_bin, jamcrackerpro_bin_len);
    hash_insert(playerBase, "lastninja3", lastninja3_bin, lastninja3_bin_len);
    hash_insert(playerBase, "lastninjaiii", lastninjaiii_bin, lastninjaiii_bin_len);
    hash_insert(playerBase, "lcd", lcd_bin, lcd_bin_len);
    hash_insert(playerBase, "lx_mmme", lx_mmme_bin, lx_mmme_bin_len);
    hash_insert(playerBase, "mcoder", mcoder_bin, mcoder_bin_len);
    hash_insert(playerBase, "megatizer", megatizer_bin, megatizer_bin_len);
    hash_insert(playerBase, "metrocross", metrocross_bin, metrocross_bin_len);
    hash_insert(playerBase, "midi", midi_bin, midi_bin_len);
    hash_insert(playerBase, "mmme", mmme_bin, mmme_bin_len);
    hash_insert(playerBase, "mmmesid", mmmesid_bin, mmmesid_bin_len);
    hash_insert(playerBase, "mon", mon_bin, mon_bin_len);
    hash_insert(playerBase, "mon_old_deli", mon_old_deli_bin, mon_old_deli_bin_len);
    hash_insert(playerBase, "musmon1", musmon1_bin, musmon1_bin_len);
    hash_insert(playerBase, "musmon2", musmon2_bin, musmon2_bin_len);
    hash_insert(playerBase, "ninjaremix", ninjaremix_bin, ninjaremix_bin_len);
    hash_insert(playerBase, "ninjatfmx", ninjatfmx_bin, ninjatfmx_bin_len);
    hash_insert(playerBase, "nobuddiesland", nobuddiesland_bin, nobuddiesland_bin_len);
    hash_insert(playerBase, "none", none_bin, none_bin_len);
    hash_insert(playerBase, "note2", note2_bin, note2_bin_len);
    hash_insert(playerBase, "note", note_bin, note_bin_len);
    hash_insert(playerBase, "page", page_bin, page_bin_len);
    hash_insert(playerBase, "psgtrackerv2", psgtrackerv2_bin, psgtrackerv2_bin_len);
    hash_insert(playerBase, "puma2", puma2_bin, puma2_bin_len);
    hash_insert(playerBase, "puma", puma_bin, puma_bin_len);
    hash_insert(playerBase, "quartet", quartet_bin, quartet_bin_len);
    hash_insert(playerBase, "quartet_mindbomb", quartet_mindbomb_bin, quartet_mindbomb_bin_len);
    hash_insert(playerBase, "realms", realms_bin, realms_bin_len);
    hash_insert(playerBase, "robhubbard_deli", robhubbard_deli_bin, robhubbard_deli_bin_len);
    hash_insert(playerBase, "rollingronny", rollingronny_bin, rollingronny_bin_len);
    hash_insert(playerBase, "shadowdancer", shadowdancer_bin, shadowdancer_bin_len);
    hash_insert(playerBase, "sidsound", sidsound_bin, sidsound_bin_len);
    hash_insert(playerBase, "sndh_ice", sndh_ice_bin, sndh_ice_bin_len);
    hash_insert(playerBase, "spacecrusade", spacecrusade_bin, spacecrusade_bin_len);
    hash_insert(playerBase, "spacegun", spacegun_bin, spacegun_bin_len);
    hash_insert(playerBase, "ssd1", ssd1_bin, ssd1_bin_len);
    hash_insert(playerBase, "stos", stos_bin, stos_bin_len);
    hash_insert(playerBase, "synthiv", synthiv_bin, synthiv_bin_len);
    hash_insert(playerBase, "tao_digi", tao_digi_bin, tao_digi_bin_len);
    hash_insert(playerBase, "tao_hubbard", tao_hubbard_bin, tao_hubbard_bin_len);
    hash_insert(playerBase, "tao_ms25", tao_ms25_bin, tao_ms25_bin_len);
    hash_insert(playerBase, "tao_ms27", tao_ms27_bin, tao_ms27_bin_len);
    hash_insert(playerBase, "tao_ms211", tao_ms211_bin, tao_ms211_bin_len);
    hash_insert(playerBase, "tao_tsd", tao_tsd_bin, tao_tsd_bin_len);
    hash_insert(playerBase, "tcbtracker", tcbtracker_bin, tcbtracker_bin_len);
    hash_insert(playerBase, "test", test_bin, test_bin_len);
    hash_insert(playerBase, "tfmx", tfmx_bin, tfmx_bin_len);
    hash_insert(playerBase, "tfmx_aga", tfmx_aga_bin, tfmx_aga_bin_len);
    hash_insert(playerBase, "tfmx_old", tfmx_old_bin, tfmx_old_bin_len);
    hash_insert(playerBase, "tfmx_st", tfmx_st_bin, tfmx_st_bin_len);
    hash_insert(playerBase, "timerc", timerc_bin, timerc_bin_len);
    hash_insert(playerBase, "toki", toki_bin, toki_bin_len);
    hash_insert(playerBase, "tsd1", tsd1_bin, tsd1_bin_len);
    hash_insert(playerBase, "utopia", utopia_bin, utopia_bin_len);
    hash_insert(playerBase, "vikingchild", vikingchild_bin, vikingchild_bin_len);
    hash_insert(playerBase, "xbiossound", xbiossound_bin, xbiossound_bin_len);
    hash_insert(playerBase, "zonewarrior", zonewarrior_bin, zonewarrior_bin_len);
}


char * EMSCRIPTEN_KEEPALIVE getPlayerBuf(char *key) {
	return hash_lookup(playerBase, key);
}

unsigned int EMSCRIPTEN_KEEPALIVE getPlayerBufLen(char *key) {
	return hash_lookup_len(playerBase, key);
}


