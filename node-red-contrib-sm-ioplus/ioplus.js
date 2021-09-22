module.exports = function(RED) {
    "use strict";
    var I2C = require("i2c-bus");
    const DEFAULT_HW_ADD = 0x28;

    const I2C_MEM_RELAY_VAL_ADD = 0;   
    const I2C_RELAY_SET_ADD = 1;
    const I2C_RELAY_CLR_ADD = 2;
    const I2C_MEM_OPTO_IN_VAL = 3;
    const I2C_MEM_U0_10_OUT_VAL1 = 40;
    const I2C_MEM_OD_PWM1 = 48;
    const I2C_MEM_ADC_MV_VAL1 = 24;
    const I2C_MEM_OPTO_RISING_ENABLE = 56;
    const I2C_MEM_OPTO_FALLING_ENABLE = 57;
    const I2C_MEM_OPTO_CH_CONT_RESET = 60;
    const I2C_MEM_OPTO_COUNT1 = 128; //4 bytes integers


    function RelayNode(n) {
        RED.nodes.createNode(this, n);
        this.stack = parseInt(n.stack);
        this.relay = parseInt(n.relay);
        this.payload = n.payload;
        this.payloadType = n.payloadType;
        var node = this;

        node.port = I2C.openSync( 1 );
        node.on("input", function(msg) {
            var myPayload;
            var stack = node.stack;
            if (isNaN(stack)) stack = msg.stack;
            stack = parseInt(stack);
            var relay = node.relay;
            if (isNaN(relay)) relay = msg.relay;
            relay = parseInt(relay);
            //var buffcount = parseInt(node.count);
            if (isNaN(stack + 1)) {
                this.status({fill:"red",shape:"ring",text:"Stack level ("+stack+") value is missing or incorrect"});
                return;
            } else if (isNaN(relay) ) {
                this.status({fill:"red",shape:"ring",text:"Relay number  ("+relay+") value is missing or incorrect"});
                return;
            } else {
                this.status({});
            }
            var hwAdd = DEFAULT_HW_ADD;
            if(stack < 0){
                stack = 0;
            }
            if(stack > 7){
              stack = 7;
            }
            hwAdd += stack;
            
            try {
                if (this.payloadType == null) {
                    myPayload = this.payload;
                } else if (this.payloadType == 'none') {
                    myPayload = null;
                } else {
                    myPayload = RED.util.evaluateNodeProperty(this.payload, this.payloadType, this,msg);
                }
               
               
                if(relay < 0){
                  relay = 0;
                }
                if(relay > 8){
                  relay = 8;
                }
                if(relay > 0){
                    if (myPayload == null || myPayload == false || myPayload == 0 || myPayload == 'off') {
                      node.port.writeByte(hwAdd, I2C_RELAY_CLR_ADD, relay,  function(err) {
                        if (err) { node.error(err, msg);
                        } else {
                          node.send(msg);
                        }
                      });
                    } else {
                      node.port.writeByte(hwAdd, I2C_RELAY_SET_ADD, relay,  function(err) {
                        if (err) { node.error(err, msg);
                        } else {
                          node.send(msg);
                        }
                    });
                    }
                } else {
                    var relVal = parseInt(myPayload);
                    if(isNaN(relVal) || relVal > 255 || relVal < 0)
                    {
                        this.status({fill:"red",shape:"ring",text:"Set value for all relays is missing or incorrect"});
                        return;
                    } else {
                        msg.payload = relVal;
                        node.port.writeByte(hwAdd, I2C_MEM_RELAY_VAL_ADD, relVal,  function(err) {
                        if (err) { node.error(err, msg);
                        } else {
                          node.send(msg);
                        }
                    });
                    }

                }
            } catch(err) {
                this.error(err,msg);
            }
        });

        node.on("close", function() {
            node.port.closeSync();
        });
    }
    RED.nodes.registerType("IOPLUS RELAY", RelayNode);

    function VInNode(n) {
        RED.nodes.createNode(this, n);
        this.stack = parseInt(n.stack);
        this.channel = parseInt(n.channel);
        this.payload = n.payload;
        this.payloadType = n.payloadType;
        var node = this;
        var buffer = Buffer.alloc(2);
        
        node.port = I2C.openSync( 1 );
        node.on("input", function(msg) {
            var myPayload;
            var stack = node.stack; 
            if (isNaN(stack)) stack = msg.stack;
            var channel = node.channel;
            if (isNaN(channel)) channel = msg.channel;
            stack = parseInt(stack);
            channel = parseInt(channel);
            //var buffcount = parseInt(node.count);
            if (isNaN(stack)) {
                this.status({fill:"red",shape:"ring",text:"Stack level ("+stack+") value is missing or incorrect"});
                return;
            } else if (isNaN(channel) ) {
                this.status({fill:"red",shape:"ring",text:"Sensor number  ("+channel+") value is missing or incorrect"});
                return;
            } else {
                this.status({});
            }
            try {
                var hwAdd = DEFAULT_HW_ADD;
                if(stack < 0){
                    stack = 0;
                }
                if(stack > 7){
                  stack = 7;
                }
                hwAdd += stack;
                
                if(channel < 1){
                  channel = 1;
                }
                if(channel > 8){
                  channel = 8;
                }
                
                if (this.payloadType == null) {
                    myPayload = this.payload;
                } else if (this.payloadType == 'none') {
                    myPayload = null;
                } else {
                    myPayload = RED.util.evaluateNodeProperty(this.payload, this.payloadType, this,msg);
                }
                node.port.readI2cBlock(hwAdd, I2C_MEM_ADC_MV_VAL1 + (channel - 1)*2, 2, buffer,  function(err, size, res) {
                    if (err) { 
                        node.error(err, msg);
                    } 
                    else{
                        msg.payload = res.readIntLE(0, 2) / 1000.0;                       
                        node.send(msg);
                    }
                    });     
                    
            } catch(err) {
                this.error(err,msg);
            }
            
        });

        node.on("close", function() {
            node.port.closeSync();
        });
    }
    RED.nodes.registerType("IOPLUS ADC in", VInNode);

    function VOutNode(n) {
        RED.nodes.createNode(this, n);
        this.stack = parseInt(n.stack);
        this.channel = parseInt(n.channel);
        this.payload = n.payload;
        this.payloadType = n.payloadType;
        var node = this;
        var buffer = Buffer.alloc(2);
        
        node.port = I2C.openSync( 1 );
        node.on("input", function(msg) {
            var myPayload;
            var stack = node.stack; 
            if (isNaN(stack)) stack = msg.stack;
            var channel = node.channel;
            if (isNaN(channel)) channel = msg.channel;
            stack = parseInt(stack);
            channel = parseInt(channel);
            //var buffcount = parseInt(node.count);
            if (this.payloadType == null) {
                myPayload = this.payload;
            } else if (this.payloadType == 'none') {
                myPayload = null;
            } else {
                myPayload = RED.util.evaluateNodeProperty(this.payload, this.payloadType, this,msg);
            }
            if (isNaN(stack)) {
                this.status({fill:"red",shape:"ring",text:"Stack level ("+stack+") value is missing or incorrect"});
                return;
            } else if (isNaN(channel) ) {
                this.status({fill:"red",shape:"ring",text:"Sensor number  ("+channel+") value is missing or incorrect"});
                return;
            } else if(isNaN(myPayload)){
              this.status({fill:"red",shape:"ring",text:"Payload type must be a number  ("+this.payload+") value is missing or incorrect myPayload: ("+myPayload+")"});
                return;
            }
            else{
                this.status({});
            }
            try {
                var hwAdd = DEFAULT_HW_ADD;
                if(stack < 0){
                    stack = 0;
                }
                if(stack > 7){
                  stack = 7;
                }
                hwAdd += stack;
                
                if(channel < 1){
                  channel = 1;
                }
                if(channel > 4){
                  channel = 4;
                }

                if(myPayload < 0){
                  myPayload = 0;
                }
                if(myPayload > 10){
                  myPayload = 10;
                }
                var intVal = Math.round(myPayload * 1000);
                
                node.port.writeWord(hwAdd, I2C_MEM_U0_10_OUT_VAL1 + (channel - 1)*2, intVal,  function(err, size, res) {
                    if (err) { 
                        node.error(err, msg);
                    } 
                    else{                    
                        node.send(msg);
                    }
                    });     
                    
            } catch(err) {
                this.error(err,msg);
            }
            
        });

        node.on("close", function() {
            node.port.closeSync();
        });
    }
    RED.nodes.registerType("IOPLUS 0-10V out", VOutNode);   

    function OptoCounterNode(n) {
        RED.nodes.createNode(this, n);
        this.stack = parseInt(n.stack);
        this.channel = parseInt(n.channel);
        this.falling = n.falling;
        this.rising = n.rising;
        this.payload = n.payload;
        this.payloadType = n.payloadType;
        var node = this;
        var buffer = Buffer.alloc(4);
        var lastCfgCh = 0;
        var cfgByte = 0;
        
        node.port = I2C.openSync( 1 );
        node.on("input", function(msg) {
            var myPayload;
            var stack = node.stack; 
            if (isNaN(stack)) stack = msg.stack;
            var channel = node.channel;
            if (isNaN(channel)) channel = msg.channel;
            stack = parseInt(stack);
            channel = parseInt(channel);
            var rising = true;
            var falling = true;
            if(node.rising == false || node.rising == "false" || node.rising == 0)
            {
              rising = false;
            }
            if(node.falling == false || node.falling == "false" || node.falling == 0)
            {
              falling = false;
            }
            //var buffcount = parseInt(node.count);
            if (isNaN(stack)) {
                this.status({fill:"red",shape:"ring",text:"Stack level ("+stack+") value is missing or incorrect"});
                return;
            } else if (isNaN(channel) ) {
                this.status({fill:"red",shape:"ring",text:"Sensor number  ("+channel+") value is missing or incorrect"});
                return;
            } else {
                this.status({});
            }
            var hwAdd = DEFAULT_HW_ADD;
            if(stack < 0){
                stack = 0;
            }
            if(stack > 7){
              stack = 7;
            }
            hwAdd += stack;
            
            if(channel < 1){
              channel = 1;
            }
            if(channel > 8){
              channel = 8;
            }
            if(lastCfgCh != channel)
            {
              //node.log("Check configuration");
              node.port.readByte(hwAdd, I2C_MEM_OPTO_RISING_ENABLE,  function(err, rbyte) {
                if (err) { 
                    node.error(err, msg);
                } 
                else{
                  if(((rising == true) && ((rbyte & (1 << (channel - 1))) == 0)) || ((rising == false) && ((rbyte & (1 << (channel - 1))) != 0))){
                    cfgByte = rbyte;
                    if(rising){
                      cfgByte |= 1 << (channel - 1);
                      //node.log("Enable rising edge counting on channel " + channel );
                    }
                    else{
                      cfgByte &= 0xff ^ (1 << (channel -1));
                      //node.log("Disable rising edge counting on channel " + channel );
                    }
                    
                    node.port.writeByte(hwAdd, I2C_MEM_OPTO_RISING_ENABLE, cfgByte, function(err) {
                      if (err) {
                        node.error(err, msg);
                      }
                    });
                  }
                }
                });     
              
              node.port.readByte(hwAdd, I2C_MEM_OPTO_FALLING_ENABLE,  function(err, rbyte) {
                if (err) { 
                    node.error(err, msg);
                } 
                else{
                  if(((falling == true) && ((rbyte & (1 << (channel - 1))) == 0)) || ((falling == false) && ((rbyte & (1 << (channel - 1))) != 0))){
                    cfgByte = rbyte;
                    if(falling){
                      cfgByte |= 1 << (channel - 1);
                      //node.log("Enable falling edge counting on channel " + channel );
                    }
                    else{
                      cfgByte &= 0xff ^ (1 << (channel -1));
                      //node.log("Disable falling edge counting on channel " + channel );
                    }
                    
                    node.port.writeByte(hwAdd, I2C_MEM_OPTO_FALLING_ENABLE, cfgByte, function(err) {
                      if (err) {
                        node.error(err, msg);
                      }
                    });
                  }
                }
                });  
              lastCfgCh = channel;    
            }
            try {
                                
                if (this.payloadType == null) {
                    myPayload = this.payload;
                } else if (this.payloadType == 'none') {
                    myPayload = null;
                } else {
                    myPayload = RED.util.evaluateNodeProperty(this.payload, this.payloadType, this,msg);
                }
                node.port.readI2cBlock(hwAdd, I2C_MEM_OPTO_COUNT1 + (channel - 1)*4, 4, buffer,  function(err, size, res) {
                    if (err) { 
                        node.error(err, msg);
                    } 
                    else{
                        msg.payload = res.readIntLE(0, 4);                       
                        node.send(msg);
                    }
                    });     
                    
            } catch(err) {
                this.error(err,msg);
            }
            
        });

        node.on("close", function() {
            node.port.closeSync();
        });
    }
    RED.nodes.registerType("IOPLUS OPT cnt", OptoCounterNode);
    
    
    function OptoInNode(n) {
        RED.nodes.createNode(this, n);
        this.stack = parseInt(n.stack);
        this.channel = parseInt(n.channel);
        this.payload = n.payload;
        this.payloadType = n.payloadType;
        var node = this;
        var buffer = Buffer.alloc(4);
      
        
        node.port = I2C.openSync( 1 );
        node.on("input", function(msg) {
            var myPayload;
            var stack = node.stack; 
            if (isNaN(stack)) stack = msg.stack;
            var channel = node.channel;
            if (isNaN(channel)) channel = msg.channel;
            stack = parseInt(stack);
            channel = parseInt(channel);
           
            //var buffcount = parseInt(node.count);
            if (isNaN(stack)) {
                this.status({fill:"red",shape:"ring",text:"Stack level ("+stack+") value is missing or incorrect"});
                return;
            } else if (isNaN(channel) ) {
                this.status({fill:"red",shape:"ring",text:"Sensor number  ("+channel+") value is missing or incorrect"});
                return;
            } else {
                this.status({});
            }
            var hwAdd = DEFAULT_HW_ADD;
            if(stack < 0){
                stack = 0;
            }
            if(stack > 7){
              stack = 7;
            }
            hwAdd += stack;
            
            if(channel < 0){
              channel = 0;
            }
            if(channel > 8){
              channel = 8
            }
            try {
                                
                if (this.payloadType == null) {
                    myPayload = this.payload;
                } else if (this.payloadType == 'none') {
                    myPayload = null;
                } else {
                    myPayload = RED.util.evaluateNodeProperty(this.payload, this.payloadType, this,msg);
                }
                node.port.readByte(hwAdd, I2C_MEM_OPTO_IN_VAL ,  function(err, res) {
                    if (err) { 
                        node.error(err, msg);
                    } 
                    else{
                        //node.log("Opto val read " + res );
                        if(channel == 0){
                            msg.payload = res;
                        }
                        else{
                            if ((res & (1 << (channel - 1))) != 0) {
                              msg.payload = true;
                            }
                            else{
                              msg.payload = false;
                            }
                        }
                        node.send(msg);
                    }
                    });     
                    
            } catch(err) {
                this.error(err,msg);
            }
            
        });

        node.on("close", function() {
            node.port.closeSync();
        });
    }
    RED.nodes.registerType("IOPLUS OPT in", OptoInNode);
    
    function PWMOutNode(n) {
        RED.nodes.createNode(this, n);
        this.stack = parseInt(n.stack);
        this.channel = parseInt(n.channel);
        this.payload = n.payload;
        this.payloadType = n.payloadType;
        var node = this;
        var buffer = Buffer.alloc(2);
        
        node.port = I2C.openSync( 1 );
        node.on("input", function(msg) {
            var myPayload;
            var stack = node.stack; 
            if (isNaN(stack)) stack = msg.stack;
            var channel = node.channel;
            if (isNaN(channel)) channel = msg.channel;
            stack = parseInt(stack);
            channel = parseInt(channel);
            //var buffcount = parseInt(node.count);
            if (this.payloadType == null) {
                myPayload = this.payload;
            } else if (this.payloadType == 'none') {
                myPayload = null;
            } else {
                myPayload = RED.util.evaluateNodeProperty(this.payload, this.payloadType, this,msg);
            }
            if (isNaN(stack)) {
                this.status({fill:"red",shape:"ring",text:"Stack level ("+stack+") value is missing or incorrect"});
                return;
            } else if (isNaN(channel) ) {
                this.status({fill:"red",shape:"ring",text:"Sensor number  ("+channel+") value is missing or incorrect"});
                return;
            } else if(isNaN(myPayload)){
              this.status({fill:"red",shape:"ring",text:"Payload type must be a number  ("+this.payload+") value is missing or incorrect myPayload: ("+myPayload+")"});
                return;
            }
            else{
                this.status({});
            }
            try {
                var hwAdd = DEFAULT_HW_ADD;
                if(stack < 0){
                    stack = 0;
                }
                if(stack > 7){
                  stack = 7;
                }
                hwAdd += stack;
                
                if(channel < 1){
                  channel = 1;
                }
                if(channel > 4){
                  channel = 4;
                }

                if(myPayload < 0){
                  myPayload = 0;
                }
                if(myPayload > 100){
                  myPayload = 100;
                }
                var intVal = Math.round(myPayload * 100);
                
                node.port.writeWord(hwAdd, I2C_MEM_OD_PWM1 + (channel - 1)*2, intVal,  function(err, size, res) {
                    if (err) { 
                        node.error(err, msg);
                    } 
                    else{                   
                        node.send(msg);
                    }
                    });     
                    
            } catch(err) {
                this.error(err,msg);
            }
            
        });

        node.on("close", function() {
            node.port.closeSync();
        });
    }
    RED.nodes.registerType("IOPLUS OD out", PWMOutNode);  
}
