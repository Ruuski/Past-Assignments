/* Reads an inverted_index to a dynamic struct, prints some data from inside 
 * the index and implements a simple interactive search method to the user
 *
 * Submission for comp10002 2016s2 Assignment 2.
 * Prepared by Liam Aharon, laharon@student.unimelb.edu.au
 * October 2016
 *
 * mygetchar() by Alister Moffat
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>

#define MAX_WORD_LEN 999        /* max possible term len and stdin line len */
#define MAX_PAIRS_STAGE1 10     /* max pairs for each term in stage 1 output */
#define EPSILON 0.000000001     /* accuracy threshold for float comparison */

typedef struct pair pair_t;
typedef struct index_entry index_entry_t;
typedef struct buffer buffer_t;
typedef struct index index_t;
typedef struct input input_t;
typedef struct top_scores top_scores_t;
typedef char *term_t;

/* recursive struct used to store all pairs for a particular term */
struct pair {   
    int doc;
    int n;
    pair_t *next;

};

/* struct used to store all infomation for a term entry in index */
struct index_entry {
    char *term;
    int ndocs;
    int pair_depth;
    pair_t *pair;
};

/* character buffer of fixed length, used as a temporary place to hold chars
 * as they're being read from file and stdin */
struct buffer {
    int i;
    char arr[MAX_WORD_LEN+1];
};

/* inverted index of terms, and some data about it */
struct index {
    index_entry_t *arr;
    int i;
    int cap;
    int npairs;
    int ndocs;
    int* doc_len;
};

/* an array of terms input from a line in stdin, and infomation about them */
struct input {
    int nterms;
    int cap;
    term_t *term_list;
    int* term_index;
};

/* stores top scoring docs and their scores for stage 3 */
struct top_scores {
    float top;
    int top_doc;
    float mid;
    int mid_doc;
    float low;
    int low_doc;
};

/**************************************************************/

/* function prototypes */
index_t *index_new();
void index_write(index_t *index, char file_name[], buffer_t *buffer);
int index_write_s1(index_entry_t *new, buffer_t *buffer, char c);
int index_write_s2(index_entry_t *new, buffer_t *buffer, char c);
int index_write_s3(pair_t *tmp_pair, buffer_t *buffer, char c);
int index_write_s4(index_t *index, index_entry_t **new, pair_t **tmp_pair,
                         buffer_t *buffer, char c);
void index_append(index_t *index, index_entry_t *new_entry);
int index_search(index_t *index, term_t term);
void index_init_arr(index_t *index);
void index_double_arr(index_t *index);
void index_write_doc_len(index_t *index);
index_entry_t *entry_new();
void entry_append_pair(index_entry_t**new, pair_t**tmp_pair, buffer_t *buffer);
buffer_t *buffer_new();
void buffer_end(buffer_t *buffer);
void buffer_append(buffer_t *buffer, char c);
pair_t *pair_new();
pair_t *pair_append(pair_t *curr_pair, pair_t *new_pair);
int pair_depth(pair_t *pair);
pair_t *pair_free(pair_t *pair);
input_t *input_new();
void input_write(input_t *input, index_t *index, buffer_t *buffer);
void input_init_index(input_t *input);
void input_double_index(input_t *input);
void input_append(input_t *input, index_t *index, term_t new_term);
top_scores_t *compute_score(input_t *input, index_t *index);
void doc_compute_lens(index_t *index);
float compute_avgd(index_t *index);
void stage1_out(index_t *index);
void stage1_print_node(index_t *index, int entry_pos);
void stage2_out(index_t *index, input_t *input);
void stage3_out(top_scores_t *top_scores);
int retrieve_fdt(index_t *index, int entry_pos, int doc);
void read_stdin(index_t *index, buffer_t *buffer);
char *dup_array(char arr[], int start, int finish);
void top_scores_insert(top_scores_t *top_scores, float score, int doc);
void replace_1st_place(top_scores_t *top_scores, float score, int doc);
void replace_2nd_place(top_scores_t *top_scores, float score, int doc);
void replace_3rd_place(top_scores_t *top_scores, float score, int doc);
int float_compare(float a, float b);
int mygetchar();
void index_free(index_t *index);
void input_free(input_t *input);

/**************************************************************/

/* main program
 */
