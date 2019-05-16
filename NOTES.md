# Notes
This file is intended for maintainers of this project. It contains high level 
infromation about the project layout as well as some important low level details.
A maintainer should be able to get up to speed by reading this document.

## Project Structure
The source is divided into three subdirectories:
- `src-pru`: Files required for the program running on the PRU which interfaces with the adc over GPIO pins.
- `src-cpp`: Source files for the C++ interface. This code communicates with the code on the PRU
- `src-python`: Source files for the python interface
- `examples`: Example code is placed here. Adding a new example is the preferred way to test o
- `include`: `.h` files which must be included by users of the cape
- `lib`: Dynamically linked binaries which must be included by users of the cape

The pru and c++ sources build with the included makefiles.

## C++ Implementation notes
The library allows for samples to be retrieved either as 32-bit signed integers or floating 
point numbers. Templates are used to keep the code generic.

## PRU Implementation notes
When running `make pru-run` for the first time, the script may complain about not being able to stop the
PRU. This is expected since the PRU has not started yet. This may also occur when the assembly program
has errors.

## Moving data between the PRU and ARM Core
The PRU stores the samples in memory designated for the PRU0 core. This memory is then mmap'd so that
the arm core may read the data. This memory is split into two buffers for double buffering. Since the
memory is 8KiB in size, each buffer is 4KiB in length. There 4 channels, and each sample is stored in 
32 bits, which allows for 256 samples per channel in each of the two buffers.

PRU0 memory: 8KiB
Buffer Size: 8KiB / 2 buffers = 4KiB
Memory per channel: 4Kib / 4 channels = 1KiB
Samples per channel in each buffer: 1Kib / 4 bytes = 256 samples

Ideally, the PRU signals to the arm core when it has data available. This could potentailly be accomplished
through the RPMsg interface (although it seems quite heavyweight). Until such a solution is implemented, 
the arm core will poll a section in memory containing a monotonically increasing counter, called the 
buffer number. Any time this number is incremented, a new buffer is available. The parity of this 
counter is used to determine which buffer is available.

## Logistics
### Editors.
I found the [Cloud9](http://beaglebone.local:3000/ide.html) editor to be somewhat clunky. There's no auto-save,
and there's a bug where files in the editor may not be in sync with the files on disk. In addition, being web
based, the performance of the editor itself is not great. 

I was able to set up remote editing using JetBrains' CLion. The only caveat is that you must manually save files
(by pressing Shift-Alt-Q by default). It required an FTP server to be installed on the beaglebone: 
[Setting up FTP](https://www.digitalocean.com/community/tutorials/how-to-set-up-vsftpd-for-a-user-s-directory-on-debian-9)
```
sudo apt-get install vsftpd
```
And uncomment the `write_enable=YES` in `/etc/vsftpd.conf`. Then,
```
sudo systemctl restart vsftpd
```

Then in CLion, add the BeagleBone under `Build, Exeution, Deployment > Deployment`. If you want to be able to open
an SSH session in CLion, use teh SFTP option. Then, enable uploading changed files automatically under
`Build, Exeution, Deployment > Deployment > Options`. It's also possible to start an SSH session from within 
CLion, by going to `Tools > Start SSH Session...`.

[IntelliJ Reference](https://www.jetbrains.com/help/idea/uploading-and-downloading-files.html#automaticUploadOnUpdate)