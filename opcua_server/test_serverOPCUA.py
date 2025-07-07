
 
URL = "opc.tcp://127.0.0.1:4840"
 
 
import sys
import random
import time
 
from opcua import Server
 
 
if __name__ == "__main__":
  server = Server()
  server.set_endpoint(URL)
   
  objects   = server.get_objects_node()
  ns        = server.register_namespace("Мои понятия")
  voltmeter = objects.add_object(ns, "Вольтметр")    
  voltage   = voltmeter.add_variable(ns, "Напряжение", 0.0)

  server.start()
     
  V = 220.0
  while True:            
    V = random.uniform(190.0, 240.0)
    print("{:8.1f} В".format(V))
    voltage.set_value(V)
     
    time.sleep(1)
 
  server.stop()