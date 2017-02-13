/* COMP10002 Assignment 1 2016
 * Implements sorting and formated printing of a .tsv file
 *
 * Written by Liam Aharon, September, 2016
 *
 * credit to Alister Moffart for mygetchar()
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* After generating my own 1000 row 30 col 50 char full test file I found that
errors would occur if my max constant values were anything less than these.
They are this size to make my code compatible with large test cases, and not
just random values as they may at first appear */

#define MAX_ROW 1002  /* amount of rows required for 1000 data rows */
#define MAX_COL 39    /* amount of cols required for 30 data cols */
#define MAX_CHAR 59   /* amount of chars required for 50 chars per entry */
#define SPACING_10 10 /* spacing used in various stages of printing */

/****************************************************************/

/* function prototypes */

void tsv2array(char arr[][MAX_COL][MAX_CHAR], int *col_amt, int *row_amt);
void stage1out(char arr[][MAX_COL][MAX_CHAR], int col_amt, int row_amt, 
    int argc);
void argv2array(int sort[], int argc, char *argv[]);
void rowswp(char arr[][MAX_COL][MAX_CHAR], int row1, int row2, int row_amt);
void sort2Darray(char arr[][MAX_COL][MAX_CHAR], int sort[], int s_size,
    int row_amt, int col_amt);
void stage2out(char arr[][MAX_COL][MAX_CHAR], int col_amt, int row_amt);
void stage3out(char arr[][MAX_COL][MAX_CHAR], int sort[], int col_amt, 
    int row_amt, int s_size);
int mygetchar();
void print_array(char arr[][MAX_COL][MAX_CHAR], int col_amt, int row_amt);
int s3_horizontal_size(char arr[][MAX_COL][MAX_CHAR], int row_amt,  int sort[], 
    int s_size);
int rowcmp(char row1[MAX_COL][MAX_CHAR], char row2[MAX_COL][MAX_CHAR], 
    int sort[], int s_size);
void print_s3_separators(int h_size);

/****************************************************************/

/* main program
 */

int
main(int argc, char *argv[]) {

    int row_amt=0, col_amt=0, sort[MAX_COL], s_size=argc-1;

    char arr[MAX_ROW][MAX_COL][MAX_CHAR];

    tsv2array(arr, &col_amt, &row_amt);
    stage1out(arr, col_amt, row_amt, argc);
    /* check if we need to print stage 2 & 3 */
    if (argc > 1) {
        argv2array(sort, argc, argv);
        sort2Darray(arr, sort, s_size, row_amt, col_amt);
        stage2out(arr, col_amt, row_amt);
        stage3out(arr, sort, col_amt, row_amt, s_size);
    }

    return 0;
}

/****************************************************************/

/* put .tsv data into a 2D array [row][col][char]
 */

void 
tsv2array(char arr[][MAX_COL][MAX_CHAR], int *col_amt, int *row_amt) {

    int str_pos=0, header=1, curr_col=0;

    char input;

    while ((input = mygetchar()) != EOF) {

        /* when new col */
        if (input == '\t') {
            /* add null byte to end of array before moving to next one */
            str_pos++;
            arr[*row_amt][curr_col][str_pos] = '\0';
            
            str_pos=0;
            curr_col++;

        /* when new row */
        } else if (input == '\n') {
            /* At the end of reading the first line record number of columns */
            if (header) {
                *col_amt = curr_col;
                header = 0;
            } 

            /* add null byte to end of array before moving to next one */
            str_pos++;
            arr[*row_amt][curr_col][str_pos] = '\0';

            str_pos=0;
            (*row_amt)++;
            curr_col = 0;

        /* when new char in str */
        } else {
            arr[*row_amt][curr_col][str_pos] = input;
            str_pos++;

        }
    }

    /* account for no \t in 1st column and empty last row in input */
    (*col_amt)++;
    (*row_amt)--;
}

/****************************************************************/

/* print Stage 1 Output
 */

