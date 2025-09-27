// stalker-c/helper/pathfinding.c

#include "pathfinding.h"
#include <stdlib.h>
#include <string.h> // <-- Add this for memset
#include <float.h>
#include <stdbool.h>

// --- A* Node Structure (unchanged) ---
typedef struct Node {
    int x, y;
    float g_score;
    float f_score;
    struct Node* parent;
    bool in_open_set;
} Node;

// --- Priority Queue (Min-Heap) Implementation (unchanged) ---
typedef struct {
    Node** nodes;
    int count;
    int capacity;
} PriorityQueue;

static void swap_nodes(Node** a, Node** b) {
    Node* temp = *a;
    *a = *b;
    *b = temp;
}

static void heapify_up(PriorityQueue* pq, int index) {
    while (index > 0) {
        int parent_index = (index - 1) / 2;
        if (pq->nodes[index]->f_score < pq->nodes[parent_index]->f_score) {
            swap_nodes(&pq->nodes[index], &pq->nodes[parent_index]);
            index = parent_index;
        } else {
            break;
        }
    }
}

static void heapify_down(PriorityQueue* pq, int index) {
    int left_child_index, right_child_index, smallest_child_index;
    while (1) {
        left_child_index = 2 * index + 1;
        right_child_index = 2 * index + 2;
        smallest_child_index = index;

        if (left_child_index < pq->count && pq->nodes[left_child_index]->f_score < pq->nodes[smallest_child_index]->f_score) {
            smallest_child_index = left_child_index;
        }
        if (right_child_index < pq->count && pq->nodes[right_child_index]->f_score < pq->nodes[smallest_child_index]->f_score) {
            smallest_child_index = right_child_index;
        }

        if (smallest_child_index != index) {
            swap_nodes(&pq->nodes[index], &pq->nodes[smallest_child_index]);
            index = smallest_child_index;
        } else {
            break;
        }
    }
}

// --- FIX 1: Pass map dimensions to pq_create ---
static PriorityQueue* pq_create(int width, int height) {
    PriorityQueue* pq = (PriorityQueue*)malloc(sizeof(PriorityQueue));
    pq->capacity = width * height;
    pq->nodes = (Node**)malloc(sizeof(Node*) * pq->capacity);
    pq->count = 0;
    return pq;
}

static void pq_push(PriorityQueue* pq, Node* node) {
    if (pq->count >= pq->capacity) return;
    pq->nodes[pq->count] = node;
    node->in_open_set = true;
    heapify_up(pq, pq->count);
    pq->count++;
}

static Node* pq_pop(PriorityQueue* pq) {
    if (pq->count == 0) return NULL;
    Node* top_node = pq->nodes[0];
    pq->nodes[0] = pq->nodes[pq->count - 1];
    pq->count--;
    heapify_down(pq, 0);
    top_node->in_open_set = false;
    return top_node;
}

static void pq_destroy(PriorityQueue* pq) {
    free(pq->nodes);
    free(pq);
}

