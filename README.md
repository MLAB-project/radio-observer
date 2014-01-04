waterfall
=========

This README is for version `0.2dev`.

Radioastronomy utility. For more information, see [the MLAB wiki](http://wiki.mlab.cz/doku.php?id=en:programming_tasks#open-source_meteor_detection_toolkit).

### Table of Contents
* [Compilation](#compilation)
* [Configuration](#configuration)
* [Usage](#usage)
* [ChangeLog](#changelog)


Compilation
-----------

1. Install the following libraries:
      - libfftw3 (http://www.fftw.org/download.html
      - cfitsio (http://heasarc.gsfc.nasa.gov/fitsio/
      - JACK (http://jackaudio.org/download)

`sudo apt-get install libfftw3-dev cfitsio-dev clang`

2. Clone the repository using (for instance):
   `git clone https://github.com/MLAB-project/radio-observer.git`.
   
3. Checkout and build the `cppapp` submodule:
        
        $ git submodule init
        $ git submodule update
        $ cd cppapp
        $ make

4. In the `radio-observer` directory, run `make`. The resulting binary, named
   `waterfall`, should appear in the project's root directory.
5. If anything goes wrong, please send me an email with the output at
   milikjan@fit.cvut.cz .


Configuration
-------------

The program attempts to read a config file in the user's home directory called
`.waterfall`. Example config file is stored in `waterfall/watefall.cfg`. You
can copy it to your home directory and edit as you like (`$ mv waterfall.cfg
$HOME/.waterfall`).


Usage
-----

    $ waterfall [WAV_FILE]

Without the `WAV_FILE` argument, `waterfall` attempts to connect to a jack
server and then listen forever to the data sent by Jack. If `WAV_FILE` is
specifed, `waterfall` uses WAV frontend, reads the WAV file and exits.  In
either case, the program stores the resulting data in a series of FITS files
(snapshots) in its current working directory (the directory from which you run
the program).

Currently, the format of the snapshot file name is
`snapshot_LOCATION_YEAR_MM_DD_HH_mm_ss.fits`, where `LOCATION` is the value of
the `location` configuration option, `YEAR` is a four-digit year, `MM` is
two-digit month, `DD` two-digit day and so on.

Despite there being a `log_file` configuration option, the log is currently
written only to the stderr.  To append it to a file, do output redirection (`$
waterfall 2> your_log_file.log`).


ChangeLog
---------

See [CHANGELOG.md](CHANGELOG.md).


