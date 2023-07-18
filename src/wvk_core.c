#include "wvk_core.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>


VkResult vkCreateDebugUtilsMessengerEXT(VkInstance instance,
					const VkDebugUtilsMessengerCreateInfoEXT
					*create_info,
					const VkAllocationCallbacks *alloc,
					VkDebugUtilsMessengerEXT *out_messenger)
{
  PFN_vkCreateDebugUtilsMessengerEXT fn = (PFN_vkCreateDebugUtilsMessengerEXT)
    vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if (fn) {
    return fn(instance, create_info, alloc, out_messenger);
  }
  return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void vkDestroyDebugUtilsMessengerEXT(VkInstance instance,
				     VkDebugUtilsMessengerEXT messenger,
				     const VkAllocationCallbacks *alloc) {
  PFN_vkDestroyDebugUtilsMessengerEXT fn = (PFN_vkDestroyDebugUtilsMessengerEXT)
    vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if (fn) {
    fn(instance, messenger, alloc);
  }
}

VKAPI_ATTR VkBool32
VKAPI_CALL wvk_default_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT msg_sev,
			  VkDebugUtilsMessageTypeFlagsEXT msg_type,
		          const VkDebugUtilsMessengerCallbackDataEXT *cb_data,
			  void *user_data) {
  (void) msg_sev;
  (void) msg_type;
  (void) user_data;
  fprintf(stderr, "Validation Layer: %s\n", cb_data->pMessage);

  return VK_FALSE;
}

VkApplicationInfo wvk_app_info_default_fill(VkApplicationInfo info) {
  VkApplicationInfo out = info;
  out.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

  out.pApplicationName = (!out.pApplicationName) ?
    "wvk application" : out.pApplicationName;

  out.applicationVersion = (!out.applicationVersion) ?
    VK_MAKE_VERSION(0, 0, 1) : out.applicationVersion;

  out.pEngineName = (!out.pEngineName) ?
    "no engine" : out.pEngineName;

  out.engineVersion = (!out.engineVersion) ?
    VK_MAKE_VERSION(0, 0, 1) : out.engineVersion;

  assert(out.apiVersion && "An API version must be provided and cannot be default");

  return out;
}

VkDebugUtilsMessengerCreateInfoEXT
wvk_debug_messenger_create_info_default_fill
(VkDebugUtilsMessengerCreateInfoEXT info) {
  VkDebugUtilsMessengerCreateInfoEXT res = info;
  res.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  if (res.messageSeverity == 0) {
    res.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  }

  if (res.messageType == 0) {
    res.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT    |
                      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  }

  if (!res.pfnUserCallback) {
    res.pfnUserCallback = wvk_default_debug_callback;
  }

  return res;
}

// we only use the desired_queue_family_bits of the context, but users can use
// the p_dev_heuristic_user_data to pass more info if necessary
uint32_t wvk_default_device_heuristic(VkPhysicalDevice p_dev,
				      int *desired_queue_families,
				      void *user_data) {
  
}

VkPhysicalDevice
wvk_select_physical_device(wvk_p_dev_heuristic heuristic,
			   VkInstance instance, wvk_context_create_info info) {
  uint32_t p_dev_cnt = 0;
  vkEnumeratePhysicalDevices(instance, &p_dev_cnt, NULL);
  if (p_dev_cnt == 0) {
    fprintf(stderr, "Failed to find a Vulkan compatible physical device\n");
    assert(false);
  }
  VkPhysicalDevice *p_devs = malloc(sizeof(VkPhysicalDevice) * p_dev_cnt);
  vkEnumeratePhysicalDevices(instance, &p_dev_cnt, p_devs);
  uint32_t *p_dev_ratings = calloc(0, sizeof(*p_dev_ratings) * p_dev_cnt);

  if(!heuristic) {
    heuristic = wvk_default_p_dev_heuristic;
  }
  
  for(uint32_t i = 0; i < p_dev_cnt; ++i) {
    p_dev_ratings[i] = heuristic(p_devs[i], info.desired_queue_family_bits,
				 info.p_dev_heuristic_user_data);
  }

  uint32_t max = 0, max_id = p_dev_cnt;
  for(uint32_t i = 0; i < p_dev_cnt; ++i) {
    if (p_dev_ratings[i] > max) {
      max = p_dev_ratings[i];
      max_id = i;
    }
  }

  if (max_id == p_dev_cnt) {
    // failed to find
    free(p_devs);
    free(p_dev_ratings);
    return VK_NULL_HANDLE;
  }

  VkPhysicalDevice out = p_devs[max_id];
  free(p_devs);
  free(p_dev_ratings);
  return out;
}

wvk_context wvk_context_create(wvk_context_create_info info) {
  wvk_context ctx = {
    .alloc_cb = info.alloc_cb,
    .debug_is_enabled = info.enable_debug_messenger,
  };
  
  VkInstanceCreateInfo create_info = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &info.app_info,
    .enabledExtensionCount = (uint32_t)info.instance_extensions.sz / sizeof(char *), // 0 is default so this is fine
    .ppEnabledExtensionNames = info.instance_extensions.data,
    .enabledLayerCount = (uint32_t)info.instance_layers.sz / sizeof(char *), // 0 is default so this is fine
    .ppEnabledLayerNames = info.instance_layers.data,
  };

  VkResult res = vkCreateInstance(&create_info, info.alloc_cb, &ctx.instance);
  if (res != VK_SUCCESS) {
    fprintf(stderr, "Failed to create VkInstance for wvk_context\n");
    assert(false); // TODO: fail better
  }

  if (info.enable_debug_messenger) {
    VkDebugUtilsMessengerCreateInfoEXT dbg_info =
      wvk_debug_messenger_create_info_default_fill(info.debug_create_info);
    res = vkCreateDebugUtilsMessengerEXT(ctx.instance,
					 &dbg_info,
					 info.alloc_cb,
					 &ctx.messenger);
    if (res != VK_SUCCESS) {
      fprintf(stderr, "Failed to create VkDebugUtilsMessengerEXT for wvk_context\n");
      assert(false);
    }
  }

  
  // // create an effective map between info.desired_queue_family_bits
  // // and q_families such that the index of any queue family is the same in both
  // uint32_t q_family_cnt = info.desired_queue_family_bits.sz / sizeof(int);
  // wvk_queue_family *q_families = malloc(sizeof(*q_families) * q_family_cnt);
  // for(uint32_t i = 0; i < q_family_cnt; ++i) {
    
  // }
  
  return ctx;
}

void wvk_context_destroy(wvk_context ctx) {
  vkDestroyDebugUtilsMessengerEXT(ctx.instance, ctx.messenger, ctx.alloc_cb);
  vkDestroyInstance(ctx.instance, ctx.alloc_cb);
}
