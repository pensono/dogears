# DogEars

## Writing code for the DogEars cape

The driver is currently in development, so the easiest way to write an application is to add a file to the `examples` directory along the ones provided.

In addition to the examples, the tests also demonstrate how the cape can be used. If you would like to run the tests, run the command `make test`.

For information about setting up a development environment, see [the maintainer's notes](NOTES.md).

Currently, you'll need to run the following command after rebooting to see any data:

```sh
make pru-configPins
```

## Writing Python Code

Since python isn't a compiled language, there is no `make` command needed to build your program. However, it is necessary to compile, build, and install code from the python DogEars driver to get everything working. Luckily, this can be accomplished with a make command. Unless you already haev `numpy` and `setuptools` installed, this will require an internet connection.

```sh
make pythonsetup
```

To run an example, use something like the following:

```sh
sudo python3 examples/frequency_detect.py
```