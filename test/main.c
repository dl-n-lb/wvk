#include <stdlib.h>
#include <stdio.h>

#include <GLFW/glfw3.h>
#include "../src/wvk_core.h"

const char *validation_layers[] = {
  "VK_LAYER_KHRONOS_validation",
};

const char *extra_extensions[] = {
  VK_EXT_DEBUG_UTILS_EXTENSION_NAME
};

const int queue_family_bits[] = {
  VK_QUEUE_GRAPHICS_BIT
};

const char **get_all_required_extensions(size_t *sz_bytes) {
  uint32_t glfw_extension_cnt = 0;
  const char **glfw_extensions =
    glfwGetRequiredInstanceExtensions(&glfw_extension_cnt); // managed  by glfw

  size_t extensions_sz_b = sizeof(char *) *  glfw_extension_cnt
    + sizeof(extra_extensions);
  const char **extensions = malloc(extensions_sz_b);
  for(uint32_t i = 0; i < glfw_extension_cnt; ++i) {
    extensions[i] = glfw_extensions[i];
  }
  for(uint32_t i = 0; i < sizeof(extra_extensions)/sizeof(char *); ++i) {
    extensions[i + glfw_extension_cnt] = extra_extensions[i];
  }
  *sz_bytes = extensions_sz_b;
  return extensions;
}

int main(void) {
  glfwInit();
  
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  GLFWwindow *win = glfwCreateWindow(800, 600, "Window", NULL, NULL);

  size_t extensions_sz_b = 0;
  const char **extensions = get_all_required_extensions(&extensions_sz_b);
  
  wvk_context ctx = wvk_context_create((wvk_context_create_info) {
      .app_info = wvk_app_info_default_fill((VkApplicationInfo) {
	  .apiVersion = VK_API_VERSION_1_2
	}),
      .instance_extensions = (wvk_range) {extensions, extensions_sz_b},
      .instance_layers = WVK_RANGE(validation_layers),
      .enable_debug_messenger = true,
      .desired_queue_family_bits = WVK_RANGE(queue_family_bits),
      .desire_surface_support = true,
    });
  
  printf("Hiya\n");

  wvk_context_destroy(ctx);
  
  free(extensions);
  
  glfwDestroyWindow(win);
  glfwTerminate();
  
  return 0;
}
