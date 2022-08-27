#include <kos.h>

//#include <mp3/sndserver.h>
#include <plx/matrix.h>
#include <plx/prim.h>
#include <stdlib.h>
#include <tsu/font.h>
#include <sstream>
#include <string.h>

#include "lrrsoft.h"

using namespace std;

// Initialize KOS
KOS_INIT_FLAGS(INIT_DEFAULT);

// Initialize the ROM disk
extern uint8 romdisk[];
KOS_INIT_ROMDISK(romdisk);

RefPtr<Font> fnt;
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
bool bgRUp = true;
bool bgGUp = true;
bool bgBUp = true;
int score = 0;
float ring_0_y = 0;
float ring_0_x = 0;

float player_verts[8][4] = {
  {  0.2f,  0.2f,  0.2f, 0.2f },
  { -0.2f,  0.2f,  0.2f, 0.2f },
  {  0.2f, -0.2f,  0.2f, 0.2f },
  { -0.2f, -0.2f,  0.2f, 0.2f },
  {  0.2f,  0.2f, -0.2f, 0.2f },
  { -0.2f,  0.2f, -0.2f, 0.2f },
  {  0.2f, -0.2f, -0.2f, 0.2f },
  { -0.2f, -0.2f, -0.2f, 0.2f }
};

// Vertices for a cube
vector_t verts_in[8] = {
  {  0.2f,  0.2f,  0.2f, 0.2f },
  { -0.2f,  0.2f,  0.2f, 0.2f },
  {  0.2f, -0.2f,  0.2f, 0.2f },
  { -0.2f, -0.2f,  0.2f, 0.2f },
  {  0.2f,  0.2f, -0.2f, 0.2f },
  { -0.2f,  0.2f, -0.2f, 0.2f },
  {  0.2f, -0.2f, -0.2f, 0.2f },
  { -0.2f, -0.2f, -0.2f, 0.2f }
};

// Normals for a cube
vector_t normals[6] = {
  {  0.0f,  0.0f,  1.0f, 0.0f },
  {  0.0f,  0.0f, -1.0f, 0.0f },
  {  1.0f,  0.0f,  0.0f, 0.0f },
  { -1.0f,  0.0f,  0.0f, 0.0f },
  {  0.0f,  1.0f,  0.0f, 0.0f },
  {  0.0f, -1.0f,  0.0f, 0.0f }
};

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

// Vertices for a cube
vector_t ring_verts_in_list[5][8] = {
  {
    {  0.2f,  0.2f,  0.2f, 0.2f },
    { -0.2f,  0.2f,  0.2f, 0.2f },
    {  0.2f, -0.2f,  0.2f, 0.2f },
    { -0.2f, -0.2f,  0.2f, 0.2f },
    {  0.2f,  0.2f, -0.2f, 0.2f },
    { -0.2f,  0.2f, -0.2f, 0.2f },
    {  0.2f, -0.2f, -0.2f, 0.2f },
    { -0.2f, -0.2f, -0.2f, 0.2f }
  },
  {
    {  0.2f,  0.2f,  0.2f, 0.2f },
    { -0.2f,  0.2f,  0.2f, 0.2f },
    {  0.2f, -0.2f,  0.2f, 0.2f },
    { -0.2f, -0.2f,  0.2f, 0.2f },
    {  0.2f,  0.2f, -0.2f, 0.2f },
    { -0.2f,  0.2f, -0.2f, 0.2f },
    {  0.2f, -0.2f, -0.2f, 0.2f },
    { -0.2f, -0.2f, -0.2f, 0.2f }
  },
  {
    {  0.2f,  0.2f,  0.2f, 0.2f },
    { -0.2f,  0.2f,  0.2f, 0.2f },
    {  0.2f, -0.2f,  0.2f, 0.2f },
    { -0.2f, -0.2f,  0.2f, 0.2f },
    {  0.2f,  0.2f, -0.2f, 0.2f },
    { -0.2f,  0.2f, -0.2f, 0.2f },
    {  0.2f, -0.2f, -0.2f, 0.2f },
    { -0.2f, -0.2f, -0.2f, 0.2f }
  },
  {
    {  0.2f,  0.2f,  0.2f, 0.2f },
    { -0.2f,  0.2f,  0.2f, 0.2f },
    {  0.2f, -0.2f,  0.2f, 0.2f },
    { -0.2f, -0.2f,  0.2f, 0.2f },
    {  0.2f,  0.2f, -0.2f, 0.2f },
    { -0.2f,  0.2f, -0.2f, 0.2f },
    {  0.2f, -0.2f, -0.2f, 0.2f },
    { -0.2f, -0.2f, -0.2f, 0.2f }
  },
  {
    {  0.2f,  0.2f,  0.2f, 0.2f },
    { -0.2f,  0.2f,  0.2f, 0.2f },
    {  0.2f, -0.2f,  0.2f, 0.2f },
    { -0.2f, -0.2f,  0.2f, 0.2f },
    {  0.2f,  0.2f, -0.2f, 0.2f },
    { -0.2f,  0.2f, -0.2f, 0.2f },
    {  0.2f, -0.2f, -0.2f, 0.2f },
    { -0.2f, -0.2f, -0.2f, 0.2f }
  }
};

