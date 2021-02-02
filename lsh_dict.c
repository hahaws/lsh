

/**
 *
 */

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>

#include "lsh_dict.h"

sd_map * new_map() {
    sd_map * map = (sd_map *)malloc(sizeof(sd_map));
    sd_entry * entry = (sd_entry *)malloc(INCREASE_SIZE * sizeof(sd_entry));
    unsigned char * idx_use = (unsigned char *)malloc(INCREASE_SIZE * sizeof(unsigned char));
    memset(map, 0x00, sizeof(sd_map));
    memset(entry, 0x00, INCREASE_SIZE * sizeof(sd_entry));
    memset(idx_use, 0x00, INCREASE_SIZE * sizeof(unsigned char));

    map->entry = entry;
    map->SIZE = INCREASE_SIZE;
    map->idx_use = idx_use;
    map->idx = 1;

    return map;
}

void free_map(sd_map * map) {
    free(map->entry);
    free(map);
}

void insert_map(sd_map * map, void * key, void * value) {
    size_t idx;
    unsigned long id = ptr2id(key);
    if ((idx = idx_entry(map, key)) > 0) {

        free(((map->entry) + idx)->key);
        free(((map->entry) + idx)->value);

        ((map->entry) + idx)->key = key;
        ((map->entry) + idx)->value = value;
        ((map->entry) + idx)->id = id;
    } else {
        ((map->entry) + map->idx)->key = key;
        ((map->entry) + map->idx)->value = value;
        ((map->entry) + map->idx)->id = id;
        (map->idx_use)[map->idx] = 1;
        map->idx += 1;
    }

    if (map->idx >= map->SIZE) {
        resize_map(&map);
    }
}

void delete_entry(sd_map * map, void * key) {
    unsigned long id = ptr2id(key);
    size_t i = 1;
    for (; i < map->idx; ++i) {
        sd_entry * entry = map->entry + i;
        if (entry->id == id) {
            free(entry->key);
            free(entry->value);
            (map->idx_use)[i] = 0;
            break;
        }
    }
}

void delete_entry_idx(sd_map * map, int idx) {
    sd_entry * entry = map->entry + idx;
    if ((map->idx_use)[idx] == 0)
        return;
    free(entry->key);
    free(entry->value);
    (map->idx_use)[idx] = 0;
    return;
}

size_t idx_entry(sd_map * map, void * key) {
    unsigned long id = ptr2id(key);
    size_t i = 1;
    for (; i < map->idx; ++i) {
        if ((map->idx_use)[i] == 0) 
            continue;
        sd_entry * entry = map->entry + i;
        if (entry->id == id) {
            return i;
        }
    }
    return 0;
}

sd_entry * find_entry(sd_map * map, void * key) {
    unsigned long id = ptr2id(key);
    size_t i = 1;
    for (; i < map->idx; ++i) {
        if ((map->idx_use)[i] == 0) 
            continue;
        sd_entry * entry = map->entry + i;
        if (entry->id == id) {
            return entry;
        }
    }
    return NULL;
}

/* RS Hash */
unsigned long ptr2id(void * ptr) {
    unsigned long a = 63689;
    unsigned long b = 378551;
    unsigned long hash = 0;
    size_t i = 0;
    while (*(char*)ptr) {
        hash = hash * a + (unsigned long)(*(char*)ptr);
        (char*)ptr++;
        a *= b;
    }
    return (hash & 0x7FFFFFFF);
}

void resize_map(sd_map ** map) {
    printf("resize\n");
    sd_map * origin = *map; 
    unsigned long entry_cnt = 0;
    int i = 1;
    for (i = 1; i < origin->idx; ++i) {
        if ((origin->idx_use)[i] == 1) 
            entry_cnt += 1;
    }

    if (entry_cnt < origin->SIZE / 2) {
        entry_cnt = origin->SIZE;
    } else {
        entry_cnt = origin->SIZE + INCREASE_SIZE;
    }

    sd_map * new_map = (sd_map *)malloc(sizeof(sd_map));
    sd_entry * entry = (sd_entry *)malloc(entry_cnt * sizeof(sd_entry));
    unsigned char * idx_use = (unsigned char *)malloc(entry_cnt * sizeof(unsigned char));
    memset(new_map, 0x00, sizeof(sd_map));
    memset(entry, 0x00, entry_cnt * sizeof(sd_entry));
    memset(idx_use, 0x00, entry_cnt * sizeof(unsigned char));

    new_map->entry = entry;
    new_map->SIZE = entry_cnt;
    new_map->idx_use = idx_use;
    new_map->idx = 1;

    for (i = 1; i < origin->idx; ++i)  {
        if ((origin->idx_use)[i] == 1) {
            (new_map->entry)[new_map->idx] = (origin->entry)[i];
            (new_map->idx_use)[new_map->idx] = 1;
            new_map->idx += 1;
        }
    }

    free(origin->entry);
    free(origin->idx_use);

    origin->idx_use = new_map->idx_use;
    origin->entry = new_map->entry;
    origin->idx = new_map->idx;
    origin->SIZE = new_map->SIZE;

    free(new_map);

}
