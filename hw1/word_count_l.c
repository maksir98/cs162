/*
 * Implementation of the word_count interface using Pintos lists.
 *
 * You may modify this file, and are expected to modify it.
 */

/*
 * Copyright © 2019 University of California, Berkeley
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PINTOS_LIST
#error "PINTOS_LIST must be #define'd when compiling word_count_l.c"
#endif

#include <string.h>
#include "word_count.h"

void init_words(word_count_list_t *wclist) {
  /* TODO */
  list_init(wclist);
}

size_t len_words(word_count_list_t *wclist) {
  /* TODO */
  return list_size(wclist);
}

word_count_t *find_word(word_count_list_t *wclist, char *word) {
  /* TODO */
  struct list_elem *e;

  for (e = list_begin(wclist); e != list_end(wclist); e = list_next(e))
  {
    struct word_count* current_word = list_entry(e, word_count_t, elem);
    if (!strcmp(current_word->word, word))
    {
      return current_word;
    }   
  }
  
  return NULL;
}

word_count_t *add_word(word_count_list_t *wclist, char *word) {
  /* TODO */
  word_count_t* word_found = find_word(wclist, word);
  if (word_found)
  {
    word_found->count++;
  } else
  {
    word_count_t *new_word = (word_count_t *)malloc(sizeof(word_count_t));
    new_word->word = word;
    new_word->count = 1;
    new_word->elem.next = NULL;
    new_word->elem.prev = NULL;
    list_push_front(wclist, &new_word->elem);
    return new_word;
  }
  
  return word_found;
}

void fprint_words(word_count_list_t *wclist, FILE *outfile) {
  /* TODO */
  struct list_elem *e;

  for (e = list_begin (wclist); e != list_end (wclist); e = list_next (e))
  {
    word_count_t *f = list_entry (e, word_count_t, elem);
    fprintf(outfile, "\t%d\t%s\n",f->count, f->word);
  }
}

static bool less_list(const struct list_elem *ewc1,
                      const struct list_elem *ewc2, void *aux) {
  /* TODO */
  word_count_t *wc1 = list_entry (ewc1, word_count_t, elem);
  word_count_t *wc2 = list_entry (ewc2, word_count_t, elem);

  if (strcmp(wc1->word, wc2->word) < 0)
  {
    return true;
  }

  return false;
}

void wordcount_sort(word_count_list_t *wclist,
                    bool less(const word_count_t *, const word_count_t *)) {
  list_sort(wclist, less_list, less);
}
