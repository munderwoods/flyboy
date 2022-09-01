#include <kos.h>

//#include <mp3/sndserver.h>
#include <plx/matrix.h>
#include <plx/prim.h>
#include <stdlib.h>
#include <tsu/font.h>
#include <tsu/texture.h>
#include <sstream>
#include <string.h>
#include <plx/sprite.h>
#include <png/png.h>

#include "lrrsoft.h"

using namespace std;

// Initialize KOS
KOS_INIT_FLAGS(INIT_DEFAULT);

// Initialize the ROM disk
extern uint8 romdisk[];
KOS_INIT_ROMDISK(romdisk);

RefPtr<Font> fnt;
RefPtr<Texture> lial;
// Global variables
bool exitProgram = false;
pvr_poly_hdr_t nontexturedHeader;

float playerPosY = 0.0f;
float playerPosX = 0.0f;

pvr_ptr_t texMemory[6];
pvr_poly_hdr_t texHeaders[6];

// Lighting parameters
float    ambientLight  = 0.3f;
float    diffuseLight  = 0.7f;
vector_t lightPosition = { 0.0f, 0.0f, 10.0f, 1.0f };

// Background color parameters
float bgR = 0.00f;
float bgG = 0.00f;
float bgB = 0.00f;
// This means that this color has bottomed out and needs to come back up
bool bgRUp = true;
bool bgGUp = true;
bool bgBUp = true;

int score = 0;

// dpad or joystick
int inputMode = 0;

bool displayDebug = false;
bool displayLial = false;

class Controller {
  public:
    bool d_pad_up;
    bool d_pad_down;
    bool d_pad_right;
    bool d_pad_left;
    bool a;
    bool b;
    bool x;
    bool y;
    bool start;
    bool l;
    bool r;
    float joyX;
    float joyY;
};

Controller controller;

// The starting positions for the player's vertices. Used to compare where they've moved
// and to reset them.
float player_verts_default[8][4] = {
  {  0.2f,  0.2f,  0.2f, 0.2f },
  { -0.2f,  0.2f,  0.2f, 0.2f },
  {  0.2f, -0.2f,  0.2f, 0.2f },
  { -0.2f, -0.2f,  0.2f, 0.2f },
  {  0.2f,  0.2f, -0.2f, 0.2f },
  { -0.2f,  0.2f, -0.2f, 0.2f },
  {  0.2f, -0.2f, -0.2f, 0.2f },
  { -0.2f, -0.2f, -0.2f, 0.2f }
};

// This is where I update the position of the player from frame to frame.
float player_verts[8][4] = {
  {  0.0f,  0.2f,  0.0f, 0.2f },
  {  0.0f,  0.0f,  0.4f, 0.2f },
  {  0.2f,  0.0f,  0.0f, 0.2f },
  {  0.0f, -0.2f,  0.0f, 0.2f },
  {  0.0f,  0.0f, -0.4f, 0.2f },
  { -0.2f,  0.0f,  0.0f, 0.2f },
};

// These are the vectors that get submitted to the renderer for the player.
// They are set from the player's verts every frame after those are updated.
vector_t player_vectors[8];

// Abstracted positions for all the rings as single points in space, xyz values.
float rings[5][3] = {
  { 0.0f, 0.0f, -50.0f },
  { 0.0f, 0.0f, -40.0f },
  { 0.0f, 0.0f, -30.0f },
  { 0.0f, 0.0f, -20.0f },
  { 0.0f, 0.0f, -10.0f }
};

float ring_verts[8][4] = {
  {  0.2f,  0.2f,  0.2f, 0.2f },
  { -0.2f,  0.2f,  0.2f, 0.2f },
  {  0.2f, -0.2f,  0.2f, 0.2f },
  { -0.2f, -0.2f,  0.2f, 0.2f },
  {  0.2f,  0.2f, -0.2f, 0.2f },
  { -0.2f,  0.2f, -0.2f, 0.2f },
  {  0.2f, -0.2f, -0.2f, 0.2f },
  { -0.2f, -0.2f, -0.2f, 0.2f }
};

