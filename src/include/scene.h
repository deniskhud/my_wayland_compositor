#ifndef SCENE_H
#define SCENE_H


#include <wlr/types/wlr_scene.h>



struct server_scene {
	struct wlr_scene* scene;
	struct wlr_scene_tree* background;
	struct wlr_scene_tree* windows;

};

void server_scene_init(struct server_scene* scene);
void server_scene_destroy(struct server_scene* scene);

#endif //SCENE_H
