#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define BUFLEN 24
#define MAX_COUNT 15
#define NUMTHREAD 3 /* number of threads */
int cnt = 0;

// Small note: Packagers are doing FIFO. YES. But sometimes this problem is
// happening: So, when for example packager "1", is about to finish its size, the
// other packager type 1 is coming into game, and stealing first "1"'s pozition,
// and the books that packager "1" packaged are never being able to get printed.
// Because that "1" which was about to complete it's size, going into WAIT
// situation, and while waiting, other 1 is coming into game, and consuming the
// books which are belongs to that type If you want, with some editing i can show
// you that every packager contains some books. But not 6.

// Summarize: Not every packager is being able to complete its size.
// Diğer bir özet: Bir packager size'ını dolduramadanhem buffer bitmiş, hem
// kendisine ait packager kalmamış oluyor. Kalmayınca da gidip başkasını seçiyor
// zaten.

int number_of_types;       // Amount of publisher types
int random_number;         //
int number_for_each_type;  // Amount of publisher threads for each publisher
                           // type
int packager_size;  // max books that can each packager store
int buffer_size;    // Buffer size that user enters. This int will be passsed to
                  // array called "Max"
int num_of_books;      // Number of books for each publisher thread
int num_of_packagers;  // Number of packager thread
int bufferlen1[500];  // Array that stores buffer lenght for each type.(grows by
                      // time)
int Max[500];  // Array that stores maximum buffer lenght for each type.

int publisher_types_book_numbers[50];  // This will store created book amount
                                       // for each publisher type.

int Randoms(int low, int up, int counter) {
  int i;  // this function is allows us to generate random number.

  for (i = 0; i < counter; i++) {
    random_number = (rand() % (up - low + 1)) + low;
  }
  return random_number;
}
int return_value = 0;
int cnt2 = 0;
struct book {  // struct for storing each book.
  int type;
  int sira;
  int available;
};
struct thread {  // struct for storing each publisher thread.
  int tid;
  int type;  // type of thread
  int sira;
  int created_books;  // created books for each thread. We are declearing this
                      // for making sure that created books are not exceeding max
                      // book number that each thread should produce. If it
                      // exceeds, we will make thi producer thread exit the
                      // system.
};

struct packager {  // struct for storing each packager thread.
  int tid;
  int type;
  int random_choice_of_thread_type;  // type of package which will be selected
                                     // randomly.
  char everything[500][100];  // this represents books that packager packages
                              // (package for each package).
  int size_of_package;
};

struct thread publishers[100];
struct book books[100];

void *consumer(struct packager *thread_object);
void *producer(
    struct thread *thread_object);  // creating consumer and producer functions

// Declearing Conditional variables.
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;

