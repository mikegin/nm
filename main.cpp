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
  u32 load;
  bool isActive;
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
    Link * link = links->values[i];
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
      
      Link * link = links->values[i];
      if (!link->isActive) continue; // skip inactive links
#ifdef DEBUG
      fprintf(stdout, "Link: %s -> %s\n", link->start, link->end);
#endif

      char * other = NULL;
      if (strcmp(current, link->start) == 0) other = link->end;
      if (strcmp(current, link->end) == 0) other = link->start;

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

      if (weights[currentIndex] + link->weight < weights[otherIndex]) 
      {
        weights[otherIndex] = weights[currentIndex] + link->weight;
        shortestPaths[otherIndex] = link;
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
      links->values[line - 1]->isActive = true;
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
            link->start[strlen(token)] = '\0';

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
            link->end[strlen(token)] = '\0';
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

    char * first = source;
    for (int j = 0; j < path->size; j++)
    {
      Link * link = path->values[j];
      char * other = NULL;
      if (strcmp(first, link->start) == 0) other = link->end;
      if (strcmp(first, link->end) == 0) other = link->start;
      fprintf(stdout, " %s -> %s", first, other);
      u32 capacity = link->capacity;
      fprintf(stdout, "  load = %u/%u (%.2f%%)\n", demand, capacity, ((double)demand / (double)capacity * 100));
      first = other;

      link->load += demand;
    }

    free(pathValues);
    free(path);
  }

  fprintf(stdout, "Total Load:\n");
  for (int i = 0; i < links->size; i++)
  {
    fprintf(stdout, "%s -> %s", links->values[i]->start, links->values[i]->end);
    fprintf(stdout, "  load = %u/%u (%.2f%%)\n", links->values[i]->load, links->values[i]->capacity, ((double)links->values[i]->load / (double)links->values[i]->capacity) * 100);
  }


  // determine worst case failure (wcf)
  // for each link (how would you extend this to multiple link removal)
  // reset load for all links
  // remove it from the network
  // go through all traffic demands
  // find shortest path
  // if path exists, calculate load
  fprintf(stdout, "------------------\n");
  fprintf(stdout, "Testing Wors Case Failure\n");
  for (int i = 0; i < links->size; i++)
  {

    // reset load
    for (int j = 0; j < links->size; j++)
    {
      links->values[j]->load = 0;
    }

    Link * link = links->values[i];
    link->isActive = false;

    fprintf(stdout, "\n\nDeactivating link %s -> %s\n\n", link->start, link->end);


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

      char * first = source;
      for (int j = 0; j < path->size; j++)
      {
        Link * link = path->values[j];
        char * other = NULL;
        if (strcmp(first, link->start) == 0) other = link->end;
        if (strcmp(first, link->end) == 0) other = link->start;
        fprintf(stdout, " %s -> %s", first, other);
        u32 capacity = link->capacity;
        fprintf(stdout, "  load = %u/%u (%.2f%%)\n", demand, capacity, ((double)demand / (double)capacity * 100));
        first = other;

        link->load += demand;
      }


      free(pathValues);
      free(path);
    }
    fprintf(stdout, "Total Load:\n");
    for (int i = 0; i < links->size; i++)
    {
      fprintf(stdout, "%s -> %s", links->values[i]->start, links->values[i]->end);
      fprintf(stdout, "  load = %u/%u (%.2f%%)\n", links->values[i]->load, links->values[i]->capacity, ((double)links->values[i]->load / (double)links->values[i]->capacity) * 100);
    }


    link->isActive = true;
  }

  return 0;
}
