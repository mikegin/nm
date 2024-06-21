#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#ifdef GUI
#include "raylib.h"
#endif

typedef uint32_t u32;

typedef struct Node {
    unsigned int id;
    char name[10];
    float x, y;
} Node;

typedef struct {
  Node ** nodes;
  u32 size;
} Nodes;

typedef struct Link {
  u32 id;
  Node * start;
  Node * end;
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



#define MAX_NODES 1000
#define MAX_LINKS 1000




// typedef struct {
//     int id;
//     char start[10];
//     char end[10];
//     int capacity;
//     int weight;
//     int load;
//     bool isActive;
// } Link;

// typedef struct {
//     char source[10];
//     char destination[10];
//     int demand;
// } Traffic;

// Node nodes[MAX_NODES];
// Link links[MAX_LINKS];
// Traffic traffic[MAX_NODES];
// int nodeCount = 0;
// int linkCount = 0;
// int trafficCount = 0;

// Function to find or create a node
// int FindOrCreateNode(const char *id) {
//     for (int i = 0; i < nodeCount; i++) {
//         if (strcmp(nodes[i].id, id) == 0) {
//             return i;
//         }
//     }
//     Node newNode;
//     strcpy(newNode.id, id);
//     newNode.x = (nodeCount % 10) * 80 + 50;  // Positioning nodes roughly
//     newNode.y = (nodeCount / 10) * 80 + 50;
//     nodes[nodeCount] = newNode;
//     return nodeCount++;
// }

// // Function to parse network CSV
// void ParseNetworkCSV(const char *fileName)
// {
//     FILE *file = fopen(fileName, "r");
//     if (!file)
//     {
//       perror("Error opening file");
//       return;
//     }

//     char line[256];
//     fgets(line, sizeof(line), file); // Skip header
//     while (fgets(line, sizeof(line), file))
//     {
//       Link link;
//       sscanf(line, "%d,%[^,],%[^,],%d,%d", &link.id, link.start, link.end, &link.capacity, &link.weight);
//       links[linkCount++] = link;
//       FindOrCreateNode(link.start);
//       FindOrCreateNode(link.end);
//     }

//     fclose(file);
// }

// // Function to parse traffic CSV
// void ParseTrafficCSV(const char *fileName) {
//     FILE *file = fopen(fileName, "r");
//     if (!file) {
//         perror("Error opening file");
//         return;
//     }

//     char line[256];
//     fgets(line, sizeof(line), file); // Skip header
//     while (fgets(line, sizeof(line), file)) {
//         Traffic t;
//         sscanf(line, "%[^,],%[^,],%d", t.source, t.destination, &t.demand);
//         traffic[trafficCount++] = t;
//     }

//     fclose(file);
// }


u32 maxNumberOfLinks = 1024;
u32 maxSizeOfCSVLine = 1024;

int ParseNetworkCSV(Links * links, FILE * networkFile)
{
  char buffer[maxSizeOfCSVLine];
  int line = 0;
  while (fgets(buffer, maxSizeOfCSVLine, networkFile))
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
            Node * node = (Node *)malloc(sizeof(Node));
            // node->name = (char *)malloc(strlen(token) + 1); // +1 for the null terminator
            if (node == NULL || node->name == NULL) {
              fprintf(stderr, "Memory allocation failed\n");
              return EXIT_FAILURE;
            }
            node->name[strlen(token)] = '\0';
            link->start = node;
            
            strcpy(link->start->name, token);

#ifdef DEBUG
            printf("%s ", link->start->name);
#endif
            break;
          }
          case 2: {
            // Similarly allocate memory for link->end
            Node * node = (Node *)malloc(sizeof(Node));
            // node->name = (char *)malloc(strlen(token) + 1); // +1 for the null terminator
            if (node == NULL || node->name == NULL) {
              fprintf(stderr, "Memory allocation failed\n");
              return EXIT_FAILURE;
            }
            node->name[strlen(token)] = '\0';
            link->end = node;

            strcpy(link->end->name, token);
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

  return 0;
}

int ParseTrafficCSV(TrafficLinks * trafficLinks, FILE * trafficFile)
{
  int line = 0;
  char buffer[maxSizeOfCSVLine];
  while (fgets(buffer, maxSizeOfCSVLine, trafficFile))
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

  return 0;
}

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
      if (strcmp(links->values[i]->start->name, nodes[j]) == 0) foundStart = 1;
      if (strcmp(links->values[i]->end->name, nodes[j]) == 0) foundEnd = 1;
    }
    
    if (!foundStart)
    {
      nodes[nodesSize] = links->values[i]->start->name;
      weights[nodesSize] = UINT32_MAX;
      if (strcmp(links->values[i]->start->name, node1) == 0)
      {
        weights[nodesSize] = 0;
        currentIndex = nodesSize;
      }
      nodesSize++;
    }
    if (!foundEnd)
    {
      nodes[nodesSize] = links->values[i]->end->name;
      weights[nodesSize] = UINT32_MAX;
      if (strcmp(links->values[i]->end->name, node1) == 0)
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
      if (strcmp(current, link->start->name) == 0) other = link->end->name;
      if (strcmp(current, link->end->name) == 0) other = link->start->name;

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
    if (strcmp(current, p->start->name) == 0) other = p->end->name;
    if (strcmp(current, p->end->name) == 0) other = p->start->name;

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


void recursiveWorstCaseFailure(Links * links, TrafficLinks * trafficLinks, u32 maxNumberOfLinks, int depth, u32 startingPoint)
{
  for (u32 i = startingPoint; i < links->size; i++)
  {
    // set isActive i to false
    // if (depth > 0) recurse
    // if (depth == 0) determine traffic
    // set isActive i to true

    Link * link = links->values[i];

    link->isActive = false;

    if (depth > 0) recursiveWorstCaseFailure(links, trafficLinks, maxNumberOfLinks, depth - 1, i + 1);

    if (depth == 0)
    {
      fprintf(stdout, "\n\n## Deactivated\n");
      for (int j = 0; j < links->size; j++) // reset load
      {
        links->values[j]->load = 0;
        if (!links->values[j]->isActive)
        {
          fprintf(stdout, "%s -> %s\n", links->values[j]->start, links->values[j]->end);
        }
      }

      for (int i = 0; i < trafficLinks->size; i++)
      {
        Link ** pathValues = (Link **)malloc(maxNumberOfLinks * sizeof(Link **));
        Links * path = (Links *)malloc(sizeof(pathValues) + sizeof(u32));
        path->values = pathValues;

        TrafficLink * trafficLink = trafficLinks->values[i];
        char * source = trafficLink->source;
        char * destination = trafficLink->destination;
        fprintf(stdout, "\n### Route\n%s --> %s\n", source, destination);
        u32 demand = trafficLink->demand;

        findShortestPath(links, path, source, destination);

        if (path->size > 0)
        {
          fprintf(stdout, "\nLink, Demand/Capacity, Percentage\n");
          char * first = source;
          for (int j = 0; j < path->size; j++)
          {
            Link * link = path->values[j];
            char * other = NULL;
            if (strcmp(first, link->start->name) == 0) other = link->end->name;
            if (strcmp(first, link->end->name) == 0) other = link->start->name;
            u32 capacity = link->capacity;
            fprintf(stdout, "%s -> %s, %u/%u, %.2f%%\n", first, other, demand, capacity, ((double)demand / (double)capacity * 100));
            first = other;

            link->load += demand;
          }
        }



        free(pathValues);
        free(path);
      }
      fprintf(stdout, "\n### Total Load\nLink, Demand/Capacity, Percentage\n");
      for (int i = 0; i < links->size; i++)
      {
        Link * link = links->values[i];
        fprintf(stdout, "%s -> %s, %u/%u, (%.2f%%)\n", link->start, link->end, link->load, link->capacity, ((double)link->load / (double)link->capacity * 100));
      }
    }

    links->values[i]->isActive = true;
  }
}

#ifdef GUI
int RunGUI()
{


    FILE * networkFile = NULL;
    FILE * trafficFile = NULL;
   //--------------------------------------------------------------------------------------
    // Set the window to be resizable and support drag-and-drop
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_ALWAYS_RUN);

    const int screenWidth = 2400;
    const int screenHeight = 1800;


    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second


    Link ** values = (Link **)malloc(maxNumberOfLinks * sizeof(Link *));
    Links * links = (Links *)malloc(sizeof(values) + sizeof(u32));
    links->values = values;

    TrafficLink ** trafficValues = (TrafficLink **)malloc(maxNumberOfLinks * sizeof(TrafficLink *));
    TrafficLinks * trafficLinks = (TrafficLinks *)malloc(sizeof(trafficValues) + sizeof(u32));
    trafficLinks->values = trafficValues;

    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        int count = 0;
        FilePathList droppedFiles = LoadDroppedFiles();
        count = droppedFiles.count;

        if (count > 0) {
            for (int i = 0; i < count; i++) {
                // Check if the dropped file is a CSV file
                char * dropFilePath = droppedFiles.paths[i];
                if (IsFileExtension(droppedFiles.paths[i], ".csv")) {
                    if (strstr(droppedFiles.paths[i], "network.csv")) {
                        networkFile = fopen(droppedFiles.paths[i], "r");
                        if (networkFile == NULL) {
                            perror("Error opening network file");
                            return EXIT_FAILURE;
                        }
                        int parseNetworkCSVError = ParseNetworkCSV(links, networkFile);
                    } else if (strstr(droppedFiles.paths[i], "traffic.csv")) {
                      trafficFile = fopen(droppedFiles.paths[i], "r");
                      if (trafficFile == NULL) {
                          perror("Error opening traffic file");
                          fclose(networkFile);
                          return EXIT_FAILURE;
                      }
                        int parseTrafficCSVError = ParseTrafficCSV(trafficLinks, trafficFile);
                    }
                }
            }
            UnloadDroppedFiles(droppedFiles);
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        // Draw links
        // for (int i = 0; i < links->size; i++) {
        //   int startNodeIndex = FindOrCreateNode(links[i]->start);
        //   int endNodeIndex = FindOrCreateNode(links->values[i]->end);
        //   DrawLineV((Vector2){nodes[startNodeIndex].x, nodes[startNodeIndex].y}, 
        //             (Vector2){nodes[endNodeIndex].x, nodes[endNodeIndex].y}, BLACK);
        // }

        // // Draw nodes
        // for (int i = 0; i < nodeCount; i++) {
        //   DrawCircleV((Vector2){nodes[i].x, nodes[i].y}, 5, RED);
        //   DrawText(nodes[i].id, nodes[i].x + 10, nodes[i].y, 10, DARKGRAY);
        // }

        // // Draw traffic demands
        // for (int i = 0; i < trafficCount; i++) {
        //   int sourceNodeIndex = FindOrCreateNode(traffic[i].source);
        //   int destNodeIndex = FindOrCreateNode(traffic[i].destination);
        //   DrawLineV((Vector2){nodes[sourceNodeIndex].x, nodes[sourceNodeIndex].y}, 
        //             (Vector2){nodes[destNodeIndex].x, nodes[destNodeIndex].y}, BLUE);
        //   DrawText(TextFormat("%d", traffic[i].demand), 
        //             (nodes[sourceNodeIndex].x + nodes[destNodeIndex].x) / 2, 
        //             (nodes[sourceNodeIndex].y + nodes[destNodeIndex].y) / 2, 
        //             10, DARKGRAY);
        // }

        DrawText("Drag and drop a network or traffic CSV file onto the window", 10, 10, 20, DARKGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
#endif

static struct option long_options[] = {
    {"gui", no_argument, 0, 'g'},
    {0, 0, 0, 0}
};

int main(int argc, char ** args)
{
  int opt;
  char *networkFileName = NULL;
  char *trafficFileName = NULL;
  char *linksToKill = NULL;
  char *nodesToKill = NULL;
  int simulateNumberOfLinksToKill = 1;
  int simulateNumberOfNodesToKill = -1;
  bool runGUI = false;

  int option_index = 0;

  while ((opt = getopt_long(argc, args, "gn:t:l:k:s:m:", long_options, &option_index)) != -1) {
      switch (opt) {
          case 'g':
              runGUI = true;
              break;
          case 'n':
              networkFileName = optarg;
              break;
          case 't':
              trafficFileName = optarg;
              break;
          case 'l':
              linksToKill = optarg;
              break;
          case 'k':
              nodesToKill = optarg;
              break;
          case 's':
              simulateNumberOfLinksToKill = atoi(optarg);
              break;
          case 'm':
              simulateNumberOfNodesToKill = atoi(optarg);
              break;
          default:
              fprintf(stderr, "Usage: %s [-g|--gui] -n <network filename> -t <traffic filename> [-l <links to kill>] [-k <nodes to kill>] [-s <simulate n links to kill>] [-m <simulate n nodes to kill>]\n", args[0]);
              return EXIT_FAILURE;
      }
  }

#ifdef GUI
  if (runGUI)
  {
    RunGUI();
    return 0;
  }
#endif

  FILE * networkFile = NULL;
  FILE * trafficFile = NULL;

  if (networkFileName == NULL || trafficFileName == NULL) {
    fprintf(stderr, "Both -n and -t options are required.\n");
    fprintf(stderr, "Usage: %s [-g|--gui] -n <network filename> -t <traffic filename> [-l <links to kill>] [-k <nodes to kill>] [-s <simulate n links to kill>] [-m <simulate n nodes to kill>]\n", args[0]);
    return EXIT_FAILURE;
  }

  networkFile = fopen(networkFileName, "r");
  if (networkFile == NULL) {
      perror("Error opening network file");
      return EXIT_FAILURE;
  }

  trafficFile = fopen(trafficFileName, "r");
  if (trafficFile == NULL) {
      perror("Error opening traffic file");
      fclose(networkFile);
      return EXIT_FAILURE;
  }

  // todo: determine this programmatically from the files

  Link ** values = (Link **)malloc(maxNumberOfLinks * sizeof(Link *));
  Links * links = (Links *)malloc(sizeof(values) + sizeof(u32));
  links->values = values;

  int parseNetworkCSVError = ParseNetworkCSV(links, networkFile);
  if (parseNetworkCSVError) return parseNetworkCSVError;

  TrafficLink ** trafficValues = (TrafficLink **)malloc(maxNumberOfLinks * sizeof(TrafficLink *));
  TrafficLinks * trafficLinks = (TrafficLinks *)malloc(sizeof(trafficValues) + sizeof(u32));
  trafficLinks->values = trafficValues;

  int parseTrafficCSVError = ParseTrafficCSV(trafficLinks, trafficFile);
  if (parseTrafficCSVError) return parseTrafficCSVError;

  if (trafficLinks->size > 0)
  {
    fprintf(stdout, "## Determining Traffic\n");
    for (int i = 0; i < trafficLinks->size; i++)
      {
        Link ** pathValues = (Link **)malloc(maxNumberOfLinks * sizeof(Link **));
        Links * path = (Links *)malloc(sizeof(pathValues) + sizeof(u32));
        path->values = pathValues;

        TrafficLink * trafficLink = trafficLinks->values[i];
        char * source = trafficLink->source;
        char * destination = trafficLink->destination;
        fprintf(stdout, "\n\n### Route\n%s --> %s\n\n", source, destination);
        u32 demand = trafficLink->demand;

        findShortestPath(links, path, source, destination);

        if (path->size > 0)
        {
          fprintf(stdout, "Link, Demand/Capacity, Percentage\n");
          char * first = source;
          for (int j = 0; j < path->size; j++)
          {
            Link * link = path->values[j];
            char * other = NULL;
            if (strcmp(first, link->start->name) == 0) other = link->end->name;
            if (strcmp(first, link->end->name) == 0) other = link->start->name;
            u32 capacity = link->capacity;
            fprintf(stdout, "%s -> %s, %u/%u, %.2f%%\n", first, other, demand, capacity, ((double)demand / (double)capacity * 100));
            first = other;

            link->load += demand;
          }
        }

        free(pathValues);
        free(path); // todo: path may be NULL
      }

    fprintf(stdout, "\n\n### Total Load\n");
    for (int i = 0; i < links->size; i++)
    {
      Link * link = links->values[i];
      fprintf(stdout, "%s -> %s, %u/%u, (%.2f%%)\n", link->start, link->end, link->load, link->capacity, ((double)link->load / (double)link->capacity * 100));
    }
  }

  if (simulateNumberOfLinksToKill > 0)
  {
    fprintf(stdout, "\n\n\n### Simulating Network Failure\n");
    recursiveWorstCaseFailure(links, trafficLinks, maxNumberOfLinks, simulateNumberOfLinksToKill - 1, 0);
  }

  return 0;
}