int
main(int argc, char *argv[]) {
    if (argv[1] == NULL) exit(EXIT_FAILURE);
    /* create struct to hold inverted index and buffer as tmp fixed size 
     * storage */
    index_t *index = index_new();
    buffer_t *buffer = buffer_new();
    /* write inverted index from file specified in argv[1] to index */
    index_write(index, argv[1], buffer);
    stage1_out(index);
    /* stores the amount of words in each doc */
    index_write_doc_len(index);
    /* reads user input from. stage 2 and 3 are output while reading input */
    read_stdin(index, buffer);
    /* program finished, free everything */
    index_free(index);
    free(buffer);
    printf("Ta da-da-daaah...\n");
    return 0;
}

/**************************************************************/

/* reads input from stdin to buffer until a newline is read. terms in buffer
 * are seperated and stored in struct 'input', then stage 2 and 3 are called. 
 * input is then freed and process repeats for any new lines entered
 */
void
read_stdin(index_t *index, buffer_t *buffer) {
    char c;
    input_t *input = input_new();
    top_scores_t *top_scores;

    while((c = mygetchar()) != EOF) {
        if (c != '\n') buffer_append(buffer, c);
        else {
            /* add ' ' at the end of the last term in the line, as input_write 
               uses ' ' to signal the end of a term */
            buffer_append(buffer, ' ');
            buffer_end(buffer);
            input_write(input, index, buffer);
            stage2_out(index, input);
            top_scores = compute_score(input, index);
            stage3_out(top_scores);
            free(top_scores);
            input_free(input);
            input = input_new();
        }
    }
    input_free(input);
}

/**************************************************************/

/* prints stage 3 output
 */
void
stage3_out(top_scores_t *top_scores) {
    printf("Stage 3 Output\n");
    if (top_scores->top != 0) {
        printf("    document%4d: score%7.3f\n", 
              top_scores->top_doc, top_scores->top);
    }
    if (top_scores->mid != 0) {
        printf("    document%4d: score%7.3f\n", 
              top_scores->mid_doc, top_scores->mid);
    }
    if (top_scores->low != 0) {
        printf("    document%4d: score%7.3f\n", 
              top_scores->low_doc, top_scores->low);
    }
    printf("\n");
}

/**************************************************************/

/* computes a score based on terms in input for every document in our index,
 * the score gets added to top_scores if it belongs in the top 3
 */
top_scores_t
*compute_score(input_t *input, index_t *index) {
    int i, j;
    float ft, fdt, k=1.2, b=0.75, doc_len, Ld, avgd=compute_avgd(index), 
          score=0;

    top_scores_t *top_scores = (top_scores_t*)calloc(1, sizeof(top_scores_t));
    assert(top_scores != NULL);

    /* create score for every doc */
    for (i=0; i<index->ndocs; i++) {
        doc_len = index->doc_len[i];
        Ld = k*((1-b)+b*doc_len / avgd);
        score = 0;
        /* add score for each term to overall doc score */
        for (j=0; j<input->nterms; j++) {
            fdt = retrieve_fdt(index, input->term_index[j]-1, i+1);
            /* if term not in any docs, don't add anything to score */
            if (input->term_index[j] == -1) { /* do nothing */ }
            else {
                ft = index->arr[input->term_index[j]-1].ndocs;
                score += ((1+k)*fdt)/(Ld+fdt) * log2((index->ndocs+0.5)/ft);
            }
        }
        top_scores_insert(top_scores, score, i+1);
    }
    return top_scores;
}

/**************************************************************/

/* inserts a score and its corresponding document in top_scores if it belongs
 * in the top 3
 */
void
top_scores_insert(top_scores_t *top_scores, float score, int doc) {

    /* score too low to be in top_scores */
    if (score < top_scores->low) return;

    /* score to be inserted in 3rd place */
    if ((float_compare(score, top_scores->low) && doc < top_scores->low_doc) ||
       (score < top_scores->mid && score > top_scores->low) ||
       (float_compare(score, top_scores->mid) && doc > top_scores->mid_doc)) {

        replace_3rd_place(top_scores, score, doc);
        return;
    }

    /* score to be inserted in 2nd place */
    if ((float_compare(score, top_scores->mid) && doc < top_scores->mid_doc) ||
       (score < top_scores->top && score > top_scores->mid) ||
       (float_compare(score, top_scores->top) && doc > top_scores->top_doc)) {

        replace_3rd_place(top_scores, top_scores->mid, top_scores->mid_doc);
        replace_2nd_place(top_scores, score, doc);
        return;
    }

    /* score to be inserted in 1st place */
    if ((float_compare(score, top_scores->top) && doc < top_scores->top_doc) ||
        (score > top_scores->top)) {

        replace_3rd_place(top_scores, top_scores->mid, top_scores->mid_doc);
        replace_2nd_place(top_scores, top_scores->top, top_scores->top_doc);
        replace_1st_place(top_scores, score, doc);
    }
}

