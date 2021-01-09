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

word_count provides lists of words and associated count

Functional methods take the head of a list as first arg.
Mutators take a reference to a list as first arg.
*/

#include <string.h>
#include "word_count.h"

/* Basic utililties */

char *new_string(char *str) {
  return strcpy((char *)malloc(strlen(str)+1), str);
}

void init_words(WordCount **wclist) {
  /* Initialize word count.  */
  WordCount* initList = (WordCount*) malloc(sizeof(WordCount));
  initList->word = NULL;
  initList->next = NULL;
  initList->count = 0;

  *wclist = initList;
}

size_t len_words(WordCount *wchead) {
    size_t len = 0;

    while (wchead)
    {
      len++;
      wchead = wchead->next;
    }  
    
    return len;
}

WordCount *find_word(WordCount *wchead, char *word) {
  /* Return count for word, if it exists */
  WordCount *wc = NULL;

  while (wchead != NULL)
  {
    if (!strcmp(wchead->word, word))
    {
      wc = wchead;
      break;
    }
    
    wchead = wchead->next;
  }
  
  return wc;
}

void add_word(WordCount **wclist, char *word) {
  /* If word is present in word_counts list, increment the count, otw insert with count 1. */
  bool is_exist = false;
  WordCount* current_word = *wclist;
  
  while (1)
  {
    if (!strcmp(current_word->word, word))
    {
      current_word->count ++;
      is_exist = true;
      break;
    }
    
    if (!current_word->next)
    {
      break;
    }

    current_word = current_word->next;
  }
  
  if (!is_exist)
  {
    WordCount* new_word = (WordCount*) malloc(sizeof(WordCount));
    new_word->word = word;
    new_word->count = 1;
    new_word->next = NULL;
    current_word->next = new_word;
  }
  
  
}

void fprint_words(WordCount *wchead, FILE *ofile) {
  /* print word counts to a file */
  WordCount *wc;
  for (wc = wchead; wc; wc = wc->next) {
    fprintf(ofile, "%i\t%s\n", wc->count, wc->word);
  }
}
