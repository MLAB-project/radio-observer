radio-observer
==============

This README is for version `0.4`.

Radioastronomy utility. For more information, see [the MLAB wiki](http://wiki.mlab.cz/doku.php?id=en:programming_tasks#open-source_meteor_detection_toolkit).

branch | status
------ | ------
master | [![Build Status](https://travis-ci.org/MLAB-project/radio-observer.svg?branch=master)](https://travis-ci.org/MLAB-project/radio-observer)
dev    | [![Build Status](https://travis-ci.org/MLAB-project/radio-observer.svg?branch=dev)](https://travis-ci.org/MLAB-project/radio-observer/tree/dev)


### Table of Contents
* [Compilation](#compilation)
* [Configuration](#configuration)
* [Usage](#usage)
* [ChangeLog](#changelog)


Compilation
-----------

1. Install the following libraries:
      - libfftw3 (http://www.fftw.org/download.html)
      - cfitsio (http://heasarc.gsfc.nasa.gov/fitsio/)
      - JACK (http://jackaudio.org/download)
   
   On a debian system (Ubuntu), they can by installed using:
   
        $ sudo apt-get install libfftw3-dev cfitsio-dev libjack-jackd2-dev clang

2. Clone the repository using (for instance):
   `git clone https://github.com/MLAB-project/radio-observer.git`.
   
3. Checkout and build the `cppapp` submodule:
        
        $ git submodule init
        $ git submodule update
        $ cd cppapp
        $ make

4. In the `radio-observer` directory, run `make`. The resulting binary, named
   `radio-observer`, should appear in the project's root directory.
5. If anything goes wrong, please send me an email with the output at
   milikjan@fit.cvut.cz .


Configuration
-------------

The program attempts to read a config file in the user's home directory called
`.radio-observer.json`. Example config file is stored in
`radio-observer/radio-observer.json`. You can copy it to your home directory
and edit as you like (`$ mv radio-observer.json $HOME/.radio-observer.json`).


Usage
-----

    $ radio-observer [-v] [-c CONFIG_FILE] [WAV_FILE]

- `-v` prints out program version and exits
- `-c CONFIG_FILE` makes the program use config file `CONFIG_FILE`
  rather than the default (`$HOME/.radio-observer.json`)
- `WAF_FILE` makes the program read input from WAV file `WAF_FILE`
  rather than JACK

Without the `WAV_FILE` argument, `radio-observer` attempts to connect to a jack
server and then listen forever to the data sent by Jack. If `WAV_FILE` is
specifed, `radio-observer` uses WAV frontend, reads the WAV file and exits.  In
either case, the program stores the resulting data in a series of FITS files
(snapshots) in its current working directory (the directory from which you run
the program).

Currently, the format of the snapshot file name is
`snapshot_LOCATION_YEAR_MM_DD_HH_mm_ss.fits`, where `LOCATION` is the value of
the `location` configuration option, `YEAR` is a four-digit year, `MM` is
two-digit month, `DD` two-digit day and so on.

Despite there being a `log_file` configuration option, the log is currently
written only to the stderr.  To append it to a file, do output redirection (`$
radio-observer 2> your_log_file.log`).

Fits file handling: FITS can be converted in png by fits2png script. 
`sudo apt-get install python-pyfits`

ChangeLog
---------

See [CHANGELOG.md](CHANGELOG.md).