/**************************************************************/

/* replace top score doc pair 
 */
void
replace_1st_place(top_scores_t *top_scores, float score, int doc) {
    top_scores->top = score;
    top_scores->top_doc = doc;
}

/* replace mid score doc pair
 */
void
replace_2nd_place(top_scores_t *top_scores, float score, int doc) {
    top_scores->mid = score;
    top_scores->mid_doc = doc;
}

/* replace low score doc pair
 */
void
replace_3rd_place(top_scores_t *top_scores, float score, int doc) {
    top_scores->low = score;
    top_scores->low_doc = doc;
}

/**************************************************************/

/* safely compare floats. return 1 if equal within range EPSILON, else return 0
 */
int
float_compare(float a, float b) {
    return fabs(a - b) < EPSILON;
}

/**************************************************************/

/* computes avgd (average length of a document) 
 */
float
compute_avgd(index_t *index) {
    int i;
    float sum, ndocs=index->ndocs;

    for (i=0; i<index->ndocs; i++) {
        sum += index->doc_len[i];
    }
    return sum/ndocs;
}

/**************************************************************/

/* retrieves the fdt (term occurance in particular doc) from index
 */
int
retrieve_fdt(index_t *index, int entry_pos, int doc) {
    pair_t *next;

    /* term is not in any documents */
    if (entry_pos == -2) return 0;

    /* look through every pair for a particular term  */
    next = index->arr[entry_pos].pair;
    while (next != NULL) {
        if (next->doc == doc) return next->n;
        next = next->next;
    }
    return 0;
}

/**************************************************************/

/* writes an array of document lengths (words in each document) to index
 */
void
index_write_doc_len(index_t *index) {
    int i, doc, n;
    pair_t *next;

    index->doc_len = (int*)calloc(index->ndocs, sizeof(int));
    assert(index->doc_len != NULL);

    /* iterate through each entry in index */
    for (i=0; i<index->i; i++) {

        /* iterate through every pair within the entry, adding n to its
           corresponding document as we go */
        next = index->arr[i].pair;
        while (next != NULL) {
            doc = next->doc-1;
            n = next->n;
            index->doc_len[doc] += n;
            next = next->next;
        }
    }
}

/**************************************************************/

/* takes a buffer with terms seperated by a single ' ' and writes their index 
 * position into input
 */
void
input_write(input_t *input, index_t *index, buffer_t *buffer) {
    term_t new_term;
    int i, start=0, finish=0;

    for (i=0; i < strlen(buffer->arr); i++) {
        if (buffer->arr[i] == ' ') {
            finish = i;
            /* for all except 1st term start is ' ' char - skip it */
            if (start != 0) start++;
            new_term = dup_array(buffer->arr, start, finish);
            input_append(input, index, new_term);
            start = finish;
        }
    }
}

/**************************************************************/

/* prints stage 2 output
 */
void
stage2_out(index_t *index, input_t *input) {
    int i, curr_index;
    term_t curr_term;

    printf("Stage 2 Output\n");
    for (i=0; i < input->nterms; i++) {
        curr_index = input->term_index[i];
        curr_term = input->term_list[i];
        /* account for terms in input that are not a part of index */
        if (curr_index == -1) printf("    \"%s\" is not indexed\n", curr_term);
        else printf("    \"%s\" is term %d\n", curr_term, curr_index);
    }
    printf("\n");
}

/**************************************************************/

/* allocates more memory if necesarry, appends a new term to input->term_list
 * and terms corresponding location in index if applicable
 */
void
input_append(input_t *input, index_t *index, term_t new_term) {
    /* first insersion, initiate array size to 1 */
    if (input->cap == 0) {
        input_init_index(input);
    } else if (input->nterms >= input->cap) {
        input_double_index(input);
    }
    input->term_list[input->nterms] = new_term;
    input->term_index[input->nterms] = index_search(index, new_term);
    (input->nterms)++;
}

/**************************************************************/

/* performs binary search on terms in index for a term and returns its 
 * position in the term array. if term is not found returns -1 */
