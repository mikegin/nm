#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint32_t u32;

typedef struct Link {
  u32 id;
  char * start;
  char * end;
  u32 capacity;
  u32 weight;
} Link;

// void addName(char ** names, int size, char * name)
// {
//   for (int i = 0; i < size; i++)
//   {
//     if (strcmp(names[i], name) == 0)
//     {
//       return; // exit if found
//     }
//   }

//   names[size] = name;
// }

int main(int argc, char ** args)
{
  char * filename = (char *)"input.csv";
  FILE * fp = fopen(filename, "rb");
  int maxLineLength = 1024;
  char buffer[maxLineLength];

  int maxLineSize = 1024;
  Link * links[maxLineSize];
  // char ** names = (char **)malloc(2 * maxLineLength * sizeof(char *)); // since 2 characters can be per line, start and end
  // int size = 0;

  int line = 0;
  while (fgets(buffer, maxLineLength, fp))
  {
    if (line != 0) // skip header
    {
      char * token = strtok(buffer, ",");
      int pos = 0;
      Link * link = (Link *)malloc(sizeof(Link));
      links[line] = link;
      while (token)
      {
        // printf("%s ", token);
        switch(pos) {
          case 0: {
            link->id = atoi(token);
#ifdef DEBUG
            printf("%d ", link->id);
#endif            
            break;
          }
          case 1: {
            // Allocate memory properly for the string and check for allocation failure
            link->start = (char *)malloc(strlen(token) + 1); // +1 for the null terminator
            if (link->start == NULL) {
              fprintf(stderr, "Memory allocation failed\n");
              return EXIT_FAILURE;
            }
            strcpy(link->start, token);
#ifdef DEBUG
            printf("%s ", link->start);
#endif
            break;
          }
          case 2: {
            // Similarly allocate memory for link->end
            link->end = (char *)malloc(strlen(token) + 1);
            if (link->end == NULL) {
              fprintf(stderr, "Memory allocation failed\n");
              return EXIT_FAILURE;
            }
            strcpy(link->end, token);
#ifdef DEBUG
            printf("%s ", link->end);
#endif
            break;
          }
          case 3: {
            link->capacity = atoi(token);
#ifdef DEBUG            
            printf("%d ", link->capacity);
#endif            
            break;
          }
          case 4: {
            link->weight = atoi(token);
#ifdef DEBUG            
            printf("%d ", link->weight);
#endif            
            break;
          }
        }
        token = strtok(NULL, ",");
        pos += 1;
      }
#ifdef DEBUG
      printf("\n");
#endif
    }

    line += 1;
  }

  // find shortest path from node A to node F
  // TODO: specify this via user input somehow

  char * node1 = "A";
  char * node2 = "F";

  

  return 0;
}
