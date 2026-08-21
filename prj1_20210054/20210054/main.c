#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <time.h>
#include"bitmap.h"
#include"debug.h"
#include"hash.h"
#include"hex_dump.h"
#include"limits.h"
#include"list.h"
#include"round.h"
#define NUM_OF_DATA 20

bool _less(const struct list_elem* a, const struct list_elem* b, void* aux) {
    const struct list_item* itemA = list_entry(a, struct list_item, elem);
    const struct list_item* itemB = list_entry(b, struct list_item, elem);
    return itemA->data < itemB->data;
}


int commandAnalysis(char* input, char command[5][30]) {
    int ci = 0, wi = 0;
    size_t len = strlen(input);

    for (size_t i = 0; i < len; ++i) {
        if (input[i] == ' ' || input[i] == '\n') {
            command[ci][wi] = '\0';
            ci++;
            wi = 0;
        }
        else {
            command[ci][wi++] = input[i];
        }
    }
    return ci;
}

/*ÇïÆÛ ÇÔ¼ö 3°³*/
int get_bitmap_index(const char* name) {
    return atoi(name + 2);
}

bool parse_bool(const char* str) {
    return strcmp(str, "true") == 0;
}

void print_bool_result(bool result) {
    printf(result ? "true\n" : "false\n");
}

void handle_create(char command[][30], struct list** LIST, struct hash** HASH, struct bitmap** BITMAP) {
    char* name = command[2];

    if (!strcmp(command[1], "list")) {
        createList(LIST, name);
    }
    else if (!strcmp(command[1], "bitmap")) {
        int idx = get_bitmap_index(name);
        BITMAP[idx] = bitmap_create((size_t)atoi(command[3]));
    }
    else if (!strcmp(command[1], "hashtable")) {
        int idx = atoi(name + 4);
        HASH[idx] = createHash();
    }
}

void handle_delete(char command[][30], struct list** LIST, struct hash** HASH, struct bitmap** BITMAP) {
    char* name = command[1];

    if (!strncmp(name, "list", 4)) {
        deleteList(LIST, name);
    }
    else if (!strncmp(name, "bm", 2)) {
        bitmap_destroy(BITMAP[get_bitmap_index(name)]);
    }
    else if (!strncmp(name, "hashtable", 9)) {
        hash_destroy(HASH[atoi(name + 4)], removeHashElem);
    }
}

void handle_dumpdata(char command[][30], struct list** LIST, struct hash** HASH, struct bitmap** BITMAP) {
    char* name = command[1];

    if (!strncmp(name, "list", 4)) {
        dumpList(LIST, name);
    }
    else if (!strncmp(name, "bm", 2)) {
        printBitmap(BITMAP, name);
    }
    else if (!strncmp(name, "hash", 4)) {
        int idx = atoi(name + 4);
        printHash(HASH[idx]);
    }
}

