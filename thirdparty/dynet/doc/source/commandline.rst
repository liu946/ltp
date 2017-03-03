DyNet Command Line Options
==========================

All programs using DyNet have a few command line options. These must be
specified at the very beginning of the command line, before other
options.

-  ``--dynet-mem NUMBER``: DyNet runs by default with 512MB of memory
   each for the forward and backward steps, as well as parameter
   storage. You will often want to increase this amount. By setting
   NUMBER here, DyNet will allocate more memory. Note that you can also
   individually set the amount of memory for forward calculation,
   backward calculation, and parameters by using comma separated
   variables ``--dynet-mem FOR,BACK,PARAM``. This is useful if, for
   example, you are performing testing and don't need to allocate any
   memory for backward calculation.
-  ``--dynet-l2 NUMBER``: Specifies the level of l2 regularization to
   use (default 1e-6).
-  ``--dynet-gpus NUMBER``: Specify how many GPUs you want to use, if
   DyNet is compiled with CUDA. Currently, only one GPU is supported.
-  ``--dynet-gpu-ids X,Y,Z``: Specify the GPUs that you want to use by
   device ID. Currently only one GPU is supported, but if you use this
   command you can select which one to use.
