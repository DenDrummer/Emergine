# Presentation modes
A short description off the different presentation modes for the swap chain and their pros and cons.
sources:
- https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain
- https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkPresentModeKHR.html
- https://www.khronos.org/registry/vulkan/specs/1.2-khr-extensions/html/chap46.html

### VK_PRESENT_MODE_IMMEDIATE_KHR
__**Tutorial:**__
Images submitted by your application are transferred to the screen right away, which may result in tearing.

__**Documentation:**__
`VK_PRESENT_MODE_IMMEDIATE_KHR` specifies that the presentation engine does not wait for a vertical blanking period to update the current image, meaning this mode **may** result in visible tearing.
No internal queuing of presentation requests is needed, as the requests are applied immediately.

### VK_PRESENT_MODE_FIFO_KHR
__**Tutorial:**__
Guaranteed to be available.

The swap chain is a queue where the display takes an image from the front of the queue when the display is refreshed and the program inserts rendered images at the back of the queue. If the queue is full then the program has to wait.
This is most similar to vertical sync as found in modern games.
The moment that the display is refreshed is known as "vertical blank".

Recommended for mobile devices due to lower energy usage.

__**Documentation:**__
`VK_PRESENT_MODE_FIFO_KHR` specifies that the presentation engine waits for the next vertical blanking period to update the current image.
Tearing **cannot** be observed.
An internal queue is used to hold pending presentation requests.
New requests are appended to the end of the queue, and one request is removed from the beginning of the queue and processed during each vertical blanking period in which the queue is non-empty.
This is the only value of `presentMode` that is **required** to be supported.

### VK_PRESENT_MODE_FIFO_RELAXED_KHR
**Tutorial:**
This mode only differs from the previous one if the application is late and the queue was empty at the last vertical blank.
Instead of waiting for the next vertical blank, the image is transferred right away when it finally arrives.
This may result in visible tearing.

__**Documentation:**__
`VK_PRESENT_MODE_FIFO_RELAXED_KHR` specifies that the presentation engine generally waits for the next vertical blanking period to update the current image.
If a vertical blanking period has already passed since the last update of the current image then the presentation engine does not wait for another vertical blanking period for the update, meaning this mode **may** result in visible tearing in this case.
This mode is useful for reducing visual stutter with an application that will mostly present a new image before the next vertical blanking period, but may occasionally be late, and present a new image just after the next vertical blanking period.
An internal queue is used to hold pending presentation requests.
New requests are appended to the end of the queue, and one request is removed from the beginning of the queue and processed during or after each vertical blanking period in which the queue is non-empty.

### VK_PRESENT_MODE_MAILBOX_KHR
__** Tutorial:**__
This is another variation of the second mode.
Instead of blocking the application when the queue is full, the images that are already queued are simply replaced with the newer ones.
This mode can be used to render frames as fast as possible while still avoiding tearing, resulting in fewer latency issues than standard vertical sync.
This is commonly known as "triple buffering", although the existence of three buffers alone does not necessarily mean that the framerate is unlocked.

Recommended if energy usage is not a concern.

__**Documentation:**__
`VK_PRESENT_MODE_MAILBOX_KHR` specifies that the presentation engine waits for the next vertical blanking period to update the current image.
Tearing **cannot** be observed.
An internal single-entry queue is used to hold pending presentation requests.
If the queue is full when a new presentation request is received, the new request replaces the existing entry, and any images associated with the prior entry become available for re-use by the application.
One request is removed from the queue and processed during each vertical blanking period in which the queue is non-empty.

### VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR
Introduced in Vulkan `1.2`.

__**Documentation:**__
`VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR` specifies that the presentation engine and application have concurrent access to a single image, which is referred to as a *shared presentable image*.
The presentation engine periodically updates the current image on its regular refresh cycle.
The application is only required to make one initial presentation request, after which the presentation engine **must** update the current image without any need for further presentation requests.
The application **can** indicate the image contents have been updated by making a presentation request, but this does not guarantee the timing of when it will be updated.
This mode **may** result in visible tearing if rendering to the image is not timed correctly.

### VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR
Introduced in Vulkan `1.2`.

__**Documentation**__
`VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR` specifies that the presentation engine and application have concurrent access to a single image, which is referred to as a *shared presentable image*.
The presentation engine is only required to update the current image after a new presentation request is received.
Therefore the application **must** make a presentation request whenever an update is required.
However, the presentation engine **may** update the current image at any point, meaning this mode **may** result in visible tearing.