int
index_search(index_t *index, term_t term) {
    int lo=0, hi=index->i-1, mid, res;

    while(lo <= hi) {
        mid = (lo+hi)/2;
        if ((res = strcmp(index->arr[mid].term, term)) > 0) hi = mid - 1;
        else if (res < 0) lo = mid + 1;
        else if (res == 0) return mid+1;
    }
    return -1;
}

/**************************************************************/

/* create an empty input_t struct and return a pointer to it
 */
input_t
*input_new() {
    input_t *input;
    input = (input_t*)calloc(1, sizeof(input_t));
    assert(input != NULL);
    return input;
}

/**************************************************************/

/* doubles term capacity inside the struct input
 */
void
input_double_index(input_t *input) {
    /* double memory for term_list */
    term_t *new_term;
    new_term = (term_t*)realloc(input->term_list,input->cap*2*sizeof(term_t*));
    assert(new_term != NULL);
    input->term_list = new_term;

    /* double memory for term_index */
    int* new_index;
    new_index = (int*)realloc(input->term_index, input->cap*2*sizeof(int));
    assert(new_index != NULL);
    input->term_index = new_index;
    input->cap *= 2;
}

/**************************************************************/

/* provides memory to store a single item in term_index
 */
void
input_init_index(input_t *input) {
    input->term_list = (term_t*)calloc(1, sizeof(term_t));
    assert(input->term_list != NULL);
    input->term_index = (int*)calloc(1, sizeof(int));
    assert(input->term_index != NULL);
    input->cap = 1;
}

/**************************************************************/

/* handles printing of stage 1
 */
void
stage1_out(index_t *index) {
    int first_node = 0;
    int second_node = 1;
    int second_last_node = (index->i)-2;
    int last_node = (index->i)-1;

    /* print header */
    printf("Stage 1 Output\n");
    printf("index has %d terms and %d (d,fdt) pairs\n", 
           index->i, index->npairs);

    /* print body and final newline */
    stage1_print_node(index, first_node);
    stage1_print_node(index, second_node);
    stage1_print_node(index, second_last_node);
    stage1_print_node(index, last_node);
    printf("\n");
}

/**************************************************************/

/* prints infomation about an index_entry formatted for stage 1
 */
void
stage1_print_node(index_t *index, int entry_pos) {
    int pairs_printed=0;
    pair_t *pair = index->arr[entry_pos].pair;

    /* print term in index_entry that was chosen */
    printf("term %d is \"%s\":\n    ", 
          entry_pos+1, index->arr[entry_pos].term);

    /* print (doc, n) pairs associated with the term up to max pairs */
    while (pair != NULL) {
        if (pairs_printed >= MAX_PAIRS_STAGE1) break;
        if (pairs_printed > 0) printf("; ");
        printf("%d,%d", pair->doc, pair->n);
        /* move on to next recursive pair (or NULL) */
        pair = pair->next;
        pairs_printed++;
    }
    if (pairs_printed == MAX_PAIRS_STAGE1) printf("; ...");
    printf("\n");
}

/**************************************************************/

/* create an empty index_t struct and return a pointer to it
 */
index_t
*index_new() {
    index_t *index;
    index = (index_t*)calloc(1, sizeof(index_t));
    assert(index != NULL);
    return index;
}

/**************************************************************/

/* allocates more memory if necesarry, creates a copy of a index_entry_t struct
 * inserts it at the end of the index. also updates total npairs and counter
 * indicating how many entries are currently stored in the index 
 */
void
index_append(index_t *index, index_entry_t *new_entry) {
    /* first insersion, initiate array size to 1 */
    if (index->cap == 0) {
        index_init_arr(index);
    } else if (index->i >= index->cap) {
        index_double_arr(index);
    }
    index->arr[index->i] = *new_entry;
    (index->i)++;
    (index->npairs) += new_entry->pair_depth;
}

/**************************************************************/

/* doubles the memory avaliable for an array of entries within a index
 */
void
index_double_arr(index_t *index) {
    index_entry_t *new;
    new = (index_entry_t*)realloc(index->arr, 
                                  index->cap*2*sizeof(index_entry_t));
    assert(new != NULL);
    index->arr = new;
    index->cap *= 2;
}

/**************************************************************/

/* provides memory for a single index_entry inside index
 */
void
index_init_arr(index_t *index) {
    index->arr = (index_entry_t*)calloc(1, sizeof(index_entry_t));
    assert(index->arr != NULL);
    index->cap = 1;
}