void
stage1out (char arr[][MAX_COL][MAX_CHAR], int col_amt, int row_amt, int argc) {
    int i;
    printf("Stage 1 Output\n");
    printf("input tsv file has %d rows and %d columns\n", 
        row_amt, col_amt);
    printf("row %d is:\n", 
        row_amt);

    for (i=0; i<col_amt; i++) {
        /* if no argv values input do not leave white space after last string
        on each line, like Alister did in the example output files */
        if (argc > 1) {
            printf("   %d: %-*s %-*s\n", 
                i+1, SPACING_10, arr[0][i], SPACING_10, arr[row_amt][i]);
        } else {
            printf("   %d: %-*s %s\n", 
                i+1, SPACING_10, arr[0][i], arr[row_amt][i]);
        }
    }
    printf("\n");
}

/****************************************************************/

/* sorts a 2D array hierarchically in order specified in sort[]
 */

void 
sort2Darray(char arr[][MAX_COL][MAX_CHAR], int sort[], int s_size,
    int row_amt, int col_amt) {

    int i, j, pt1, pt1tmp, pt2, cur_key, prev_key, ok_to_swap=1;

    /* insersion sort on the 2D array, with added hierarchical functionality */

    /* iterate through the sorting hierarchy */
    for (i=0; i<s_size; i++) {
        cur_key = sort[i];

        /* start at point 3 and 2, as point 0 in the array is the header */
        for (pt1=3; pt1<=row_amt; pt1++) {
            pt2 = pt1-1;
            pt1tmp = pt1;

            while (pt2>0){

                if (strcmp(arr[pt2][cur_key-1], arr[pt1tmp][cur_key-1])>0) {

                    /* before sorting check if already sorted at higher
                    hierarchical level order */

                    for (j=0; j<i; j++) {
                        prev_key = sort[j];

                        if (strcmp(arr[pt1tmp][prev_key-1], 
                            arr[pt2][prev_key-1])>0) {
                            ok_to_swap = 0;
                            break;
                        }
                    }

                    if (ok_to_swap) {
                        rowswp(arr, pt1tmp, pt2, col_amt);
                    }
                }
                ok_to_swap = 1;
                pt2--;
                pt1tmp--;
            }
        }
    }
}

/****************************************************************/

/* compare columns in sort[] between 2 rows. If same return 1 else return 0
 */

int
rowcmp(char row1[MAX_COL][MAX_CHAR], char row2[MAX_COL][MAX_CHAR], int sort[], 
    int s_size) {

    int i, key;
    /* check each col in sort[] */
    for (i=0; i<s_size; i++) {
        key = sort[i]-1;
        if ((strcmp(row1[key], row2[key])) != 0) {
            return 0;
        }
    }
    return 1;
}

void
stage3out(char arr[][MAX_COL][MAX_CHAR], int sort[], int col_amt, int row_amt, 
    int s_size) {

    char tmp_row[MAX_COL][MAX_CHAR]={"\0"};

    int i, j, key, tmp_key, l_dist, cnt=0, row, k,
    h_size=s3_horizontal_size(arr, row_amt, sort, s_size);

    printf("Stage 3 Output\n");

    print_s3_separators(h_size);

    /* print header. l_dist is the pos of the last char before we need to
    print "Count", used for calculating spacing before count */
    for (i=0; i<s_size; i++) {

        l_dist=0;

        key = sort[i]-1;
        if (i>0) {
            printf("\n");
            for (j=i; j>0; j--) {
                printf("    ");
                l_dist += 4;
            }
        }

        printf("%s", arr[0][key]);
        l_dist += strlen(arr[0][key]);

    }

    printf("%*s", h_size-l_dist+1, "Count\n");

    print_s3_separators(h_size);

    /* start printing body */

    /* check every row */

    for (row=1; row<=row_amt; row++) {

        if (row > 1 && rowcmp(arr[row], tmp_row, sort, s_size) == 0) {

            printf("%5d\n", cnt);
            cnt = 0;
        }

        for (i=0; i<s_size; i++) {
            key = sort[i]-1;

            if ((strcmp(arr[row][key], tmp_row[key])) != 0) {

                /* print spacing before each element */

                for (k=0; k<i; k++) {
                    printf("    ");
                }

                /* print element and additional spacing as needed */
                if (i==s_size-1) {
                    printf("%-*s", h_size-i*4-5, arr[row][key]);
                } else {
                    printf("%s", arr[row][key]);
                }
                /* print newline if count not about to be printed */
                if (i != s_size-1) {
                    printf("\n");
                }

                /* reset everything lower heirarchy */
                strcpy(tmp_row[key], arr[row][key]);
                for (j=i+1; j<s_size; j++) {
                    tmp_key = sort[j]-1;
                    strcpy(tmp_row[tmp_key], "\0");
                }
            }
        }

        cnt++;
    }

    printf("%5d\n", cnt);
    
    print_s3_separators(h_size);

}