float ring_pool[5][8][4];

// I don't know wtf vector_t is, but I need it for the renderer, so I'm handling all the
// calculations in my own matrices then mapping them to these vector_t's in a very stupid way.
// You can't set the individual values in the vectors directly... Same thing is done for the
// player higher up.
vector_t rings_vectors[5][8];

// Normals for a cube. These are for lighting. That's all I know.
vector_t player_normals[6] = {
  {  0.0f,  0.0f,  1.0f, 0.0f },
  {  0.0f,  0.0f, -1.0f, 0.0f },
  {  1.0f,  0.0f,  0.0f, 0.0f },
  { -1.0f,  0.0f,  0.0f, 0.0f },
  {  0.0f,  1.0f,  0.0f, 0.0f },
  {  0.0f, -1.0f,  0.0f, 0.0f }
};

vector_t ring_normals[6] = {
  {  0.0f,  0.0f,  1.0f, 0.0f },
  {  0.0f,  0.0f, -1.0f, 0.0f },
  {  1.0f,  0.0f,  0.0f, 0.0f },
  { -1.0f,  0.0f,  0.0f, 0.0f },
  {  0.0f,  1.0f,  0.0f, 0.0f },
  {  0.0f, -1.0f,  0.0f, 0.0f }
};

const char * int_to_char(int i)
{
  stringstream strs;
  strs << i;
  string temp_str = strs.str();
  char const* y = temp_str.c_str();
  return y;
}
// Load a texture from the filesystem into video RAM
pvr_ptr_t loadTexture(const char *texName)
{
  const size_t HEADER_SIZE  = 32;
  const size_t TEXTURE_SIZE = 174768;

  FILE *texFile = fopen(texName, "rb");
  pvr_ptr_t textureMemory = pvr_mem_malloc(TEXTURE_SIZE);
  char header[HEADER_SIZE];
  fread(header, 1, HEADER_SIZE, texFile);
  fread(textureMemory, 1, TEXTURE_SIZE, texFile);
  fclose(texFile);

  return textureMemory;
}

// Create a tristrip header using a loaded texture
pvr_poly_hdr_t createTexHeader(pvr_ptr_t texture)
{
  pvr_poly_cxt_t context;
  pvr_poly_cxt_txr(&context, PVR_LIST_OP_POLY, PVR_TXRFMT_RGB565, 256, 256, texture, PVR_FILTER_BILINEAR);

  context.gen.culling = PVR_CULLING_CW;
//  context.txr.mipmap  = PVR_MIPMAP_ENABLE; // use this for pvr texture

  pvr_poly_hdr_t header;
  pvr_poly_compile(&header, &context);
  return header;
}

// Calculate light intensity using the Phong reflection model
float calculateDiffuseIntensity(vector_t light, vector_t point, vector_t normal)
{
  vec3f_normalize(normal.x, normal.y, normal.z);

  vector_t lightDirection;
  vec3f_sub_normalize(
    light.x, light.y, light.z,
    point.x, point.y, point.z,
    lightDirection.x, lightDirection.y, lightDirection.z
  );

  float intensity;
  vec3f_dot(
    normal.x, normal.y, normal.z,
    lightDirection.x, lightDirection.y, lightDirection.z,
    intensity
  );

  if (intensity > 0)
    return intensity;
  else
    return 0;
}

// Send a lit vertex to the graphics hardware
void submitVertex(
  vector_t light,
  vector_t lightVertex,
  vector_t vertex,
  vector_t normal,
  float u,
  float v,
  bool endOfStrip = false
)
{
  int flags = endOfStrip ? PVR_CMD_VERTEX_EOL : PVR_CMD_VERTEX;

  float intensity = calculateDiffuseIntensity(light, lightVertex, normal);
  float color = ambientLight + (intensity * diffuseLight);

  plx_vert_ffp(flags, vertex.x, vertex.y, vertex.z, 1.0f, color, color, color, u, v);
}