/**************************************************************/

/* build entire input from file into the index_t structure. building split
 * into 4 stages; 1: reading a term. 2: reading ndocs. 3 and 4: reading
 * (doc, n) pairs. data is kept in temporary entry struct until it is completed
 * and copied into index, freed and used for the next entry
 */
void
index_write(index_t *index, char file_name[], buffer_t *buffer) {
    int stage=1;
    char c=' ';
    FILE *fp;
    fp = fopen(file_name, "r");

    /* initalise structures to be used */
    index_entry_t *new = entry_new();
    pair_t *tmp_pair = pair_new();

    while (c != EOF) {
        c = getc(fp);

        /* skip DOS 'cr' characters */
        if (c == 13) { /* do nothing */ }

        /* stage 1: during this stage term is read into new index_entry */
        else if (stage == 1) {
            stage = index_write_s1(new, buffer, c);

        /* stage 2: during this stage ndocs is read into new index_entry */
        } else if (stage == 2) {
            stage = index_write_s2(new, buffer, c);

        /* stage 3: during this stage doc is read into tmp_pair */
        } else if (stage == 3) {
            stage = index_write_s3(tmp_pair, buffer, c);

        /* stage 4: during this stage n is read into tmp_pair, and tmp_pair ptr
           added to new entry. stage 3 will be called if more pairs need to be
           added to new index_entry. once all pairs have been added new will be
           appended to index and we will continue onto reading further items
           in new. ndocs will also be checked, and updated if a higher value is 
           found */
        } else if (stage == 4) {
            stage = index_write_s4(index, &new, &tmp_pair, buffer, c);
        }
    }
    free(tmp_pair);
    free(new);
    buffer_end(buffer);
    fclose(fp);
}

/**************************************************************/

/* responsible for reading term to new entry. returns status which should be
 * execueted for next character read from file
 */
int
index_write_s1(index_entry_t *new, buffer_t *buffer, char c) {
    /* append char to buffer until a space is found */
    if (c != ' ') {
        buffer_append(buffer, c);
        return 1;
    /* end of term; terminate buffer and cpy it to new, move onto
       reading ndocs (stage 2) */
    } else {
        buffer_end(buffer);
        new->term = dup_array(buffer->arr, 0, strlen(buffer->arr));
        return 2;
    }
}

/**************************************************************/

/* responsible for reading ndocs to new. returns status which should be
 * executed for next character in file
 */
int
index_write_s2(index_entry_t *new, buffer_t *buffer, char c) {
    /* build ndocs into buffer */
    if (c != ' ') {
        buffer_append(buffer, c);
        return 2;
    /* reached the end of ndocs. send it to new, move onto stage 3 */
    } else {
        buffer_end(buffer);
        new->ndocs = atoi(buffer->arr);
        return 3;
    }
}

/**************************************************************/

/* responsible for reading doc to tmp_pair. returns status which should be
 * execueted for next character in file
 */
int
index_write_s3(pair_t *tmp_pair, buffer_t *buffer, char c) {
    /* build doc into buffer */
    if (c != ' ') {
        buffer_append(buffer, c);
        return 3;
    /* reached the end of doc. send it to tmp_pair */
    } else {
        buffer_end(buffer);
        tmp_pair->doc = atoi(buffer->arr);
        return 4;
    }
}

/**************************************************************/

/* responsible for reading n to tmp_pair, sending tmp_pair pointer to new
 * when pairs have been completed and determining when new has been
 * filled and needs to be appended to index 
 */
int
index_write_s4(index_t *index, index_entry_t **new, pair_t **tmp_pair,
                    buffer_t *buffer, char c) {
    /* check if we need to update ndocs */
    if ((*tmp_pair)->doc > index->ndocs) index->ndocs = (*tmp_pair)->doc;
    /* when end of line found, finish building tmp_pair and sending it to new,
       then append new index_entry to index and move onto building the next
       term_index */
    if (c == '\n') {
        buffer_end(buffer);
        entry_append_pair(new, tmp_pair, buffer);
        index_append(index, *new);
        free(*new);
        *new = entry_new();
        return 1;
    }
    else {
        /* build n into buffer */
        if (c != ' ') {
            buffer_append(buffer, c);
            return 4;
        /* n finished, but more pairs to be read. send tmp_pair to new
           and start reading the next pair */
        } else {
            buffer_end(buffer);
            entry_append_pair(new, tmp_pair, buffer);
            return 3;
        }
    }
}

