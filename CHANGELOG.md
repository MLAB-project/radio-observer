ChangeLog
=========

### v0.1 (2013-07-03)

Features:

  - WAV and Jack frontends.
  - FITS backend makes continuous snapshots of configured length in full resolution.

### v0.1.1 HOTFIX (2013-08-11)

Fixes:
  
  - There is a bug in the time-keeping code (file `src/WaterfallBackend.cpp`,
    line `WFTime time = outBuffer_.times[0];` To circumvent this, the current
    time at the time of writing the file is used for now.

### v0.1.2 HOTFIX (2013-08-16)

Fixes:
  
  - Switch left and right halves of the spectrum in the waterfall output.

### v0.2 (2014-06-11)

Features:
  - I/Q signal gain and phase correction.
  - New ring buffer that would support both continuous snapshots and
    event-triggered data collection.
  - JSON configuration file supporting Spring-like dependency injection.
  - Raw I/Q data recording.

Issues:
  - Addition overflow causes the application to stop recording any new
    data after about 10 hours (work in progress).

Planned Features
----------------

### v0.3

### v0.4

Features:

  - Implement a scripting language API and interpreter that could be used to
    detect events and control data collection.

