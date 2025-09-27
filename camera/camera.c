#include "camera.h"
#include "../defs/defs.h"
#include "../player/player.h"

void camera_update(Camera* camera, const struct player* player, const Map* map, float zoom) {
    // 1. Set the camera's dimensions based on the zoom level.
    // A higher zoom means a smaller viewing area.
    camera->w = 320;
    camera->h = 180;

    // 2. Center the camera on the player using the new dimensions.
    camera->x = player->rect.x - (camera->w / 2.0f);
    camera->y = player->rect.y - (camera->h / 2.0f);

    // 3. Clamp the camera to the map boundaries (this logic remains the same)
    const float map_pixel_width = map->width * TILE_SIZE;
    const float map_pixel_height = map->height * TILE_SIZE;

    if (camera->x < 0) {
        camera->x = 0;
    }
    if (camera->y < 0) {
        camera->y = 0;
    }
    if (camera->x + camera->w > map_pixel_width) {
        camera->x = map_pixel_width - camera->w;
    }
    if (camera->y + camera->h > map_pixel_height) {
        camera->y = map_pixel_height - camera->h;
    }
}
