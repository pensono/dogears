# Notes
This file is intended for maintainers of this project. It contains high level 
information about the project layout as well as some important low level details.
A maintainer should be able to get up to speed by reading this document.

## Project Structure
The source is divided into three subdirectories:
- `src/pru`: Files required for the program running on the PRU which interfaces with the adc over GPIO pins.
- `src/cpp`: Source files for the C++ interface. This code communicates with the code on the PRU
- `src/python`: Source files for the python interface
- `examples`: Example code is placed here. Adding a new example is the preferred way to test o
- `include`: `.h` files which must be included by users of the cape
- `lib`: Dynamically linked binaries which must be included by users of the cape
- `test`: Files used for testing

The pru and c++ sources build with the included makefiles.

## Testing
There are some unit tests in the `test` directory. These can be invoked with `make test`.

Tests which require physical setup are also in this directory. These tests are designed as experiments
for capturing data to evaluate the performance of the cape. 

## C++ Implementation notes
The library allows for samples to be retrieved in a variety of formats. The problem of addressing how to 
deal with multiple formats in library code has many solutions, but I believe that the solution chosen
here balances client and implementer usability well. In `format.h` there are several structs which have
static properties used to manipulate the formats. If a method wishes to be format agnostic, it adds
a `format` template parameter, and can then use the format classes generically. With this method, using 
templates is not required, the formats may also be hard-coded.

## PRU Implementation notes
When running `make pru-run` for the first time, the script may complain about not being able to stop the
PRU. This is expected since the PRU has not started yet. This may also occur when the assembly program
has errors.

## Moving data between the PRU and ARM Core
The PRU stores the samples in memory designated for the PRU0 and PRU1 cores. This memory is then mmap'd so that
the arm core may read the data. This memory is split into two buffers for double buffering. Since the
memory is 16KiB in size, each buffer is 8KiB in length. There 4 channels, and each sample is stored in 
32 bits, which allows for 512 samples per channel in each of the two buffers.

PRU0 memory + PRU1 memory: 16KiB
Buffer Size: 16KiB / 2 buffers = 8KiB
Memory per channel: 8Kib / 4 channels = 2KiB
Samples per channel in each buffer: 2Kib / 4 bytes = 512 samples

Ideally, the PRU signals to the arm core when it has data available. This could potentially be accomplished
through the RPMsg interface (although it seems quite heavyweight). Until such a solution is implemented, 
the arm core will poll a section in memory containing a monotonically increasing counter, called the 
buffer number. Any time this number is incremented, a new buffer is available. The parity of this 
counter is used to determine which buffer is available.

Data in each buffer is laid out first by channel, and then by sample. The entire PRU0 memory will look as follows:
```
0x0000    0x0800     0x1000     0x1800     0x2000                                      0x4000
  #-------------------------------------------#-------------------------------------------#
  | CH0 BUF0 | CH1 BUF0 | CH2 BUF0 | CH3 BUF0 | CH0 BUF1 | CH1 BUF1 | CH2 BUF1 | CH3 BUF1 |
  #-------------------------------------------#-------------------------------------------#
```
Each sample is stored as a 32 bit signed integer. This could later be changed to 24 for better system performance.
Data is placed in the buffer this way to reduce the amount of work needed to be done by the CPU. There's extra
time between each transmission from the ADC that the PRU can do a little more work putting things in the right place.

## Beaglebone Setup

Enabling drivers: http://catch22.eu/beaglebone/beaglebone-pru-uio/

## Beaglebone setup notes

Disable HDMI overlay

am335x_pru_package must be installed. Make sure to install the libraries in `/home/debian/am335x_pru_package/pru_sw/app_loader/lib/`

If copying to /usr/lib, then also run `ldconfig -n -v /usr/lib`

Enable UIO overlay in `/boot/uEnv.txt`

## Logistics
### Editors.
I found the [Cloud9](http://beaglebone.local:3000/ide.html) editor to be somewhat clunky. There's no auto-save,
and there's a bug where files in the editor may not be in sync with the files on disk. In addition, being web
based, the performance of the editor itself is not great.

The Remote - SSH extension in VS Code is by far the best development setup.

#### Editing through VS Code

The "Remote - SSH" extension to VS code lets you write code for the beaglebone as if it were your local machine. At the time of writing, it's only available in the [VS Code Insider's](https://code.visualstudio.com/insiders/). Once installed, get the "Remote - SSH" extension, and add `beaglebone.local` as a remote. The password for the default user is `temppwd`.

#### Editing through a networked FS

I was able to set up remote editing using JetBrains' CLion over an NFS share. On the beaglebone:
```
sudo apt install nfs-kernel-server
# For access over USB
echo "/home/debian/adc-cape 192.168.7.0/255.255.255.252(rw,no_root_squash,subtree_check)" | sudo tee -a /etc/exports
```

The process of mounting this varies by host. Here's how to do it on linux:
```
mount beaglebone.local:/home/debian /mnt/bbb
```

If you're on Windows, unfortunately microsoft has removed NFS functionality from some versions of Windows. You can still
set up an FTP connection. On the beaglebone:
[Setting up FTP](https://www.digitalocean.com/community/tutorials/how-to-set-up-vsftpd-for-a-user-s-directory-on-debian-9)
```
sudo apt-get install vsftpd
```
And uncomment the `write_enable=YES` in `/etc/vsftpd.conf`. Then,
```
sudo systemctl restart vsftpd
```

Then in CLion, add the BeagleBone under `Build, Exeution, Deployment > Deployment`. If you want to be able to open
an SSH session in CLion, use the SFTP option. Then, enable uploading changed files automatically under
`Build, Exeution, Deployment > Deployment > Options`. It's also possible to start an SSH session from within 
CLion, by going to `Tools > Start SSH Session...`. You might want to set the encoding to UTF-8 in the settings,
 `Tools > SSH Terminal > Default Encoding`. 

[IntelliJ Reference](https://www.jetbrains.com/help/idea/uploading-and-downloading-files.html#automaticUploadOnUpdate)

## Useful resources

[Sharing PRU memory](http://catch22.eu/beaglebone/beaglebone-pru-ipc/)
[PRU Device Tree Overlays/U-Boot](https://www.ofitselfso.com/BeagleNotes/Beaglebone_Black_And_Device_Tree_Overlays.php)
[Template for Python library](https://github.com/beagleboard/am335x_pru_package/blob/master/pru_sw/app_loader/python/prussdrv/clib.py)

### PRU Toolchain
[Basic PRU Build Steps](http://catch22.eu/beaglebone/beaglebone-pru-ipc/)
[Hexpru](http://downloads.ti.com/docs/esd/SPRUHV6/invoking-the-hex-conversion-utility-with-a-command-file-stdz079298.html)