vector_t ring_verts_in[8] = {
  {  0.2f,  0.2f,  0.2f, 0.2f },
  { -0.2f,  0.2f,  0.2f, 0.2f },
  {  0.2f, -0.2f,  0.2f, 0.2f },
  { -0.2f, -0.2f,  0.2f, 0.2f },
  {  0.2f,  0.2f, -0.2f, 0.2f },
  { -0.2f,  0.2f, -0.2f, 0.2f },
  {  0.2f, -0.2f, -0.2f, 0.2f },
  { -0.2f, -0.2f, -0.2f, 0.2f }
};

// Normals for a cube
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
  pvr_poly_cxt_txr(
    &context,
    PVR_LIST_OP_POLY,
    PVR_TXRFMT_RGB565 | PVR_TXRFMT_TWIDDLED,
    256,
    256,
    texture,
    PVR_FILTER_NONE
  );

  context.gen.culling = PVR_CULLING_CW;
  context.txr.mipmap  = PVR_MIPMAP_ENABLE;

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
//  texMemory[1] = loadTexture("/rd/lrrlogo.pvr");
//  texMemory[2] = loadTexture("/rd/ihorner.pvr");
//  texMemory[3] = loadTexture("/rd/gstar.pvr");
//  texMemory[4] = loadTexture("/rd/turner.pvr");
//  texMemory[5] = loadTexture("/rd/savidan.pvr");

//  for (int i = 0; i < 6; ++i)
  texHeaders[0] = createTexHeader(texMemory[0]);

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
  fnt = new Font("/rd/axaxax.txf");
  fnt->setSize(24.0f);
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
  fnt->draw(10.0f, 60.0f, 10.0f, player_y_string);

  char *player_x_string = concat_char("Player X: ", int_to_char(playerPosX * 100));
  fnt->draw(10.0f, 90.0f, 10.0f, player_x_string);

  char *ring_0_y_string = concat_char("Ring o Y: ", int_to_char(ring_0_y * 100));
  fnt->draw(10.0f, 120.0f, 10.0f, ring_0_y_string);

  char *ring_0_x_string = concat_char("Ring o X: ", int_to_char(ring_0_x * 100));
  fnt->draw(10.0f, 150.0f, 10.0f, ring_0_x_string);
}

void handle_rings()
{
  for (int q = 0; q < 5; ++q)
  {
    rings[q][2] += 0.1f;
    for (int i = 0; i < 8; ++i) {
      ring_pool[q][i][2] = ring_verts[i][2] + rings[q][2];
    }
    if (rings[q][2] > 0.0f)
    {
      ring_0_y = rings[q][1];
      ring_0_x = rings[q][0];
      if(
        playerPosX < ring_0_x + 0.5f &&
        playerPosX > ring_0_x - 0.5f &&
        playerPosY < ring_0_y + 0.5f &&
        playerPosY > ring_0_y - 0.5f
      )
      {
        score++;
      }
      rings[q][0] = -6.0f + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(6.0f))) + 3;
      rings[q][1] = -4.0f + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(4.0f))) + 2;
      rings[q][2] = -60.0f;
      for (int b = 0; b < 8; ++b)
      {
        ring_pool[q][b][0] = ring_verts[b][0] + rings[q][0];
        ring_pool[q][b][1] = ring_verts[b][1] + rings[q][1];
        ring_pool[q][b][2] = ring_verts[b][2] - rings[q][2];
      }
    }
    for (int i = 0; i < 8; ++i)
      ring_verts_in_list[q][i] = {
        ring_pool[q][i][0],
        ring_pool[q][i][1],
        ring_pool[q][i][2],
        ring_pool[q][i][3]
      };
  }
}

