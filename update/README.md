# update

This is the [Sequent Microsystems](https://www.sequentmicrosystems.com) IO-Plus board  firmware update tool.

## Usage

```bash
~$ git clone https://github.com/SequentMicrosystems/ioplus-rpi.git
~$ cd ioplus-rpi/update/
~/ioplus-rpi/update$ ./update 0
``` 

If you clone the repository already, skip the first step. The command will download the newest firmware version from our server and write it  to the board.
The stack level of the board must be provided as a parameter. During firmware update we strongly recommend to disconnect all outputs from the board since they can change state unpredictably.
