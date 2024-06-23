#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include "set.cpp"
#include "hash.cpp"

#ifdef GUI
#include "raylib.h"
#endif

typedef uint32_t u32;

#define MAX_NODES 1000
#define MAX_LINKS MAX_NODES * MAX_NODES
#define MAX_NODE_NAME_LENGTH 10
#define MAX_LINE_LENGTH 1024
typedef struct Node {
    unsigned int id;
    char name[MAX_NODE_NAME_LENGTH];
    
    // used in calculating shortest path for a route
    bool visited;
    u32 weight;
    struct Link * shortestPath;
    struct Link * associatedLinks[MAX_NODES];
    u32 numberOfAssociatedLinks;
    
    // gui specific fields
    float x, y;
} Node;

typedef struct Nodes {
  Node ** values;
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
  Node * source;
  Node * destination;
  u32 demand;
} TrafficLink;

typedef struct TrafficLinks {
  TrafficLink ** values;
  u32 size;
} TrafficLinks;



Node * getNodeByName(Nodes * nodes, char * name)
{
  for (int i = 0; i < nodes->size; i++)
  {
    if (strcmp(nodes->values[i]->name, name) == 0)
    {
      return nodes->values[i];
    }
  }

  return NULL;
}

int insertNode(Nodes * nodes, Node * node)
{
  if (nodes->size == MAX_NODES) return EXIT_FAILURE;

  node->id = nodes->size;
  nodes->values[nodes->size] = node;
  nodes->size++;

  return 0;
}

void addAssociatedLink(Node * node, Link * link)
{
  for (int i = 0; i < node->numberOfAssociatedLinks; i++)
  {
    if (node->associatedLinks[i]->id == link->id)
    {
      return; // link already added
    }
  }

  node->associatedLinks[node->numberOfAssociatedLinks] = link;
  node->numberOfAssociatedLinks++;
}



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
// Link links[MAX_NODES];
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

// unsigned int nodeHash(void* value) {
//     char * name = ((Node *) value)->name;
//     uint32_t hash = SuperFastHash(name, 10);
//     return hash % TABLE_SIZE;
// }

// int nodeCompare(void* a, void* b) {
//     return strcmp(((Node *) a)->name, ((Node *) b)->name);
// }



int createNodeFromToken(Node ** p, Nodes * nodes, char * token)
{
  // check does the node exists, if it does, set p to that
  // otherwise, create the node, set it to p, insert it into nodes

  Node * found = getNodeByName(nodes, token);

  if (found)
  {
    *p = found;
    return 0;
  }


  Node * node = (Node *)malloc(sizeof(Node));
  if (node == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    return EXIT_FAILURE;
  }
  strcpy(node->name, token);
  node->name[strlen(token)] = '\0';
  node->visited = false;
  node->weight = 0;
  node->shortestPath = NULL;

  int err = insertNode(nodes, node);
  if (err)
  {
    free(node);
    return err;
  }

  *p = node;

  return 0;
}

int ParseNetworkCSV(Links * links, Nodes * nodes, FILE * networkFile)
{
  char buffer[MAX_LINE_LENGTH];
  int line = 0;
  while (fgets(buffer, MAX_LINE_LENGTH, networkFile))
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
        switch(pos) {
          case 0: {
            link->id = atoi(token);
            break;
          }
          case 1: {
            createNodeFromToken(&link->start, nodes, token);
            addAssociatedLink(link->start, link);
            break;
          }
          case 2: {
            createNodeFromToken(&link->end, nodes, token);
            addAssociatedLink(link->end, link);
            break;
          }
          case 3: {
            link->capacity = atoi(token);
            break;
          }
          case 4: {
            link->weight = atoi(token);
            break;
          }
        }
        token = strtok(NULL, ",");
        pos += 1;
      }
    }

    line += 1;
  }

  links->size = line - 1;

  return 0;
}

int ParseTrafficCSV(TrafficLinks * trafficLinks, Nodes * nodes, FILE * trafficFile)
{
  int line = 0;
  char buffer[MAX_LINE_LENGTH];
  while (fgets(buffer, MAX_LINE_LENGTH, trafficFile))
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
            createNodeFromToken(&trafficLink->source, nodes, token);
            break;
          }
          case 1: {
            createNodeFromToken(&trafficLink->destination, nodes, token);
            break;
          }
          case 2: {
            trafficLink->demand = atoi(token);
            break;
          }
        }
        token = strtok(NULL, ",");
        pos += 1;
      }
    }

    line += 1;
  }

  trafficLinks->size = line - 1;

  return 0;
}