void Initialize()
{
  // Draw to the VMU
  maple_device_t *vmu = maple_enum_type(0, MAPLE_FUNC_LCD);
  vmu_draw_lcd(vmu, lrrsoft_logo);
  // Initialize the graphics and sound libraries
  pvr_init_defaults();
  plx_mat3d_init();
  snd_stream_init();
  // mp3_init();

  for (int a = 0; a < 5; ++a)
    for (int b = 0; b < 8; ++b)
      for (int c = 0; c < 4; ++c)
        ring_pool[a][b][c] = ring_verts[b][c];


  // Compile a 7polygon header with no texturing
  pvr_poly_cxt_t nontexturedContext;
  pvr_poly_cxt_col(&nontexturedContext, PVR_LIST_OP_POLY);
  nontexturedContext.gen.culling = PVR_CULLING_CW;
  pvr_poly_compile(&nontexturedHeader, &nontexturedContext);

  // Load all 6 textures
  texMemory[0] = loadTexture("/rd/dclogo.pvr");
  texMemory[1] = pvr_mem_malloc(256 * 256 * 2);
  png_to_texture("/rd/lial.png", texMemory[1], PNG_NO_ALPHA);
//  texMemory[1] = loadTexture("/rd/lrrlogo.pvr");
//  texMemory[2] = loadTexture("/rd/ihorner.pvr");
//  texMemory[3] = loadTexture("/rd/gstar.pvr");
//  texMemory[4] = loadTexture("/rd/turner.pvr");
//  texMemory[5] = loadTexture("/rd/savidan.pvr");

//  for (int i = 0; i < 6; ++i)
  texHeaders[0] = createTexHeader(texMemory[0]);
  texHeaders[1] = createTexHeader(texMemory[1]);

  // Set up the camera
  plx_mat3d_mode(PLX_MAT_PROJECTION);
  plx_mat3d_identity();
  plx_mat3d_perspective(60.0f, 640.0f / 480.0f, 0.1f, 100.0f);

  plx_mat3d_mode(PLX_MAT_MODELVIEW);
  plx_mat3d_identity();

  point_t cameraPosition = { 0.0f, 0.0f, 5.0f, 1.0f };
  point_t cameraTarget   = { 0.0f, 0.0f, 0.0f, 1.0f };
  vector_t cameraUp      = { 0.0f, 1.0f, 0.0f, 0.0f };
  plx_mat3d_lookat(&cameraPosition, &cameraTarget, &cameraUp);

  // Play music with looping
//  mp3_start("/rd/tucson.mp3", 1);
  fnt = new Font("/rd/pixel-font.txf");
  fnt->setSize(50.0f);
  fnt->setColor(1.0f, 1.0f, 1.0f);
  fnt->setFilter(0);
  //fnt->setColor(0.0f, 0.0f, 0.0f);
  //
  lial = new Texture("/rd/lial.png", true);
}

char * concat_char(const char *first_char, const char *second_char)
{
  char *temp_str = "";
  strcpy(temp_str, first_char);
  strcat(temp_str, second_char);
  return temp_str;
}
void display_debug()
{
  char *player_y_string = concat_char("Player Y: ", int_to_char(playerPosY * 100));
  fnt->draw(10.0f, 90.0f, 10.0f, player_y_string);

  char *player_x_string = concat_char("Player X: ", int_to_char(playerPosX * 100));
  fnt->draw(10.0f, 120.0f, 10.0f, player_x_string);

  char *joy_x_string = concat_char("Joy X: ", int_to_char(controller.joyX * 100));
  fnt->draw(10.0f, 210.0f, 10.0f, joy_x_string);
}