void handle_list_commands(char command[][30], int comCnt, struct list** LIST) {
    int idx = atoi(command[1] + 4);

    if (!strcmp(command[0], "list_push_front")) {
        pushList(LIST, command[1], atoi(command[2]), 0);
    }
    else if (!strcmp(command[0], "list_push_back")) {
        pushList(LIST, command[1], atoi(command[2]), 1);
    }
    else if (!strcmp(command[0], "list_pop_front")) {
        popList(LIST, command[1], 0);
    }
    else if (!strcmp(command[0], "list_pop_back")) {
        popList(LIST, command[1], 1);
    }
    else if (!strcmp(command[0], "list_front")) {
        printListFrontBack(LIST, command[1], 0);
    }
    else if (!strcmp(command[0], "list_back")) {
        printListFrontBack(LIST, command[1], 1);
    }
    else if (!strcmp(command[0], "list_insert")) {
        insertindexList(LIST, command[1], atoi(command[2]), atoi(command[3]));
    }
    else if (!strcmp(command[0], "list_insert_ordered")) {
        insertOrderedList(LIST, command[1], atoi(command[2]));
    }
    else if (!strcmp(command[0], "list_empty")) {
        print_bool_result(list_empty(LIST[idx]));
    }
    else if (!strcmp(command[0], "list_size")) {
        printf("%zu\n", list_size(LIST[idx]));
    }
    else if (!strcmp(command[0], "list_min")) {
        printListItemFun(list_min(LIST[idx], _less, NULL), '\n');
    }
    else if (!strcmp(command[0], "list_max")) {
        printListItemFun(list_max(LIST[idx], _less, NULL), '\n');
    }
    else if (!strcmp(command[0], "list_remove")) {
        removeList(LIST, command[1], atoi(command[2]));
    }
    else if (!strcmp(command[0], "list_reverse")) {
        if (LIST[idx]) list_reverse(LIST[idx]);
    }
    else if (!strcmp(command[0], "list_shuffle")) {
        if (LIST[idx]) list_shuffle(LIST[idx]);
    }
    else if (!strcmp(command[0], "list_sort")) {
        list_sort(LIST[idx], _less, NULL);
    }
    else if (!strcmp(command[0], "list_splice")) {
        listSplice(LIST, command[1], atoi(command[2]), command[3], atoi(command[4]), atoi(command[5]));
    }
    else if (!strcmp(command[0], "list_swap")) {
        listSwap(LIST, command[1], atoi(command[2]), atoi(command[3]));
    }
    else if (!strcmp(command[0], "list_unique")) {
        if (comCnt == 3) {
            list_unique(LIST[idx], LIST[atoi(command[2] + 4)], _less, NULL);
        }
        else {
            list_unique(LIST[idx], NULL, _less, NULL);
        }
    }
}


void handle_bitmap_commands(char command[][30], struct bitmap** BITMAP) {
    int idx = get_bitmap_index(command[1]);

    if (!strcmp(command[0], "bitmap_mark")) {
        bitmap_mark(BITMAP[idx], (size_t)atoi(command[2]));
    }
    else if (!strcmp(command[0], "bitmap_all")) {
        print_bool_result(bitmap_all(BITMAP[idx], atoi(command[2]), atoi(command[3])));
    }
    else if (!strcmp(command[0], "bitmap_any")) {
        print_bool_result(bitmap_any(BITMAP[idx], atoi(command[2]), atoi(command[3])));
    }
    else if (!strcmp(command[0], "bitmap_contains")) {
        bool val = parse_bool(command[4]);
        print_bool_result(bitmap_contains(BITMAP[idx], atoi(command[2]), atoi(command[3]), val));
    }
    else if (!strcmp(command[0], "bitmap_count")) {
        bool val = parse_bool(command[4]);
        printf("%zu\n", bitmap_count(BITMAP[idx], atoi(command[2]), atoi(command[3]), val));
    }
    else if (!strcmp(command[0], "bitmap_dump")) {
        bitmap_dump(BITMAP[idx]);
    }
    else if (!strcmp(command[0], "bitmap_expand")) {
        BITMAP[idx] = bitmap_expand(BITMAP[idx], atoi(command[2]));
    }
    else if (!strcmp(command[0], "bitmap_set")) {
        bitmap_set(BITMAP[idx], atoi(command[2]), parse_bool(command[3]));
    }
    else if (!strcmp(command[0], "bitmap_set_all")) {
        bitmap_set_all(BITMAP[idx], parse_bool(command[2]));
    }
    else if (!strcmp(command[0], "bitmap_flip")) {
        bitmap_flip(BITMAP[idx], atoi(command[2]));
    }
    else if (!strcmp(command[0], "bitmap_none")) {
        print_bool_result(bitmap_none(BITMAP[idx], atoi(command[2]), atoi(command[3])));
    }
    else if (!strcmp(command[0], "bitmap_reset")) {
        bitmap_reset(BITMAP[idx], atoi(command[2]));
    }
    else if (!strcmp(command[0], "bitmap_scan_and_flip")) {
        printf("%zu\n", bitmap_scan_and_flip(BITMAP[idx], atoi(command[2]), atoi(command[3]), parse_bool(command[4])));
    }
    else if (!strcmp(command[0], "bitmap_scan")) {
        printf("%zu\n", bitmap_scan(BITMAP[idx], atoi(command[2]), atoi(command[3]), parse_bool(command[4])));
    }
    else if (!strcmp(command[0], "bitmap_set_multiple")) {
        bitmap_set_multiple(BITMAP[idx], atoi(command[2]), atoi(command[3]), parse_bool(command[4]));
    }
    else if (!strcmp(command[0], "bitmap_size")) {
        printf("%zu\n", bitmap_size(BITMAP[idx]));
    }
    else if (!strcmp(command[0], "bitmap_test")) {
        print_bool_result(bitmap_test(BITMAP[idx], atoi(command[2])));
    }
}


