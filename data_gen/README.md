# Training Data Generation with Octave

## Get Octave

Install GNU Octave (version >= 3.7.2) from your distro's package manager, or from the [download page][1].

## Install the required octave forge packages

Install `io` and `statistics` packages from octave's package manager

```bash
$ octave
octave:1> pkg install -verbose -forge io
octave:2> pkg install -verbose -forge statistics
```

## Run the script

```bash
$ octave mvnorm.m out_file.csv 200 '[1 0;0 1]' '[0;0]'
$ octave mvnorm.m help
```

[1]: https://www.gnu.org/software/octave/download.html
