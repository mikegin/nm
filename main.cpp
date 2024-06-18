#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

typedef uint32_t u32;

typedef struct Link {
  u32 id;
  char * start;
  char * end;
  u32 capacity;
  u32 weight;
} Link;

typedef struct Links {
  Link ** values;
  u32 size;
} Links;

typedef struct TrafficLink {
  char * source;
  char * destination;
  u32 demand;
} TrafficLink;

typedef struct TrafficLinks {
  TrafficLink ** values;
  u32 size;
} TrafficLinks;

/**
 * This uses Dijakstra's shortest path algorithm
*/
void findShortestPath(Links * links, Links * path, char * node1, char * node2)
{
  // have list of all nodes
  // have list of all node weights
  // have list of shortest path links
  // set current node to node1
  // loop: stop if current node is node2
  // find all links for current
  // update weights for those nodes if link weight + prev node weight is shorter
  // pick next node (smallest weight)
  // repeat
  char ** nodes = (char **)malloc(2 * links->size * sizeof(char *));
  u32 * weights = (u32 *)malloc(2 * links->size * sizeof(u32));
  u32 * visited = (u32 *)calloc(2 * links->size, sizeof(u32)); // avoid nodes
  u32 nodesSize = 0;
  
  Link ** shortestPaths = (Link **)malloc(2 * links->size * sizeof(Link *));


  char * current = node1;
  u32 currentIndex;

  // fill up nodes and set initial weights
  for (int i = 0; i < links->size; i++)
  {
    char foundStart = 0;
    char foundEnd = 0;
    for (int j = 0; j < nodesSize; j++)
    {
      if (strcmp(links->values[i]->start, nodes[j]) == 0) foundStart = 1;
      if (strcmp(links->values[i]->end, nodes[j]) == 0) foundEnd = 1;
    }
    
    if (!foundStart)
    {
      nodes[nodesSize] = links->values[i]->start;
      weights[nodesSize] = UINT32_MAX;
      if (strcmp(links->values[i]->start, node1) == 0)
      {
        weights[nodesSize] = 0;
        currentIndex = nodesSize;
      }
      nodesSize++;
    }
    if (!foundEnd)
    {
      nodes[nodesSize] = links->values[i]->end;
      weights[nodesSize] = UINT32_MAX;
      if (strcmp(links->values[i]->end, node1) == 0)
      {
        weights[nodesSize] = 0;
        currentIndex = nodesSize;
      }
      nodesSize++;
    }
  }

#ifdef DEBUG
  for (int i = 0; i < nodesSize; i++)
  {
    fprintf(stdout, "Node %d: %s %lu\n", i, nodes[i], weights[i]);
  }
  fprintf(stdout, "----------------------------\n");
#endif

  while (true)
  {

#ifdef DEBUG
    fprintf(stdout, "=======================\n");
    fprintf(stdout, "Node %d: %s, weight = %d\n", currentIndex, current, weights[currentIndex]);
#endif

    for (int i = 0; i < links->size; i++)
    {
      
#ifdef DEBUG
      fprintf(stdout, "Link: %s -> %s\n", links->values[i]->start, links->values[i]->end);
#endif

      char * other = NULL;
      if (strcmp(current, links->values[i]->start) == 0) other = links->values[i]->end;
      if (strcmp(current, links->values[i]->end) == 0) other = links->values[i]->start;

#ifdef DEBUG
      if (!other) fprintf(stdout, " -- skipping\n");
#endif

      if (!other) continue; // link doesn't touch current

#ifdef DEBUG
      fprintf(stdout, " --> %s", other);
#endif


      u32 otherIndex = 0;
      for (int j = 0; j < nodesSize; j++)
      {
        if (strcmp(nodes[j], other) == 0)
        {
          otherIndex = j;
          break;
        }
      }

      if (visited[otherIndex])
      {
#ifdef DEBUG
        fprintf(stdout, " (visited)\n");
#endif
        continue;
      }
      
#ifdef DEBUG
      fprintf(stdout, "\n");
#endif

#ifdef DEBUG
      fprintf(stdout, " --> Node %d\n", otherIndex);
#endif

      if (weights[currentIndex] + links->values[i]->weight < weights[otherIndex]) 
      {
        weights[otherIndex] = weights[currentIndex] + links->values[i]->weight;
        shortestPaths[otherIndex] = links->values[i];
      }
    }

    visited[currentIndex] = 1;

    if (strcmp(current, node2) == 0) break;


    // pick next
    char * next = NULL;
    u32 min = INT32_MAX;
    for (int i = 0; i < nodesSize; i++)
    {
      if (visited[i] == 0 && weights[i] < min)
      {
        min = weights[i];
        next = nodes[i];
        currentIndex = i;
      }
    }

    if (!next) break;

#ifdef DEBUG
    fprintf(stdout, "Next: %s\n", next);
    for (int i = 0; i < nodesSize; i++)
    {
      fprintf(stdout, " ++ Node %d: %s, weight = %lu, visited = %d\n", i, nodes[i], weights[i], visited[i]);
    }
#endif


    current = next;
  }

  if (strcmp(current, node2) != 0) 
  {
    path = NULL; // path not found
    return;
  }

  // traverse shortest paths in reverse to get shortest path
  
  current = node2;

  for (int i = 0; i < nodesSize; i++)
  {
    if (strcmp(nodes[i], node2) == 0)
    {
      currentIndex = i;
      break;
    }
  }

  u32 iterationCount = 0;
  while (strcmp(current, node1) != 0)
  {
    Link * p = shortestPaths[currentIndex];
    path->values[iterationCount] = p;
    path->size = iterationCount + 1;

    char * other;
    if (strcmp(current, p->start) == 0) other = p->end;
    if (strcmp(current, p->end) == 0) other = p->start;

    for (int i = 0; i < nodesSize; i++)
    {
      if (strcmp(nodes[i], other) == 0)
      {
        currentIndex = i;
      }
    }
    current = other;
    iterationCount++;
  }

  // reverse backwards path
  for (int i = 0; i < iterationCount / 2; i++)
  {
    Link * temp = &*path->values[i];
    path->values[i] = &*path->values[path->size - i - 1];
    path->values[path->size - i - 1] = temp;
  }
}

