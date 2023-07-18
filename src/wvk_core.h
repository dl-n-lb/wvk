#ifndef WVK_CORE_H_
#define WVK_CORE_H_

#include <vulkan/vulkan.h>
#include <stdbool.h>

// TODO; REPLACE ALL MALLOC CALLS WITH A CUSTOMIZEABLE ALLOCATOR

VkResult vkCreateDebugUtilsMessengerEXT(VkInstance instance,
					const VkDebugUtilsMessengerCreateInfoEXT
					*create_info,
					const VkAllocationCallbacks *alloc,
					VkDebugUtilsMessengerEXT *out_messenger);

void vkDestroyDebugUtilsMessengerEXT(VkInstance instance,
				     VkDebugUtilsMessengerEXT messenger,
				     const VkAllocationCallbacks *alloc);

VkBool32
VKAPI_CALL wvk_default_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT msg_sev,
				  VkDebugUtilsMessageTypeFlagsEXT msg_type,
				  const VkDebugUtilsMessengerCallbackDataEXT
				  *cb_data,
				  void *user_data);

typedef struct {
  const void *data;
  size_t sz; // size in bytes(?)
} wvk_range;

#define WVK_RANGE(v) (wvk_range) { .data = &(v), .sz = sizeof((v)) }

typedef struct {
  int value;
  bool exists;
} wvk_queue_family;

typedef uint32_t (*wvk_p_dev_heuristic)(VkPhysicalDevice, int *,  void *);

typedef struct {
  VkInstance instance;

  bool debug_is_enabled;
  VkDebugUtilsMessengerEXT messenger;
  // TODO: Fix this
  // Not needed: wvk_range queue_family_indices;
  
  VkPhysicalDevice physical_dev;

  VkDevice dev;
  VkQueue *dev_queues; // TODO: associate each queue with the correct family

  const VkAllocationCallbacks *alloc_cb;
} wvk_context;

typedef struct {
  VkApplicationInfo app_info;
  wvk_range instance_extensions; // range of char *
  wvk_range instance_layers; // range of char *

  bool enable_debug_messenger;
  VkDebugUtilsMessengerCreateInfoEXT debug_create_info;

  wvk_range desired_queue_family_bits; // range of ints
  bool desire_surface_support;
  wvk_p_dev_heuristic heuristic_fn;
  void *p_dev_heuristic_user_data;
  
  const VkAllocationCallbacks *alloc_cb;
} wvk_context_create_info;

VkApplicationInfo wvk_app_info_default_fill(VkApplicationInfo info);

VkDebugUtilsMessengerCreateInfoEXT
wvk_debug_messenger_create_info_default_fill(VkDebugUtilsMessengerCreateInfoEXT info);

// return positive on success, 0 on failure
uint32_t wvk_default_p_dev_heuristic(VkPhysicalDevice p_dev,
				     int *desired_queue_families,
				     void *user_data);

VkPhysicalDevice
wvk_select_physical_device(wvk_p_dev_heuristic heuristic,
			   VkInstance inst, wvk_context_create_info info);

wvk_context wvk_context_create(wvk_context_create_info info);

void wvk_context_destroy(wvk_context ctx);

#endif //WVK_CORE_H_
