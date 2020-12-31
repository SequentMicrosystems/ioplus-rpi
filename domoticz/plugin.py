# IOplus
#
# Author: Josef Pavlik, 2020
#
#
"""
<plugin key="IOplus" name="Raspberry IOplus" author="Josef Pavlik" version="0.1" externallink="">
    <description>
    IOplus 8 relay board on raspberry
    </description>
    <params>
        <param field="Mode1" label="board number" width="75px" default="0"/>
        <param field="Mode6" label="Debug" width="75px">
          <options>
              <option label="True" value="Debug"/>
              <option label="False" value="Normal"  default="true" />
          </options>
      </param>

    </params>
</plugin>
"""

# units 1-8 are  relays
# units 9-16 are inputs

import Domoticz
import libioplus as ioplus

class BasePlugin:
    board = 0
    running = 0
    debug = 0
   
    def set_relay(self,Unit, val):
        if self.debug: Domoticz.Log("set_relay "+str(Unit)+" to "+str(val))
        ioplus.setRelayCh(self.board, Unit, int(val))
    
    def __init__(self):
        return

    def connection(self):
        return 
      
    def onStart(self):        
        self.board=int(Parameters["Mode1"])
        self.debug=Parameters["Mode6"]=="Debug"
        Domoticz.Log("onStart - Plugin is starting, debug="+str(self.debug))
        for x in range(1, 9):
            if (x not in Devices): Domoticz.Device(Name="Relay_"+str(x), Unit=x, TypeName="Switch", Used=1).Create()
            if (x+8 not in Devices): Domoticz.Device(Name="Input_"+str(x), Unit=x+8, TypeName="Switch", Used=1).Create()
            self.set_relay(x, Devices[x].nValue)
        Domoticz.Heartbeat(3)
        self.running=1


    def onStop(self):
        Domoticz.Log("onStop - Plugin is stopping.")

    def onConnect(self, Connection, Status, Description):
        return

    def onMessage(self, Connection, Data):
        return
      
    def onCommand(self, Unit, Command, Level, Hue):
        val=Command=="On" and "1" or "0"
        if (Unit in range(1,9)): 
          self.set_relay(Unit, Command=="On")
          Devices[Unit].Update(int(val),val)

    def onDisconnect(self, Connection):
        unused=0

    def onHeartbeat(self):
        if self.running:
          val=ioplus.getRelays(self.board)
          inp=ioplus.getOpto(self.board)
          if (self.debug): Domoticz.Log("read relays -> %02x, inputs -> %02x" % (val,inp))
          for Unit in range(1,9):
            val1=(val >> (Unit-1)) & 1
            inp1=(inp >> (Unit-1)) & 1
            if (Unit in Devices): Devices[Unit].Update(int(val1),str(val1))
            if (Unit+8 in Devices): Devices[Unit+8].Update(int(inp1),str(val1))


global _plugin
_plugin = BasePlugin()

def onStart():
    global _plugin
    _plugin.onStart()

def onStop():
    global _plugin
    _plugin.onStop()

def onConnect(Connection, Status, Description):
    global _plugin
    _plugin.onConnect(Connection, Status, Description)

def onMessage(Connection, Data):
    global _plugin
    _plugin.onMessage(Connection, Data)

def onCommand(Unit, Command, Level, Hue):
    global _plugin
    _plugin.onCommand(Unit, Command, Level, Hue)

def onNotification(Name, Subject, Text, Status, Priority, Sound, ImageFile):
    global _plugin
    _plugin.onNotification(Name, Subject, Text, Status, Priority, Sound, ImageFile)

def onDisconnect(Connection):
    global _plugin
    _plugin.onDisconnect(Connection)

def onHeartbeat():
    global _plugin
    _plugin.onHeartbeat()

