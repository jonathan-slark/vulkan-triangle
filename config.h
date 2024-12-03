#include <vulkan/vulkan.h>

static const char         appname[] = "Vulkan Triangle";
static const unsigned int appver    = VK_MAKE_VERSION(1, 0, 0);
static const unsigned int appwidth  = 800;
static const unsigned int appheight = 600;

static const char *vertexshader   = "shaders/vertex.spv";
static const char *fragmentshader = "shaders/fragment.spv";
static const char *shaderentry    = "main";
static const uint32_t vertexcount = 3;