/****************************************************************/

/* print stage 2 output
 */

void
stage2out(char arr[][MAX_COL][MAX_CHAR], int col_amt, int row_amt) {

    int i;
    int mid = (int)ceil(row_amt*0.5);

    printf("Stage 2 Output\n");

    printf("row 1 is:\n");
    for (i=0; i<col_amt; i++) {

        printf("   %d: %-*s %-*s\n", 
            i+1, SPACING_10, arr[0][i], SPACING_10, arr[1][i]);
    }

    printf("row %d is:\n", mid);
    for (i=0; i<col_amt; i++) {

        printf("   %d: %-*s %-*s\n", 
            i+1, SPACING_10, arr[0][i], SPACING_10, arr[mid][i]);
    }

    printf("row %d is:\n", row_amt);
    for (i=0; i<col_amt; i++) {

        printf("   %d: %-*s %-*s\n", 
            i+1, SPACING_10, arr[0][i], SPACING_10, arr[row_amt][i]);
    }

    printf("\n");
}

/****************************************************************/

/* prints the arr array. used for debugging
 */

void
print_array(char arr[][MAX_COL][MAX_CHAR], int col_amt, int row_amt) {

    int i, j;

    for (i=0; i<=row_amt; i++) {

        for (j=0; j<=col_amt; j++) {
            printf("\t %s", arr[i][j]);

        }
        printf("\n");
    }
    
}

/****************************************************************/

/* returns amount of '-' required for stage 3 output 
 */

int
s3_horizontal_size(char arr[][MAX_COL][MAX_CHAR], int row_amt, int sort[], 
    int s_size) {

    int longest=0, i, j, k='a', spacing_cnt=0, cnt=0, col;

    /* Nested loop to check len of every string in every row that will be 
    included in S3 output */
    for (i=0; i<row_amt; i++) {

        for (j=0; j<s_size; j++) {
            col = sort[j]-1;

            /* j*4 accounts for indentation, -1 because null byte is treated as 
            a character in this algorithm and we do not want to use it */
            spacing_cnt = (j*4)-1; 

            /* Add 1 to spacing_cnt for every character in currently examined
            string */
            while (k != '\0') {
                k = arr[i][col][cnt];
                spacing_cnt++;
                cnt++;
            }
            if (spacing_cnt > longest) {
                longest = spacing_cnt;
            }
            cnt = 0;
            k = 'a';
        }
    }

    /* Amount of '-' is 6+last_char_pos, so we add 6 to the returned value */
    return longest+6;
}

/****************************************************************/

/* adds command line argv inputs to their own array
 */

void
argv2array(int sort[], int argc, char *argv[]) {

    int i;
    for (i=0; i<argc-1; i++) {
        sort[i] = atoi(argv[i+1]);
    }
}

/****************************************************************/

/* swaps 2 rows from inside a 2D array
 */

void
rowswp(char arr[][MAX_COL][MAX_CHAR], int row1, int row2, int col_amt) {
    char tmp[MAX_COL][MAX_CHAR];
    int i;

    for (i=0; i<=col_amt; i++) {
        strcpy(tmp[i], arr[row1][i]);
        strcpy(arr[row1][i], arr[row2][i]);
        strcpy(arr[row2][i], tmp[i]);
    }
}

/****************************************************************/

/* prints line of '-' characters of appropriate length when needed in
 * in stage 3
 */

void
print_s3_separators(int h_size) {
    int i;
    
    for (i=0; i<h_size; i++) {
        printf("-");
    }
    printf("\n");
}

/****************************************************************/

/* modified getchar for greater compatibility
 */

int
mygetchar() {
    int c;
    while ((c=getchar())=='\r') {
    }
    return c;
}