// Reset ring to starting position. If the player goes through a ring it resets early
// so a buffer is necessary to make sure they are always spaced evenly.
// The rings are just looping for now since it's easier.
void reset_ring(int r, float buffer)
{
  rings[r][0] = -6.0f + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(6.0f))) + 3;
  rings[r][1] = -4.0f + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(4.0f))) + 2;
  rings[r][2] = -60.0f + buffer;
  for (int b = 0; b < 8; ++b)
  {
    ring_pool[r][b][0] = ring_verts[b][0] + rings[r][0];
    ring_pool[r][b][1] = ring_verts[b][1] + rings[r][1];
    ring_pool[r][b][2] = ring_verts[b][2] - rings[r][2];
  }
}
void handle_rings()
{
  for (int r = 0; r < 5; ++r)
  {
    rings[r][2] += 0.1f;

    // move all the rings forward every frame
    for (int i = 0; i < 8; ++i) {
      ring_pool[r][i][2] = ring_verts[i][2] + rings[r][2];
    }

    // detect collision with player
    if(
      0 < rings[r][2] + 0.5f &&
      0 > rings[r][2] - 0.5f &&
      playerPosX < rings[r][0] + 0.5f &&
      playerPosX > rings[r][0] - 0.5f &&
      playerPosY < rings[r][1] + 0.5f &&
      playerPosY > rings[r][1] - 0.5f
    )
    {
      score++;
      reset_ring(r, -3.0f + rings[r][2]);
    }

    // reset for when palyer misses the ring
    if (rings[r][2] > 3.0f)
    {
      reset_ring(r, 0.0f);
    }

    // set the vectors for the rings to match their vertex arrays (wtf am I doing)
    for (int i = 0; i < 8; ++i)
      rings_vectors[r][i] = {
        ring_pool[r][i][0],
        ring_pool[r][i][1],
        ring_pool[r][i][2],
        ring_pool[r][i][3]
      };
  }
}

void reset_player_position()
{
  playerPosY = 0.0f;
  playerPosX = 0.0f;
  for (int i = 0; i < 8; ++i)
  {
    player_verts[i][0] = player_verts_default[i][0];
    player_verts[i][1] = player_verts_default[i][1];
  }
}

void handle_input()
{
  maple_device_t *mapleController = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
  cont_state_t *mapleControllerState = reinterpret_cast<cont_state_t*>(maple_dev_status(mapleController));

  // Map the maple controller to my much more manageable abstraction
  Controller previousController = controller;
  controller.d_pad_up = mapleControllerState->buttons & CONT_DPAD_UP;
  controller.d_pad_down = mapleControllerState->buttons & CONT_DPAD_DOWN;
  controller.d_pad_left = mapleControllerState->buttons & CONT_DPAD_LEFT;
  controller.d_pad_right = mapleControllerState->buttons & CONT_DPAD_RIGHT;
  controller.a = mapleControllerState->buttons & CONT_A;
  controller.b = mapleControllerState->buttons & CONT_B;
  controller.x = mapleControllerState->buttons & CONT_X;
  controller.y = mapleControllerState->buttons & CONT_Y;
  controller.l = mapleControllerState->buttons & CONT_C;
  controller.r = mapleControllerState->buttons & CONT_D;
  controller.start = mapleControllerState->buttons & CONT_START;
  controller.joyX = mapleControllerState->joyx;
  controller.joyY = mapleControllerState->joyy;

  if (controller.a != previousController.a && controller.a)
  {
    displayDebug = !displayDebug;
  }

  // This is just a test for sprites
  if (controller.b != previousController.b && controller.b)
  {
    displayLial = !displayLial;
  }

  // Input mode 0 is D Pad, 1 is joystick. Start switches between them.
  if (inputMode == 0)
  {
    if (controller.start != previousController.start && controller.start) {
      inputMode = 1;
      reset_player_position();
    }
    if (controller.d_pad_up && playerPosY < 2.0f) {
      playerPosY += 0.1f;
      for (int i = 0; i < 8; ++i)
        player_verts[i][1] += 0.1f;
    }
    if (controller.d_pad_down && playerPosY > -2.0f) {
      playerPosY += -0.1f;
      for (int i = 0; i < 8; ++i)
        player_verts[i][1] -= 0.1f;
    }
    if (controller.d_pad_right && playerPosX < 3.0f) {
      playerPosX += 0.1f;
      for (int i = 0; i < 8; ++i)
        player_verts[i][0] += 0.1f;
    }
    if (controller.d_pad_left && playerPosX > -3.0f) {
      playerPosX += -0.1f;
      for (int i = 0; i < 8; ++i)
        player_verts[i][0] -= 0.1f;
    }
  } else
  {
    if (controller.start != previousController.start && controller.start) {
      inputMode = 0;
      reset_player_position();
    }
    if (mapleControllerState->joyx != 0)
    {
      controller.joyX = mapleControllerState->joyx / 40.0f;
      if(controller.joyX > 3.0f)
      {
        playerPosX = 3.0f;
      }
      else if(controller.joyX < -3.0f)
      {
        playerPosX = -3.0f;
      } else {
        playerPosX = controller.joyX;
      }
      for (int i = 0; i < 8; ++i)
        player_verts[i][0] = player_verts_default[i][0] + playerPosX;
    }

    if (mapleControllerState->joyy != 0)
    {
      controller.joyY = mapleControllerState->joyy / 40.0f;
      if(controller.joyY > 2.0f)
      {
        playerPosY = 2.0f;
      }
      else if(controller.joyY < -2.0f)
      {
        playerPosY = -2.0f;
      } else {
        playerPosY = controller.joyY;
      }
      for (int i = 0; i < 8; ++i)
        player_verts[i][1] = player_verts_default[i][1] - playerPosY;
    }
  }

  // Map player's vectors to vertices (again wtf am I doing)
  for (int i = 0; i < 8; ++i)
    player_vectors[i] = {
      player_verts[i][0],
      player_verts[i][1],
      player_verts[i][2],
      player_verts[i][3]
    };

}

