% Jordan Brindza
% brindza@seas.upenn.edu
% University of Pennsylvania
% March 2011

% The first time you call s626 it will start the encoder thread and you will see:
%   Initializing Sensoray626 Board 0...  
%   Board(0) - "/dev/s626a0" is opened.
%   Board(0) on PCI bus 12; slot 0
%   
%   done
%   Initializing quatrature encoder counters...done 
%   Starting Thread...
%   Starting s626 thread...
%
% the first time you run any command using the interface it will initialize the
% interface
s626('encoder');

% the interface calculates the poseition, velocity and acceleration of each
% encoder.

% encTicksPerRev - the number of ticks per revolution for the encoder
%   takes an array of values and an (optional) starting encoder index
% set all encoders ticks per revolution to 2000 
s626('set', 'encTicksPerRev', [2000, 2000, 2000, 2000, 2000, 2000]);
% set encoder 2's ticks per revolution to 4000
s626('set', 'encTicksPerRev', 4000, 2);
% set encoder 3's ticks per revolution to 1000 and encoder 4's ticks per
% revolution to 3000
s626('set', 'encTicksPerRev', [1000, 3000], 3);

disp('encTicksPerRev:');
disp(s626('get', 'encTicksPerRev'));


% velFilterWeight - the weight used by the IIR filter for the velocity
% set all velocity filter weights to 0.06
s626('set', 'velFilterWeight', [0.06, 0.06, 0.06, 0.06, 0.06, 0.06]);
% set the velocity filter weight for encoder 2 to 0.10
s626('set', 'velFilterWeight', 0.10, 2);
% set the velocity filter weight for encoder 3 to 0.05 and encoder 4 to 0.03
s626('set', 'velFilterWeight', [0.05, 0.03], 3);

disp('velFilterWeight:');
disp(s626('get', 'velFilterWeight'));

% accelFilterWeight - the weight used by the IIR filter for the velocity
% set all acceleration filter weights to 0.06
s626('set', 'accelFilterWeight', [0.06, 0.06, 0.06, 0.06, 0.06, 0.06]);
% set the acceleration filter weight for encoder 2 to 0.10
s626('set', 'accelFilterWeight', 0.10, 2);
% set the acceleration filter weight for encoder 3 to 0.05 and encoder 4 to 0.03
s626('set', 'accelFilterWeight', [0.05, 0.03], 3);

disp('accelFilterWeight:');
disp(s626('get', 'accelFilterWeight'));


% encoder - main accessor for the current encoder readings.
%   it returns a 6x4 array where each row contains: 
%     [ticks position velocity acceleration]
%     ticks - current encoder ticks
%     position - current position in radians (based on encTicksPerRev)
%     velocity - current velocity in rad/s
%     accleration - current acceleration in rad/s^2
disp('encoder:');
disp(s626('encoder'));

% zero - takes an array of encoder ids to zero (1-6)
% zeros encoder 3
s626('zero', 3);
% zero encoders 1, 2, 4 and 5
s626('zero', [1, 2, 4, 5]);
% zero all encoders 
s626('zero');

% dac - interface for the 4 DAC ouputs on the s626 board
%   it takes an array and an (option) starting index
% set all DAC to 1.0
s626('dac', [1.0, 1.0, 1.0, 1.0]);
% set DAC 2 to 3.0
s626('dac', 3.0, 2);
% set DAC 3 to 0.0 and DAC 4 to 2.0
s626('dac', [0.0, 2.0], 3);

% If you want to stop the encoder thread for some reason you can use: 
%   clear s626
% or just close MATLAB.

