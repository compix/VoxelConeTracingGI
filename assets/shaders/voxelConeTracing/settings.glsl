#ifndef SETTINGS_GLSL
#define SETTINGS_GLSL

#define VOXEL_TEXTURE_WITH_BORDER

const int BORDER_WIDTH = 1;

const uint CLIP_LEVEL_COUNT = 6;
const uint FACE_COUNT = 6;

const float FACE_COUNT_INV = 1.0 / float(FACE_COUNT);
const float CLIP_LEVEL_COUNT_INV = 1.0 / float(CLIP_LEVEL_COUNT);

const int MAX_DIR_LIGHT_COUNT = 3;

#endif