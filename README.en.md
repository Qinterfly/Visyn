
[![ru](https://img.shields.io/badge/lang-ru-red.svg)](https://github.com/Qinterfly/Visyn/blob/master/README.md)
[![en](https://img.shields.io/badge/lang-en-green.svg)](https://github.com/Qinterfly/Visyn/blob/master/README.en.md)

## About

The `Visyn` application is designed to compute harmonic responses from time-domain signals collected by accelerometers mounted on a structure. To obtain time responses, the structure is excited in `SineStep` mode. In this case, excitation frequency and interval data may be missing or incomplete, which makes them unusable.

![GUI](help/images/full.png)

## Algorithm

The following algorithm is used to compute spectra:
* A primary estimate of the process frequency change intervals is performed by finding the zero crossings of the synchronization channel, which varies synchronously with the excitation frequency and has a constant amplitude.
* To eliminate oscillations in the raw data, outliers are filtered and interpolated using median absolute deviations (`MAD`).
* Then piecewise-constant segments are extracted in frequency using a total variance denoising optimization.

![Segments](help/images/segments.png)

* A border detection algorithm (`Canny Edge Detector`) is used to find the segment boundaries.
* On each interval, the signal is convolved with a harmonic signal from the synchronization channel, and the real and imaginary spectrum components are estimated. The results are aggregated across all segments.

The computed spectra are loaded into a project with records through the `LMS Testlab Automation` interface. Thus, when the system lacks measurement channels, a measurement system from another manufacturer can be used and the data synchronized later using the `Visyn` program.

![Spectrums](help/images/spectrums.png)
