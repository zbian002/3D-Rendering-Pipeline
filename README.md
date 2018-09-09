# 3D rendering pipeline

In this assignment, we implemented a simplified 3D rendering pipeline (with flat shading). This will consist of several parts, introduced through a series of tests:

1.Vertex and viewing transformations
2.Rasterization and interpolation
3.Clipping
4.Using a z-buffer for hidden surfaces

## Getting Started

1.Read the comments in main.cpp, minigl.h, and minigl.cpp. Instruction on how to call the program are provided in main.cpp.

2.Look through the first test case. Get the mglBegin(), mglEnd(), and mglVertex2() calls working. Make mglReadPixels draw the vertices in the output so you can verify that they are approximately where they should be. They may be as much as two or three pixels off at this stage; this is as good as you will be able to do this way.

3.Once the pixels are in approximately the right places, modify mglReadPixels to rasterize each triangle.

4.Using the drawn triangles, fine-tune the placement of your pixels. Remember that the locatons (-1,-1) and (1,1) map to the centers of the corner pixels, not to the corners of the image. The -p and -g options will be very helpful here; these will show you visually the difference.

5.Go through the list of inputs one at a time, implementing features as they are needed and fixing bugs as you encounter them. The tests are roughly arranged from easy to hard and are intended to assist you in debugging your program.

6.Each function that you are expected to write in miniGL corresponds to an appropriate OpenGL command. I strongly recommend you look at this before implementing the appropriate functions. You only have to implement enough to pass the tests. You are not creating a full OpenGL implementation.

### Installing

Clone this repository to a directory of your choice.

## Running the tests

We are providing you with the grading script that we will be using to grade this project. You can run it like this: "./grading-script.sh .". This will automatically compile your code, run all of the tests in the table above, and compute your score based on the point values in the table. This means you can run it yourself to see your progress. This will allow you to quickly identify the next test to work on. You can also run it before you submit your project so that you know what your score will be before you submit it.

Note that scores can vary from run to run due to hardware and natural variability. For example, if run on a machine slower than delta-01, you may see a lower score by failing some of the efficiency tests that would pass on delta-01. Some runs will be slightly faster and some slightly slower; if you are very close to the time limit on a test, it may pass sometimes and fail others. To give you the benefit of the doubt with timing variability, timing tests are automatically run three times each (or until you pass). If any of the three runs is within the time limit, you will get credit for passing the test.

Be sure to test your project on one of the delta machines. All of the delta machines are identical in hardware and software; if it works on one, it should work on all. It is possible for your project to score well on your own machine but fail on delta-01. For example, the version of the compiler on your machine may accept your code while the version on delta may flag a compilation error and fail.

## Authors

* **Zhen Bian** - *Initial work* - [3D rendering pipeline](https://github.com/zbian002/3D-Rendering-Pipeline)
