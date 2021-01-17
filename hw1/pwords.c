/*
 * Word count application with one thread per input file.
 *
 * You may modify this file in any way you like, and are expected to modify it.
 * Your solution must read each input file from a separate thread. We encourage
 * you to make as few changes as necessary.
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

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <pthread.h>

#include "word_count.h"
#include "word_helpers.h"

word_count_list_t* wc;

void *p_count_words(void *file_name) {
  FILE *infile = fopen((char *)file_name, "r");
  printf("%s is processing.\n", (char *)file_name);
  count_words(wc, infile); 
  pthread_exit(NULL);
}

/*
 * main - handle command line, spawning one thread per file.
 */
int main(int argc, char *argv[]) {
  /* Create the empty data structure. */
  word_count_list_t word_counts;
  init_words(&word_counts);

  if (argc <= 1) {
    /* Process stdin in a single thread. */
    count_words(&word_counts, stdin);
  } else {
    /* TODO */
    int rc;
    int nthreads = argc - 1;
    pthread_t threads[nthreads];
    wc = &word_counts;
    
    for (int i = 0; i < nthreads; i++)
    {
      rc = pthread_create(&threads[i], NULL, p_count_words, (void *)argv[i + 1]);
      if (rc) {
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
      }
    }
    
    for (int i = 0; i < nthreads; i++)
    {
      rc = pthread_join(threads[i], NULL);
      if (rc)
      {
        printf("ERROR; return code from pthread_join() is %d\n", rc);
        exit(-1);
      }     
    }
    
  }

  // add_word(&word_counts, "aaaa");
  // add_word(&word_counts, "aaaA");
  // add_word(&word_counts, "bbb");
  // add_word(&word_counts, "ccc");
  // printf("len_words: %ld\n", len_words(&word_counts));
  // printf("find_word: %d\n", (find_word(&word_counts, "bbb"))->count);


  /* Output final result of all threads' work. */
  wordcount_sort(&word_counts, less_count);
  fprint_words(&word_counts, stdout);
  return 0;
}