void handle_input()
{
  maple_device_t *controller = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
  cont_state_t *controllerState = reinterpret_cast<cont_state_t*>(maple_dev_status(controller));
  if (controllerState->buttons & CONT_START) {
    exitProgram = true;
  }
  if (controllerState->buttons & CONT_DPAD_UP && playerPosY < 2.0f) {
    playerPosY += 0.1f;
    for (int i = 0; i < 8; ++i)
      player_verts[i][1] += 0.1f;
  }
  if (controllerState->buttons & CONT_DPAD_DOWN && playerPosY > -2.0f) {
    playerPosY += -0.1f;
    for (int i = 0; i < 8; ++i)
      player_verts[i][1] -= 0.1f;
  }
  if (controllerState->buttons & CONT_DPAD_RIGHT && playerPosX < 3.0f) {
    playerPosX += 0.1f;
    for (int i = 0; i < 8; ++i)
      player_verts[i][0] += 0.1f;
  }
  if (controllerState->buttons & CONT_DPAD_LEFT && playerPosX > -3.0f) {
    playerPosX += -0.1f;
    for (int i = 0; i < 8; ++i)
      player_verts[i][0] -= 0.1f;
  }

  for (int i = 0; i < 8; ++i)
    verts_in[i] = {
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
  plx_mat3d_push();

  cycle_background_color();
  // Set a background color
  pvr_set_bg_color(bgR, bgG, bgB);

  // Do lighting calculations
  plx_mat_identity();
  plx_mat3d_apply(PLX_MAT_MODELVIEW);

  // Transform light position
  vector_t light = lightPosition;
  mat_trans_single4(light.x, light.y, light.z, light.w);

  // Update player position
  //plx_mat3d_translate(playerPosX, playerPosY, 0.0f);

  handle_input();
  handle_rings();
  // Transform normals
  vector_t transformedNormals[6];
  for (int i = 0; i < 6; ++i)
  {
    mat_trans_normal3_nomod(
      normals[i].x, normals[i].y, normals[i].z,
      transformedNormals[i].x, transformedNormals[i].y, transformedNormals[i].z
    );
  }

  // Transform vertices into camera space
  vector_t lightVertices[8];
  plx_mat_transform(verts_in, lightVertices, 8, 4 * sizeof(float));
  plx_mat_transform(ring_verts_in_list[0], lightVertices, 8, 4 * sizeof(float));

  // Transform vertices for graphics chip
  plx_mat_identity();
  plx_mat3d_apply_all();

  vector_t transformedVerts[8];
  plx_mat_transform(verts_in, transformedVerts, 8, 4 * sizeof(float));

  vector_t ringTransformedVerts[5][8];
  for (int i = 0; i < 5; ++i)
    plx_mat_transform(ring_verts_in_list[i], ringTransformedVerts[i], 8, 4 * sizeof(float));

  plx_mat3d_pop();

  // Wait for the PVR to accept a frame
  pvr_wait_ready();

  pvr_scene_begin();
  pvr_list_begin(PVR_LIST_OP_POLY);

  pvr_prim(&texHeaders[0], sizeof(pvr_poly_hdr_t));
  //pvr_prim(&nontexturedHeader, sizeof(pvr_poly_hdr_t));
  submitVertex(light, lightVertices[0], transformedVerts[0], transformedNormals[0], 1.0f, 0.0f);
  submitVertex(light, lightVertices[1], transformedVerts[1], transformedNormals[0], 0.0f, 0.0f);
  submitVertex(light, lightVertices[2], transformedVerts[2], transformedNormals[0], 1.0f, 1.0f);
  submitVertex(light, lightVertices[3], transformedVerts[3], transformedNormals[0], 0.0f, 1.0f, true);

  submitVertex(light, lightVertices[5], transformedVerts[5], transformedNormals[1], 1.0f, 0.0f);
  submitVertex(light, lightVertices[4], transformedVerts[4], transformedNormals[1], 0.0f, 0.0f);
  submitVertex(light, lightVertices[7], transformedVerts[7], transformedNormals[1], 1.0f, 1.0f);
  submitVertex(light, lightVertices[6], transformedVerts[6], transformedNormals[1], 0.0f, 1.0f, true);

  submitVertex(light, lightVertices[4], transformedVerts[4], transformedNormals[2], 1.0f, 0.0f);
  submitVertex(light, lightVertices[0], transformedVerts[0], transformedNormals[2], 0.0f, 0.0f);
  submitVertex(light, lightVertices[6], transformedVerts[6], transformedNormals[2], 1.0f, 1.0f);
  submitVertex(light, lightVertices[2], transformedVerts[2], transformedNormals[2], 0.0f, 1.0f, true);

  submitVertex(light, lightVertices[1], transformedVerts[1], transformedNormals[3], 1.0f, 0.0f);
  submitVertex(light, lightVertices[5], transformedVerts[5], transformedNormals[3], 0.0f, 0.0f);
  submitVertex(light, lightVertices[3], transformedVerts[3], transformedNormals[3], 1.0f, 1.0f);
  submitVertex(light, lightVertices[7], transformedVerts[7], transformedNormals[3], 0.0f, 1.0f, true);

  submitVertex(light, lightVertices[4], transformedVerts[4], transformedNormals[4], 1.0f, 0.0f);
  submitVertex(light, lightVertices[5], transformedVerts[5], transformedNormals[4], 0.0f, 0.0f);
  submitVertex(light, lightVertices[0], transformedVerts[0], transformedNormals[4], 1.0f, 1.0f);
  submitVertex(light, lightVertices[1], transformedVerts[1], transformedNormals[4], 0.0f, 1.0f, true);

  submitVertex(light, lightVertices[7], transformedVerts[7], transformedNormals[5], 1.0f, 0.0f);
  submitVertex(light, lightVertices[6], transformedVerts[6], transformedNormals[5], 0.0f, 0.0f);
  submitVertex(light, lightVertices[3], transformedVerts[3], transformedNormals[5], 1.0f, 1.0f);
  submitVertex(light, lightVertices[2], transformedVerts[2], transformedNormals[5], 0.0f, 1.0f, true);


  for (int i = 0; i < 5; ++i)
  {
    pvr_prim(&nontexturedHeader, sizeof(pvr_poly_hdr_t));
    submitVertex(light, lightVertices[0], ringTransformedVerts[i][0], normals[0], 1.0f, 0.0f);
    submitVertex(light, lightVertices[1], ringTransformedVerts[i][1], normals[0], 0.0f, 0.0f);
    submitVertex(light, lightVertices[2], ringTransformedVerts[i][2], normals[0], 1.0f, 1.0f);
    submitVertex(light, lightVertices[3], ringTransformedVerts[i][3], normals[0], 0.0f, 1.0f);

    submitVertex(light, lightVertices[5], ringTransformedVerts[i][5], normals[1], 1.0f, 0.0f);
    submitVertex(light, lightVertices[4], ringTransformedVerts[i][4], normals[1], 0.0f, 0.0f);
    submitVertex(light, lightVertices[7], ringTransformedVerts[i][7], normals[1], 1.0f, 1.0f);
    submitVertex(light, lightVertices[6], ringTransformedVerts[i][6], normals[1], 0.0f, 1.0f);

    submitVertex(light, lightVertices[4], ringTransformedVerts[i][4], normals[2], 1.0f, 0.0f);
    submitVertex(light, lightVertices[0], ringTransformedVerts[i][0], normals[2], 0.0f, 0.0f);
    submitVertex(light, lightVertices[6], ringTransformedVerts[i][6], normals[2], 1.0f, 1.0f);
    submitVertex(light, lightVertices[2], ringTransformedVerts[i][2], normals[2], 0.0f, 1.0f);

    submitVertex(light, lightVertices[1], ringTransformedVerts[i][1], normals[3], 1.0f, 0.0f);
    submitVertex(light, lightVertices[5], ringTransformedVerts[i][5], normals[3], 0.0f, 0.0f);
    submitVertex(light, lightVertices[3], ringTransformedVerts[i][3], normals[3], 1.0f, 1.0f);
    submitVertex(light, lightVertices[7], ringTransformedVerts[i][7], normals[3], 0.0f, 1.0f);

    submitVertex(light, lightVertices[4], ringTransformedVerts[i][4], normals[4], 1.0f, 0.0f);
    submitVertex(light, lightVertices[5], ringTransformedVerts[i][5], normals[4], 0.0f, 0.0f);
    submitVertex(light, lightVertices[0], ringTransformedVerts[i][0], normals[4], 1.0f, 1.0f);
    submitVertex(light, lightVertices[1], ringTransformedVerts[i][1], normals[4], 0.0f, 1.0f);

    submitVertex(light, lightVertices[7], ringTransformedVerts[i][7], normals[5], 1.0f, 0.0f);
    submitVertex(light, lightVertices[6], ringTransformedVerts[i][6], normals[5], 0.0f, 0.0f);
    submitVertex(light, lightVertices[3], ringTransformedVerts[i][3], normals[5], 1.0f, 1.0f);
    submitVertex(light, lightVertices[2], ringTransformedVerts[i][2], normals[5], 0.0f, 1.0f, true);
  }

  pvr_list_begin(PVR_LIST_TR_POLY);

  char *score_string = concat_char("Score: ", int_to_char(score));
  fnt->draw(10.0f, 30.0f, 10.0f, score_string);

  display_debug();

  pvr_list_finish();
  pvr_scene_finish();

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
