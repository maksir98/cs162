/*

  Word Count using dedicated lists

*/

/*
Copyright © 2019 University of California, Berkeley

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <assert.h>
#include <getopt.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>

#include "word_count.h"

/* Global data structure tracking the words encountered */
WordCount *word_counts = NULL;

/* The maximum length of each word in a file */
#define MAX_WORD_LEN 64

/*
 * 3.1.1 Total Word Count
 *
 * Returns the total amount of words found in infile.
 * Useful functions: fgetc(), isalpha().
 */
int num_words(FILE* infile) {
  int num_words = 0;
  int c;
  int word_length = 0;

  c = fgetc(infile);
  while (c != EOF)
  {

    if (isalpha(c))
    {
      word_length ++;
    } else if (word_length > 1)
    {
      num_words ++;
      word_length = 0;
    } else
    {
      word_length = 0;
    }
    c = fgetc(infile);
  }

  if (word_length > 1)
  {
    num_words ++;
  }
  
  return num_words;
}

/*
 * 3.1.2 Word Frequency Count
 *
 * Given infile, extracts and adds each word in the FILE to `wclist`.
 * Useful functions: fgetc(), isalpha(), tolower(), add_word().
 */
void count_words(WordCount **wclist, FILE *infile) {
  bool is_new_list = ((*wclist)->word == NULL);
  int word_length = 0;
  char* input_buffer = (char*) malloc((MAX_WORD_LEN + 1) * sizeof(char));
  int c;

  c = fgetc(infile);
  while (1)
  {
    
    if (isalpha(c))
    {
      *(input_buffer + word_length) = tolower(c);
      word_length ++;
    } else if (word_length <= 1)
    {
      // not a word
      word_length = 0;
    } else
    {
      // find a word
      *(input_buffer + word_length) = '\0';

      if (is_new_list)
      {
        (*wclist)->word = input_buffer;
        (*wclist)->count = 1;
        (*wclist)->next = NULL;
        is_new_list = false;
        word_length = 0;
        input_buffer = (char*) malloc((MAX_WORD_LEN + 1) * sizeof(char));     
        continue;
      }

      add_word(wclist, input_buffer);
      word_length = 0;
      input_buffer = (char*) malloc((MAX_WORD_LEN + 1) * sizeof(char));   

      if (c == EOF) {
        break;        
      }   
    }
    c = fgetc(infile);
  }

  free(input_buffer);
  
}

/*
 * Comparator to sort list by frequency.
 * Useful function: strcmp().
 */
static bool wordcount_less(const WordCount *wc1, const WordCount *wc2) {
  char* word1 = wc1->word;
  char* word2 = wc2->word;
  int cmp;

  if (word1 && word2)
  {
    cmp = strcmp(word1, word2);
  } else if (!word1)
  {
    cmp = 1;
  } else
  {
    cmp = -1;
  }

  if (cmp < 0)
  {
    return true;
  }
  
  return false;
}

// In trying times, displays a helpful message.
static int display_help(void) {
	printf("Flags:\n"
	    "--count (-c): Count the total amount of words in the file, or STDIN if a file is not specified. This is default behavior if no flag is specified.\n"
	    "--frequency (-f): Count the frequency of each word in the file, or STDIN if a file is not specified.\n"
	    "--help (-h): Displays this help message.\n");
	return 0;
}

/*
 * Handle command line flags and arguments.
 */
int main (int argc, char *argv[]) {

  // Count Mode (default): outputs the total amount of words counted
  bool count_mode = true;
  int total_words = 0;

  // Freq Mode: outputs the frequency of each word
  bool freq_mode = false;

  FILE *infile = NULL;

  // Variables for command line argument parsing
  int i;
  static struct option long_options[] =
  {
      {"count", no_argument, 0, 'c'},
      {"frequency", no_argument, 0, 'f'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
  };

  // Sets flags
  while ((i = getopt_long(argc, argv, "cfh", long_options, NULL)) != -1) {
      switch (i) {
          case 'c':
              count_mode = true;
              freq_mode = false;
              break;
          case 'f':
              count_mode = false;
              freq_mode = true;
              break;
          case 'h':
              return display_help();
      }
  }

  if (!count_mode && !freq_mode) {
    printf("Please specify a mode.\n");
    return display_help();
  }

  /* Create the empty data structure */
  init_words(&word_counts);

  if ((argc - optind) < 1) {
    // No input file specified, instead, read from STDIN instead.
    infile = stdin;
  } else {
    // At least one file specified. Useful functions: fopen(), fclose().
    // The first file can be found at argv[optind]. The last file can be
    // found at argv[argc-1].   
    infile = fopen(argv[optind], "r");
  }

  if (count_mode) {
    total_words = num_words(infile);
    printf("The total number of words is: %i\n", total_words);
  } else {
    count_words(&word_counts, infile);
    wordcount_sort(&word_counts, wordcount_less);

    printf("The frequencies of each word are: \n");
    fprint_words(word_counts, stdout);
  }
  return 0;
}