void cycle_background_color()
{
  if (bgRUp == true)
    bgR += 0.01f;
  else
    bgR -= 0.01f;

  if (bgR >= 0.95f)
    bgRUp = false;
  if (bgR <= 0.25f)
    bgRUp = true;

  if (bgGUp == true)
    bgG += 0.02f;
  else
    bgG -= 0.02f;

  if (bgG >= 0.95f)
    bgGUp = false;
  if (bgG <= 0.25f)
    bgGUp = true;

  if (bgBUp == true)
    bgB += 0.03f;
  else
    bgB -= 0.03f;

  if (bgB >= 0.95f)
    bgBUp = false;
  if (bgB <= 0.25f)
    bgBUp = true;
}

void Update()
{
  handle_input();
  handle_rings();
  cycle_background_color();

  plx_mat3d_push();
  // Do lighting calculations
  plx_mat_identity();
  plx_mat3d_apply(PLX_MAT_MODELVIEW);

  // Transform light position
  vector_t light = lightPosition;
  mat_trans_single4(light.x, light.y, light.z, light.w);

  // Transform normals
  vector_t transformedNormals[6];
  for (int i = 0; i < 6; ++i)
  {
    mat_trans_normal3_nomod(
      player_normals[i].x, player_normals[i].y, player_normals[i].z,
      transformedNormals[i].x, transformedNormals[i].y, transformedNormals[i].z
    );
  }

  // Transform vertices into camera space
  vector_t lightVertices[8];
  plx_mat_transform(player_vectors, lightVertices, 8, 4 * sizeof(float));
  plx_mat_transform(rings_vectors[0], lightVertices, 8, 4 * sizeof(float));

  // Transform vertices for graphics chip
  plx_mat_identity();
  plx_mat3d_apply_all();

  vector_t transformedVerts[8];
  plx_mat_transform(player_vectors, transformedVerts, 8, 4 * sizeof(float));

  vector_t ringTransformedVerts[5][8];
  for (int i = 0; i < 5; ++i)
    plx_mat_transform(rings_vectors[i], ringTransformedVerts[i], 8, 4 * sizeof(float));

  plx_mat3d_pop();

  // Wait for the PVR to accept a frame
  pvr_wait_ready();

  pvr_scene_begin();
  pvr_list_begin(PVR_LIST_OP_POLY);

  pvr_prim(&texHeaders[1], sizeof(pvr_poly_hdr_t));
  //pvr_prim(&nontexturedHeader, sizeof(pvr_poly_hdr_t));
  submitVertex(light, lightVertices[0], transformedVerts[0], transformedNormals[0], 1.0f, 0.0f);
  submitVertex(light, lightVertices[1], transformedVerts[1], transformedNormals[0], 0.0f, 0.0f);
  submitVertex(light, lightVertices[2], transformedVerts[2], transformedNormals[0], 1.0f, 1.0f, true);

  submitVertex(light, lightVertices[1], transformedVerts[1], transformedNormals[0], 1.0f, 0.0f);
  submitVertex(light, lightVertices[3], transformedVerts[3], transformedNormals[0], 0.0f, 0.0f);
  submitVertex(light, lightVertices[2], transformedVerts[2], transformedNormals[0], 1.0f, 1.0f, true);

  submitVertex(light, lightVertices[1], transformedVerts[1], transformedNormals[0], 1.0f, 0.0f);
  submitVertex(light, lightVertices[5], transformedVerts[5], transformedNormals[0], 0.0f, 0.0f);
  submitVertex(light, lightVertices[3], transformedVerts[3], transformedNormals[0], 1.0f, 1.0f, true);

  submitVertex(light, lightVertices[0], transformedVerts[0], transformedNormals[0], 1.0f, 0.0f);
  submitVertex(light, lightVertices[4], transformedVerts[4], transformedNormals[0], 0.0f, 1.0f);
  submitVertex(light, lightVertices[5], transformedVerts[5], transformedNormals[0], 1.0f, 1.0f, true);

  submitVertex(light, lightVertices[0], transformedVerts[0], transformedNormals[0], 1.0f, 0.0f);
  submitVertex(light, lightVertices[2], transformedVerts[2], transformedNormals[0], 0.0f, 1.0f);
  submitVertex(light, lightVertices[4], transformedVerts[4], transformedNormals[0], 1.0f, 1.0f, true);

  submitVertex(light, lightVertices[2], transformedVerts[2], transformedNormals[0], 1.0f, 0.0f);
  submitVertex(light, lightVertices[3], transformedVerts[3], transformedNormals[0], 0.0f, 1.0f);
  submitVertex(light, lightVertices[4], transformedVerts[4], transformedNormals[0], 1.0f, 1.0f, true);

  submitVertex(light, lightVertices[1], transformedVerts[1], transformedNormals[0], 1.0f, 0.0f);
  submitVertex(light, lightVertices[0], transformedVerts[0], transformedNormals[0], 0.0f, 1.0f);
  submitVertex(light, lightVertices[5], transformedVerts[5], transformedNormals[0], 1.0f, 1.0f, true);

  submitVertex(light, lightVertices[3], transformedVerts[3], transformedNormals[0], 1.0f, 0.0f);
  submitVertex(light, lightVertices[5], transformedVerts[5], transformedNormals[0], 0.0f, 1.0f);
  submitVertex(light, lightVertices[4], transformedVerts[4], transformedNormals[0], 0.0f, 1.0f, true);


  for (int i = 0; i < 5; ++i)
  {
    pvr_prim(&nontexturedHeader, sizeof(pvr_poly_hdr_t));
    submitVertex(light, lightVertices[0], ringTransformedVerts[i][0], ring_normals[0], 1.0f, 0.0f);
    submitVertex(light, lightVertices[1], ringTransformedVerts[i][1], ring_normals[0], 0.0f, 0.0f);
    submitVertex(light, lightVertices[2], ringTransformedVerts[i][2], ring_normals[0], 1.0f, 1.0f);
    submitVertex(light, lightVertices[3], ringTransformedVerts[i][3], ring_normals[0], 0.0f, 1.0f);

    submitVertex(light, lightVertices[5], ringTransformedVerts[i][5], ring_normals[1], 1.0f, 0.0f);
    submitVertex(light, lightVertices[4], ringTransformedVerts[i][4], ring_normals[1], 0.0f, 0.0f);
    submitVertex(light, lightVertices[7], ringTransformedVerts[i][7], ring_normals[1], 1.0f, 1.0f);
    submitVertex(light, lightVertices[6], ringTransformedVerts[i][6], ring_normals[1], 0.0f, 1.0f);

    submitVertex(light, lightVertices[4], ringTransformedVerts[i][4], ring_normals[2], 1.0f, 0.0f);
    submitVertex(light, lightVertices[0], ringTransformedVerts[i][0], ring_normals[2], 0.0f, 0.0f);
    submitVertex(light, lightVertices[6], ringTransformedVerts[i][6], ring_normals[2], 1.0f, 1.0f);
    submitVertex(light, lightVertices[2], ringTransformedVerts[i][2], ring_normals[2], 0.0f, 1.0f);

    submitVertex(light, lightVertices[1], ringTransformedVerts[i][1], ring_normals[3], 1.0f, 0.0f);
    submitVertex(light, lightVertices[5], ringTransformedVerts[i][5], ring_normals[3], 0.0f, 0.0f);
    submitVertex(light, lightVertices[3], ringTransformedVerts[i][3], ring_normals[3], 1.0f, 1.0f);
    submitVertex(light, lightVertices[7], ringTransformedVerts[i][7], ring_normals[3], 0.0f, 1.0f);

    submitVertex(light, lightVertices[4], ringTransformedVerts[i][4], ring_normals[4], 1.0f, 0.0f);
    submitVertex(light, lightVertices[5], ringTransformedVerts[i][5], ring_normals[4], 0.0f, 0.0f);
    submitVertex(light, lightVertices[0], ringTransformedVerts[i][0], ring_normals[4], 1.0f, 1.0f);
    submitVertex(light, lightVertices[1], ringTransformedVerts[i][1], ring_normals[4], 0.0f, 1.0f);

    submitVertex(light, lightVertices[7], ringTransformedVerts[i][7], ring_normals[5], 1.0f, 0.0f);
    submitVertex(light, lightVertices[6], ringTransformedVerts[i][6], ring_normals[5], 0.0f, 0.0f);
    submitVertex(light, lightVertices[3], ringTransformedVerts[i][3], ring_normals[5], 1.0f, 1.0f);
    submitVertex(light, lightVertices[2], ringTransformedVerts[i][2], ring_normals[5], 0.0f, 1.0f, true);
  }

  pvr_list_begin(PVR_LIST_TR_POLY);

  char *score_string = concat_char("Score: ", int_to_char(score));
  fnt->draw(10.0f, 60.0f, 10.0f, score_string);

  if (displayLial)
  {
    lial->sendHdr(PVR_LIST_TR_POLY);
    plx_spr_inp(lial->getW(), lial->getH(), 500, 320, 20, 0xaaaaaaaa);
  }
  if (displayDebug)
  {
    display_debug();
  }

  pvr_list_finish();
  pvr_scene_finish();

  pvr_set_bg_color(bgR, bgG, bgB);
}

void Cleanup()
{
  // Clear the VMU screen
  maple_device_t *vmu = maple_enum_type(0, MAPLE_FUNC_LCD);
  vmu_draw_lcd(vmu, vmu_clear);

  // Clean up the texture memory we allocated earlier
  pvr_mem_free(texMemory[5]);
  pvr_mem_free(texMemory[4]);
  pvr_mem_free(texMemory[3]);
  pvr_mem_free(texMemory[2]);
  pvr_mem_free(texMemory[1]);
  pvr_mem_free(texMemory[0]);

  // Stop playing music
//  mp3_stop();

  // Shut down libraries we used
//  mp3_shutdown();
  snd_stream_shutdown();
  pvr_shutdown();
}

int main(int argc, char *argv[])
{
  Initialize();

  while (!exitProgram)
  {
    Update();
  }

  Cleanup();

  return 0;
}
