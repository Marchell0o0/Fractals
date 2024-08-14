# Usage

In the root folder run:

`./create_pipes.sh`

`make`

In two different terminals:

`./front_end/prgsem`

`./back_end/prgsem-comp_module`

# Two possible ways to compute an image

## Locally

Computes using (chosen in compute_control.c file) number of threads with rows of the image being split between them.
To achieve better quality of the image in the animation local computation uses something like super-sampling.
For one pixel it's computing four different spots of the fractal and the pixel color is the median value between them.
Also to account for the depth change the number of iterations to check growth proportionally to the amount of black pixels in the last image (All of the parameters for that are in compute_control.c). And also to make the computation easier and colors of the image not to change while the depth is growing, the minimal number of iteration also changes. It starts from 0 and if the minimum value it reached in the last frame was 5 for example, it changes to 5\*0.7 to not lose any accuracy and still check if lower values will come.

To color the image color interpolation with CDF is used, so you can choose which colors and in which part of the values (from 0 to 1) will be displayed(bottom of compute_control.c update_image()). Even if the minimal_k changes, the colors will still go from 0 to 1.
Here is the [reference image](https://i.stack.imgur.com/XUbZR.png) for current colors.

## With a module

Same coloring here, and all of the "must do" functionality.

# SDL Part

Window is resized according to the monitor resolution(take 70% of its width and height is to keep the proportion 16:9). Stdout and stderr are redirected in terminal_redirect_thread into a file called "output.txt". Then anything that gets into it is drawn in the main window in the bottom white part. The fractal image is centered to be in the black area and the black area is made to be kept with ratio 16:9. But even if the fractal image is bigger it will be still displayed with black area sizes.
All of the buttons and fields are reacting to being hovered on, pressed and focused for text fields. The window also reacts to key presses(of course not if one of the fields is focused).
Creating gui elements using sdl was an absolute nightmare because you literally have to create every button rectangle by rectangle, but now everything works smoothly and for me worked with 1920x1080 and 3840x2160 resolutions.
