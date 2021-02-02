
/**
 *
 */

#ifndef __SH_DICT_H__
#define __SH_DICT_H__


#include <sys/types.h>

#define INCREASE_SIZE 20


typedef struct _sh_dict_entry
{
    void * key;
    void * value;
    unsigned long id;
} sd_entry;

typedef struct __sh_dict_map
{
    sd_entry * entry;
    unsigned char * idx_use;
    size_t SIZE;
    size_t idx;
} sd_map;


sd_map * new_map();
void free_map(sd_map *);
void insert_map(sd_map *, void *, void *);
void delete_entry(sd_map *, void *);
void delete_entry_idx(sd_map *, int);
sd_entry * find_entry(sd_map *, void *);
size_t idx_entry(sd_map *, void *);
unsigned long ptr2id(void *);
void resize_map(sd_map **);

#endif // __SH_DICT_H__
