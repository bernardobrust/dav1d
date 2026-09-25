# What is this fork?
This is a fork from `libdav1d` that was created with the objective of exporting motion vectors and other compression-related data in such a way that it is accessible to FFmpeg and can be pulled by programs using it.

More specifically, to my project [VideoReconstruction](https://github.com/bernardobrust/VideoReconstruction), which requires such data to be accessible.

Note that these modifications do not alter the behaviour of the decoder: we are just intercepting data present in the video but not made avaliable.

## Note on AI code
AI agents were used to understand the codebase and debug/test faster.