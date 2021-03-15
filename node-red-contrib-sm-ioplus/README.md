# node-red-contrib-sm-ioplus

This is the node-red node to control Sequent Microsystems [Home Automation](https://sequentmicrosystems.com/shop/home-automation/raspberry-pi-home-automation-card/) card.

## Install

Clone or update the repository, follow the instrutions fron the [first page.](https://github.com/SequentMicrosystems/ioplus-rpi)

In your node-red user directory, tipicaly ~/.node-red

```bash
~$ cd ~/.node-red
```

Run the following command:

```bash
~/.node-red$ npm install ~/ioplus-rpi/node-red-contrib-sm-ioplus
```

In order to see the node in the palette and use-it you need to restart node-red. If you run node-red as a service:
 ```bash
 ~$ node-red-stop
 ~$ node-red-start
 ```

## Usage

After install and restart the node-red you will see on the node palete, under Sequent Microsystems category:

### IOPLUS RELAY

This node control relays of the card, one relay if you set "relay" parameter in range [1..8] and all if you set "relay" to 0.
The card stack level and channel number can be set in the node dialog box or dynamically through ```msg.stack``` and ```msg.channel```.
The payload of the input message contain the state of one relay or a 8 bits number with the state of all 8 relays.

### IOPLUS 0 10V out

This node controls one 0-10V output channel.
The card stack level and channel number can be set in the node dialog box or dynamically through ```msg.stack``` and ```msg.channel```.
The value in volts is set dynamically as a number between 0..10 thru ```msg.payload```.

### IOPLUS ADC in

This node reads one analog input channel. 
The card stack level and channel number can be set in the node dialog box or dynamically through ```msg.stack``` and ```msg.channel```.
The read is triggered by the message input and output can be found in the output message payload as a number representing the voltage.

### IOPLUS OPT cnt

This node reads the optically coupled input cunter for one particular channel and sets the counting edges for that channel.
Card stack level and optically coupled input counter channel number can be set in the node dialog box or dynamically through ```msg.stack``` and ```msg.channel```.
Rising edge counting and/or falling edge counting can be enabled/diabled with corresponding check box in the node dialog.
Edge settings are sent to the card every time you deploy this node or the flow starts or you select a different channel for reading through ```msg.channel```.
The read is triggered by the message input and output can be found in the output message payload.

### IOPLUS OPT in

This node reads the optically coupled input state for one particular channel.
Card stack level and optically coupled input channel number can be set in the node dialog box or dynamically through ```msg.stack``` and ```msg.channel```.
The read is triggered by the message input and output can be found in the output message payload as boolean.

### IOPLUS OD out

This node controls one Open-Drain output channel.
Card stack level and channel number can be set in the node dialog box or dynamically through ```msg.stack``` and ```msg.channel```.
The value in percentage is set dynamically as a number between 0..100 thru ```msg.payload```.

## Important note

This node is using the I2C-bus package from @fivdi, you can visit his work on github [here](https://github.com/fivdi/i2c-bus). 
The inspiration for this node came from @nielsnl68 work with [node-red-contrib-i2c](https://github.com/nielsnl68/node-red-contrib-i2c).
Thank both for the great job.
