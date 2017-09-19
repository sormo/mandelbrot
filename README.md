## Mandelbrot
### using Oxygine framework

Visualization of mandelbrot set:  
![mandelbrot](https://user-images.githubusercontent.com/7541054/30611304-46c32e92-9d81-11e7-9c75-d45fd3c29c7c.png)

Limitations:
* Depth is limited by number of maximum escape iterations. After reaching certain zoom level contours of set loose complexity.  
* Simple coloring with shades of grey. With increasing zoom colors tends to white.

Set is recomputed with each zoom/move of camera asynchronously. _Mandelbrot_ class divides screen into squares each with size 200x200 pixels. Each square is recomputed by _MandelbrotPart_ class in separate thread.  

Worker thread in _MandelbrotPart_ simply iterates over all pixels of square computing number of escape iterations. After that maps number of escape iterations to color according to pallete.  

Maximum number of escape iterations is constant 1024. After reaching this number pixel is considered as part of mandelbrot set.
