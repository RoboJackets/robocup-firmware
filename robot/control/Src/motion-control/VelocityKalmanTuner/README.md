# Velocity Kalman Tuner
## Introduction
Each of the RoboCup robots uses a Velocity Kalman Filter (RobotEstimator.cpp) in order to estimate linear x-axis 
and y-axis velocities from noisy encoder data and angular z-axis velocity from noisy gyro data. 

### What is a Kalman Filter? (Finish later)
In brief, a Kalman Filter 

| Matrix | Name | Size | Description |
| ------ | --- | ---- | ----------- |
| **A** | **State Transition Matrix** | 3x3 | Maps current state to next state, assuming dynamics hold
| **B** | **Control Matrix** | 5x5 | Maps control input to change in state
| **P** | **Estimate Covariance Matrix** | 3x3 | Correlation between each state variable |
| **H** | **Observation Matrix** | 4x3 | Maps current states to measurements |
| **Q** | **Process Covariance Matrix** | 3x3 | Covariance of state variable uncertainty due to unmodeled influences |
| **R** | **Observation Covariance Matrix** | 5x5 | Covariance of measurement uncertainty due to sensor noise and other influences  | 
The Velocity Kalman Tuner tool allows the user to tune the Q and R gains of a robot's Velocity Kalman Filter

For more detailed and better understanding, here are a few useful sources:
- https://www.bzarg.com/p/how-a-kalman-filter-works-in-pictures/
- https://en.wikipedia.org/wiki/Kalman_filter
- https://www.youtube.com/playlist?list=PLX2gX-ftPVXU3oUFNATxGXY90AULiqnWT

## Using the Tool
###Installation
This tool was last tested with Python 3.7 and requires the following major packages:
- matplotlib
- numpy
- PyQt5
- scipy

A detailed list of package requirements and dependencies can be found in [requirement.txt](requirements.txt)



###Running The Program
To open the program via the command line, navigate to the ```VelocityKalmanTuner``` folder
and execute

```$ python vkt.py```

####Modes

The Velocity Kalman Tuner (VKT) can suit user preferences in one of three modes:
##### **GUI:** 
The GUI interface allows the user to easily change gains via textboxes, adjust settings,
view simulation graphs, and save/load gains with a few button clicks

##### **GUI with code parameters / Hybrid:** 
For users who prefer rapidly tweaking code and underlying formulas, one can tweak the code
or specify the timesteps or total time via the command line

##### **Headless:** 
By specifying the ```-hd``` or ```--headless``` argument in the command line, the
program can fully run without GUI

Here is a table of all possible CLI arguments for headless mode

| Short Syntax | Long Syntax | Description |
| ------------ | ----------- | ----------- |
| ```-h``` | ```--help``` | Display help menu showing argument descriptions and syntaxes
| ```-hd``` | ```--headless``` | Run code in headless mode |
| ```-l``` | ```--loadfile``` | Specify .txt file to load gains from |
| ```-o``` | ```--outfile``` | Specify .hpp file to export code to |
|```-f``` | ```--savefig``` | Specify filename to save simulation plot as. If none specified, file will be saved as a PNG with datetime string
|```-r``` | ```--regular``` | By default, the simulation runs a steady-state Kalman filter. Using this flag will allow the Kalman filter to iterate the `P` and `K` matrices.
| ```-s``` | ```--savefile``` | Specify .txt file to save gains to |
| ```-t``` | ```--time``` | Specify total time a simulation should run |
| ```-dt``` | ```--timestep``` | Specify timestep for the simulation
| ```-v``` | ```--verbose``` | Print gains values at every time step to console |