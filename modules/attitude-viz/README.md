# Attitude Visualization
Python script for visualization the BNO 055 attitude estimation. This modules requires a serial connection to the STM proccessor via ```/dev/ttyUSB0``` to be accessible and the BNO 055 9-DOF IMU to be connected to the STM processor with the ```../stm-ekf``` firmware running.


```$ python3 --version ```
``` 3.8.10 ```

## Setup
1. Setup and activate virtual environment
   
   ```$ python3 -m venv <your-env-name>```
   
   ```$ source <your-env-path>/bin/activate```
  
2. Install the necessary dependencies

   ```python3 -m pip install -r <path-to-this-module>/requirements.txt```

[NOTE] : you may get some errors about jupyter notebook client which is ok since we aren't using a notebook 
   
## Running

```$ python3 visualization.py```