void *consumer(struct packager *thread_object) {  // Packager thread function
  ///////////////////////////////////////////
  while (1) {  // while loop until all publishers finish.
    pthread_mutex_lock(
        &count_mutex);  // we are locking for not allowing any other packager
                        // thread enter while loop while some other packager
                        // thread is in this function.
    int k = 0;
    while (k ==
           0) {  // There is for loop for checking if buffer lenght of
                 // thread_object is empty, then select another. Do it until it
                 // finds one. If it cant find anything,(if every packager thread
                 // exited system and there is no size for packaging),then it
                 // will exit the system with exit message.
      while (thread_object->size_of_package <
             packager_size) {  // this while loop is for checking if packager
                               // size is full for packager (thread_object).
        int flag = 0;
        // this flag is for checking algorithm.

        if (bufferlen1[thread_object->random_choice_of_thread_type] ==
            0) {  // If type's bufferlenght is 0 (which is empty).
          for (int t = 0; t < number_for_each_type * number_of_types; t++) {
            if (thread_object->random_choice_of_thread_type ==
                publishers[t].type) {
              if (publishers[t].created_books != num_of_books) {
                flag = 1;
                break;
              }
            }
          }
          if (flag == 0) {  // If type is not in publishers array,

            int number = Randoms(0, number_of_types - 1, 1);
            // we are changin type of packager with another type with
            // randomizing.
            thread_object->random_choice_of_thread_type = number;

            if (cnt2 >= number_of_types * number_for_each_type * num_of_books) {
              // if there are no publisher left in system.
              printf(
                  "There are no publishers left in the system. The package "
                  "contains:\n");
              for (int i = 0;
                   i < number_for_each_type * number_of_types * num_of_books;
                   i++) {
                if (thread_object->everything[i][0] == 'B') {
                  printf("%s\n", thread_object->everything[i]);
                }
              }
              printf("Exiting the system");
              exit(0);
            }
            cnt2++;
          } else {
            // else, if bufferlenght is 0, but there is still publishers in
            // system, we will wait.
            pthread_cond_wait(&fill, &count_mutex);
          }

        } else {
          // if everything is okay, bufferlenght is not 0, we will package with
          // corresponding packager type.
          for (int i = 0;
               i < number_for_each_type * number_of_types * num_of_books;
               i++) {  // publisherlari geziyor,
            if (books[i].type != 99 && books[i].type != 88) {
              // 99 in books function means "EMPTY", 88 means "This book has
              // been packaged".So if book is not packaged or the place in books
              // array is not empty, consume, package that book.
              if (books[i].type ==
                  thread_object->random_choice_of_thread_type) {
                for (int k = 0;
                     k < number_for_each_type * number_of_types * num_of_books;
                     k++) {
                  if (strcmp(thread_object->everything[k], "empty") == 0) {
                    // everything represents books that packager packaged.We are
                    // traversing through this array and finding empty place to
                    // put book, when we find, we are putting book to package.
                    printf(
                        "Packager %d         Put Book%d_%d into the package\n",
                        thread_object->random_choice_of_thread_type,
                        books[i].type, books[i].sira);

                    cnt++;
                    char s1[20];
                    char s2[20];
                    sprintf(s1, "Book%d_", books[i].type);  // type
                    sprintf(s2, "%d", books[i].sira);
                    // we are copying book to books, and copying book to
                    // "everything" array which represents package for each
                    // packager.

                    strcpy(thread_object->everything[i], strcat(s1, s2));

                    bufferlen1[thread_object->random_choice_of_thread_type] -=
                        1;
                    thread_object->size_of_package++;

                    books[i].type = 88;  // marking book as "CONSUMED,
                                         // PACKAGED."

                    break;
                  }
                }
                break;
              }
            }
          }
        }
      }
      // after finishing preparing one package, we are getting out of "while
      // (thread_object->size_of_package < packager_size"-> this  while loop. And
      // we are traversing through every book that that package packaged.
      printf(
          "Packager %d         Finished preparing one package. The package "
          "contains:\n",
          thread_object->random_choice_of_thread_type);
      for (int i = 0; i < number_for_each_type * number_of_types * num_of_books;
           i++) {
        if (thread_object->everything[i][0] == 'B') {
          printf("%s\n", thread_object->everything[i]);
        }
      }
      // after that for making that thread be able to run again, we are making
      // its size 0 and marking every position in "everything" array of packager
      // : empty.
      thread_object->size_of_package = 0;
      for (int i = 0; i < number_for_each_type * number_of_types * num_of_books;
           i++) {
        strcpy(thread_object->everything[i], "empty");
      }
    }

    pthread_mutex_unlock(&count_mutex);  // unlocking
  }
}

