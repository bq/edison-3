WHAT IS THIS?
=============

Linux Kernel source code for the devices:
* bq edison 3
* bq edison 3 3G


BUILD INSTRUCTIONS?
===================

Specific sources are separated by branches and each version is tagged with it's corresponding number. First, you should
clone the project:

        $ git clone git@github.com:bq/edison-3.git

After it, choose the version you would like to build:

*Edison 3*

        $ cd edison-3/
        $ git checkout edison-3

*Edison 3 3G*

        $ cd edison-3/
        $ git checkout edison-3-3g


Finally, build the kernel according the next table of product names:

| device                                                                                | product                                                               |
| --------------------------|-------------------------|
| bq edison 3                                      | bq_edison3                                      |
| bq edison 3 3G                 | bq_edison3_3g_e1010                     |

        $ ./makeMtk -t {product} n k