void handle_hash_commands(char command[][30], struct hash** HASH) {
    int idx = atoi(command[1] + 4);

    if (!strcmp(command[0], "hash_insert")) {
        hash_insert(HASH[idx], makeHashElem(atoi(command[2])));
    }
    else if (!strcmp(command[0], "hash_delete")) {
        hash_delete(HASH[idx], makeHashElem(atoi(command[2])));
    }
    else if (!strcmp(command[0], "hash_empty")) {
        print_bool_result(hash_empty(HASH[idx]));
    }
    else if (!strcmp(command[0], "hash_size")) {
        printf("%zu\n", hash_size(HASH[idx]));
    }
    else if (!strcmp(command[0], "hash_clear")) {
        hash_clear(HASH[idx], removeHashElem);
    }
    else if (!strcmp(command[0], "hash_find")) {
        struct hash_elem* he = hash_find(HASH[idx], makeHashElem(atoi(command[2])));
        if (he != NULL) {
            printf("%d\n", he->data);
        }
    }
    else if (!strcmp(command[0], "hash_replace")) {
        hash_replace(HASH[idx], makeHashElem(atoi(command[2])));
    }
    else if (!strcmp(command[0], "hash_apply")) {
        applyHash(HASH[idx], command[2]);
    }
}


int main() {
    srand(time(NULL));
    char input[100];
    char command[5][30];

    struct list** LIST = calloc(NUM_OF_DATA, sizeof(struct list*));
    struct hash** HASH = calloc(NUM_OF_DATA, sizeof(struct hash*));
    struct bitmap** BITMAP = calloc(NUM_OF_DATA, sizeof(struct bitmap*));

    if (!LIST || !HASH || !BITMAP) {
        fprintf(stderr, "Memory allocation failed in main.\n");
        exit(EXIT_FAILURE);
    }

    while (fgets(input, sizeof(input), stdin)) {
        int comCnt = commandAnalysis(input, command);

        if (!strcmp(command[0], "create")) {
            handle_create(command, LIST, HASH, BITMAP);
        }
        else if (!strcmp(command[0], "delete")) {
            handle_delete(command, LIST, HASH, BITMAP);
        }
        else if (!strcmp(command[0], "dumpdata")) {
            handle_dumpdata(command, LIST, HASH, BITMAP);
        }
        else if (!strcmp(command[0], "quit")) {
            break;
        }
        else if (!strncmp(command[0], "list_", 5)) {
            handle_list_commands(command, comCnt, LIST);
        }
        else if (!strncmp(command[0], "bitmap_", 7)) {
            handle_bitmap_commands(command, BITMAP);
        }
        else if (!strncmp(command[0], "hash_", 5)) {
            handle_hash_commands(command, HASH);
        }
    }

    free(LIST);
    free(HASH);
    free(BITMAP);
    return 0;
}