int main(int argc, char ** args)
{
  char * filename = (char *)"input.csv";
  FILE * fp = fopen(filename, "rb");
  int maxLineLength = 1024;
  char buffer[maxLineLength];

  int maxLineSize = 1024;
  Link ** values = (Link **)malloc(maxLineLength * sizeof(Link **));
  Links * links = (Links *)malloc(sizeof(values) + sizeof(u32));
  links->values = values;
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
      links->values[line - 1] = link;
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

  links->size = line - 1;

  // find shortest path from node A to node F
  // TODO: specify this via user input somehow

  char * node1 = (char *)"B";
  char * node2 = (char *)"H";

  bool foundNode1 = false;
  bool foundNode2 = false;

  for (int i = 0; i < links->size; i++)
  {
    if (strcmp(links->values[i]->start, node1) == 0) foundNode1 = true;
    if (strcmp(links->values[i]->end, node1) == 0) foundNode1 = true;
    if (strcmp(links->values[i]->start, node2) == 0) foundNode2 = true;
    if (strcmp(links->values[i]->end, node2) == 0) foundNode2 = true;
  }

  if (!foundNode1)
  {
    fprintf(stderr, "No node %s found\n", node1);
    return EXIT_FAILURE;
  }

  if (!foundNode2)
  {
    fprintf(stderr, "No node %s found\n", node2);
    return EXIT_FAILURE;
  }

  //initialize path, copy size allocation of path since unknown
  Link ** pathValues = (Link **)malloc(maxLineLength * sizeof(Link **));
  Links * path = (Links *)malloc(sizeof(pathValues) + sizeof(u32));
  path->values = pathValues;
  findShortestPath(links, path, node1, node2);

  if (path->size > 0)
  {

    fprintf(stdout, "Shortest path for nodes %s and %s is :\n", node1, node2);
    char * first = node1;
    for (int i = 0; i < path->size; i++)
    {
      Link * link = path->values[i];
      char * other = NULL;
      if (strcmp(first, link->start) == 0) other = link->end;
      if (strcmp(first, link->end) == 0) other = link->start;
      fprintf(stdout, " %s -> %s\n", first, other);
      first = other;
    }

    // load traffic data
    fprintf(stdout, "==================\n");
    fprintf(stdout, "Loading Traffic Data: \n");
    filename = (char *)"traffic.csv";
    fp = fopen(filename, "rb");

    TrafficLink ** trafficValues = (TrafficLink **)malloc(maxLineLength * sizeof(TrafficLink **));
    TrafficLinks * trafficLinks = (TrafficLinks *)malloc(sizeof(trafficValues) + sizeof(u32));
    trafficLinks->values = trafficValues;

    line = 0;
    while (fgets(buffer, maxLineLength, fp))
    {
      if (line != 0) // skip header
      {
        char * token = strtok(buffer, ",");
        int pos = 0;
        TrafficLink * trafficLink = (TrafficLink *)malloc(sizeof(TrafficLink));
        trafficLinks->values[line - 1] = trafficLink;
        while (token)
        {
          switch(pos) {
            case 0: {
              trafficLink->source = (char *)malloc(strlen(token) + 1); // +1 for the null terminator
              trafficLink->source[strlen(token)] = '\0';

              if (trafficLink->source == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                return EXIT_FAILURE;
              }

              strcpy(trafficLink->source, token);
  #ifdef DEBUG
              printf("%s ", trafficLink->source);
  #endif
              break;
            }
            case 1: {
              trafficLink->destination = (char *)malloc(strlen(token) + 1);
              trafficLink->destination[strlen(token)] = '\0';

              if (trafficLink->destination == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                return EXIT_FAILURE;
              }
              strcpy(trafficLink->destination, token);
  #ifdef DEBUG
              printf("%s ", trafficLink->destination);
  #endif
              break;
            }
            case 2: {
              trafficLink->demand = atoi(token);
  #ifdef DEBUG            
              printf("%d ", trafficLink->demand);
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

    trafficLinks->size = line - 1;

    for (int i = 0; i < trafficLinks->size; i++)
    {
      Link ** pathValues = (Link **)malloc(maxLineLength * sizeof(Link **));
      Links * path = (Links *)malloc(sizeof(pathValues) + sizeof(u32));
      path->values = pathValues;

      TrafficLink * trafficLink = trafficLinks->values[i];
      char * source = trafficLink->source;
      char * destination = trafficLink->destination;
      fprintf(stdout, "Determing traffic for %s -> %s\n", source, destination);
      u32 demand = trafficLink->demand;

      findShortestPath(links, path, source, destination);

      first = source;

      for (int j = 0; j < path->size; j++)
      {
        Link * link = path->values[j];
        char * other = NULL;
        if (strcmp(first, link->start) == 0) other = link->end;
        if (strcmp(first, link->end) == 0) other = link->start;
        fprintf(stdout, " %s -> %s", first, other);
        u32 capacity = link->capacity;
        fprintf(stdout, " w/ capacity %u/%u (%.2f%)\n", demand, capacity, ((double)demand / (double)capacity * 100));
        first = other;
      }
    }
  }
  else
  {
    fprintf(stdout, "Path for nodes %s and %s not found\n", node1, node2);
  }

  return 0;
}