void *producer(struct thread *thread_object) {
  ///////////////////////////////////////////
  /* lock the variable */
  for (int i = 0; i < num_of_books; i++) {  // this represents how many books
                                            // will each publisher will publish.

    pthread_mutex_lock(
        &count_mutex);  // locking for loop for blocking other producers entry
                        // to for loops of each other.

    if (bufferlen1[thread_object->type] >= Max[thread_object->type]) {
      // if bufferlenght is full, we are doubling size.
      printf(
          "Publisher %d of type%d         Buffer is full. Resizing the "
          "buffer\n",
          thread_object->type, thread_object->sira);
      Max[thread_object->type] = Max[thread_object->type] * 2;
      for (int k = 0; k < number_for_each_type * number_of_types; k++) {
        if (publishers[k].tid == thread_object->tid) {
          for (int i = 0;
               i < number_for_each_type * number_of_types * num_of_books; i++) {
            if (books[i].type == 99) {
              // After doubling size, we are adding publisher type's book into
              // books array. If the place is empty which represented by :99.

              // increasing buffer.
              bufferlen1[thread_object->type] += 1;
              publisher_types_book_numbers[thread_object->type] += 1;
              // saying that this publisher created 1 book.
              publishers[k].created_books++;
              books[i].type = thread_object->type;

              // in here, sira->order of book. So order of each type of book
              // will be stored in array called "publisher_types_book_numbers"
              books[i].sira = publisher_types_book_numbers[thread_object->type];

              printf(
                  "Publisher %d of type %d   Book%d_%d is published and put "
                  "into buffer%d\n",
                  thread_object->sira, thread_object->type, books[i].type,
                  books[i].sira, thread_object->type);

              break;
            }
          }
          break;
        }
      }
    } else {
      // if bufferlen is not full
      for (int k = 0; k < number_of_types * number_for_each_type; k++) {
        if (publishers[k].tid == thread_object->tid) {
          // in here, we are finding thread object in publishers array.For fully
          // modify and access.

          for (int i = 0;
               i < number_for_each_type * number_of_types * num_of_books; i++) {
            if (books[i].type == 99) {
              // if place is empty,
              bufferlen1[thread_object->type] += 1;
              publisher_types_book_numbers[thread_object->type] += 1;
              books[i].type =
                  thread_object
                      ->type;  // burada publishers[i] olması gerekebilir.
                               // publishers [i] olunca çalışmazsa çok büyük bir
                               // problem var demektir.
              books[i].sira = publisher_types_book_numbers[thread_object->type];
              publishers[k].created_books++;

              printf(
                  "Publisher %d of type %d   Book%d_%d is published and put "
                  "into buffer%d\n",
                  thread_object->sira, thread_object->type, books[i].type,
                  books[i].sira, thread_object->type);
              if (publishers[k].created_books == num_of_books) {
                printf(
                    "Publisher %d of type %d   Finished publishing 5 books. "
                    "Exiting the system\n",
                    thread_object->sira, thread_object->type);
              }

              break;
            }
          }
          break;
        }
      }

      pthread_cond_signal(&fill);
      // after adding, we are signalling consumer and removing consumer's
      // (packager's) wait situation.
    }

    pthread_mutex_unlock(&count_mutex);
  }
}