/**************************************************************/

/* appends (doc, n) pair to entry
 */
void
entry_append_pair(index_entry_t **new, pair_t **tmp_pair, buffer_t *buffer) {
    (*tmp_pair)->n = atoi(buffer->arr);
    (*new)->pair = pair_append((*new)->pair, *tmp_pair);
    ((*new)->pair_depth)++;
    *tmp_pair = pair_new();
}

/**************************************************************/

/* allocate memory for a index_entry_t struct, return a pointer to it
 */
index_entry_t
*entry_new() {
    index_entry_t *index_entry;
    index_entry = (index_entry_t*)calloc(1, sizeof(*index_entry));
    assert(index_entry != NULL);
    return index_entry;
}

/**************************************************************/

/* reserve memory for a buffer_t struct, return a pointer to it
 */
buffer_t
*buffer_new() {
    buffer_t *buffer;
    buffer = (buffer_t*)calloc(1, sizeof(buffer_t));
    assert(buffer != NULL);
    buffer->i = 0;
    return buffer;
}

/**************************************************************/

/* prepares a buffer to be read by adding a null byte to the end of its array
 * and resets its counter so it's ready to be used for a new string 
 */
void 
buffer_end(buffer_t *buffer) {
        buffer->arr[buffer->i] = '\0';
        buffer->i = 0;
}

/**************************************************************/

/* new character, add it to buffer 
 */
void 
buffer_append(buffer_t *buffer, char c) {
    buffer->arr[buffer->i] = c;
    (buffer->i)++;
}

/**************************************************************/

/* reserve memory for a pair_t struct, return a pointer to it
 */
pair_t
*pair_new() {
    pair_t *pair;
    pair = (pair_t*)calloc(1, sizeof(*pair));
    assert(pair != NULL);
    return pair;
}

/**************************************************************/

/* finds the foot of a pair_t struct, and assigns a pair_t ptr to it
 */
pair_t
*pair_append(pair_t *curr_pair, pair_t *new_pair) {
    pair_t *head;
    /* first insersion */
    if (curr_pair == NULL) {
        return new_pair;
        }
    head = curr_pair;
    /* recursivly dive into the index to find the last item, append there */
    while (curr_pair->next != NULL) {
        curr_pair = curr_pair->next;
    }
    curr_pair->next = new_pair;
    return head;
}

/**************************************************************/

/* recursively frees all memory inside a pair type struct and returns a NULL
 * pointer
 */
pair_t
*pair_free(pair_t *pair) {
    if (pair->next != NULL) pair_free(pair->next);
    free(pair);
    return NULL;
}

/**************************************************************/

/* takes a pair_t type and returns the amount of (doc, n) pairs stored
 * recursivly inside
 */
int
pair_depth(pair_t *pair) {
    int depth=1;
    while (pair->next != NULL) depth++;
    return depth;
}

/**************************************************************/

/* takes an array, creates a duplicate (up to and including \0) and returns a
 * pointer to the duplicate array
 */
char
*dup_array(char arr[], int start, int finish) {
    int i;
    term_t new;

    new = (term_t)calloc((finish - start)+1, sizeof(char));


    for (i=0; i<finish - start; i++) {
        new[i] = arr[i+start];
    }

    return new;
}

/**************************************************************/

/* read a single character, but watching out for CR characters and
 * just discarding them if they are identified. If EOF is encountered
 * it is passed through, as are newline characters (whether composed
 * of CR+LF or just LF, doesn't matter)
 * written by Alister Moffat
 */
int
mygetchar() {
    int c;
    while ((c=getchar())=='\r') {
    }
    return c;
}

/**************************************************************/

/* free all memory used for index
 */
void index_free(index_t *index) {
    int i; 
    pair_t *head;
    pair_t *tmp;

    /* free each entry */
    for (i=0; i<index->i; i++) {
        free(index->arr[i].term);
        /* free all pairs in entry */
        head = index->arr[i].pair;
        while(head != NULL) {
            tmp = head;
            head = head->next;
            free(tmp);
        }
    }
    free(index->arr);
    free(index->doc_len);
    free(index);
}

/**************************************************************/

/* free all memory used for index
 */
void input_free(input_t *input) {
    int i; 

    /* free each term in term_list */
    for (i=0; i<input->nterms; i++) free(input->term_list[i]);
    free(input->term_list);
    free(input->term_index);
    free(input);
}

/* algorithms are fun! */