// --- Heuristic function (unchanged) ---
static float heuristic(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

// --- Pathfinding function (MODIFIED) ---
Path* pathfinding_find_path(const Map* map, Vector2f start_pos, Vector2f end_pos) {
    // --- Coordinate conversion (unchanged) ---
    const float map_pixel_width = map->width * TILE_SIZE;
    const float map_pixel_height = map->height * TILE_SIZE;

    int start_x = (start_pos.x) / TILE_SIZE;
    int start_y = (start_pos.y) / TILE_SIZE;
    int end_x = (end_pos.x) / TILE_SIZE;
    int end_y = (end_pos.y ) / TILE_SIZE;

    // --- Boundary and wall checks (MODIFIED) ---
    if (end_y < 0 || end_y >= map->height || end_x < 0 || end_x >= map->width || map->tiles[end_y][end_x] == 1) {
        return NULL;
    }
    if (start_y < 0 || start_y >= map->height || start_x < 0 || start_x >= map->width) {
        return NULL;
    }


    // --- A* setup (MODIFIED) ---
    // --- FIX 2: Use dynamic allocation and memset ---
    Node*** all_nodes = (Node***)malloc(map->height * sizeof(Node**));
    for (int i = 0; i < map->height; i++) {
        all_nodes[i] = (Node**)malloc(map->width * sizeof(Node*));
        memset(all_nodes[i], 0, map->width * sizeof(Node*));
    }
    
    PriorityQueue* open_set = pq_create(map->width, map->height); // <-- Pass dimensions here

    Node* start_node = (Node*)malloc(sizeof(Node));
    start_node->x = start_x;
    start_node->y = start_y;
    start_node->g_score = 0;
    start_node->f_score = heuristic(start_x, start_y, end_x, end_y);
    start_node->parent = NULL;
    start_node->in_open_set = false;

    all_nodes[start_y][start_x] = start_node;
    pq_push(open_set, start_node);

    while (open_set->count > 0) {
        Node* current = pq_pop(open_set);

        if (current->x == end_x && current->y == end_y) {
            // --- Path reconstruction (unchanged) ---
            Path* path = (Path*)malloc(sizeof(Path));
            path->count = 0;
            Node* temp = current;
            while (temp != NULL) {
                path->points[path->count++] = (Vector2f){
                    (temp->x * TILE_SIZE),
                    (temp->y * TILE_SIZE)
                };
                temp = temp->parent;
            }
            for(int i = 0; i < path->count / 2; i++){
                Vector2f temp_point = path->points[i];
                path->points[i] = path->points[path->count - i - 1];
                path->points[path->count - i - 1] = temp_point;
            }
            path->current_node = 0;

            // --- Cleanup (MODIFIED) ---
            pq_destroy(open_set);
            for (int y = 0; y < map->height; y++) {
                for (int x = 0; x < map->width; x++) {
                    if(all_nodes[y][x]) free(all_nodes[y][x]);
                }
                free(all_nodes[y]);
            }
            free(all_nodes);
            return path;
        }

        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (abs(dx) == abs(dy)) continue; // Only cardinal directions

                int neighbor_x = current->x + dx;
                int neighbor_y = current->y + dy;

                if (neighbor_x < 0 || neighbor_x >= map->width || neighbor_y < 0 || neighbor_y >= map->height || map->tiles[neighbor_y][neighbor_x] == 1) {
                    continue; // Out of bounds or a wall
                }

                // NEW: Add a cost to tiles near walls to encourage the AI to avoid them.
                float cost = 1.0f;
                for (int nx = -1; nx <= 1; nx++) {
                    for (int ny = -1; ny <= 1; ny++) {
                        if (nx == 0 && ny == 0) continue;

                        int check_x = neighbor_x + nx;
                        int check_y = neighbor_y + ny;

                        if (check_x >= 0 && check_x < map->width && check_y >= 0 && check_y < map->height) {
                            if (map->tiles[check_y][check_x] == 1) {
                                cost += 15.0f;
                            }
                        }
                    }
                }

                float tentative_g_score = current->g_score + cost;
                Node* neighbor = all_nodes[neighbor_y][neighbor_x];

                if (!neighbor || tentative_g_score < neighbor->g_score) {
                    if (!neighbor) {
                        neighbor = (Node*)malloc(sizeof(Node));
                        all_nodes[neighbor_y][neighbor_x] = neighbor;
                        neighbor->in_open_set = false;
                    }
                    neighbor->x = neighbor_x;
                    neighbor->y = neighbor_y;
                    neighbor->parent = current;
                    neighbor->g_score = tentative_g_score;
                    neighbor->f_score = neighbor->g_score + heuristic(neighbor_x, neighbor_y, end_x, end_y);

                    if (!neighbor->in_open_set) {
                        pq_push(open_set, neighbor);
                    }
                }
            }
        }
    }

    // --- Cleanup on failure (MODIFIED) ---
    pq_destroy(open_set);
    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            if(all_nodes[y][x]) free(all_nodes[y][x]);
        }
        free(all_nodes[y]);
    }
    free(all_nodes);
    return NULL;
}

// --- path_destroy (unchanged) ---
void path_destroy(Path* path) {
    if (path) {
        free(path);
    }
}