/**
 * This uses Dijakstra's shortest path algorithm
*/
void findShortestPath(Links * links, Links * path, Nodes * nodes, Node * node1, Node * node2)
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

  for (int i = 0; i < nodes->size; i++)
  {
    Node * node = nodes->values[i];
    node->weight = UINT32_MAX;
    node->visited = false;
  }
  node1->weight = 0;

  Node * current = node1;

  // list of nodes we can visit
  Node * canVisitNodes[MAX_NODES];
  size_t canVisitNodesSize = 0;

  while (current)
  {
    // update current's connected node weights
    for (int i = 0; i < current->numberOfAssociatedLinks; i++)
    {
      
      Link * link = current->associatedLinks[i];
      if (!link->isActive) continue; // skip inactive links

      Node * other = NULL;
      if(current->id == link->start->id) other = link->end;
      if(current->id == link->end->id) other = link->start;

      // fprintf(stdout, "Examining %s weight = %lu, Other %s weight = %lu\n", current->name, current->weight, other->name, other->weight);

      // if (!other) continue; // link doesn't touch current

      // if (other->visited) continue;

      if (current->weight + link->weight < other->weight) 
      {
        other->weight = current->weight + link->weight;
        other->shortestPath = link;
      }

      if (!other->visited)
      {
        canVisitNodes[canVisitNodesSize] = other;
        canVisitNodesSize++;
      }
    }

    current->visited = 1;

    // pick next
    Node * next = NULL;
    u32 min = INT32_MAX;
    int canVisitNodesIndex = -1;
    for (int i = 0; i < canVisitNodesSize; i++)
    {
      Node * node = canVisitNodes[i];
      if (!node->visited && node->weight < min)
      {
        min = node->weight;
        next = node;
        canVisitNodesIndex = i;
      }
    }
    // if (next) fprintf(stdout, "Current: %s weight = %d, Next: %s weight = %d\n", current->name, current->weight, next->name, next->weight);

    current = next;

    if (current && current->id == node2->id) break;
  }

  if (!current || current->id != node2->id) 
  {
    path = NULL; // path not found
    return;
  }

  // traverse shortest paths in reverse to get shortest path
  current = node2;
  u32 iterationCount = 0;
  while (current->id != node1->id)
  {
    Link * link = current->shortestPath;
    path->values[iterationCount] = link;
    path->size = iterationCount + 1;

    Node * other;
    if(current->id == link->start->id) other = link->end;
    if(current->id == link->end->id) other = link->start;

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


void recursiveWorstCaseFailure(Links * links, TrafficLinks * trafficLinks, Nodes * nodes, int depth, u32 startingPoint)
{
  for (u32 i = startingPoint; i < links->size; i++)
  {
    // set isActive i to false
    // if (depth > 0) recurse
    // if (depth == 0) determine traffic
    // set isActive i to true

    Link * link = links->values[i];

    link->isActive = false;

    if (depth > 0) recursiveWorstCaseFailure(links, trafficLinks, nodes, depth - 1, i + 1);

    if (depth == 0)
    {
      fprintf(stdout, "\n\n## Deactivated\n");
      for (int j = 0; j < links->size; j++) // reset load
      {
        links->values[j]->load = 0;
        if (!links->values[j]->isActive)
        {
          fprintf(stdout, "%s -> %s\n", links->values[j]->start->name, links->values[j]->end->name);
        }
      }

      for (int i = 0; i < trafficLinks->size; i++)
      {
        Link ** pathValues = (Link **)malloc(MAX_NODES * sizeof(Link **));
        Links * path = (Links *)malloc(sizeof(pathValues) + sizeof(u32));
        path->values = pathValues;

        TrafficLink * trafficLink = trafficLinks->values[i];
        char * source = trafficLink->source->name;
        char * destination = trafficLink->destination->name;
        fprintf(stdout, "\n### Route\n%s --> %s\n", source, destination);
        u32 demand = trafficLink->demand;

        findShortestPath(links, path, nodes, trafficLink->source, trafficLink->destination);

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
        fprintf(stdout, "%s -> %s, %u/%u, (%.2f%%)\n", link->start->name, link->end->name, link->load, link->capacity, ((double)link->load / (double)link->capacity * 100));
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


    Node ** nodeValues = (Node **)malloc(MAX_NODES * sizeof(Node *))
    Nodes * nodes = (Nodes *)malloc(sizeof(nodeValues) + sizeof(u32));
    nodes->values = nodeValues;

    Link ** linkValues = (Link **)malloc(MAX_LINKS * sizeof(Link *));
    Links * links = (Links *)malloc(sizeof(linkValues) + sizeof(u32));
    links->values = linkValues;

    TrafficLink ** trafficValues = (TrafficLink **)malloc(MAX_LINKS * sizeof(TrafficLink *));
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
                        int parseNetworkCSVError = ParseNetworkCSV(links, nodes, networkFile);
                    } else if (strstr(droppedFiles.paths[i], "traffic.csv")) {
                      trafficFile = fopen(droppedFiles.paths[i], "r");
                      if (trafficFile == NULL) {
                          perror("Error opening traffic file");
                          fclose(networkFile);
                          return EXIT_FAILURE;
                      }
                        int parseTrafficCSVError = ParseTrafficCSV(trafficLinks, nodes, trafficFile);
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

  // initialize
  Node ** nodeValues = (Node **)malloc(MAX_NODES * sizeof(Node *));
  Nodes * nodes = (Nodes *)malloc(sizeof(nodeValues) + sizeof(u32));
  nodes->values = nodeValues;

  Link ** linkValues = (Link **)malloc(MAX_LINKS * sizeof(Link *));
  Links * links = (Links *)malloc(sizeof(linkValues) + sizeof(u32));
  links->values = linkValues;

  TrafficLink ** trafficValues = (TrafficLink **)malloc(MAX_LINKS * sizeof(TrafficLink *));
  TrafficLinks * trafficLinks = (TrafficLinks *)malloc(sizeof(trafficValues) + sizeof(u32));
  trafficLinks->values = trafficValues;



  // parse
  int parseNetworkCSVError = ParseNetworkCSV(links, nodes, networkFile);
  if (parseNetworkCSVError) return parseNetworkCSVError;

  int parseTrafficCSVError = ParseTrafficCSV(trafficLinks, nodes, trafficFile);
  if (parseTrafficCSVError) return parseTrafficCSVError;


  if (trafficLinks->size > 0)
  {
    fprintf(stdout, "## Determining Traffic\n");
    for (int i = 0; i < trafficLinks->size; i++)
      {
        Link ** pathValues = (Link **)malloc(MAX_NODES * sizeof(Link *));
        Links * path = (Links *)malloc(sizeof(pathValues) + sizeof(u32));
        path->values = pathValues;

        TrafficLink * trafficLink = trafficLinks->values[i];
        Node * source = trafficLink->source;
        Node * destination = trafficLink->destination;
        fprintf(stdout, "\n\n### Route\n%s --> %s\n\n", source->name, destination->name);

        findShortestPath(links, path, nodes, trafficLink->source, trafficLink->destination);

        if (path->size > 0)
        {
          fprintf(stdout, "Link, Demand/Capacity, Percentage\n");
          Node * first = source;
          for (int j = 0; j < path->size; j++)
          {
            Link * link = path->values[j];
            Node * other = NULL;
            if (first->id == link->start->id) other = link->end;
            if (first->id == link->end->id) other = link->start;
            u32 demand = trafficLink->demand;
            u32 capacity = link->capacity;
            fprintf(stdout, "%s -> %s, %u/%u, %.2f%%\n", first->name, other->name, demand, capacity, ((double)demand / (double)capacity * 100));
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
      fprintf(stdout, "%s -> %s, %u/%u, (%.2f%%)\n", link->start->name, link->end->name, link->load, link->capacity, ((double)link->load / (double)link->capacity * 100));
    }
  }

  if (simulateNumberOfLinksToKill > 0)
  {
    fprintf(stdout, "\n\n\n### Simulating Network Failure\n");
    recursiveWorstCaseFailure(links, trafficLinks, nodes, simulateNumberOfLinksToKill - 1, 0);
  }

  return 0;
}