int parse_input(int argc, char *argv[], int castedIntN[3], int castedIntS[2],
                int *bookNum) {
  if (argc < 10) {
    fprintf(
        stderr, "%s\n",
        "An error occured, you have not entered all the required arguments");
    return -1;  // makes sure user enters all the inputs
  }
  int i = 1, j = 0, foundN = 0, foundB = 0, foundS = 0;
  while (i < argc) {  // goes through the inputs
    if (strcmp(argv[i], "-n") == 0) {
      foundN = 1;  // tells the program that it found the n arguement
      int counter = 0;
      for (j = i + 1; j < 4 + i; j++) {
        if (atoi(argv[j]) ==
            0) {  // if the following char is 0 or non digit char
          fprintf(stderr, "%s\n",
                  "You've entered incorrect value/s for -n");  // warn user
          return -1;
        }
        castedIntN[counter] =
            atoi(argv[j]);  // stores the casted values of the n arg
        counter++;
      }
      i = j - 1;  // increments i to the next arg
    } else if (strcmp(argv[i], "-b") == 0) {
      foundB = 1;
      if (atoi(argv[i + 1]) == 0) {  // error handling
        fprintf(stderr, "%s\n", "You've entered an incorrect input for -b");
        return -1;
      }
      *bookNum = atoi(argv[i + 1]);
    } else if (strcmp(argv[i], "-s") == 0) {
      foundS = 1;
      if (atoi(argv[i + 1]) == 0) {
        fprintf(stderr, "%s\n", "You've entered an incorrect input for -s");
        return -1;
      } else if (atoi(argv[i + 2]) == 0) {
        fprintf(stderr, "%s\n", "You've entered an incorrect input for -s");
        return -1;
      }
      castedIntS[0] = atoi(argv[i + 1]);
      castedIntS[1] = atoi(argv[i + 2]);
    }
    i++;
  }

  if (!foundN) {  // checks if the user gave all the args, if not warn him/her
    fprintf(stderr, "%s\n", "You must give the -n argument");
    return -1;
  } else if (!foundB) {
    fprintf(stderr, "%s\n", "You must give the -b argument");
    return -1;
  } else if (!foundS) {
    fprintf(stderr, "%s\n", "You must give the -s argument");
    return -1;
  }
}

int main(int argc, char *argv[]) {
  int nInputs[3], sInputs[2], bInput = 0;  // stores the inputs of each
                                           // arguement
  if (parse_input(argc, argv, nInputs, sInputs, &bInput) ==
      -1)  // parses the input to validate it
    return -1;

  number_of_types = nInputs[0];
  number_for_each_type = nInputs[1];
  packager_size = sInputs[0];
  buffer_size = sInputs[1];
  num_of_books = bInput;
  num_of_packagers = nInputs[2];
  // in here we are putting inputs to variables.

  for (int i = 0; i < number_of_types * number_for_each_type * num_of_books;
       i++) {
    struct book *a = malloc(sizeof *a);
    // preparing books array.
    a->type = 99;
    a->sira = 99;
    a->available = 0;
    books[i] = *a;
  }  // making every element " " in books array

  for (int j = 0; j < number_of_types; j++) {
    bufferlen1[j] = 0;
    Max[j] = buffer_size;
    publisher_types_book_numbers[j] = 0;
  }

  /////////////////////////////////////////////////////////////

  // 5 kitap

  pthread_t producer_threads1[50];
  pthread_t consumer_threads[50];

  int l = 0;  // duzgun count
  for (int j = 0; j < number_of_types; j++) {
    for (int k = 0; k < nInputs[1]; k++) {
      // preparing publishers array and creating publisher threads.
      struct thread *a = malloc(sizeof *a);  // for the first
      a->type = j;
      a->sira = k;
      a->tid = l;
      a->created_books = 0;
      publishers[l] = *a;

      pthread_create(&producer_threads1[l], NULL, (void *)producer, a);

      l = l + 1;

    }  ///
    ////////////////////////////////////////////////////
  }
  for (int k = 0; k < num_of_packagers; k++) {
    struct packager *d = malloc(sizeof *d);
    // creating packager threads.
    d->type = k;
    d->tid = k + 100;
    d->random_choice_of_thread_type = Randoms(0, number_of_types - 1, 1);

    for (int i = 0; i < number_for_each_type * number_of_types * num_of_books;
         i++) {
      strcpy(d->everything[i], "empty");
    }

    pthread_create(&consumer_threads[k], NULL, (void *)consumer, d);
  }

  for (int i = 0; i < num_of_packagers;

       i++) {
    pthread_join(consumer_threads[i], NULL);
    // DONT EXIT PROGRAM UNTIL EVERY CONSUMER(PACKAGER) FINISH
  }
  for (int i = 0; i < number_for_each_type * number_of_types;
       i++) {  // buralar doğru, kafana takma.
    pthread_join(producer_threads1[i], NULL);
    // DONT EXIT PROGRAM UNTIL EVERY PRODUCER (PUBLISHER) FINISH
  }
}
